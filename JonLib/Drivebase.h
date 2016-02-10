void setLeftWheelSpeed ( int speed = 127 );
void setRightWheelSpeed ( int speed = 127 );

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

void spin ( int wheelSpeed = 127 ) {
	setWheelSpeed(-wheelSpeed, wheelSpeed);
}