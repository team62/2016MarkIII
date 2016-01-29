#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in8,    indexHigh,      sensorLineFollower)
#pragma config(Sensor, dgtl1,  encoderError,   sensorLEDtoVCC)
#pragma config(Sensor, dgtl2,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl10, tune,           sensorTouch)
#pragma config(Sensor, dgtl11, debug,          sensorTouch)
#pragma config(Sensor, dgtl12, encoderTest,    sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           rightWheel2,   tmotorVex393TurboSpeed_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           flywheel4,     tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           rightWheel13,  tmotorVex393TurboSpeed_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port4,           flywheel3,     tmotorVex393HighSpeed_MC29, openLoop)
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
bool debugMode = true; //prints to console
bool encoderTestMode = false; //checks encoders at runtime

int autonomousChoice = 0;
//Stores the differient speeds for the velocity states of the robot
enum { VELOCITY_LONG = 790, VELOCITY_PIPE = 728, VELOCITY_HOLD = 30 }; //MAY NEED TO SWITCH BACK TO typedef and a name before the semicolon
//enum { VELOCITY_LONG = /*192*/160, VELOCITY_PIPE = 125, VELOCITY_HOLD = 30 };
//enum { VELOCITY_LONG = 18000, VELOCITY_PIPE = 125, VELOCITY_HOLD = 30 };

typedef struct {
	double kP;
	double kI;
	double kD;
	double target;
	double error;
	double integral;
	double derivative;
	double lastError;
	double threshold;
} pid;

int min(int num1, int num2) {
	if(num1>num2)
		return num2;
	else
		return num1;
}

int max(int num1, int num2) {
	if(num1>num2)
		return num1;
	else
		return num2;
}

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


void turn(int leftTarget, int rightTarget) {
	nMotorEncoder[leftWheel13] = 0;
	nMotorEncoder[rightWheel13] = 0;
	pid l;
	pid r;
	//double kP = 0.018;
	//double kI = 0.0002;
	//double kD = 0.01;

	double kP = 0.055;
	double kI = 0.0005;
	double kD = 0.5;
	double threshold = 20;

	l.threshold = threshold;
	r.threshold = threshold;

	l.kP = kP;
	r.kP = kP;

	l.kI = kI;
	r.kI = kI;

	l.kD = kD;
	r.kD = kD;

	l.target = leftTarget;
	r.target = rightTarget;

	do{
		l.error = l.target - nMotorEncoder[leftWheel13]; //add sensor
		r.error = r.target - nMotorEncoder[rightWheel13]; //same

		l.integral += l.error;
		r.integral += r.error;

		if(l.error == 0) { l.integral = 0; }
		if(r.error == 0) { r.integral = 0; }

		l.derivative = l.error - l.lastError;
		r.derivative = l.error - l.lastError;

		l.lastError = l.error;
		r.lastError = r.error;

		int leftOut = l.kP*l.error + l.kI*l.integral + l.kD*l.derivative;
		int rightOut = r.kP*r.error + r.kI*r.integral + r.kD*r.derivative;

		leftOut = leftOut>127?127:leftOut;
		rightOut = rightOut>127?127:rightOut;

		leftOut = leftOut<-127?-127:leftOut;
		rightOut = rightOut<-127?-127:rightOut;

		setLeftWheelSpeed(leftOut);
		setRightWheelSpeed(rightOut);
		delay(50);
	}	while(abs(l.error)>l.threshold  &&  abs(r.error)>r.threshold)
		setWheelSpeed(0);
}

