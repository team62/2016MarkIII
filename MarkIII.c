#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in2,    gyro,           sensorGyro)
#pragma config(Sensor, dgtl1,  encoderError,   sensorLEDtoVCC)
#pragma config(Sensor, dgtl2,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl4,  indexHigh,      sensorTouch)
#pragma config(Sensor, dgtl5,  indexLow,       sensorTouch)
#pragma config(Sensor, dgtl9,  encoderTest,    sensorTouch)
#pragma config(Sensor, dgtl10, tune,           sensorTouch)
#pragma config(Sensor, dgtl11, debug,          sensorTouch)
#pragma config(Sensor, dgtl12, upToSpeed,      sensorLEDtoVCC)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           rightWheel2,   tmotorVex393TurboSpeed_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           flywheel4,     tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port3,           rightWheel13,  tmotorVex393TurboSpeed_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port4,           flywheel3,     tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port5,           leftWheel2,    tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port6,           flywheel1,     tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           flywheel2,     tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           leftWheel13,   tmotorVex393TurboSpeed_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port9,           indexer,       tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port10,          intake,        tmotorVex393HighSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(0)
#pragma userControlDuration(60)
#pragma systemFile            	// eliminates warning for "unreferenced" functions
int autonomousChoice=0;

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!
#include "JonLib/PID.h"
#include "JonLib/Math.h"
#include "JonLib/Gyro.h"
#include "JonLib/Drivebase.h"
#include "LCD.c"
#include "autonomous.c"

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
///// Skills Code                              	        /////
///// Authors: Jonathan Damico, Griffin Tabor           /////
///// Since: Jan. 22, 2016                              /////
*////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
/// JUMPER CABLE CONFIGURATIONS                                   ///
/// dgtl9  = encoder test mode (checks encoder works at runtime)  ///
/// dgtl10 = tune mode (acts like you're holding down 5U and 6U)  ///
/// dgtl11 = debug mode (logs flywheel info to debug stream)      ///
/////////////////////////////////////////////////////////////////////
//DEBUG VARIABLES
bool tuneMode = false; //acts like you're holding 5U and 6U
bool debugMode = false; //prints to console
bool encoderTestMode = false; //checks encoders at runtime

int waitTime = 0;

//Stores the differient speeds for the velocity states of the robot
enum { VELOCITY_LONG = 810, VELOCITY_MID = 640, VELOCITY_PIPE = 530, VELOCITY_HOLD = 300 }; //MAY NEED TO SWITCH BACK TO typedef and a name before the semicolon
enum { HIGH_SPEED_LONG = 127, HIGH_SPEED_MID = 127, HIGH_SPEED_PIPE = 127, HIGH_SPEED_HOLD = 90 };
enum { LOW_SPEED_LONG = 60, LOW_SPEED_MID = 45, LOW_SPEED_PIPE = 40, LOW_SPEED_HOLD = 45 };
enum { WAIT_LONG = 400, WAIT_MID = 0, WAIT_PIPE = 0, WAIT_HOLD = 0 };


bool autonIntake = false;
bool autonIndex = false;
bool autonShoot = false;

//Sets the speed of wheels on the left side of the robot
#warning "setLeftWheelSpeed"
void setLeftWheelSpeed ( int speed ) {
	motor[leftWheel13] = speed;
	motor[leftWheel2] = speed;
}

//Sets the speed of the wheels on the right side of the robot
#warning "setRightWheelSpeed"
void setRightWheelSpeed ( int speed ) {
	motor[rightWheel13] = speed;
	motor[rightWheel2] = speed;
}

void clearEncoders () {
	nMotorEncoder(leftWheel13) = 0;
	nMotorEncoder(rightWheel13) = 0;
}

