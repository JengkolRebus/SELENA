Dokumentasi Teknis Sistem Kontrol Teleskop SELENA

1.0 Pendahuluan

Tujuan utama sistem SELENA adalah untuk menyediakan kontrol pergerakan fisik teleskop. Sistem ini dirancang untuk menerima perintah dari perangkat lunak Stellarium Mobile melalui protokol yang kompatibel dengan LX200, mengonversi koordinat langit menjadi koordinat mekanis, dan melakukan pelacakan objek langit secara kontinu untuk mengkompensasi rotasi bumi.
Perangkat lunak SELENA yang menangani fungsi spesifik, seperti kalkulasi astronomi sederhana, kontrol motor stepper, interpretasi perintah, dan manajemen status.

2.0 Arsitektur Perangkat Lunak

Berikut adalah komponen-komponen perangkat lunak utama yang membentuk sistem SELENA:

* main (main.cpp): Berfungsi sebagai entry point dan event loop utama sistem, yang mengorkestrasi siklus hidup aplikasi. Modul ini bertanggung jawab untuk inisialisasi semua perangkat keras dan perangkat lunak saat startup, mengelola koneksi klien melalui WiFi, dan memanggil fungsi-fungsi dari modul lain untuk menjalankan alur kerja sistem secara keseluruhan.
* Config (Config.h, Config.cpp): Merupakan pusat konfigurasi global dan manajemen status. File ini menyimpan semua konstanta sistem (kredensial WiFi, parameter fisik motor, rasio roda gigi) dan variabel status global (posisi teleskop saat ini, posisi target, status alignment) yang perlu diakses oleh berbagai modul lain.
* Command (Command.cpp): Berfungsi sebagai interpreter perintah. Modul ini menerima string perintah dari klien (misalnya, Stellarium) yang mengikuti subset protokol LX200. Ia mem-parsing perintah tersebut dan memicu tindakan yang sesuai di dalam sistem, seperti mengatur koordinat target, memulai gerakan, atau menghentikan motor.
* Move (Move.cpp): Modul ini mengabstraksi semua interaksi dengan perangkat keras motor. Tanggung jawab utamanya adalah mengonversi target koordinat dalam satuan derajat (Altitude/Azimuth) menjadi gerakan motor stepper yang presisi. Ini dicapai dengan memanfaatkan pustaka AccelStepper untuk mengontrol kecepatan, akselerasi, dan posisi target motor.
* Astro (Astro.cpp): Dapat dianggap sebagai "otak" matematis dari sistem. Modul ini menangani semua kalkulasi astronomi yang kompleks. Fungsi utamanya adalah melakukan konversi antara sistem koordinat langit (Right Ascension/Declination) dan sistem koordinat lokal teleskop (Altitude/Azimuth), yang merupakan inti dari fungsionalitas "GoTo" dan pelacakan.
* Alignment (Alignment.cpp): Merupakan modul model koreksi. Modul ini bertugas menghitung dan menerapkan model matematis untuk mengkompensasi ketidakakuratan mekanis pada dudukan teleskop. Dengan mengumpulkan data dari beberapa bintang referensi, modul ini dapat menghasilkan nilai koreksi yang signifikan meningkatkan akurasi penunjukan.
* Time (Time.cpp): Bertanggung jawab atas manajemen waktu. Modul ini melakukan sinkronisasi waktu sistem dengan server NTP (Network Time Protocol) saat startup untuk memastikan keakuratan waktu, yang sangat krusial untuk kalkulasi astronomi. Setelah itu, ia menyediakan fungsi untuk mendapatkan waktu saat ini yang telah disinkronkan.

2.1 Ketergantungan Antar Komponen

Struktur modular ini dihubungkan oleh ketergantungan yang terdefinisi dengan baik, yang dapat dianalisis melalui direktif #include di setiap file header. Pemahaman tentang hubungan ini penting untuk melacak alur data dan kontrol di seluruh sistem.

