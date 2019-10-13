unsigned long endTimeMotor_A = 0;
unsigned long endTimeMotor_B = 0;
boolean isRunningMotor_A = false;
boolean isRunningMotor_B = false;

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

int pwm_a = 3;  //PWM control for motor outputs 1 and 2 
int pwm_b = 9;  //PWM control for motor outputs 3 and 4 
int dir_a = 2;  //direction control for motor outputs 1 and 2 
int dir_b = 8;  //direction control for motor outputs 3 and 4 

void setup()
{
  pinMode(pwm_a, OUTPUT);  //Set control pins to be outputs
  pinMode(pwm_b, OUTPUT);
  pinMode(dir_a, OUTPUT);
  pinMode(dir_b, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  recvWithEndMarker();
  processNewData();
  checkIfTimeToStop();
}

void checkIfTimeToStop() {
  unsigned long currentMillis = millis();
  
  // Serial.print("currentMillis: ");
  // Serial.println(currentMillis);
  
  // Serial.print("endTimeMotor_A: ");
  // Serial.println(endTimeMotor_A);
  
  // Serial.print("endTimeMotor_B: ");
  // Serial.println(endTimeMotor_B);
  
  // Serial.print("stopMotor_A: ");
  // Serial.println(endTimeMotor_A != 0 && currentMillis > endTimeMotor_A);
  
  // Serial.print("stopMotor_B: ");
  // Serial.println(endTimeMotor_B != 0 && currentMillis > endTimeMotor_B);

  if (endTimeMotor_A != 0 && currentMillis > endTimeMotor_A) {
    stopMotor_A();
    endTimeMotor_A = 0;
  }

  if (endTimeMotor_B != 0 && currentMillis > endTimeMotor_B) {
    stopMotor_B();
    endTimeMotor_B = 0;
  }
}

void recvWithEndMarker() {
  /*
    Expected Info
    Motor to spin: A or B
    Length of time (milli) for spin

    Format expected:

    <Motor>:<Spin time>\n

    Ex:

    A:132322\n

  */
  static byte index = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[index] = rc;
      index++;
      if (index >= numChars) {
        index = numChars - 1;
      }
    }
    else {
      receivedChars[index] = '\0'; // terminate the string
      index = 0;
      newData = true;
      
      Serial.print("Received Message: ");
      Serial.println(receivedChars);
    }
  }
}

void processNewData() {
  if (newData == true) {

    char motorToSpin;
    unsigned long milliToSpin;

    milliToSpin = strtoul(receivedChars + 2, NULL, 10);
    motorToSpin = receivedChars[0];
    
    Serial.println(motorToSpin);

    if (motorToSpin == 'A') {
      if (isRunningMotor_A) {
        endTimeMotor_A = endTimeMotor_A + milliToSpin;
      }
      else {
        unsigned long m = millis();
        endTimeMotor_A = m + milliToSpin;
        
        // Serial.print("Millis: ");
        // Serial.println(m);
        
        // Serial.print("endTimeMotor_A ");
        // Serial.println(endTimeMotor_A);
        startMotor_A();
      }
    }
    else if (motorToSpin == 'B') {
      if (isRunningMotor_B) {
        endTimeMotor_B = endTimeMotor_B + milliToSpin;
      }
      else {
        unsigned long m = millis();
        endTimeMotor_B = m + milliToSpin;
        
        // Serial.print("Millis: ");
        // Serial.println(m);
        
        // Serial.print("endTimeMotor_B ");
        // Serial.println(endTimeMotor_B);
        startMotor_B();
      }
    }

    newData = false;
  }
}

void stopMotor_A() {
  analogWrite(pwm_a, 0);  //Engage the Brake for Channel A

  isRunningMotor_A = false;
}

void stopMotor_B() {
  analogWrite(pwm_b, 0);  //Engage the Brake for Channel B

  isRunningMotor_B = false;
}

void startMotor_A() {
  //Motor A forward @ full speed
  analogWrite(pwm_a, 255);   //Spins the motor on Channel A at full speed

  isRunningMotor_A = true;
}

void startMotor_B() {
  //Motor B forward @ full speed
  analogWrite(pwm_b, 255);   //Spis the motor on Channel B at full speed

  isRunningMotor_B = true;
}