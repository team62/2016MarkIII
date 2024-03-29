#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           flywheel1,     tmotorVex393HighSpeed_HBridge, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port2,           Intake,        tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           Indexer,       tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           flywheel2,     tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           flywheel3,     tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port10,          flywheel4,     tmotorVex393HighSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

bool lastUpButton=false;
bool lastDownButton=false;
bool currentUpButton;
bool currentDownButton;
int currentGoalVelocity=105;
int currentVelocity;

float error=0;
float integral=0;
int output;
int velocities[5];
task flywheelVelocity(){
	int nextIndex=0;
	while(true){
		velocities[nextIndex]=getMotorVelocity(flywheel1);
		nextIndex++;
		if(nextIndex==5)
			nextIndex=0;
		delay(5);
	}
}
int getFlywheelVelocity(){
	int sum=0;
	for(int i=0;i<5;i++)
		sum = sum + velocities[i];
	return sum/5;
}


task flywheelControl(){
	clearDebugStream();
	float kP=2.9;
	float kI=0.057;
	while(true){

		currentVelocity = getFlywheelVelocity();//might need work
		error = currentGoalVelocity - currentVelocity;
		integral = integral + error;
		if(integral>(100/kI))
			integral = 100/kI;
		output = error*kP + integral*kI;
		if(output >15){
			if(output>motor[flywheel1]+15){
				motor[flywheel1]=motor[flywheel1]+15;
				}else if(output<motor[flywheel1]-15){
				motor[flywheel1]=motor[flywheel1]-15;
				}else{

				motor[flywheel1]=output;
			}
			}else if(output<0){
			motor[flywheel1]=0;
			integral=0;
		}
		writeDebugStreamLine("Motors: %d, Error: %d, P: %d, I: %d", motor[flywheel1], error, error*kP, integral*kI);
		delay(50);
	}
}

task main()
{
	slaveMotor(flywheel2,flywheel1);
	slaveMotor(flywheel3,flywheel1);
	slaveMotor(flywheel4,flywheel1);
	startTask(flywheelVelocity);
	startTask(flywheelControl);



	while(true){
		lastUpButton=currentUpButton;
		lastDownButton=currentDownButton;
		currentUpButton = vexRT[Btn5U];
		currentDownButton= vexRT[Btn5D];

		if(currentUpButton && !lastUpButton)
			currentGoalVelocity+=2;
		if(currentDownButton && !lastDownButton)
			currentGoalVelocity-=2;



		motor[Intake]=(vexRt[Btn6U]-vexRt[Btn6D])*127;
		motor[Indexer]=(vexRt[Btn6U]-vexRt[Btn6D])*127;
		delay(50);

	}

}
