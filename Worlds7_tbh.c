#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in7,    indexLow,       sensorAnalog)
#pragma config(Sensor, in8,    powerExpander,  sensorAnalog)
#pragma config(Sensor, dgtl1,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  indexHigh,      sensorTouch)
#pragma config(Sensor, dgtl9,  upToSpeed,      sensorLEDtoVCC)
#pragma config(Sensor, dgtl10, encoderTest,    sensorTouch)
#pragma config(Sensor, dgtl11, tune,           sensorTouch)
#pragma config(Sensor, dgtl12, debug,          sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           flywheel3,     tmotorVex393TurboSpeed_HBridge, openLoop)
#pragma config(Motor,  port2,           indexer,       tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port3,           flywheel2,     tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           flywheel1,     tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port5,           rightWheel2,   tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           leftWheel13,   tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port7,           flywheel4,     tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           leftWheel2,    tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port9,           rightWheel13,  tmotorVex393TurboSpeed_MC29, openLoop, reversed, encoderPort, I2C_1)
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

int flywheelVelocity;
int flywheelVelocityUpdateFrequency = 25;
int flywheelReverseStartThreshold = 10;
int flywheelSlowDownVelocity = 4000;
int flywheelControlUpdateFrequency = flywheelVelocityUpdateFrequency;

int intakeMoveUpTime = 200;
int intakeMoveDownTime = 250;
int intakeShootVelocityThreshold = 50;
int intakeLightThreshold = 2500; //higher is more sensitive
bool intakeAutonomousIntake;
bool intakeAutonomousIndexer;
bool intakeAutonomousShoot;

int autonomousChoice = -1;

#include "JonLib/Drivebase.h"
#include "JonLib/PID.h"
#include "autonomous.h"

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

