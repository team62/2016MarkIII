#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in8,    indexHigh,      sensorLineFollower)
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

typedef enum { VELOCITY_LONG = 175, VELOCITY_MID = 155, VELOCITY_PIPE = 130 } flywheelVelocities;

#warning "setLeftWheelSpeed"
void setLeftWheelSpeed ( int speed = 127 ) {
	motor[leftWheel13] = speed;
	motor[leftWheel2] = speed;
}

#warning "setRightWheelSpeed"
void setRightWheelSpeed ( int speed = 127 ) {
	motor[rightWheel13] = speed;
	motor[rightWheel2] = speed;
}

#warning "setWheelSpeed"
void setWheelSpeed ( int leftWheelSpeed = 127, int rightWheelSpeed = 127 ) {
	setLeftWheelSpeed(leftWheelSpeed);
	setRightWheelSpeed(rightWheelSpeed);
}

#warning "logDrive"
void logDrive () {
	int rawLeft = vexRT(Ch3);
	int rawRight = vexRT(Ch2);

	setWheelSpeed(rawLeft*rawLeft/127,rawRight*rawRight/127);
}

bool lastUpButton=false;
bool lastDownButton=false;
bool currentUpButton;
bool currentDownButton;
int currentGoalVelocity=VELOCITY_LONG;
int currentVelocity;

float error=0;
float integral=0;
int output;
int velocities[5];
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

#warning "getFlywheelVelocity"
int getFlywheelVelocity(){
	int sum=0;
	for(int i=0;i<5;i++)
		sum = sum + velocities[i];
	return sum/5;
}

#warning "flywheelControl"
task flywheelControl(){
	clearDebugStream();

	float kP=1.1;
	float kI=0.05736;
	int limit = 15;
	while(true){

		currentVelocity = getFlywheelVelocity();//might need work
		error = (currentGoalVelocity - currentVelocity);
		integral = integral + error;
		if(integral>(100/kI))
			integral = 100/kI;
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
		writeDebugStreamLine("Motors: %d, Error: %d, P: %d, I: %d Integral: %d", motor[flywheel1], error, error*kP, integral*kI, integral);
		delay(80);
	}
}

#warning "init"
void init() {
	slaveMotor(flywheel2,flywheel4);
	slaveMotor(flywheel3,flywheel4);
	slaveMotor(flywheel1,flywheel4);
	startTask(flywheelVelocity);
	startTask(flywheelControl);
}

void pre_auton() {
	bStopTasksBetweenModes = true;
}

task autonomous() {
	AutonomousCodePlaceholderForTesting();  // Remove this function call once you have "real" code.
}

task usercontrol() {
	init();

	while (true) {
		lastUpButton=currentUpButton;
		lastDownButton=currentDownButton;
		currentUpButton = vexRT[Btn5U];
		currentDownButton = vexRT[Btn5D];

		if(currentUpButton && !lastUpButton)
			currentGoalVelocity+=2;
		if(currentDownButton && !lastDownButton)
			currentGoalVelocity-=2;

		//TODO needs calib for sensor high
		//TODO comment everything
		motor[Intake]=(vexRt[Btn6U]-vexRt[Btn6D])*127;
		motor[Indexer]=(vexRt[Btn6U]-vexRt[Btn6D])*127;

		logDrive();
	}
}