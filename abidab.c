#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    indexLow,       sensorNone)
#pragma config(Sensor, in2,    powerExpander,  sensorAnalog)
#pragma config(Sensor, dgtl1,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  indexHigh,      sensorTouch)
#pragma config(Sensor, dgtl6,  flywheelForward, sensorTouch)
#pragma config(Sensor, dgtl9,  upToSpeed,      sensorLEDtoVCC)
#pragma config(Sensor, dgtl10, encoderTest,    sensorTouch)
#pragma config(Sensor, dgtl11, tune,           sensorTouch)
#pragma config(Sensor, dgtl12, debug,          sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           rightWheel2,   tmotorVex393TurboSpeed_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           leftWheel13,   tmotorVex393TurboSpeed_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port3,           indexer,       tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port4,           flywheel1,     tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port5,           flywheel3,     tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           leftWheel2,    tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           rightWheel13,  tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port8,           flywheel4,     tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port9,           flywheel2,     tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port10,          intake,        tmotorVex393HighSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(0)
#pragma userControlDuration(60)
#pragma systemFile            	// eliminates warning for "unreferenced" functions

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!


bool debugMode = true;
bool debugDrivebaseActive = false;
bool debugFlywheelActive = false;

//Stores the differient speeds for the velocity states of the robot
typedef struct {
	int velocity;
	int highSpeed;
	int lowSpeed;
	int ramp;
	int wait;
	float kP;
} flywheelShot;

flywheelShot longShot, midShot, pipeShot, holdShot;
flywheelShot currentShot;

void flywheelShots() {
	longShot.velocity = 4600;
	longShot.highSpeed = 100;
	longShot.lowSpeed = 38;
	longShot.ramp = 0;
	longShot.wait = 400;
	longShot.kP = 0.03;

	midShot.velocity = 3700;
	midShot.highSpeed = 100;
	midShot.lowSpeed = 55;
	midShot.ramp = 0;
	midShot.wait = 0;
	midShot.kP = 0.07;

	pipeShot.velocity = 3400;
	pipeShot.highSpeed = 100;
	pipeShot.lowSpeed = 45;
	pipeShot.ramp = 0;
	pipeShot.wait = 0;
	pipeShot.kP = 0.07;

	holdShot.velocity = 300;
	holdShot.highSpeed = 90;
	holdShot.lowSpeed = 45;
	holdShot.ramp = 45;
	holdShot.wait = 0;
}

int flywheelVelocity;
int flywheelReverseStartThreshold = 10;
int flywheelSlowDownPower = -5;
int flywheelReverseEncoderTicks = 15000;
bool flywheelReverseEngaged = false;

int intakeMoveUpTime = 200;
int intakeMoveDownTime = 250;
int intakeShootVelocityThreshold = 50;
int intakeLightThreshold = 1300;
bool intakeAutonomousIntake;
bool intakeAutonomousIndexer;
bool intakeAutonomousShoot;

int autonomousChoice;

#include "JonLib/Drivebase.h"
#include "JonLib/PID.h"
#include "autonomous.c"

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

#warning "setFlywheel"
void setFlywheel ( int speed ) {
	motor[flywheel1] = speed;
	motor[flywheel2] = speed;
	motor[flywheel3] = speed;
	motor[flywheel4] = speed;
}

//Logarithmic drivebase control
#warning "logDrive"
void logDrive () {
	setWheelSpeed(
	abs(vexRT(Ch3))*vexRT(Ch3)/127,
	(abs(vexRT(Ch2))*vexRT(Ch2)/127)>100?100:abs(vexRT(Ch2))*vexRT(Ch2)/127);
}

#warning "flywheelVelocityCalculation"
task flywheelVelocityCalculation() {
	int LastValue; int CurrentValue; int Error;
	while(true){
   LastValue = SensorValue(flywheelEncoder);
   wait1Msec(20);
   CurrentValue = SensorValue(flywheelEncoder);
   Error = CurrentValue - LastValue;
   SensorValue(flywheelEncoder) = 0;
   flywheelVelocity = -(Error*15000)/359;
 }
}

void flywheelLCD () {
	clearLCDLine(0);
	displayLCDNumber(0,1,flywheelVelocity);
	displayLCDNumber(0,5,currentShot.velocity);
	displayLCDNumber(0,10,motor[flywheel4]);
}

void flywheelLED() {
	if(motor[flywheel4]>90)
		SensorValue[upToSpeed] = 1;
	else
		SensorValue[upToSpeed] = 0;
}

void flywheelRampUp (int target) {
	while(motor[flywheel4] < target) {
		motor[flywheel4]+=2;
		motor[flywheel3]+=2;
		motor[flywheel2]+=2;
		motor[flywheel1]+=2;
		wait1Msec(35);
	}
}

task jonflywheelControl () {
	float kP = 0.02;

	setFlywheel(20);

	flywheelRampUp (currentShot.lowSpeed);

	int flywheelSpeed;

	while(true) {
		flywheelSpeed = motor[flywheel4] + (currentShot.velocity-flywheelVelocity)*kP;

		clearLCDLine(0);
		displayLCDNumber(0,1,flywheelSpeed);
		displayLCDNumber(0,5,flywheelVelocity);
		displayLCDNumber(0,10,(currentShot.velocity-flywheelVelocity)*kP);

		flywheelSpeed = flywheelSpeed>100?100:flywheelSpeed;
		flywheelSpeed = flywheelSpeed<0?0:flywheelSpeed;

		setFlywheel(flywheelSpeed);
	}
}