Modul	Ketergantungan & Penjelasan
Command	Command.h menyertakan Move.h, Astro.h, Time.h, dan Alignment.h. Ini karena fungsi handleCmd perlu memanggil fungsi dari semua modul tersebut untuk mengeksekusi perintah. Contohnya, perintah :MS memanggil SLEW_TO_TARGET (Move), perintah :Sd memanggil RADEC_TO_ALTAZ (Astro), dan perintah :CM memanggil addDataPoint (Alignment).
Move	Move.h menyertakan Alignment.h. Ketergantungan ini krusial karena fungsi SLEW_TO_TARGET harus memanggil APPLY_MODEL dari modul Alignment sebelum mengeksekusi gerakan motor apa pun, untuk menerapkan koreksi yang diperlukan.
main	main.cpp menyertakan Config.h, Time.h, Command.h, Astro.h, dan Move.h. Sebagai orkestrator utama, main perlu mengakses fungsi inisialisasi, loop utama, dan pemrosesan dari hampir semua modul lain untuk menjalankan sistem secara kohesif.
Alignment	Alignment.h menyertakan Config.h untuk mengakses variabel status global seperti targetAZ, targetALT, alignmentStatus, pointCount, dan model.
Astro	Astro.h menyertakan Config.h untuk mengakses konstanta matematis seperti DEG_TO_RAD dan RAD_TO_DEG.
Time	Time.h menyertakan Config.h untuk mengakses UTC_OFFSET. Modul ini juga bergantung pada pustaka eksternal NTPClient.h dan WiFiUdp.h untuk fungsionalitas sinkronisasi waktu jaringan.

Pemahaman yang jelas tentang arsitektur dan ketergantungan ini menjadi fondasi untuk mengikuti alur data dan logika kontrol yang lebih kompleks yang akan dibahas pada bagian selanjutnya.

3.0 Alur Data dan Logika Kontrol

Memahami alur data dari penerimaan perintah hingga eksekusi gerakan fisik adalah kunci untuk men-debug dan mengembangkan sistem SELENA. Bagian ini akan memecah proses-proses kunci dalam sistem menjadi langkah-langkah yang dapat diikuti, menyoroti bagaimana berbagai komponen berinteraksi untuk mencapai tujuan fungsional.

3.1 Inisialisasi Sistem

Proses inisialisasi, yang seluruhnya terjadi di dalam fungsi setup() dari main.cpp, mengatur sistem dari keadaan mati menjadi siap menerima perintah. Urutan prosesnya adalah sebagai berikut:

1. Inisialisasi Perangkat Keras: Pin I/O untuk driver motor stepper dikonfigurasi (pinMode, digitalWrite). Pustaka AccelStepper untuk sumbu Altitude dan Azimuth diinisialisasi dengan parameter kecepatan dan akselerasi maksimum yang diambil dari Config.h. Posisi awal motor diatur ke nol.
2. Koneksi Jaringan: Sistem mencoba untuk terhubung ke jaringan WiFi menggunakan kredensial (SSID dan kata sandi) yang didefinisikan dalam Config.h. Setelah terhubung, objek WiFiServer diinisialisasi untuk mulai mendengarkan koneksi klien yang masuk pada port MOUNTING_PORT.
3. Sinkronisasi Waktu: Panggilan ke timeClient.begin() dan syncNTP() dilakukan untuk menyinkronkan waktu sistem dengan server NTP eksternal. Waktu yang akurat sangat penting untuk kalkulasi astronomi. Setelah sinkronisasi pertama berhasil, flag useNTP diatur ke false untuk mencegah pembaruan lebih lanjut yang tidak perlu.
4. Inisialisasi Posisi Awal: Setelah waktu disinkronkan, sistem menghitung posisi langit awalnya. Fungsi ALTAZ_TO_RADEC dari modul Astro dipanggil untuk mengonversi posisi Altitude/Azimuth default (currentALT, currentAZ) menjadi koordinat Right Ascension dan Declination awal (currentRA, currentDEC).

3.2 Alur Perintah Slew (Gerak ke Target)

Alur kerja ini adalah proses paling fundamental dalam sistem, yang memungkinkan teleskop bergerak ke objek langit yang ditentukan. Proses ini melibatkan interaksi yang erat antara modul Command, Astro, Alignment, dan Move.

