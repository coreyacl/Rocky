// This code should help you get started with your balancing robot.
// The code performs the following steps
// Calibration phase:
//  In this phase the robot should be stationary lying on the ground.
//  The code will record the gyro data for a couple of seconds to zero
//  out any gyro drift.
//
//  The robot has a hardcoded angle offset between the lying down and the
//    standing up configuration.  This offset can be modified in Balance.cpp around the lines below:
//
//      // this is based on coarse measurement of what I think the angle would be resting on the flat surface. 
//      // this corresponds to 94.8 degrees
//      angle = 94827-6000;
//
// Waiting phase:
//  The robot will now start to integrate the gyro over time to estimate
//  the angle.  Once the angle gets within +/- 3 degrees of vertical,
//  we transition into the armed phase.  A buzzer will sound to indicate
//  this transition.
//
// Armed phase:
//  The robot is ready to go, however, it will not start executing its control
//  loop until the angle leaves the region of [-3 degrees, 3 degrees].  This
//  allows you to let go of your robot, and it won't start moving until it's started
//  to fall just a little bit.  Once it leaves the region around vertical, it enters
//  the controlled phase.
//
// Controlled phase:
//  Here you can implement your control logic to do your balancing (or any of the
//  other Olympic events.


#include <Balboa32U4.h>
#include "Balance.h"

#define METERS_PER_CLICK 3.141592*80.0*(1/1000.0)/12.0/(162.5)
#define MOTOR_MAX 300
#define MAX_SPEED 0.75  // m/s
#define FORTY_FIVE_DEGREES_IN_RADIANS 0.78
#define func 2

extern int32_t angle_accum;
extern int32_t speedLeft;
extern int32_t driveLeft;
extern int32_t distanceRight;
extern int32_t speedRight;
extern int32_t distanceLeft;
extern int32_t distanceRight;

float vL, vR, totalDistanceLeft, totalDistanceRight;
float leftMotorPWM = 0;
float rightMotorPWM = 0;

void balanceDoDriveTicks();

extern int32_t displacement;
int32_t prev_displacement=0;

#if func==1
float Kp = -60;
float Ki = -55;
float Jp = 14;
float Ji = 90;
float Kt = -0.1;
#elif func==2
float Kp = -28.6;
float Ki = -135;
float Jp = 30.4;
float Ji = 40.7;  
float Kt = 0;
#endif
float PWMR;
float error2AccumR = 0;
float error1AccumR = 0;
float AccumVR = 0;

float error1R;
float error2R;
float VdR;


float PWML;
float error2AccumL = 0;
float error1AccumL = 0;
float AccumVL = 0;

float error1L;
float error2L;


 
LSM6 imu;
Balboa32U4Motors motors;
float VdL;

float Thetad = 0;;
Balboa32U4Encoders encoders;
Balboa32U4Buzzer buzzer;
Balboa32U4ButtonA buttonA;

