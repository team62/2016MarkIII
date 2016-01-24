#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in8,    indexHigh,      sensorLineFollower)
#pragma config(Sensor, dgtl1,  encoderError,   sensorLEDtoVCC)
#pragma config(Sensor, dgtl10, tune,           sensorTouch)
#pragma config(Sensor, dgtl11, debug,          sensorTouch)
#pragma config(Sensor, dgtl12, encoderTest,    sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           rightWheel2,   tmotorVex393TurboSpeed_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           flywheel4,     tmotorVex393TurboSpeed_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port3,           rightWheel13,  tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           flywheel3,     tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port5,           leftWheel2,    tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port6,           flywheel1,     tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           flywheel2,     tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           leftWheel13,   tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port9,           indexer,       tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port10,          intake,        tmotorVex393HighSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(0)
#pragma userControlDuration(60)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

/*///////////////////////////////////////////////////////////
/////____________/\\\\\____/\\\\\\\\\_____              /////
///// ________/\\\\////___/\\\///////\\\___             /////
/////  _____/\\\///_______\///______\//\\\__            /////
/////   ___/\\\\\\\\\\\______________/\\\/___           /////
/////    __/\\\\///////\\\_________/\\\//_____          /////
/////     _\/\\\______\//\\\_____/\\\//________         /////
/////      _\//\\\______/\\\____/\\\/___________        /////
/////       __\///\\\\\\\\\/____/\\\\\\\\\\\\\\\_       /////
/////        ____\/////////_____\///////////////__      /////
///// Mark III Robot                                    /////
///// Driver Skills Code                              	/////
///// Authors: Jonathan Damico, Griffin Tabor           /////
///// Since: Jan. 22, 2016                              /////
*////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
/// JUMPER CABLE CONFIGURATIONS                                   ///
/// dgtl10 = tune mode (acts like you're holding down 5U and 6U)  ///
/// dgtl11 = debug mode (logs flywheel info to debug stream)      ///
/// dgtl12 = encoder test mode (checks encoder works at runtime)  ///
/////////////////////////////////////////////////////////////////////
//DEBUG VARIABLES
bool tuneMode = false; //acts like you're holding 5U and 6U
bool debugMode = false; //prints to console
bool encoderTestMode = false; //checks encoders at runtime

//Stores the differient speeds for the velocity states of the robot
enum { VELOCITY_LONG = 190, VELOCITY_PIPE = 125, VELOCITY_HOLD = 30 }; //MAY NEED TO SWITCH BACK TO typedef and a name before the semicolon

//Sets the speed of wheels on the left side of the robot
#warning "setLeftWheelSpeed"
void setLeftWheelSpeed ( int speed = 127 ) {
	motor[leftWheel13] = speed;
	motor[leftWheel2] = speed;
}

//Sets the speed of the wheels on the right side of the robot
#warning "setRightWheelSpeed"
void setRightWheelSpeed ( int speed = 127 ) {
	motor[rightWheel13] = speed;
	motor[rightWheel2] = speed;
}

//Sets both sides of hte drivebase to differient speeds
#warning "setWheelSpeed"
void setWheelSpeed ( int leftWheelSpeed = 127, int rightWheelSpeed = 127 ) {
	setLeftWheelSpeed(leftWheelSpeed);
	setRightWheelSpeed(rightWheelSpeed);
}

//Overloaded - lets both sides of the drivebase to the same speed
void setWheelSpeed ( int wheelSpeed = 127 ) {
	setWheelSpeed(wheelSpeed,wheelSpeed);
}

//Logarithmic drivebase control
#warning "logDrive"
void logDrive () {
	int rawLeft, rawRight, outLeft, outRight;
	rawLeft = vexRT(Ch3);
	rawRight = vexRT(Ch2);

	outLeft = rawLeft*rawLeft/127;
	outRight = rawRight*rawRight/127;

	if(rawLeft<0)
		outLeft*=-1;
	if(rawRight<0)
		outRight*=-1;

	setWheelSpeed(outLeft,outRight);
}

//Tank drive control for drivebase
#warning "tankDrive"
void tankDrive () {
	int deadbands = 10;
	setWheelSpeed(vexRT(Ch3)<deadbands?0:vexRT(Ch3),vexRT(Ch2)<deadbands?0:vexRT(Ch2));
}

//Instance variables for flywheel control
bool lastUpButton=false;
bool lastDownButton=false;
bool currentUpButton;
bool currentDownButton;
int currentGoalVelocity=VELOCITY_LONG;
int currentVelocity;