//Logarithmic drivebase control
#warning "logDrive"
void logDrive () {
	setWheelSpeed(
	abs(vexRT(Ch3))*vexRT(Ch3)/127,
	(abs(vexRT(Ch2))*vexRT(Ch2)/127)>100?100:abs(vexRT(Ch2))*vexRT(Ch2)/127);
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
float lastError=0;
float integral=0;
float derivative=0;
int output;
int velocities[5];

//Populates an array with the most recent velocities of the flywheel,
//used to calculate flywheel velocity
//TODO consider revising after 23/1/16
long lastdt=nSysTime;
#warning "flywheelVelocity"
task flywheelVelocity(){
	int nextIndex=0;
	while(true){
		long tme=nSysTime;
		velocities[nextIndex]=(((float)-SensorValue[flywheelEncoder])/360)/(((float)(tme-lastdt)==0?1:(float)(tme-lastdt)/(float)60)/1000);
		//velocities[nextIndex]=getMotorVelocity(flywheel4);
		SensorValue[flywheelEncoder]=0;
		nextIndex++;
		if(nextIndex==5)
			nextIndex=0;
		lastdt=tme;
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

	float kP=2.1;  //2.2;//was 1.675
	float kI=0.005;//1//07;//was 0.0025
	float kD=0.0;
	//float kP=3.0;//was 1.675
	//float kI=0.0;//1//07;//was 0.0025
	//float kD=0.0;

	//float kP=0.8001;//was 0.72
	//float kI=0.05532;
	int limit = 15;
	while(true){
		//	if(currentGoalVelocity==VELOCITY_PIPE)
		//		kP=2.05;//2.2 for NON AUTO - 1.5 for auto
		//	else
		//		kP=2.0;
		error = (currentGoalVelocity - getFlywheelVelocity());
		integral = integral + error;
		derivative = error-lastError;
		lastError=error;
		//if(integral>(100/kI))
		//	integral = 100/kI;
		output = error*kP + integral*kI+derivative*kD;
		if(output >25){
			if(output>motor[flywheel4]+limit){
				motor[flywheel4]=motor[flywheel4]+limit;
				}else if(output<motor[flywheel4]-limit){
				motor[flywheel4]=motor[flywheel4]-limit;
				}else{
				motor[flywheel4]=output;
			}
			}else if(output<20){
			motor[flywheel4]=20;
			//integral=0;
		}
		if(debugMode)
			writeDebugStreamLine("Motors: %d, Error: %d, P: %d, I: %d Integral: %d Derivative: %d", motor[flywheel1], error, error*kP, integral*kI, integral, derivative*kD);
		delay(50);
	}
}
int currVelo, veloA;
int speedA = 127;
int speedB = 55;
#warning "abi"
task abi() {
	float kP = 0.6;//.07; for mid/pipe
	motor[flywheel4] = 25;
	while(motor[flywheel4] < speedB+11) {
		motor[flywheel4]+=2;
		delay(40);
	}
	int motorSpeedA, motorSpeedB;
	while(true) {
		kP = 0.1;
		veloA = currentGoalVelocity;
		currVelo = getFlywheelVelocity();

		motorSpeedA = speedA + (veloA-currVelo) * kP;
		motorSpeedB = speedB + (veloA-currVelo) * kP;

		motorSpeedA = motorSpeedA>100?100:motorSpeedA;

		writeDebugStreamLine("%d, %d, %d",motorSpeedA, motorSpeedB, currVelo*kP);

		if(currVelo < (veloA==VELOCITY_LONG?veloA+50:veloA+50)) {
			motor[flywheel4] = motorSpeedA;
		} else {
			motor[flywheel4] = motorSpeedB;
		}
		clearLCDLine(0);
		displayLCDNumber(0,1,currVelo);
		displayLCDNumber(0,5,veloA);
		displayLCDNumber(0,10,motor[flywheel4]);
		delay(30);
	}
}

int rpm=0;
int setrpm=0;
float smooth=0;
int cpwr=0;
int btntoggle=0;
float fwgain=2;
int rpmoffset=30;
#warning "drunkFlywheelControl"
task drunkFlywheelControl() {
	while (true) {
		long tme=nSysTime;
		rpm=(((float)-SensorValue[flywheelEncoder])/360)/(((float)(tme-lastdt)==0?1:(float)(tme-lastdt)/(float)60)/1000);
		SensorValue[flywheelEncoder]=0;
		//rpm = getMotorVelocity(flywheelEncoder);
		int ipwr;
		if (setrpm==0) {
			ipwr=0;
			} else {
			//ipwr=min(max(((setrpm-rpm)*500)+(setrpm==0?0:32),0),127);
			ipwr=min(max((((setrpm+rpmoffset)-rpm)*1000)+((setrpm+rpmoffset)==0?0:32),0),127);
		}
		motor[flywheel4]=ipwr;
		lastdt=tme;
		wait1Msec(10);
	}
}

bool flywheelHold = false;
//Starts the flywheel at a target velocity
#warning "startFlywheel"
void startFlywheel (int targetVelocity) {
	currentGoalVelocity = targetVelocity;
	if(targetVelocity == (int) VELOCITY_HOLD) {	//If we are holding the motors,
		motor[flywheel4] = VELOCITY_HOLD;					//we don't want to startup the PID
		stopTask(flywheelControl);
		flywheelHold = true;
		} else if(!flywheelOn || flywheelHold) {		//Otherwise, we can
		startTask(flywheelControl);
		flywheelHold = false;
	}
}

bool autoIntake = false;
//Starts the flywheel for regular shots
#warning "startAutoFlywheel"
void startAutoFlywheel (int targetVelocity) {
	setrpm = targetVelocity;
	currentGoalVelocity = targetVelocity;
	while(getFlywheelVelocity()<-30) { delay(50); }
	//startFlywheel(targetVelocity);							//NEEDS TESTING
	//startTask(drunkFlywheelControl);
	startTask(abi,kHighPriority);
	autoIntake = false;
}

void startAutoFlywheel (int targetVelocity, int highSpeed, int lowSpeed) {
	speedA = highSpeed;
	speedB = lowSpeed;
	startAutoFlywheel(targetVelocity);
}

void startAutoFlywheel (int targetVelocity, int highSpeed, int lowSpeed, int waitTimeIn) {
	waitTime = waitTimeIn;
	startAutoFlywheel(targetVelocity, highSpeed, lowSpeed);
}

//Slows the flywheel down without breaking the motors
#warning "stopFlywheel"
task stopFlywheel () {
	flywheelOn = false;
	autoIntake = false;
	stopTask(flywheelControl);
	stopTask(drunkFlywheelControl);
	stopTask(abi);
	//while(motor[flywheel4]>0){
	//	motor[flywheel4] -= 2;
	//	delay(15);
	//}
	motor[flywheel4] = 0;
	stopTask(stopFlywheel);
}

//Revs flywheel for manual loaded balls
#warning "startManualFlywheel"
void startManualFlywheel () {
	startAutoFlywheel(VELOCITY_LONG, HIGH_SPEED_LONG, LOW_SPEED_LONG);
	autoIntake = true;
}

int ballIndexerLimit = 2700;
int velocityLimit = 900;
int indexerSpeed = 127;
//controls the intake of the robot
#warning "intakeControl"
task intakeControl () {
	while(true) {
		string speed;
		sprintf(speed, "%d", indexerSpeed);
		line(1,speed);
		while(true) {
			motor[intake] = ((vexRT(Btn5U)||autonIndex)-vexRT(Btn5D))*127;

			while (vexRT(Btn5U) || autonIntake) {
				if(vexRT(Btn6U) || autonShoot) {
					//if(sensorValue[indexHigh] && getFlywheelVelocity()<currentGoalVelocity+30) {
					if(SensorValue[indexHigh]) {
						while(time1[T1]<=waitTime) {
							motor[indexer] = -7;
							delay(25);
						}
						if(getFlywheelVelocity()>0) {
							motor[indexer] = 127;
							while(SensorValue[indexHigh] && (vexRT(Btn6U) || autonShoot)) { delay(5); }
							clearTimer(T1);
						}
						else {
							motor[indexer] = -7;
						}
					}
					else {
						motor[indexer] = 127;
					}
					delay(50);
				} else if(vexRT(Btn6D)) {
					motor[indexer] = -127;
					delay(250);
				} else if(SensorValue[indexLow] || SensorValue[indexHigh]) {
					motor[indexer] = -7;
				} else {
					motor[indexer] = (vexRT(Btn5U)-vexRT(Btn5D))*127;
				}
				delay(25);
			}
			motor[indexer] = vexRT(Btn5D)?-127:0;
			delay(25);
		}
	}
}

task spazIntake () {
	while(true) {
		motor[intake] = 127;
		wait1Msec(500);
		motor[intake] = -20;
		wait1Msec(500);
	}
}


//Tests the tempermental encoder for issues before executing main code
#warning "testEncoder"
bool testEncoder () {
	playSound(soundException);
	clearLCD();
	displayLCDCenteredString(0,"LIFT");
	displayLCDCenteredString(1,"ROBOT");
	delay(1000);
	SensorValue[encoderError] = 0;
	bool performsWell = true;

	//Flywheel
	clearLCD();
	displayLCDCenteredString(0,"Encoder Test");
	int initValue = SensorValue[flywheelEncoder];
	startAutoFlywheel(VELOCITY_LONG);
	delay(1000);

	if(SensorValue[flywheelEncoder]==initValue) {
		performsWell = false;
		displayLCDCenteredString(1,"Failed");
	} else {
		displayLCDCenteredString(1,"Passed");
	}
	startTask(stopFlywheel);

	//Drivebase
	delay(1000);
	clearLCD();
	displayLCDCenteredString(0,"Drivebase Test");
	int initWheelValues[2];
	initWheelValues[0] = nMotorEncoder(leftWheel13);
	initWheelValues[1] = nMotorEncoder(rightWheel13);
	setWheelSpeed();
	delay(2000);
	if(initWheelValues[0]==nMotorEncoder(leftWheel13)) {
		performsWell = false;
		displayLCDCenteredString(1,"Left Failed");
	} else if(initWheelValues[1]==nMotorEncoder(rightWheel13)) {
		performsWell = false;
		displayLCDCenteredString(1,"Right Failed");
	} else {
		displayLCDCenteredString(1,"Passed");
	}
	setWheelSpeed(0);

	return performsWell;
}

bool alarm = false;
task lowBattery() {
	alarm = true;
	while(true) {
    playTone(700, 1); delay(1000);
    playTone(400, 1); delay(1000);
    playTone(200, 1); delay(1000);
    delay(1000);
	}
}

task autonAlign () {
	clearTimer(T3);
	int target = -360;
	while(true) {
		if(nMotorEncoder(rightWheel13)>target)
			setRightWheelSpeed(-30);
		else if (nMotorencoder(rightWheel13)<target)
			setRightWheelSpeed(30);
		else
			setRightWheelSpeed(0);
		delay(25);
	}
}

#warning "reverseFlywheel"
void reverseFlywheel() {
	if(vexRT(Btn7L)) {
		if(getFlywheelVelocity()>10) {
			startTask(stopFlywheel);
			while(VexRT(Btn7L) && getFlywheelVelocity()>10) { delay(25); }
		} else {
			motor[flywheel4] = -127;
			while(vexRT(Btn7L)) { delay(25); }
			while(motor[flywheel4]<0) { motor[flywheel4]+=2; delay(25); }
			motor[flywheel4] = 0;
		}
	}
}

#warning "init"
void init() {
	playTone(700,10);
	startTask(LCD);
	startTask(flywheelVelocity);

	setBaudRate(UART1, baudRate57600);

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

	bool autonIntake = false;
	bool autonIndex = false;
	bool autonShoot = false;
}

void pre_auton() {
	init();
	bStopTasksBetweenModes = true;
}

//task log() {
//	while(true) {
//		string output;
//		sprintf(output,"E%d,%d\n",straight.error, angle.error);
//		bnsSerialSend(UART1, output);
//		delay(50);
//	}
//}

task autonomousIntake () {
	while(true) {
		if(!SensorValue[indexHigh])
			motor[indexer] = 127;
		else
			motor[indexer] = -7;
		motor[intake] = 127;
		delay(50);
	}
}

void autonomous0Red() {
	startTask(autonomousIntake);
	setWheelSpeed(75,100);
	wait1Msec(800);
	setWheelSpeed(100,75);
	wait1Msec(800);
	setWheelSpeed(80);
	wait1Msec(500);//distance to pipe
	setWheelSpeed(0);
	delay(800);
	drivePID(-450);
	delay(200);
	turnPID(360);
	stopTask(autonomousIntake);
	motor[intake] = 0;
	motor[indexer] = 0;
	delay(100);
	drivePID(1250);
	delay(100);
	turnPID(320);
	motor[indexer] = -127;
	motor[intake] = -127;
	drivePID(2000);
}

void autonomous0Blue() {
	startTask(autonomousIntake);
	setWheelSpeed(100,80);
	wait1Msec(800);
	setWheelSpeed(80,100);
	wait1Msec(800);
	setWheelSpeed(80);
	wait1Msec(500);//distance to pipe
	setWheelSpeed(0);
	delay(800);
	drivePID(-450);
	delay(200);
	turnPID(-380);
	stopTask(autonomousIntake);
	motor[intake] = 0;
	motor[indexer] = 0;
	delay(100);
	drivePID(1250);
	delay(100);
	turnPID(-375);
	motor[indexer] = -127;
	motor[intake] = -127;
	drivePID(2000);
}

void autonomous1 () {
	startTask(flywheelVelocity);
	startAutoFlywheel(VELOCITY_LONG, HIGH_SPEED_LONG, LOW_SPEED_LONG, WAIT_LONG);
	autonShoot = false;
	autonIndex = true;
	autonIntake = true;
	startTask(intakeControl);
	delay(6000);
	autonShoot = true;
	delay(7000);
	autonShoot = false;
	startTask(stopFlywheel);
}

void autonomous1Blue() {
	autonomous1();
}

void autnomous1Red() {
	autonomous1();
}

void autonomous2Red () {
	autonIndex = true;
	autonShoot = true;
	autonIntake = true;
	startTask(intakeControl);
	setWheelSpeed(75,100);
	wait1Msec(800);
	setWheelSpeed(100,75);
	wait1Msec(800);
	setWheelSpeed(0);
	drivePID(-1000);
}

void autonomous2Red () {
	autonIndex = true;
	autonShoot = true;
	autonIntake = true;
	startTask(intakeControl);
	setWheelSpeed(100,80);
	wait1Msec(800);
	setWheelSpeed(80,100);
	wait1Msec(800);
	setWheelSpeed(0);
	wait1Msec(700);
	drivePID(-1000);
}

/**
*	autonomous 0 - u shaped s curve ram balls into our goal and then turn around and shove some to us
* autonomous 1 - 4 ball auton
* autonomous 2 - get 2 close stax
**/
task autonomous () {
	autonomous0Blue();
}

task usercontrol() {

	startTask(intakeControl);

	while (true) {

		if(vexRT(Btn8U))
			startAutoFlywheel(VELOCITY_PIPE, HIGH_SPEED_PIPE, LOW_SPEED_PIPE, WAIT_PIPE);

		else if(vexRT(Btn8R))
			startAutoFlywheel(VELOCITY_MID, HIGH_SPEED_MID, LOW_SPEED_MID, WAIT_MID);

		else if(vexRT(Btn8L))
			startAutoFlywheel(VELOCITY_LONG, HIGH_SPEED_LONG, LOW_SPEED_LONG, WAIT_LONG);

		else if(vexRT(Btn7D))
			startAutoFlywheel(VELOCITY_HOLD, HIGH_SPEED_HOLD, LOW_SPEED_HOLD, WAIT_HOLD);

		else if(vexRT(Btn8D))
			startTask(stopFlywheel);

		if(vexRT(Btn7U))
			SensorValue[gyro] = 0;

		if(vexRT(Btn6D))
			startTask(orient);
		else {
			stopTask(orient);

		logDrive();
		reverseFlywheel();
		if(motor[flywheel4]>90)
			SensorValue[upToSpeed] = 1;
		else
			SensorValue[upToSpeed] = 0;
		delay(50);
		}
	}
}
