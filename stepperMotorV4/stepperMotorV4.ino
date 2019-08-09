#include <Wire.h>
#include <Adafruit_MotorShield.h>

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_StepperMotor *myStepper = AFMS.getStepper(200, 2);
    
int switchState_1 = 0;
int switchState_2 = 0;

int prev_switchState_1 = 0;
int prev_switchState_2 = 0;

const int switchPin_1 = 11;
const int switchPin_2 = 12;

int lim1State = 0;
int lim2State = 0;

const int lim1Pin = 6;
const int lim2Pin = 4;

int microButtonState = 0;
const int microButtonPin = 9;
int prevMicroButtonState = 0;

int numSteps = 0;

String stepRequest;

String dirString;

String stepTypeString;

bool paused = false;

void setup() {
  Serial.begin(9600);
  AFMS.begin();
 
  pinMode(switchPin_1, OUTPUT);
  pinMode(switchPin_2, OUTPUT);
  pinMode(lim1Pin, OUTPUT);
  pinMode(lim2Pin, OUTPUT);
  pinMode(microButtonPin, OUTPUT);
  
  prev_switchState_1 = digitalRead(switchPin_1);
  prev_switchState_2 = digitalRead(switchPin_2);
  
  prevMicroButtonState = digitalRead(microButtonPin);
  
}

void loop() {
  
  update_switchStates();

  if (paused == false) {
    if ((switchState_1 != prev_switchState_1)||(switchState_2 != prev_switchState_2)) {
      delay(100);
      if (switchState_1 == HIGH) {
        customStep(lim1State, lim1Pin, FORWARD, DOUBLE, 300);
      }
      else if ((switchState_1 == LOW)&&(switchState_2 == LOW)) {
        motor_brake();
        reset_switchStates();
        return;
      }
      else if (switchState_2 == HIGH) {
        customStep(lim2State, lim2Pin, BACKWARD, DOUBLE, 300);
      }
    }
    
//The "microButton" is the little red button.
//The Arduino prompts the user to specify some number of microsteps.
  
  microButtonState = digitalRead(microButtonPin);
  
    if (microButtonState == HIGH) {
      motor_brake();
      Serial.write(0);
      digitalWrite(microButtonPin, LOW);  
      Serial.println("How many microsteps?");
      Serial.println("Input an integer value to continue.");
      Serial.println("To exit this prompt, return switch to neutral, and then input any value.");
      
      while (Serial.available() <= 0) {}
      if (Serial.available() > 0) {
   
        String stepRequest = Serial.readStringUntil('\n');
        //.toInt() makes stepRequestInt an empty value if the user inputted a non-integer value.
        int stepRequestInt = stepRequest.toInt();
        
        update_switchStates();
        
        if (switchState_1 == HIGH) {
          delay(100);
          Serial.print("Executing "); Serial.print(stepRequest); Serial.println(" microsteps.");
          customStep(lim1State, lim1Pin, FORWARD, MICROSTEP, stepRequestInt);       
        }
        else if (switchState_2 == HIGH) {
          delay(100);
          Serial.print("Executing "); Serial.print(stepRequest); Serial.println(" microsteps.");
          customStep(lim2State, lim2Pin, BACKWARD, MICROSTEP, stepRequestInt);
        }
        else if ((switchState_1 == LOW)&&(switchState_2 == LOW)) {
          delay(100);
          Serial.println("Switch is set to neutral. Flick switch either left or right to exit.");
          Serial.write(0);
          Serial.flush();
          paused = true;
        }
      }
    }
    //Inputting anything at all breaks out of the while loop and continues loop().
    //Only integer values will cause the stepper motor to microstep.
    else {}
  }
  if (paused == true) {
    if ((switchState_1 != prev_switchState_1)||(switchState_2 != prev_switchState_2)) {
      paused = true;
      reset_switchStates();
      delay(250);
      if (switchState_1 == HIGH) {
        Serial.println("Gate set to right.");
      }
      else if (switchState_2 == HIGH) {
        Serial.println("Gate set to left.");
      }
      paused = false;
    }
  }
}


void customStep(int limNumState, int limNumPin, int dir, int stepType, int stepLimit) {

//customStep is a function that consolidates the repetitive, messy lines that would
//otherwise be everywhere in loop()

//These messy if statements facilitate Serial communication.
//All directions and stepTypes are assigned to int values in <Adafruit_motorshield.h>.
//Because they only print as class int, I manually assign the appropriate Strings to each int.

  if (dir == 1) {
    dirString = "FORWARD";
  }
  else if (dir == 2) {
    dirString = "BACKWARD";
  }
  else if (dir == 3) {
    dirString = "BRAKE";
  }
  else if (dir == 4) {
    dirString = "RELEASE";
  }

  if (stepType == 1) {
    stepTypeString = "SINGLE";
  }
  else if (stepType == 2) {
    stepTypeString = "DOUBLE";
  }
  else if (stepType == 3) {
    stepTypeString = "INTERLEAVE";
  }
  else if (stepType == 4) {
    stepTypeString = "MICROSTEP";
  }

  myStepper->setSpeed(60);

//Stepper speed is in RPM.
  for (int numSteps = 0; numSteps <= stepLimit; numSteps += 1) {
    update_switchStates();
    if ((switchState_1 == HIGH)||(switchState_2 == HIGH)) {
      limNumState = digitalRead(limNumPin);
      if (limNumState == HIGH) {
        Serial.print("Limit switch activated. "); Serial.print(numSteps-1); Serial.print(" "+dirString); Serial.print(" "+stepTypeString); Serial.println(" steps executed.");
        motor_brake();
        break;
      } 
      else if (limNumState == LOW) {
        myStepper->step(1, dir, stepType);
        //Serial.println(numSteps);
      }
    }
    if ((limNumState == LOW)&&(switchState_1 == LOW)&&(switchState_2 == LOW)) {
      Serial.print("Brake activated. "); Serial.print(numSteps-1); Serial.print(" "+dirString); Serial.print(" "+stepTypeString); Serial.println(" steps executed.");
      break;
    }
    if (numSteps == stepLimit) {
      Serial.print(stepLimit); Serial.print(" "+dirString); Serial.print(" "+stepTypeString); Serial.println(" steps executed.");
      break;
    }
  }
  motor_brake();
  reset_switchStates();
  return;
}

void reset_switchStates() {
  prev_switchState_1 = switchState_1;
  prev_switchState_2 = switchState_2;
}

void update_switchStates() {
  switchState_1 = digitalRead(switchPin_1);
  switchState_2 = digitalRead(switchPin_2);
}

void motor_brake() {
  //myStepper->setSpeed(0);
  //myStepper->release();
  return;
}