//Flywheel PID instance variables
float error=0;
float integral=0;
int output;
int velocities[5];

//Populates an array with the most recent velocities of the flywheel,
//used to calculate flywheel velocity
//TODO consider revising after 23/1/16
#warning "flywheelVelocity"
task flywheelVelocity(){
	int nextIndex=0;
	while(true){
		velocities[nextIndex]=getMotorVelocity(flywheel4);
		nextIndex++;
		if(nextIndex==5)
			nextIndex=0;
		delay(5);
	}
}

//Returns the velocity of the flywheel
//TODO consider revising after 23/1/16
#warning "getFlywheelVelocity"
int getFlywheelVelocity(){
	int sum=0;
	for(int i=0;i<5;i++)
		sum = sum + velocities[i];
	return sum/5;
}

bool flywheelOn = false;
//Controls the flywheel using PID
#warning "flywheelControl"
task flywheelControl(){
	flywheelOn = true;
	clearDebugStream();

	float kP=1.80;//was 1.675
	float kI=0.0000//1//07;//was 0.0025

	//float kP=0.8001;//was 0.72
	//float kI=0.05532;
	int limit = 15;
	while(true){

		currentVelocity = getFlywheelVelocity();//might need work
		error = (currentGoalVelocity - currentVelocity);
		integral = integral + error;
		//if(integral>(100/kI))
		//	integral = 100/kI;
		output = error*kP + integral*kI;
		if(output >25){
			if(output>motor[flywheel4]+limit){
				motor[flywheel4]=motor[flywheel4]+limit;
				}else if(output<motor[flywheel4]-limit){
				motor[flywheel4]=motor[flywheel4]-limit;
				}else{
				motor[flywheel4]=output;
			}
			}else if(output<0){
			motor[flywheel4]=0;
			//integral=0;
		}
		if(debugMode)
			writeDebugStreamLine("Motors: %d, Error: %d, P: %d, I: %d Integral: %d", motor[flywheel1], error, error*kP, integral*kI, integral);
		delay(70);
	}
}

bool flywheelHold = false;
//Starts the flywheel at a target velocity
#warning "startFlywheel"
void startFlywheel (int targetVelocity) {
	currentGoalVelocity = targetVelocity;
	if(targetVelocity == (int) VELOCITY_HOLD) {	//If we are holding the motors,
		motor[flywheel4] = VELOCITY_HOLD;					//we don't want to startup the PID
		stopTask(flywheelVelocity);
		stopTask(flywheelControl);
		flywheelHold = true;
		} else if(!flywheelOn || flywheelHold) {		//Otherwise, we can
		startTask(flywheelVelocity);
		startTask(flywheelControl);
		flywheelHold = false;
	}
}

bool autoIntake = false;
//Starts the flywheel for regular shots
#warning "startAutoFlywheel"
void startAutoFlywheel (int targetVelocity) {
	startFlywheel(targetVelocity);							//NEEDS TESTING
	autoIntake = false;
}

//Slows the flywheel down without breaking the motors
#warning "stopFlywheel"
task stopFlywheel () {
	flywheelOn = false;
	autoIntake = false;
	stopTask(flywheelControl);
	while(motor[flywheel4]>0){
		motor[flywheel4] -= 1;
		delay(20);
	}
	stopTask(flywheelVelocity);
	stopTask(stopFlywheel);
}

//Revs flywheel for manual loaded balls
#warning "startManualFlywheel"
void startManualFlywheel () {
	startFlywheel(VELOCITY_LONG);
	autoIntake = true;
}

