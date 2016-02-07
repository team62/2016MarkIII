#include "PID.c"
#include "Drivebase.h"
#include "../MarkIIIPragma.c"

#pragma systemFile

pid gyroscope;

int getGyroValue ();

void resetGyro() {
	SensorValue[gyro] = 0;
}

void resetGyroscope () {
	resetGyro();
}


task orient ();

void setTarget ( int target = 0 ) {
	gyroscope.target = target;
}
