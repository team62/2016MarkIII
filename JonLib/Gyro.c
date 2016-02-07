#include "PID.c"
#include "Drivebase.h"
//#include "../MarkIIIPragma.c"

#pragma systemFile

pid gyroscope;


void resetGyro() {
	SensorValue[gyro] = 0;
}

void resetGyroscope () {
	resetGyro();
}

void setTarget ( int target = 0 ) {
	gyroscope.target = target;
}

task orient () {
	int speed;
	int lowestMovePower = 21;

	gyroscope.kP = 0.02;
	gyroscope.kI = 0.0003;
	gyroscope.kD = 0.005;

	do {
		//P
		gyroscope.error = gyroscope.target - SensorValue[gyro];

		//I
		if(gyroscope.error == 0)
			gyroscope.integral = 0;
		else if (abs(gyroscope.error)>10)
			gyroscope.integral = 0;
		else
			gyroscope.integral += gyroscope.error;

		//D
		gyroscope.derivative = gyroscope.error - gyroscope.lastError;

		speed = gyroscope.kP*gyroscope.error + gyroscope.kI*gyroscope.integral + gyroscope.kD*gyroscope.derivative;

		speed = abs(speed)<2?0:speed;
		speed = abs(speed)>127?speed/abs(speed)*127:speed; //fancy way of high deadbands
		speed = abs(speed)<lowestMovePower && abs(speed)>0?speed/abs(speed)*lowestMovePower:speed;

		spin(speed);

	} while(true);
}