1. Penerimaan Target Koordinat: Klien (misalnya, Stellarium) mengirimkan perintah :Sr (Set RA) dan :Sd (Set DEC). Perintah ini diterima oleh main.cpp dan diteruskan ke fungsi handleCmd di modul Command. handleCmd mem-parsing nilai RA dan DEC dari string perintah dan menyimpannya di variabel global targetRA dan targetDEC.
2. Konversi Koordinat: Di dalam handler untuk perintah :Sd, sebuah panggilan krusial ke fungsi RADEC_TO_ALTAZ dari modul Astro dilakukan. Fungsi ini menggunakan targetRA, targetDEC, serta waktu dan lokasi saat ini untuk menghitung koordinat mekanis yang sesuai, yaitu targetALT dan targetAZ. Fungsi ini dieksekusi setelah perintah :Sd diterima—bukan :Sr—karena sistem memerlukan kedua koordinat (RA dan DEC) untuk melakukan konversi yang valid. Menempatkannya di handler perintah kedua memastikan semua data yang diperlukan telah diterima.
3. Inisiasi Gerakan: Pengguna kemudian mengirimkan perintah :MS (Slew) untuk memulai pergerakan.
4. Aplikasi Model Koreksi: Di dalam handleCmd, perintah :MS memicu panggilan ke fungsi SLEW_TO_TARGET di Move.cpp. Langkah pertama di dalam SLEW_TO_TARGET adalah memanggil APPLY_MODEL dari modul Alignment. Fungsi ini menghitung nilai koreksi AZ_CORR dan ALT_CORR berdasarkan model yang ada untuk mengkompensasi kesalahan mekanis pada dudukan teleskop.
5. Eksekusi Gerakan Motor: SLEW_TO_TARGET kemudian mengurangi nilai koreksi dari targetALT dan targetAZ untuk mendapatkan posisi motor yang sebenarnya. Nilai ini diubah menjadi target langkah motor, dan panggilan ke alt_stepper.moveTo() serta az_stepper.moveTo() dikeluarkan untuk memerintahkan pustaka AccelStepper memulai gerakan.

3.3 Alur Pelacakan Objek (Tracking)

Setelah teleskop mencapai targetnya (slew selesai), sistem harus terus bergerak secara perlahan untuk melawan rotasi bumi. Proses ini dikelola oleh blok if(isTracking == 'T') di dalam fungsi loop() pada main.cpp.

1. Status isTracking: Variabel global isTracking diatur ke 'T' segera setelah proses slew yang dipicu oleh perintah :MS selesai.
2. Rekalkulasi Posisi: Di setiap iterasi loop utama (dibatasi oleh PRINT_INTERVAL untuk mencegah pemanggilan yang berlebihan), sistem kembali memanggil RADEC_TO_ALTAZ. Tujuannya adalah untuk menghitung ulang posisi ALT/AZ target berdasarkan waktu saat ini. Meskipun koordinat langit (RA/DEC) sebuah bintang tetap, posisinya di langit lokal (ALT/AZ) terus berubah akibat rotasi bumi. Panggilan berulang ini menghasilkan serangkaian target ALT/AZ yang terus bergerak, yang menjadi dasar dari pelacakan.
3. Perintah Gerakan Korektif: Hasil kalkulasi ALT/AZ yang baru ini kemudian diteruskan ke SLEW_TO_TARGET. Ini menghasilkan perintah gerakan yang kecil dan kontinu ke motor, memastikan teleskop tetap terkunci pada objek target saat ia bergerak melintasi langit.

3.4 Alur Pembaruan Posisi Teleskop

Agar dapat melaporkan posisinya kembali ke klien dan melakukan perhitungan internal, sistem harus secara kontinu mengetahui ke arah mana teleskop sedang menunjuk. Proses ini terjadi di setiap iterasi loop() di main.cpp melalui dua langkah utama:

1. Fungsi GET_CURRENT_POS() dari Move.cpp dipanggil. Fungsi ini membaca posisi langkah motor mentah dari alt_stepper dan az_stepper, lalu mengonversinya ke derajat. Yang terpenting, nilai koreksi (ALT_CORR, AZ_CORR) yang dihitung oleh model alignment ditambahkan ke posisi motor ini. Hasilnya adalah representasi yang lebih akurat dari arah sebenarnya teleskop menunjuk di langit lokal (currentALT, currentAZ), bukan hanya posisi mekanisnya.
2. Fungsi ALTAZ_TO_RADEC() dari Astro.cpp kemudian dipanggil. Fungsi ini mengambil currentALT dan currentAZ yang baru diperbarui dan mengubahnya kembali menjadi koordinat langit, yaitu currentRA dan currentDEC. Nilai-nilai inilah yang siap untuk dikirim ke klien saat diminta melalui perintah :GR atau :GD.

Alur-alur ini menunjukkan bagaimana komponen-komponen yang terpisah bekerja sama dalam sebuah siklus yang terkoordinasi untuk menyediakan fungsionalitas kontrol teleskop yang canggih.

4.0 Analisis Komponen Utama

Bagian ini akan mengupas lebih dalam logika internal dan fungsionalitas dari modul-modul yang paling kompleks dan krusial dalam sistem SELENA, yaitu Alignment, Astro, dan Command. Pemahaman mendalam tentang modul-modul ini sangat penting untuk kustomisasi dan pemecahan masalah.

4.1 Modul Alignment: Model Koreksi Penunjuk

Tujuan strategis dari modul Alignment adalah untuk secara matematis mengkompensasi ketidaksejajaran mekanis dan kesalahan pemasangan teleskop. Dalam dunia nyata, dudukan teleskop tidak pernah sempurna. Modul ini menyediakan cara untuk mengukur dan memperbaiki ketidaksempurnaan ini, yang merupakan faktor kritis untuk mencapai akurasi penunjukan yang tinggi.

* Pengumpulan Data (addDataPoint): Fungsi ini dipanggil melalui perintah :CM setelah pengguna secara manual menengahkan bintang referensi di eyepiece. Fungsi ini tidak hanya menyimpan selisih kesalahan (deltaAZ, deltaALT), tetapi juga membangun baris-baris untuk matriks desain alignmentMatrices. Koefisien seperti AZ_P5_COEF = tan(targetALT * DEG_TO_RAD) * sin(targetAZ * DEG_TO_RAD) dan AZ_P6_COEF dihitung. Ini adalah implementasi dari suku-suku dalam model penunjukan TPoint atau serupa, yang memodelkan kesalahan seperti ketidaksejajaran sumbu polar dan non-ortogonalitas sumbu.
* Perhitungan Model (fitting): Fungsi ini adalah inti dari modul alignment. Fungsi ini secara efektif menyelesaikan persamaan matriks A*x = b, di mana A adalah matriks desain yang dibangun di addDataPoint, b adalah vektor kesalahan yang diamati, dan x adalah vektor koefisien model[4] yang tidak diketahui. Proses inversi matriks AT_A_inv menggunakan eliminasi Gauss-Jordan adalah metode numerik untuk menemukan x, yaitu koefisien model yang 'paling cocok' dengan data yang ada menurut kriteria least-squares.
* Aplikasi Model (APPLY_MODEL): Fungsi ini adalah hasil akhir dari seluruh proses alignment. Setiap kali teleskop akan melakukan slew, fungsi ini dipanggil. Secara spesifik, fungsi ini menerapkan formula seperti AZ_CORR = P1 + (P5 * tan(...) * sin(...)) - (P6 * tan(...) * cos(...)), di mana P1, P5, dan P6 adalah koefisien yang dihitung dalam model[]. Ini secara matematis 'memutar' dan 'menggeser' sistem koordinat target untuk mencocokkan sistem koordinat mekanis teleskop yang tidak sempurna. Perhatikan bahwa parameter fungsi APPLY_MODEL (P1, P7, P5, P6) langsung dipetakan ke elemen-elemen array model (model[0], model[1], model[2], model[3]).
* Status Alignment: Kualitas model koreksi bergantung pada jumlah data yang dikumpulkan. Variabel alignmentStatus melacak proses ini. Nilainya berubah ('0', '1', '2', '3') seiring dengan bertambahnya jumlah titik data (bintang referensi), yang mengindikasikan kepada sistem dan pengguna seberapa andal model koreksi yang sedang digunakan.