void updatePWMs(float totalDistanceLeft, float totalDistanceRight, float vL, float vR, float angleRad, float angleRadAccum, float delta_t) {
  /* You will fill this function in with your code to run the race.  The inputs to the function are:
   *    totalDistanceLeft: the total distance travelled by the left wheel (meters) as computed by the encoders
   *    totalDistanceRight: the total distance travelled by the right wheel (meters) as computed by the encoders
   *    vL: the velocity of the left wheel (m/s) measured over the last 10ms
   *    vR: the velocity of the right wheel (m/s) measured over the last 10ms
   *    angleRad: the angle in radians relative to vertical (note: not the same as error)
   *    angleRadAccum: the angle integrated over time (note: not the same as error)
   */
if(func == 1){
   Thetad = 0;
   float Dis = (totalDistanceRight+totalDistanceLeft)/2;
//   AccumVL += vL*delta_t;//integral of velocity over time 
   error1L = Thetad - angleRad + (Kt*Dis);
   error1AccumL += error1L*delta_t; //integral of error over time;
   VdL = Kp*(error1L) + Ki*(error1AccumL);

   error2L = VdL - vL;
   error2AccumL += error2L*delta_t; //integral of error over time
   PWML = Jp*(error2L) + Ji*(error2AccumL);


//   AccumVR += vR*delta_t;//integral of velocity over time 
   error1R = Thetad - angleRad + (Kt*Dis);
   error1AccumR += error1R*delta_t; //integral of error over time;
   VdR = Kp*(error1R) + Ki*(error1AccumR);

   error2R = VdR - vR;
   error2AccumR += error2R*delta_t; //integral of error over time
   PWMR = Jp*(error2R) + Ji*(error2AccumR);

   //qualitatively chosen
   int lim = 200;

   // variable = (if statement) ? value if true : value if false;
   // basically caps out velocity
   PWMR = PWMR > lim? lim : PWMR;
   PWML = PWML > lim ? lim : PWML;

   PWMR = PWMR < -lim ? -lim : PWMR;
   PWML = PWML <- lim ? -lim : PWML;

}else if(func == 2){
   Thetad = 0;
   error1L = Thetad - angleRad;
   error1AccumL += error1L*delta_t; //integral of error over time;
   VdL = Kp*(error1L) + Ki*(error1AccumL);

   error2L = VdL - vL +2;
   error2AccumL += error2L*delta_t; //integral of error over time
   PWML = Jp*(error2L) + Ji*(error2AccumL);
 
   error1R = Thetad - angleRad;
   error1AccumR += error1R*delta_t; //integral of error over time;
   VdR = Kp*(error1R) + Ki*(error1AccumR);

   error2R = VdR - vR +2;
   error2AccumR += error2R*delta_t; //integral of error over time
   PWMR = Jp*(error2R) + Ji*(error2AccumR);

   //qualitatively chosen
   int lim = 200;

   // variable = (if statement) ? value if true : value if false;
   // basically caps out velocity
   PWMR = PWMR > lim? lim : PWMR;
   PWML = PWML > lim ? lim : PWML;

   PWMR = PWMR < -lim ? -lim : PWMR;
   PWML = PWML <- lim ? -lim : PWML;
    
  
}else if(func == 3){
   Thetad = 0;
   int con = 0;

   error1L = Thetad - angleRad;
   error1AccumL += error1L*delta_t; //integral of error over time;
   VdL = Kp*(error1L) + Ki*(error1AccumL);

   error2L = VdL - vL;
   error2AccumL += error2L*delta_t; //integral of error over time
   PWML = Jp*(error2L) + Ji*(error2AccumL) + con;


   error1R = Thetad - angleRad;
   error1AccumR += error1R*delta_t; //integral of error over time;
   VdR = Kp*(error1R) + Ki*(error1AccumR);

   error2R = VdR - vR;
   error2AccumR += error2R*delta_t; //integral of error over time
   PWMR = Jp*(error2R) + Ji*(error2AccumR) - con;

   //qualitatively chosen
   int lim = 150;
   double mult1 = 1.5;
   double mult2 = 1.5;

   // variable = (if statement) ? value if true : value if false;
   // basically caps out velocity
   PWMR = PWMR > lim? lim : PWMR;
   PWML = PWML > lim ? lim : PWML;

   PWMR = PWMR < -lim ? -lim : PWMR;
   PWML = PWML <- lim ? -lim : PWML;

   PWMR *= mult1;
   PWML *= mult2;     
  
}


   //PWMR = Jp*(error2R) + Ji*(error2AccumR);
  
   
  leftMotorPWM = PWML;
  rightMotorPWM = PWMR;
  Serial1.println(PWML);
  Serial1.println(PWMR);
  Serial1.println(" ");
  Serial1.println(" ");
}

uint32_t prev_time;

void setup()
{
  Serial1.begin(115200);
  prev_time = 0;
  ledYellow(0);
  ledRed(1);
  balanceSetup();
  ledRed(0);
  angle_accum = 0;
  ledGreen(0);
  ledYellow(0);
}

extern int16_t angle_prev;
int16_t start_flag = 0;
int16_t armed_flag = 0;
int16_t start_counter = 0;
void lyingDown();
extern bool isBalancingStatus;
extern bool balanceUpdateDelayedStatus;

void newBalanceUpdate()
{
  static uint32_t lastMillis;
  uint32_t ms = millis();

  if ((uint32_t)(ms - lastMillis) < UPDATE_TIME_MS) { return; }
  balanceUpdateDelayedStatus = ms - lastMillis > UPDATE_TIME_MS + 1;
  lastMillis = ms;

  // call functions to integrate encoders and gyros
  balanceUpdateSensors();
 
  if (imu.a.x < 0)
  {
    lyingDown();
    isBalancingStatus = false;
  }
  else
  {
    isBalancingStatus = true;
  }
}


