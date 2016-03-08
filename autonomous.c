#warning "drivePID"

bool drivePID(int distance) {
	nMotorEncoder[leftWheel13] = 0;
	nMotorEncoder[rightWheel13] = 0;
	pid straight;
	pid angle;

	straight.kP = 0.04;
	straight.kI = 0.0001;
	straight.kD = 0.04;

	angle.kP = 0.02;//for
	angle.kI = 0.0000;
	angle.kD = 0//.15;

	int timeGuess = 3*abs(distance);//#magic number 5
	clearTimer(T2);
	angle.target=0;
	straight.target=distance;
	do{
		straight.error=straight.target -( nMotorEncoder[leftWheel13]+	nMotorEncoder[rightWheel13]);
		angle.error=(nMotorEncoder[leftWheel13]-nMotorEncoder[rightWheel13]); //target is 0 so lazy


		if(angle.error == 0) { angle.integral = 0; }
		if(straight.error == 0) { straight.integral = 0; }

		straight.derivative = straight.error - straight.lastError;
		angle.derivative = angle.error - angle.lastError;

		straight.lastError = straight.error;
		angle.lastError = angle.error;

		int StraightOut = straight.kP*straight.error + straight.kI*straight.integral + straight.kD*straight.derivative;
		int AngleOut = angle.kP*angle.error + angle.kI*angle.integral + angle.kD*angle.derivative;

		clearLCD();
		setLeftWheelSpeed(StraightOut+AngleOut);
		setRightWheelSpeed(StraightOut-AngleOut);
		delay(50);
		if(time1[T2]>timeGuess){ //if something went wrong give up
			setWheelSpeed(0);
			return false;
		}
	}	while(abs(straight.error)>30 || abs(straight.lastError)>30);
	setWheelSpeed(0);
	return true;
}

bool sLeftPID(int distance, int coefficient) {
	nMotorEncoder[leftWheel13] = 0;
	nMotorEncoder[rightWheel13] = 0;
	pid straight;
	pid angle;

	straight.kP = 0.04;
	straight.kI = 0.0001;
	straight.kD = 0.04;

	angle.kP = 0.02;//for
	angle.kI = 0.0000;
	angle.kD = 0//.15;

	int timeGuess = 3*abs(distance);//#magic number 5
	clearTimer(T2);
	angle.target=0;
	straight.target=distance;
	do{
		straight.error=straight.target -( nMotorEncoder[leftWheel13]+	nMotorEncoder[rightWheel13]);
		angle.error=(nMotorEncoder[leftWheel13]-nMotorEncoder[rightWheel13]); //target is 0 so lazy


		if(angle.error == 0) { angle.integral = 0; }
		if(straight.error == 0) { straight.integral = 0; }

		straight.derivative = straight.error - straight.lastError;
		angle.derivative = angle.error - angle.lastError;

		straight.lastError = straight.error;
		angle.lastError = angle.error;

		int StraightOut = straight.kP*straight.error + straight.kI*straight.integral + straight.kD*straight.derivative;
		int AngleOut = angle.kP*angle.error + angle.kI*angle.integral + angle.kD*angle.derivative - coefficient;

		clearLCD();
		setLeftWheelSpeed(StraightOut+AngleOut);
		setRightWheelSpeed(StraightOut-AngleOut);
		delay(50);
		if(time1[T2]>timeGuess){ //if something went wrong give up
			setWheelSpeed(0);
			return false;
		}
	}	while(abs(straight.error)>30 || abs(straight.lastError)>30);
	setWheelSpeed(0);
	return true;
}

bool sRightPID(int distance, int coefficient) {
	nMotorEncoder[leftWheel13] = 0;
	nMotorEncoder[rightWheel13] = 0;
	pid straight;
	pid angle;

	straight.kP = 0.04;
	straight.kI = 0.0001;
	straight.kD = 0.04;

	angle.kP = 0.02;//for
	angle.kI = 0.0000;
	angle.kD = 0//.15;

	int timeGuess = 3*abs(distance);//#magic number 5
	clearTimer(T2);
	angle.target=0;
	straight.target=distance;
	do{
		straight.error=straight.target -( nMotorEncoder[leftWheel13]+	nMotorEncoder[rightWheel13]);
		angle.error=(nMotorEncoder[leftWheel13]-nMotorEncoder[rightWheel13]); //target is 0 so lazy


		if(angle.error == 0) { angle.integral = 0; }
		if(straight.error == 0) { straight.integral = 0; }

		straight.derivative = straight.error - straight.lastError;
		angle.derivative = angle.error - angle.lastError;

		straight.lastError = straight.error;
		angle.lastError = angle.error;

		int StraightOut = straight.kP*straight.error + straight.kI*straight.integral + straight.kD*straight.derivative;
		int AngleOut = angle.kP*angle.error + angle.kI*angle.integral + angle.kD*angle.derivative + coefficient;

		clearLCD();
		setLeftWheelSpeed(StraightOut+AngleOut);
		setRightWheelSpeed(StraightOut-AngleOut);
		delay(50);
		if(time1[T2]>timeGuess){ //if something went wrong give up
			setWheelSpeed(0);
			return false;
		}
	}	while(abs(straight.error)>30 || abs(straight.lastError)>30);
	setWheelSpeed(0);
	return true;
}

#warning "turn"
bool turnPID(int distance) {
	nMotorEncoder[leftWheel13] = 0;
	nMotorEncoder[rightWheel13] = 0;
	pid angle;


	angle.kP = 0.2;//for
	angle.kI = 0.0000;
	angle.kD = 0.15;

	int timeGuess = 3*abs(distance);//#magic number 5
	clearTimer(T2);
	angle.target = distance;
	do{

	 angle.error=angle.target-(nMotorEncoder[leftWheel13]-nMotorEncoder[rightWheel13]); //target is 0 so lazy


		if(angle.error == 0) { angle.integral = 0; }


		angle.derivative = angle.error - angle.lastError;

		angle.lastError = angle.error;

		int AngleOut = angle.kP*angle.error + angle.kI*angle.integral + angle.kD*angle.derivative;

		clearLCD();
		setLeftWheelSpeed(AngleOut);
		setRightWheelSpeed(-AngleOut);
		delay(50);
		if(time1[T2]>timeGuess){ //if something went wrong give up
			setWheelSpeed(0);
			return false;
		}
	}	while(abs(angle.error)>30 || abs(angle.lastError)>30);
	setWheelSpeed(0);
	return true;

}