void drive (int target) {
	turn(target,target);
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

	float kP=2.1//2.2;//was 1.675
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
		currentVelocity = getFlywheelVelocity();//might need work
		error = (currentGoalVelocity - currentVelocity);
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
task abi(){
	startTask(flywheelVelocity);
	int kP = 0.73;
	int speedA = 127;
	int speedB = 55;
	veloA = currentGoalVelocity;
	int motorSpeedA, motorSpeedB;
	while(true) {
		currVelo = getFlywheelVelocity();

		motorSpeedA = speedA + currVelo * kP;
		motorSpeedB = speedB + currVelo * kP;

		if(currVelo < veloA) {
			motor[flywheel4] = motorSpeedA;
			} else if(currVelo > veloA) {
			motor[flywheel4] = motorSpeedB;
		}
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
	setrpm = targetVelocity;
	currentGoalVelocity = targetVelocity;
	//startFlywheel(targetVelocity);							//NEEDS TESTING
	//startTask(drunkFlywheelControl);
	startTask(abi);
	autoIntake = false;
}

//Slows the flywheel down without breaking the motors
#warning "stopFlywheel"
task stopFlywheel () {
	flywheelOn = false;
	autoIntake = false;
	stopTask(flywheelControl);
	stopTask(drunkFlywheelControl);
	stopTask(abi);
	while(motor[flywheel4]>0){
		motor[flywheel4] -= 1;
		delay(15);
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
int waitTime = 0;
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
bool testEncoder () {
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
	return performsWell;
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

void clearLCD () {
	clearLCDLine(0);
	clearLCDLine(1);
}

/*
▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄
▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌
▐░█▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀█░▌
▐░▌                    ▐░▌
▐░█▄▄▄▄▄▄▄▄▄           ▐░▌
▐░░░░░░░░░░░▌ ▄▄▄▄▄▄▄▄▄█░▌
▐░█▀▀▀▀▀▀▀█░▌▐░░░░░░░░░░░▌
▐░▌       ▐░▌▐░█▀▀▀▀▀▀▀▀▀
▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄▄▄
▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌
▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀
*/
enum { MAIN_SCREEN = 0; BATT_SCREEN = 1; AUTON_SCREEN = 2; TEST_SCREEN = 3 };
int currentScreen = MAIN_SCREEN;
task LCD () {
	clearLCD();
	string lines[15];
	lines[0] = "";
	lines[1] = "";
	lines[2] = "▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄";
	lines[3] = "▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌";
	lines[4] = "▐░█▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀█░▌";
	lines[5] = "▐░▌                    ▐░▌";
	lines[6] = "▐░█▄▄▄▄▄▄▄▄▄           ▐░▌";
	lines[7] = "▐░░░░░░░░░░░▌ ▄▄▄▄▄▄▄▄▄█░▌";
	lines[8] = "▐░█▀▀▀▀▀▀▀█░▌▐░░░░░░░░░░░▌";
	lines[9] = "▐░▌       ▐░▌▐░█▀▀▀▀▀▀▀▀▀";
	lines[10] = "▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄▄▄";
	lines[11] = "▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌";
	lines[12] = "▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀";
	lines[13] = "";
	lines[14] = ""
	for(int i = 0; i<14; i++) {
		clearLCD();
		displayLCDString(0,3, lines[i]);
		displayLCDString(0,3, lines[i+1]);
		delay(100);
	}
	displayLCDCenteredString(1,"Batts    Auton    Test");
	delay(1000);
	displayLCDCenteredString(0)
	bLCDBacklight = true;
	while(true) {
		clearLCD();
		displayLCDCenteredString(0,"62 NBN Mark III");
		displayLCDString(1,0,"Batts    Auton    Test");
		waitForPress();
		if(nLCDButtons == leftButton) {
			waitForRelease();
			string batteryStatus;
			sprintf(batteryStatus,"Main: %1.2f%c V, Bkup: %1.2f%c V",nImmediateBatteryLevel/1000.0, BackupBatteryLevel/1000.0)
			displayLCDString(0, 0, batteryStatus);
			displayLCDString(1, 0, "Back    Refresh");
			waitForPress();
		} else if(nLCDButtons == rightButton) {
			waitForRelease();
			displayLCDCenteredString(0, "ENCODER TESTING");
			displayLCDCenteredString(1, "PLEASE WAIT");
			if(testEncoder()) {
				clearLCD();
				displayLCDCenteredString(0, "Encoder Test Pass");
			} else {
				clearLCD();
				displayLCDCenteredString(0, "Encoder Test Fail");
				displayLCDCenteredString(1, "*****************");
			}
			delay(3000);
		} else if(nLCDButtons == centerButton) {
			waitForRelease();
			int choice = 0;
			while(nLCDButtons != centerButton) {
				switch (choice) {
					case 0:
						clearLCD();
						displayLCDCenteredString(0, "Auton 1");
						displayLCDCenteredString(1, "<         Enter        >");
						waitForPress();
						if(nLCDButtons == leftButton){
							waitForRelease();
							count = 3;
						} else if(nLCDButtons == rightButton) {
							waitForRelease();
							count++;
						}
					break;
					case 1:
						clearLCD();
						displayLCDCenteredString(0, "Auton 2");
						displayLCDCenteredString(1, "<         Enter        >");
						waitForPress();
						if(nLCDButtons == leftButton){
							waitForRelease();
							count--;
						} else if(nLCDButtons == rightButton) {
							waitForRelease();
							count++;
						}
					break;
					case 3:
						clearLCD();
						displayLCDCenteredString(0, "Auton 3");
						displayLCDCenteredString(1, "<         Enter        >");
						waitForPress();
						//Increment or decrement "count" based on button press
						if(nLCDButtons == leftButton) {
							waitForRelease();
							count--;
						} else if(nLCDButtons == rightButton) {
							waitForRelease();
							count = 0;
						}
					break;
					default:
						count = 0;
					break;
				}
			}
			autonomousChoice = choice;
		}
	}
}

void pre_auton() {
	init();
	bStopTasksBetweenModes = true;
}

task autonomous() {
	//turn(100,-100);
	drive(1000);
	delay(1000)
	drive(-2000);
}

task usercontrol() {

	//Start Tasks
	startTask(intakeControl);

	while (true) {

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

		//motor[intake]=((tuneMode||autoIntake||vexRT[Btn5U])-vexRT[Btn5D])*127;
		//motor[indexer]=((tuneMode||autoIntake||vexRT[Btn5U])-vexRT[Btn5D])*127;

		logDrive();
	}
}