void loop()
{
  uint32_t cur_time = 0;
  static uint32_t prev_print_time = 0;   // this variable is to control how often we print on the serial monitor
  static float angle_rad;                // this is the angle in radians
  static float angle_rad_accum = 0;      // this is the accumulated angle in radians
  static float error_ = 0;      // this is the accumulated velocity error in m/s
  static float error_left_accum = 0;      // this is the accumulated velocity error in m/s
  static float error_right_accum = 0;      // this is the accumulated velocity error in m/s


  cur_time = millis();                   // get the current time in miliseconds



  newBalanceUpdate();                    // run the sensor updates. this function checks if it has been 10 ms since the previous 
  
  if(angle > 3000 || angle < -3000)      // If angle is not within +- 3 degrees, reset counter that waits for start
  {
    start_counter = 0;
  }

  bool shouldPrint = cur_time - prev_print_time > 105;
//  if(shouldPrint)   // do the printing every 105 ms. Don't want to do it for an integer multiple of 10ms to not hog the processor
 // {
     //   Serial1.println(angle_rad);  
        /*Serial.print("\t");
        Serial.print(angle_rad_accum);  
        Serial.print("\t");
        Serial.print(leftMotorPWM);
        Serial.print("\t");
        Serial.print(rightMotorPWM);
        Serial.print("\t");
        Serial.print(vL);
        Serial.print("\t");
        Serial.print(vR);
        Serial.print("\t");
        Serial.print(totalDistanceLeft);
        Serial.print("\t");
        Serial.println(totalDistanceRight);*/

        prev_print_time = cur_time;
//Uncomment this and comment the above if doing wireless
        //Serial1.print(angle_rad);  
        //Serial1.print("\t");
        //Serial1.print(angle_rad_accum);  
        //Serial1.print("\t");
        //Serial1.print(PWM_left);
        //Serial1.print("\t");
        //Serial1.print(PWM_right);
        //Serial1.print("\t");
        //Serial1.print(vL);
        //Serial1.print("\t");
        //Serial1.println(vR);
      
 // }

  float delta_t = (cur_time - prev_time)/1000.0;

  // handle the case where this is the first time through the loop
  if (prev_time == 0) {
    delta_t = 0.01;
  }
  
  // every UPDATE_TIME_MS, check if angle is within +- 3 degrees and we haven't set the start flag yet
  if(cur_time - prev_time > UPDATE_TIME_MS && angle > -3000 && angle < 3000 && !armed_flag)   
  {
    // increment the start counter
    start_counter++;
    // If the start counter is greater than 30, this means that the angle has been within +- 3 degrees for 0.3 seconds, then set the start_flag
    if(start_counter > 30)
    {
      armed_flag = 1;
      buzzer.playFrequency(DIV_BY_10 | 445, 1000, 15);
    }
  }

  // angle is in millidegrees, convert it to radians and subtract the desired theta
  angle_rad = ((float)angle)/1000/180*3.14159;

  // only start when the angle falls outside of the 3.0 degree band around 0.  This allows you to let go of the
  // robot before it starts balancing
  if(cur_time - prev_time > UPDATE_TIME_MS && (angle < -3000 || angle > 3000) && armed_flag)   
  {
    start_flag = 1;
    armed_flag = 0;
    angle_rad_accum = 0.0;

    AccumVL = 0;
    AccumVR = 0;

    error1AccumL = 0;
    error2AccumL = 0;
    error1AccumR = 0;
    error2AccumR = 0;

    //error2AccumR = 0;
  }
  
  // every UPDATE_TIME_MS, if the start_flag has been set, do the balancing
  if(cur_time - prev_time > UPDATE_TIME_MS && start_flag)
  {
    // set the previous time to the current time for the next run through the loop
    prev_time = cur_time;

    // speedLeft and speedRight are just the change in the encoder readings
    // wee need to do some math to get them into m/s
    vL = METERS_PER_CLICK*speedLeft/delta_t;
    vR = METERS_PER_CLICK*speedRight/delta_t;

    totalDistanceLeft = METERS_PER_CLICK*distanceLeft;
    totalDistanceRight = METERS_PER_CLICK*distanceRight;
    angle_rad_accum += angle_rad*delta_t;

 

    updatePWMs(totalDistanceLeft, totalDistanceRight, vL, vR, angle_rad, angle_rad_accum, delta_t);

    // if the robot is more than 45 degrees, shut down the motor
    if(start_flag && fabs(angle_rad) > FORTY_FIVE_DEGREES_IN_RADIANS)
    {
      // reset the accumulated errors here
      start_flag = 0;   /// wait for restart
      prev_time = 0;
      motors.setSpeeds(0, 0);
    } else if(start_flag) {
      motors.setSpeeds((int)leftMotorPWM, (int)rightMotorPWM);
    }
  }

  // kill switch
  if (buttonA.getSingleDebouncedPress())
  {
      motors.setSpeeds(0,0);
      while(!buttonA.getSingleDebouncedPress());
  }
}