4.2 Modul Astro: Mesin Kalkulasi Koordinat

Modul Astro adalah fondasi matematis yang memungkinkan teleskop menerjemahkan antara "bahasa langit" (Right Ascension/Declination) dan "bahasa mekanis"-nya (Altitude/Azimuth). Tanpa konversi yang akurat ini, fungsionalitas "GoTo" dan pelacakan tidak akan mungkin terjadi.

Fungsi	Input	Output & Tujuan
JulianDay	time_t epoch	Menghasilkan nilai Julian Day, sebuah sistem penanggalan kontinu yang digunakan sebagai standar dalam kalkulasi astronomi untuk menghindari kerumitan kalender.
lst (Local Sidereal Time)	JD, lonDeg	Menghitung Waktu Bintang Lokal (LST) dalam derajat. LST adalah ukuran waktu yang didasarkan pada rotasi bumi relatif terhadap bintang-bintang, dan esensial untuk mengetahui orientasi langit pada lokasi dan waktu tertentu.
RADEC_TO_ALTAZ	RA, DECLINATION, LATITUDE, LONGITUDE, EPOCH	Menghitung koordinat Altitude (ALT) dan Azimuth (AZ) dari sebuah objek langit. Ini adalah fungsi "GoTo" inti, yang menjawab pertanyaan: "Untuk melihat objek di RA/DEC ini, ke mana saya harus mengarahkan teleskop saya sekarang?"
ALTAZ_TO_RADEC	ALTITUDE, AZIMUTH, LATITUDE, LONGITUDE, EPOCH	Menghitung koordinat Right Ascension (RA) dan Declination (DEC) dari arah yang sedang ditunjuk oleh teleskop. Ini adalah fungsi "Get Position" inti, yang menjawab pertanyaan: "Teleskop saya menunjuk ke arah ALT/AZ ini, objek apa yang ada di sana?"

4.3 Modul Command: Antarmuka Protokol LX200

Modul Command berfungsi sebagai lapisan adaptor protokol yang memungkinkan perangkat lunak pihak ketiga seperti Stellarium berkomunikasi dengan SELENA. Modul ini mengimplementasikan subset dari protokol teks LX200 yang populer, mem-parsing perintah yang masuk dan memicu tindakan yang sesuai di dalam sistem.

Tabel berikut merangkum perintah-perintah utama yang ditangani oleh fungsi handleCmd:

Perintah	Deskripsi	Aksi Utama yang Dipicu
:GR#, :GD#	Get RA/DEC: Mendapatkan Right Ascension dan Declination teleskop saat ini.	Mengembalikan nilai dari variabel global currentRA dan currentDEC setelah memformatnya ke dalam format string yang sesuai.
:Sr...#, :Sd...#	Set RA/DEC: Mengatur Right Ascension dan Declination dari objek target.	Mengisi variabel targetRA dan targetDEC. Setelah :Sd diterima, RADEC_TO_ALTAZ dipanggil untuk menghitung targetALT/targetAZ.
:MS#	Move Slew: Memerintahkan teleskop untuk bergerak (slew) ke target yang telah diatur.	Memanggil SLEW_TO_TARGET dari Move.cpp untuk memulai gerakan.
:M*# (:Me#, :Mw#, dst.)	Move Manual: Menggerakkan teleskop secara manual ke arah tertentu (Utara, Selatan, Timur, Barat).	Memanggil az_stepper.moveTo() atau alt_stepper.moveTo() dengan target langkah yang sangat besar untuk memulai gerakan kontinu.
:Q#	Quit Move: Menghentikan semua gerakan motor secara seketika.	Memanggil alt_stepper.stop() dan az_stepper.stop().
:CM#	Calibrate/Sync Mount: Menyinkronkan teleskop pada posisi objek saat ini, digunakan untuk alignment.	Memanggil addDataPoint dari Alignment.cpp untuk menambahkan titik data baru ke model koreksi.

