#include "Alignment.h"

double alignmentMatrices[MAX_POINTS][4];
double alignmentVector[MAX_POINTS];
double model[4] = { 0, 0, 0, 0};

void addDataPoint(double TARGET_AZ, double TARGET_ALT, double MOTOR_AZ, double MOTOR_ALT){
  double deltaAZ = TARGET_AZ - MOTOR_AZ;
  double deltaALT = TARGET_ALT - MOTOR_ALT;
  double AZ_P5_COEF = tan(TARGET_ALT * DEG_TO_RAD) * sin(TARGET_AZ * DEG_TO_RAD);
  double AZ_P6_COEF = -tan(TARGET_ALT * DEG_TO_RAD) * cos(TARGET_AZ * DEG_TO_RAD);
  double ALT_P5_COEF = cos(TARGET_AZ * DEG_TO_RAD);
  double ALT_P6_COEF = sin(TARGET_AZ * DEG_TO_RAD);

  if (pointCount < MAX_POINTS) {
    double data[2][4] = {
      { 1, 0, AZ_P5_COEF, AZ_P6_COEF },
      { 0, 1, ALT_P5_COEF, ALT_P6_COEF}
    };
    double vector[] = { deltaAZ, deltaALT};
    for (int i = 0; i < 2; i++){
      alignmentVector[i+pointCount] = vector[i];
      for (int j = 0; j < 4; j++){
        alignmentMatrices[i+pointCount][j+pointCount] = data[i][j];
      }
    }
    pointCount+=2;
  }
  if (pointCount == 2){
    alignmentStatus = '1';
    model[1] = deltaALT;
    model[0] = deltaAZ;
  }
  else if (pointCount == 4){
    fitting();
    alignmentStatus = '2';
  }
  else if (pointCount > 4){
    fitting();
    alignmentStatus = '3';
  }
  else{
    alignmentStatus = '0';
  }
  Serial.println("Data Point Added");
  printMatrix("Model", model, 4, 1);
}