#warning "flywheelControl"
task flywheelControl() {

	setFlywheel(20);

	flywheelRampUp (currentShot.lowSpeed);

	int flywheelSpeedA, flywheelSpeedB;

	while(true) {

		flywheelSpeedA = currentShot.highSpeed + (currentShot.velocity-flywheelVelocity) * currentShot.kP;
		flywheelSpeedB = currentShot.lowSpeed + (currentShot.velocity-flywheelVelocity) * currentShot.kP;

		flywheelSpeedA = flywheelSpeedA>100?100:flywheelSpeedA;
		flywheelSpeedB = flywheelSpeedB>100?100:flywheelSpeedB;

		flywheelSpeedA = flywheelSpeedA<0?0:flywheelSpeedA;
		flywheelSpeedB = flywheelSpeedB<0?0:flywheelSpeedB;

		if(flywheelVelocity < currentShot.velocity+currentShot.ramp) {
			setFlywheel(flywheelSpeedA);
		} else {
			setFlywheel(flywheelSpeedB);
		}

		if(debugMode) {
			flywheelLCD();
			flywheelLED();
		}

		delay(25);
	}
}

#warning "startFlywheel"
void startFlywheel (flywheelShot shot) {
	currentShot.velocity = shot.velocity;
	currentShot.highSpeed = shot.highSpeed;
	currentShot.lowSpeed = shot.lowSpeed;
	currentShot.ramp = shot.ramp;
	currentShot.wait = shot.wait;
	currentShot.kP = shot.kP;
	if(flywheelVelocity >= 0)
		startTask(flywheelControl, kHighPriority);
	else
		setFlywheel(0);
}

void startFlywheel (int targetVelocity, int lowSpeed, int highSpeed, int rampThreshold, int waitTime = 0, float kP = 0.07) {
	flywheelShot tempShot;
	tempShot.velocity = targetVelocity;
	tempShot.lowSpeed = lowSpeed;
	tempShot.highSpeed = highSpeed;
	tempShot.ramp = rampThreshold;
	tempShot.wait = waitTime;
	tempShot.kP = kP;
	startFlywheel(tempShot);
}

#warning "stopFlywheel"
void stopFlywheel () {
	stopTask(flywheelControl);
	setFlywheel(0);
}

#warning "intakeControl"
task intakeControl () {
	while(true) {
		motor[intake] = ((vexRT(Btn5U)||intakeAutonomousIntake)-vexRT(Btn5D))*127;

		//Move ball from high limit switch to low limit switch
		if(vexRT(Btn6D) && SensorValue[indexHigh]) {
			motor[indexer] = -127;
			delay(intakeMoveDownTime);
		}

		//Shooting control
		if (vexRT(Btn6U) || intakeAutonomousShoot) {
			if(flywheelVelocity>intakeShootVelocityThreshold && time1[T1]>currentShot.wait) {
				motor[indexer] = 80;
				while(SensorValue[indexHigh] && (vexRT(Btn6U)||intakeAutonomousShoot)) { delay(5); }
				clearTimer(T1);
			}
			else {
				motor[indexer] = (SensorValue[indexHigh])?0:80;
			}
		}

		//Move ball down even if there is a sensor we want
		else if (vexRT(Btn5D))
			motor[indexer] = -127;

		//Stop ball if ball is at a sensor
		else if(SensorValue[indexLow]<intakeLightThreshold && !SensorValue[indexHigh]) {
			motor[indexer] = 70;
			clearTimer(T2);
			while(time1[T2] < intakeMoveUpTime && !SensorValue[indexHigh]) { delay(20); }
			motor[indexer] = 0;
		}

		else
			motor[indexer] = 0;

		delay(10);
	}
}

task reverseFlywheel () {
	while(true) {
		if(vexRT(Btn7L) && flywheelVelocity > flywheelReverseStartThreshold) {
			stopFlywheel();
			setFlywheel(flywheelSlowDownPower);
			delay(25);
		}
		else {
			while(vexRT(Btn7L)) {
				stopFlywheel();
				setFlywheel(-127);
				delay(25);
			}
			if(flywheelVelocity < 0 && !debugFlywheelActive) {
				setFlywheel(0);
			}
		}
		delay(25);
	}
}

#include "LCD.c"

#warning "init"
void init() {
	playTone(700,10);

	setBaudRate(UART1, baudRate57600);

	//Slave Motors
	//slaveMotor(flywheel2,flywheel4);
	//slaveMotor(flywheel3,flywheel4);
	//slaveMotor(flywheel1,flywheel4);

	//Startup modes
	if(!debugMode)
		debugMode = (bool) SensorValue[debug];

	intakeAutonomousIndexer = false;
	intakeAutonomousIntake = false;
	intakeAutonomousIndexer = false;

	flywheelShots();

	startTask(intakeControl);
	startTask(flywheelVelocityCalculation);
	startTask(reverseFlywheel);

	startTask(LCD);
}

void pre_auton()
{
	bStopTasksBetweenModes = true;
}

task autonomous()
{
	drivePID(1000);
}

task usercontrol() {

	init();

	while (true) {

		if(!debugDrivebaseActive)
			logDrive();

		if(vexRT(Btn8U)) {
			startFlywheel(pipeShot);
			while(vexRT(Btn8U)) { delay(10); }
		}

		else if(vexRT(Btn8R)) {
			startFlywheel(midShot);
			while(vexRT(Btn8R)) { delay(10); }
		}

		else if(vexRT(Btn8L)) {
			startFlywheel(longShot);
			while(vexRT(Btn8L)) { delay(10); }
		}

		else if(vexRT(Btn7D)) {
			//startFlywheel(holdShot);
			startFlywheel(200, 30, 60, 10, 0);
			while(vexRT(Btn7D)) { delay(10); }
		}

		// for debugging - consider removing
		else if(vexRT(Btn7U))
			setFlywheel(60);

		else if(vexRT(Btn8D))
			stopFlywheel();
	}
}