void setFlywheel (int speed) {
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

// Update inteval (in mS) for the flywheel control loop
#define FW_LOOP_SPEED              25

// Maximum power we want to send to the flywheel motors
#define FW_MAX_POWER              100

// encoder counts per revolution depending on motor
#define MOTOR_TPR_269           240.448
#define MOTOR_TPR_393R          261.333
#define MOTOR_TPR_393S          392
#define MOTOR_TPR_393T          627.2
#define MOTOR_TPR_QUAD          360.0

// Structure to gather all the flywheel ralated data
typedef struct _fw_controller {
    long            counter;                ///< loop counter used for debug

    // encoder tick per revolution
    float           ticks_per_rev;          ///< encoder ticks per revolution

    // Encoder
    long            e_current;              ///< current encoder count
    long            e_last;                 ///< current encoder count

    // velocity measurement
    float           v_current;              ///< current velocity in rpm
    long            v_time;                 ///< Time of last velocity calculation

    // TBH control algorithm variables
    long            target;                 ///< target velocity
    long            current;                ///< current velocity
    long            last;                   ///< last velocity
    float           error;                  ///< error between actual and target velocities
    float           last_error;             ///< error last time update called
    float           gain;                   ///< gain
    float           drive;                  ///< final drive out of TBH (0.0 to 1.0)
    float           drive_at_zero;          ///< drive at last zero crossing
    long            first_cross;            ///< flag indicating first zero crossing
    float           drive_approx;           ///< estimated open loop drive

    // final motor drive
    long            motor_drive;            ///< final motor control value
    } fw_controller;

// Make the controller global for easy debugging
static  fw_controller   flywheel;

/*-----------------------------------------------------------------------------*/
/** @brief      Set the controller position                                    */
/** @param[in]  fw pointer to flywheel controller structure                    */
/** @param[in]  desired velocity                                               */
/** @param[in]  predicted_drive estimated open loop motor drive                */
/*-----------------------------------------------------------------------------*/
void
FwVelocitySet( fw_controller *fw, int velocity, float predicted_drive )
{
    // set target velocity (motor rpm)
    fw->target        = velocity;

    // Set error so zero crossing is correctly detected
    fw->error         = fw->target - fw->current;
    fw->last_error    = fw->error;

    // Set predicted open loop drive value
    fw->drive_approx  = predicted_drive;
    // Set flag to detect first zero crossing
    fw->first_cross   = 1;
    // clear tbh variable
    fw->drive_at_zero = 0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate the current flywheel motor velocity                  */
/** @param[in]  fw pointer to flywheel controller structure                    */
/*-----------------------------------------------------------------------------*/
void
FwCalculateSpeed( fw_controller *fw )
{
    int     delta_ms;
    int     delta_enc;

    // Get current encoder value
    fw->e_current = SensorValue[flywheelEncoder];

    // This is just used so we don't need to know how often we are called
    // how many mS since we were last here
    delta_ms   = nSysTime - fw->v_time;
    fw->v_time = nSysTime;

    // Change in encoder count
    delta_enc = (fw->e_current - fw->e_last);

    // save last position
    fw->e_last = fw->e_current;

    // Calculate velocity in rpm
    fw->v_current = (1000.0 / delta_ms) * delta_enc * 60.0 / fw->ticks_per_rev;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update the velocity tbh controller variables                   */
/** @param[in]  fw pointer to flywheel controller structure                    */
/*-----------------------------------------------------------------------------*/
void
FwControlUpdateVelocityTbh( fw_controller *fw )
{
    // calculate error in velocity
    // target is desired velocity
    // current is measured velocity
    fw->error = fw->target - fw->current;

    // Use Kp as gain
    fw->drive =  fw->drive + (fw->error * fw->gain);

    // Clip - we are only going forwards
    if( fw->drive > 1 )
          fw->drive = 1;
    if( fw->drive < 0 )
          fw->drive = 0;

    // Check for zero crossing
    if( sgn(fw->error) != sgn(fw->last_error) ) {
        // First zero crossing after a new set velocity command
        if( fw->first_cross ) {
            // Set drive to the open loop approximation
            fw->drive = fw->drive_approx;
            fw->first_cross = 0;
        }
        else
            fw->drive = 0.5 * ( fw->drive + fw->drive_at_zero );

        // Save this drive value in the "tbh" variable
        fw->drive_at_zero = fw->drive;
    }

    // Save last error
    fw->last_error = fw->error;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Task to control the velocity of the flywheel                    */
/*-----------------------------------------------------------------------------*/
task
FwControlTask()
{
    fw_controller *fw = &flywheel;

    // Set the gain
    fw->gain = 0.00025;

    // We are using Speed geared motors
    // Set the encoder ticks per revolution
    fw->ticks_per_rev = MOTOR_TPR_393S;

    while(1)
        {
        // debug counter
        fw->counter++;

        // Calculate velocity
        FwCalculateSpeed( fw );

        // Set current speed for the tbh calculation code
        fw->current = fw->v_current;

        // Do the velocity TBH calculations
        FwControlUpdateVelocityTbh( fw ) ;

        // Scale drive into the range the motors need
        fw->motor_drive  = (fw->drive * FW_MAX_POWER) + 0.5;

        // Final Limit of motor values - don't really need this
        if( fw->motor_drive >  127 ) fw->motor_drive =  127;
        if( fw->motor_drive < -127 ) fw->motor_drive = -127;

        // and finally set the motor control value
        setFlywheel( fw->motor_drive );

        // Run at somewhere between 20 and 50mS
        wait1Msec( FW_LOOP_SPEED );
        }
}

#warning "stopFlywheel"
void stopFlywheel () {
	stopTask(FwControlTask);
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
			if(true) {//(currentShot.velocityShot?abs(currentShot.velocity-flywheelVelocity)<currentShot.velocityThreshold:true) && flywheelVelocity>intakeShootVelocityThreshold && time1[T1]>currentShot.wait) {
				writeDebugStreamLine("%d", flywheelVelocity);
				motor[indexer] = 127;
				while(SensorValue[indexHigh] && (vexRT(Btn6U)||intakeAutonomousShoot)) { delay(25); }
				clearTimer(T1);
			}
			else {
				motor[indexer] = (SensorValue[indexHigh])?0:127;
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

		delay(25);
	}
}

task reverseFlywheel () {
	while(true) {
		if(vexRT(Btn7L) && flywheelVelocity > flywheelReverseStartThreshold) {
			stopFlywheel();
			clearLCDLine(1);
			while(flywheelVelocity>0) {
				setFlywheel(flywheelVelocity>flywheelSlowDownVelocity?0:-pow(abs((flywheelVelocity/1000)-flywheelSlowDownVelocity/1000),1.3));
				clearLCDLine(0);
				displayLCDNumber(0,0,flywheelVelocity);
				displayLCDNumber(0,10,motor[flywheel1]);
				delay(25);
			}
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

#warning "init"
void init() {
	playTone(700,10);

	clearDebugStream();

	intakeAutonomousIndexer = false;
	intakeAutonomousIntake = false;
	intakeAutonomousShoot = false;

	//Startup modes
	if(!debugMode)
		debugMode = (bool) SensorValue[debug];

	intakeAutonomousIndexer = false;
	intakeAutonomousIntake = false;
	intakeAutonomousIndexer = false;

	flywheelShots();

	startTask(intakeControl, kHighPriority);
	startTask(reverseFlywheel);

	startTask(FwControlTask);
	FwVelocitySet( &flywheel, 0, 0 );
}

void pre_auton() {
	init();
	bStopTasksBetweenModes = true;
}

//#include "autonomousPrograms.h"
task autonomous() {
	/*switch (autonomousChoice) {
		case 0: fourBalls();			break;
		case 1:	rSCurveAuto();		break;
		case 2:	rAngleShotAuto();	break;
		case 3: rFourCross(); 		break;
		case 4: lSCurveAuto();		break;
		case 5:	lAngleShotAuto(); break;
		case 6: lFourCross();			break;
	}
	*/
}

task usercontrol() {


	//init();

	while (true) {
		motor[flywheel1] = 55;
		motor[flywheel2] = 55;
		motor[flywheel3] = 55;
		motor[flywheel4] = 55;
				if(vexRT(Btn6U)){
			motor[intake] = 100;
			motor[indexer] = 100;
		}

		if(!debugDrivebaseActive)
			logDrive();

		if(vexRT(Btn8U)){
			FwVelocitySet( &flywheel, 50, 0.2 );
			while(vexRT(Btn8U)) { delay(10); }
		}
		else if(vexRT(Btn8R)) {
			FwVelocitySet( &flywheel, 120, 0.38 );
			while(vexRT(Btn8R)) { delay(10); }
		}

		else if(vexRT(Btn8L)) {
			FwVelocitySet( &flywheel, 144, 0.55 );
			while(vexRT(Btn8L)) { delay(10); }
		}

		// for debugging - consider removing
		else if(vexRT(Btn7U))
			setFlywheel(60);

		else if(vexRT(Btn8D))
			stopFlywheel();
	}
}
