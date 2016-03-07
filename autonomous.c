#warning "drivePID"
bool drivePID(int distance) {
	nMotorEncoder[leftWheel13] = 0;
	nMotorEncoder[rightWheel13] = 0;
	pid straight;
	pid angle;

	straight.kP = 0.3;
	straight.kI = 0.00;
	straight.kD = 0.5;

	angle.kP = 0.5;//for
	angle.kI =0;
	angle.kD =0;

	int timeGuess = 5*distance;//#magic number 5
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
		setLeftWheelSpeed(StraightOut-AngleOut);
		setRightWheelSpeed(StraightOut+AngleOut);
		delay(50);
		if(timer1[T2]>timeGuess){ //if something went wrong give up
			setWheelSpeed(0);
			return false;
		}
	}	while(abs(straight.error)>30);
	return true;
}


#warning "turn"
bool turnPID(int distance) {
	nMotorEncoder[leftWheel13] = 0;
	nMotorEncoder[rightWheel13] = 0;
	pid angle;


	angle.kP = 0.5;//for
	angle.kp = 0.00;
	angle.kp= 0.3;

	int timeGuess = 5*distance;//#magic number 5
	clearTimer(T2);
	angle.target=0;
	do{

	 angle.error=(nMotorEncoder[leftWheel13]-nMotorEncoder[rightWheel13]); //target is 0 so lazy


		if(angle.error == 0) { angle.integral = 0; }


		angle.derivative = angle.error - angle.lastError;

		angle.lastError = angle.error;

		int AngleOut = angle.kP*angle.error + angle.kI*angle.integral + angle.kD*angle.derivative;

		clearLCD();
		setLeftWheelSpeed(-AngleOut);
		setRightWheelSpeed(+AngleOut);
		delay(50);
		if(timer1[T2]>timeGuess){ //if something went wrong give up
			setWheelSpeed(0);
			return false;
		}
	}	while(abs(straight.error)>30);
return true;

}