int ballIndexerLimit = 2000;
int waitTime = 300;
int velocityLimit = 900;
//controls the intake of the robot
#warning "intakeControl"
task intakeControl () {
	while(true) {
		motor[intake]=((tuneMode||autoIntake||vexRT[Btn5U])-vexRT[Btn5D])*127;

		if(vexRT(Btn5U)||(tuneMode||autoIntake)) {
			if(SensorValue[indexHigh]>ballIndexerLimit) {
				motor[indexer] = ((tuneMode||autoIntake||vexRT[Btn5U])-vexRT[Btn5D])*127;
			} else if ((vexRT(Btn6U) || autoIntake || tuneMode) && time1[T1]>waitTime) {
				motor[indexer] = ((tuneMode||autoIntake||vexRT[Btn5U])-vexRT[Btn5D])*127;
				delay(150);
				clearTimer(T1);
			} else {
				motor[indexer] = 0;
			}
		} else if(vexRT(Btn5D)) {
			motor[indexer] = ((tuneMode||autointake||vexRT[Btn5U])-vexRT[Btn5D])*127; //may want to add autoIntake to this line as well, in same way as above
		} else {
			motor[indexer] = 0;
		}
	}
	//if(SensorValue[indexHigh]>=ballIndexerLimit && (vexRT(Btn5U) || tuneMode)) {
	//	motor[indexer] = (tuneMode+vexRt[Btn5U]-vexRt[Btn5D])*127;
	//} else if(!vexRT(Btn5D) && time1[T1]<velocityTime && /*abs(currentGoalVelocity-currentVelocity)>velocityLimit &&*/ SensorValue[indexHigh]<ballIndexerLimit) {
	//	motor[indexer] = 0;
	//} else if((vexRT(Btn5U) && vexRT(Btn6U)) || tuneMode || autoIntake || vexRT(Btn5D)) {
	//	motor[indexer] = (tuneMode+vexRt[Btn5U]-vexRt[Btn5D])*127;
	//	if(time1[T1]>velocityTime+150) {
	//		clearTimer(T1);
	//	}
	//  } else {
	//	motor[indexer] = 0;
	//}
}

//Tests the tempermental encoder for issues before executing main code
#warning "testEncoder"
void testEncoder () {
	int recordedEncoderValue1, recordedEncoderValue2;
	SensorValue[encoderError] = 0;
	bool performsWell = false;
	if(nMotorEncoder(flywheel4)!=10000)
		performsWell = true;
	//startFlywheel(VELOCITY_LONG);
	//clearTimer(T3);
	//delay(1000);
	//while(time1[T3]<5000 && !performsWell) {
	//	recordedEncoderValue1 = nMotorEncoder[flywheel4];
	//	delay(50);
	//	recordedEncoderValue2 = nMotorEncoder[flywheel4];
	//	if(recordedEncoderValue1!=recordedEncoderValue2)
	//		performsWell = true;
	//	delay(50);
	//}
	if(!performsWell)
		SensorValue[encoderError] = 1;
	else
		repeat(5) {
		SensorValue[encoderError] = 1;
		delay(100);
		sensorValue[encoderError] = 0;
		delay(100);
	}
	startTask(stopFlywheel);
}

//Initialises driver control code
#warning "init"
void init() {
	//Slave Motors
	slaveMotor(flywheel2,flywheel4);
	slaveMotor(flywheel3,flywheel4);
	slaveMotor(flywheel1,flywheel4);

	//Startup modes
	if(!debugMode)
		debugMode = (bool) SensorValue[debug];
	if(!tuneMode)
		tuneMode = (bool) SensorValue[tune];
	if(!encoderTestMode)
		encoderTestMode = (bool) SensorValue[encoderTest];

	//Boot into test encoder mode
	if(encoderTestMode)
		testEncoder();
}

void pre_auton() {
	init();
	bStopTasksBetweenModes = true;
}

task autonomous() {
	//startTask(intakeControl);
	clearTimer(T2);
	motor[intake] = 127;
	motor[indexer] = 127;
	startManualFlywheel();
	while(true) {
		if(getMotorVelocity(flywheel4)>142)
			SensorValue[encoderError] = 1;
		else
			SensorValue[encoderError] = 0;
		delay(25);
	}
	//startTask(stopFlywheel);
	//setWheelSpeed(-127);
	//delay(1500);
	//setWheelSpeed(127,-127);
	//delay(400);
	//setWheelSpeed(-127);
	//startFlywheel(VELOCITY_LONG);
	//delay(1500);
	//setWheelSpeed(0);
}

task usercontrol() {

	//Start Tasks
	startTask(intakeControl);

	while (true) {
		lastUpButton=currentUpButton;
		lastDownButton=currentDownButton;
		currentUpButton = (bool)vexRT[Btn5U];
		currentDownButton = (bool)vexRT[Btn5D];

		if(vexRT(Btn8R))
			startAutoFlywheel(VELOCITY_PIPE);

		else if(vexRT(Btn8D))
			startAutoFlywheel(VELOCITY_HOLD);

		else if(vexRT(Btn8L))
			startAutoFlywheel(VELOCITY_LONG);

		else if(vexRT(Btn8U))
			startTask(stopFlywheel);

		else if(vexRT(Btn7D))
			startManualFlywheel();

		if(currentUpButton && !lastUpButton)
			currentGoalVelocity+=2;
		if(currentDownButton && !lastDownButton)
			currentGoalVelocity-=2;

		logDrive();
	}
}