void fitting() {
  double A[pointCount][4];
  double b[pointCount];
  double AT[4][pointCount];  // transpose matriks A
  double AT_A[4][4] = {      // transpose matriks A dikali matriks A
                        { 0, 0, 0, 0 },
                        { 0, 0, 0, 0 },
                        { 0, 0, 0, 0 },
                        { 0, 0, 0, 0 }
  };
  double AT_A_inv[4][8] = {
    { 0, 0, 0, 0, 1, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1 }
  };
  double AT_A_inv_temp[4][8];
  double AT_A_inv_result[4][4];
  double AT_b[4] = { 0, 0, 0, 0 };
  double result[4] = { 0, 0, 0, 0 };

  // isi matriks A dan b
  for (int i = 0; i < pointCount; i++) {
    b[i] = alignmentVector[i];
    for (int j = 0; j < 4; j++) {
      A[i][j] = alignmentMatrices[i][j];
    }
  }
  // printMatrix("A", (double *)A, sizeof(A) / sizeof(A[0]), 4);

  // isi matriks AT (transpose A)
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < pointCount; j++) {
      AT[i][j] = A[j][i];
    }
  }
  // printMatrix("AT", (double *)AT, sizeof(AT) / sizeof(AT[0]), sizeof(AT[0]) / sizeof(AT[0][0]));

  // isi AT_A = AT * A
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < pointCount; k++) {
        // Serial.print("AT_A");
        // Serial.print(i);
        // Serial.print(j);
        // Serial.print(". ");
        // Serial.print(AT_A[i][j]);
        // Serial.print(" + (");
        // Serial.print(AT[j][k]);
        // Serial.print(" * ");
        // Serial.print(A[k][j]);
        // Serial.print(") = ");

        AT_A[i][j] += AT[i][k] * A[k][j];
        // Serial.println(AT_A[i][j]);
      }
    }
  }
  // printMatrix("AT_A", (double *)AT_A, sizeof(AT_A) / sizeof(AT_A[0]), sizeof(AT_A[0]) / sizeof(AT_A[0][0]));


  // isi AT_A_inv
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      AT_A_inv[i][j] = AT_A[i][j];
    }
  }
  // printMatrix("AT_A_inv", (double *)AT_A_inv, sizeof(AT_A_inv) / sizeof(AT_A_inv[0]), sizeof(AT_A_inv[0]) / sizeof(AT_A_inv[0][0]));

  //isi AT_A_inv_temp
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      AT_A_inv_temp[i][j] = AT_A_inv[i][j];
    }
  }
  // printMatrix("AT_A_inv_temp", (double *)AT_A_inv_temp, sizeof(AT_A_inv_temp) / sizeof(AT_A_inv_temp[0]), sizeof(AT_A_inv_temp[0]) / sizeof(AT_A_inv_temp[0][0]));

  // X[1] operation #row1
  double divisor = AT_A_inv_temp[0][0];
  for (int i = 0; i < 8; i++) {
    AT_A_inv[0][i] = AT_A_inv_temp[0][i] / divisor;
  }
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      if (i != 0) {
        AT_A_inv[i][j] = AT_A_inv_temp[i][j] - (AT_A_inv_temp[i][0] * AT_A_inv[0][j]);
      }
    }
  }
  // Serial.println("X[1]");
  // printMatrix("AT_A_inv", (double *)AT_A_inv, sizeof(AT_A_inv) / sizeof(AT_A_inv[0]), sizeof(AT_A_inv[0]) / sizeof(AT_A_inv[0][0]));


  // update AT_A_inv_temp
  //isi AT_A_inv_temp
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      AT_A_inv_temp[i][j] = AT_A_inv[i][j];
    }
  }
  // printMatrix("AT_A_inv_temp", (double *)AT_A_inv_temp, sizeof(AT_A_inv_temp) / sizeof(AT_A_inv_temp[0]), sizeof(AT_A_inv_temp[0]) / sizeof(AT_A_inv_temp[0][0]));


  // X[2] operation #row2
  divisor = AT_A_inv_temp[1][1];
  for (int i = 0; i < 8; i++) {
    AT_A_inv[1][i] = (1 / divisor) * AT_A_inv_temp[1][i];
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      if (i != 1) {
        AT_A_inv[i][j] = AT_A_inv_temp[i][j] - (AT_A_inv_temp[i][1] * AT_A_inv[1][j]);
      }
    }
  }
  // Serial.println("X[2]");
  // printMatrix("AT_A_inv", (double *)AT_A_inv, sizeof(AT_A_inv) / sizeof(AT_A_inv[0]), sizeof(AT_A_inv[0]) / sizeof(AT_A_inv[0][0]));

  // update AT_A_inv_temp
  //isi AT_A_inv_temp
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      AT_A_inv_temp[i][j] = AT_A_inv[i][j];
    }
  }
  // printMatrix("AT_A_inv_temp", (double *)AT_A_inv_temp, sizeof(AT_A_inv_temp) / sizeof(AT_A_inv_temp[0]), sizeof(AT_A_inv_temp[0]) / sizeof(AT_A_inv_temp[0][0]));


  // X[3] operation #row3
  divisor = AT_A_inv_temp[2][2];
  for (int i = 0; i < 8; i++) {
    AT_A_inv[2][i] = (1 / divisor) * AT_A_inv_temp[2][i];
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      if (i != 2) {
        AT_A_inv[i][j] = AT_A_inv_temp[i][j] - (AT_A_inv_temp[i][2] * AT_A_inv[2][j]);
      }
    }
  }
  // Serial.println("X[3]");
  // printMatrix("AT_A_inv", (double *)AT_A_inv, sizeof(AT_A_inv) / sizeof(AT_A_inv[0]), sizeof(AT_A_inv[0]) / sizeof(AT_A_inv[0][0]));

  // update AT_A_inv_temp
  //isi AT_A_inv_temp
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      AT_A_inv_temp[i][j] = AT_A_inv[i][j];
    }
  }
  // printMatrix("AT_A_inv_temp", (double *)AT_A_inv_temp, sizeof(AT_A_inv_temp) / sizeof(AT_A_inv_temp[0]), sizeof(AT_A_inv_temp[0]) / sizeof(AT_A_inv_temp[0][0]));


  // X[4] operation #row4
  divisor = AT_A_inv_temp[3][3];
  for (int i = 0; i < 8; i++) {
    AT_A_inv[3][i] = (1 / divisor) * AT_A_inv_temp[3][i];
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      if (i != 3) {
        AT_A_inv[i][j] = AT_A_inv_temp[i][j] - (AT_A_inv_temp[i][3] * AT_A_inv[3][j]);
      }
    }
  }
  // Serial.println("X[3]");
  // printMatrix("AT_A_inv", (double *)AT_A_inv, sizeof(AT_A_inv) / sizeof(AT_A_inv[0]), sizeof(AT_A_inv[0]) / sizeof(AT_A_inv[0][0]));

  // update AT_A_inv_temp
  //isi AT_A_inv_temp
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      AT_A_inv_temp[i][j] = AT_A_inv[i][j];
    }
  }
  // printMatrix("AT_A_inv_temp", (double *)AT_A_inv_temp, sizeof(AT_A_inv_temp) / sizeof(AT_A_inv_temp[0]), sizeof(AT_A_inv_temp[0]) / sizeof(AT_A_inv_temp[0][0]));


  // save AT invers result
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      if (j > 3) {
        AT_A_inv_result[i][j - 4] = AT_A_inv[i][j];
      }
    }
  }
  // printMatrix("AT_A_inv_result", (double *)AT_A_inv_result, sizeof(AT_A_inv_result) / sizeof(AT_A_inv_result[0]), sizeof(AT_A_inv_result[0]) / sizeof(AT_A_inv_result[0][0]));


  // isi AT_b = AT * b
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < pointCount; j++) {
      AT_b[i] += AT[i][j] * b[j];
    }
  }
  // printMatrix("AT_b", (double *)AT_b, sizeof(AT_b) / sizeof(AT_b[0]), 1);


  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      // Serial.print("R");
      // Serial.print(i);
      // Serial.print(j);
      // Serial.print(". ");
      // Serial.print(result[i]);
      // Serial.print(" + (");
      // Serial.print(AT_A_inv_result[i][j]);
      // Serial.print(" * ");
      // Serial.print(AT_b[j]);
      // Serial.print(") = ");

      result[i] += AT_A_inv_result[i][j] * AT_b[j];

      // Serial.println(result[i]);
    }
  }

  // input result to model variable;
  for (int i = 0; i < 4; i++) {
    model[i] = result[i];
  }

  printMatrix("model", (double *)model, sizeof(model) / sizeof(model[0]), 1);
}

void APPLY_MODEL(double P1, double P7, double P5, double P6, double TARGET_ALT, double TARGET_AZ){
  AZ_CORR = P1 + (P5 * tan(TARGET_ALT * DEG_TO_RAD) * sin(TARGET_AZ * DEG_TO_RAD)) - (P6 * tan(TARGET_ALT * DEG_TO_RAD) * cos(TARGET_AZ * DEG_TO_RAD));
  ALT_CORR = P7 + (P5 * cos(TARGET_AZ * DEG_TO_RAD)) + (P6 * sin(targetAZ * DEG_TO_RAD));
}

void printMatrix(String name, double *arr, int rows, int cols) {
  Serial.println("--- " + name + " ---");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      // Akses elemen: *(arr + i * cols + j)
      double value = *(arr + i * cols + j);
      Serial.print(value, 4);  // Cetak dengan 4 angka desimal
      Serial.print("\t");
    }
    Serial.println();
  }
}