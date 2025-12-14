#include "Config.h"

// ===== DEFINISI VARIABEL GLOBAL (Memori dialokasikan di sini) =====
// Variabel Lokasi dan Status Mount (dari SELENA.ino)
time_t currentEpoch = 0;
String currentDate = "";
String currentTime = "";
bool useNTP = true;

// Non Blocking Component
unsigned long lastGet = 0;

double UTC_OFFSET = 7.0;

// Mount Initial position
double currentALT = 45;
double currentAZ = 90;
double currentRA = 0;
double currentDEC = 0;

time_t targetTime = 0;
double targetRA = 0;
double targetDEC = 0;
double targetALT = 0;
double targetAZ = 0;

bool isSlewing = false;
bool haveTarget = false;
char alignmentStatus = 'H'; //0 - Not aligned, 1 - One star aligned, 2 - Two star aligned, 3 - Three star aligned, H - Aligned on Home, P - Scope was parked



// ===== MOTOR CONFIGURATION =====
const double MOTOR_STEPS = 200.0; //STEPS FOR FULL ROTATION
const double MICROSTEPS = 8; // 8 (default TMC2209)
const double GEAR_RATIO = 300.0;
const double STEPS_PER_DEGREE = (MOTOR_STEPS * MICROSTEPS * GEAR_RATIO) / 360; //1333.333333333 STEPS PER DERGREE
const double MAX_SPEED_DEGREE_PER_SEC = 3;
const double ACCELERATION_DEGREE_PER_SEC2 = 10;
const double presetSpeed[4] = {0.03, 0.30, 1.00, 3.00};
const double presetAccel[4] = {2.0, 5.0, 8.0, 10.0};
long maxSpeedSteps = (long)(MAX_SPEED_DEGREE_PER_SEC * STEPS_PER_DEGREE);
long accelerationSteps = (long)(ACCELERATION_DEGREE_PER_SEC2 * STEPS_PER_DEGREE);


// ===== MOUNTING LIMITS =====
const double ALT_MIN_DEG = -5.0;
const double ALT_MAX_DEG = 95.0;
const double AZ_MIN_DEG = 0;
const double AZ_MAX_DEG = 360;

// ===== ALIGNMENT ======
int pointCount = 0;

double AZ_CORR = 0;
double ALT_CORR = 0;