Struktur if-else if yang ekstensif di dalam fungsi handleCmd adalah inti dari logika kontrol berbasis perintah sistem, yang secara efektif mengubah pesan teks sederhana menjadi tindakan elektromekanis yang kompleks.

5.0 Konfigurasi dan Variabel Global

Peran sentral Config.h dan Config.cpp adalah sebagai satu-satunya sumber kebenaran (single source of truth) untuk semua parameter dan variabel status yang dapat dikonfigurasi dalam sistem. Dengan memusatkan semua konstanta dan variabel global di satu tempat, pemeliharaan dan penyesuaian sistem menjadi jauh lebih mudah dan tidak rentan terhadap kesalahan.

5.1 Parameter Fisik dan Motor

Tabel ini merinci konstanta yang berkaitan langsung dengan perangkat keras dan mekanika dudukan teleskop. Nilai-nilai ini fundamental untuk mengubah perintah sudut menjadi gerakan motor yang akurat.

Konstanta	Nilai Contoh dari Kode	Signifikansi
MOTOR_STEPS	200.0	Jumlah langkah penuh yang dibutuhkan motor stepper untuk melakukan satu putaran 360 derajat. Ini adalah spesifikasi dasar dari motor itu sendiri.
MICROSTEPS	8	Faktor pembagi langkah yang diatur pada driver motor (misalnya, TMC2209). Nilai 8 berarti setiap langkah penuh dibagi menjadi 8 langkah mikro, menghasilkan gerakan yang lebih halus dan resolusi yang lebih tinggi.
GEAR_RATIO	300.0	Rasio roda gigi antara poros motor dan sumbu utama teleskop. Nilai 300 berarti motor harus berputar 300 kali agar sumbu teleskop berputar satu kali.
STEPS_PER_DEGREE	1333.33...	Faktor konversi fundamental yang dihitung dari tiga konstanta di atas. Nilai ini memberi tahu sistem berapa banyak langkah mikro yang diperlukan untuk menggerakkan teleskop sebesar satu derajat. Ini adalah nilai terpenting untuk semua perhitungan gerakan.
presetSpeed[]	{0.03, 0.30, 1.00, 3.00}	Kecepatan slew yang telah ditentukan sebelumnya dalam satuan derajat per detik. Kecepatan ini dapat dipilih oleh pengguna melalui perintah LX200 :RG (Guide), :RC (Center), :RM (Medium), dan :RS (Slew).

5.2 Variabel Status dan Posisi

Tabel ini menjelaskan variabel global utama yang melacak status dinamis sistem. Variabel-variabel ini terus diperbarui selama operasi untuk mencerminkan keadaan teleskop saat ini.

Variabel	Tipe Data	Fungsi dalam Sistem
currentALT, currentAZ	double	Menyimpan posisi lokal (Altitude/Azimuth) teleskop saat ini dalam derajat. Nilai ini diperbarui secara kontinu di dalam loop() oleh fungsi GET_CURRENT_POS.
currentRA, currentDEC	double	Posisi langit teleskop saat ini, dihitung sebagai output dari ALTAZ_TO_RADEC menggunakan currentALT dan currentAZ sebagai inputnya.
targetALT, targetAZ	double	Koordinat mekanis yang dituju, dihitung sebagai output dari RADEC_TO_ALTAZ menggunakan targetRA dan targetDEC sebagai inputnya.
targetRA, targetDEC	double	Menyimpan koordinat objek yang dituju. Diatur oleh perintah :Sr/:Sd dan berfungsi sebagai input untuk konversi ke ALT/AZ.
alignmentStatus	char	Status proses alignment: 'H' (Aligned on Home), '0' (Not aligned), '1' (One star aligned), '2' (Two star aligned), '3' (Three star aligned), 'P' (Scope was parked). Menentukan apakah dan bagaimana model koreksi diterapkan.
isTracking	char	Sebuah flag yang mengontrol logika pelacakan. Diatur ke 'T' (Tracking) setelah slew selesai dan 'N' (No) saat melakukan gerakan manual.
isSlewing	bool	Sebuah flag yang menunjukkan apakah teleskop sedang dalam proses slew besar (pergerakan jarak jauh).
