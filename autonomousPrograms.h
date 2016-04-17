void resetEncoders() {
	nMotorEncoder(leftWheel13) = 0;
	nMotorEncoder(rightWheel13) = 0;
}

void autonomousInit() {
	init();

	resetEncoders();

	l.target = 0;
	r.target = 0;

	intakeAutonomousIndexer = false;
	intakeAutonomousIntake = false;
	intakeAutonomousShoot = false;

	startTask(drivebasePID);
}

//smack into bar and pipe shot
void rightAutonomous1 () {
	autonomousInit();
	startFlywheel(pipeShot);
	addTarget(800,800,90);
	wait1Msec(900);
	addTargetNoIntegral(600,1000,100);
	wait1Msec(800);
	addTargetNoIntegral(1000,600,100);
	intakeAutonomousIntake = true;
	wait1Msec(800);
	addTargetNoIntegral(1200, 1200, 30);
	wait1Msec(500);
	intakeAutonomousIndexer = true;
	intakeAutonomousShoot = true;
	wait1Msec(500);
	addTarget(0);
	wait1Msec(2000);

	resetEncoders();
	addTarget(0,-450);
	stopFlywheel();
	intakeAutonomousIntake = false;
	intakeAutonomousIndexer = false;
	intakeAutonomousShoot = false;
	wait1Msec(2000);
	addTarget(0);
	wait1Msec(200);
	addTarget(400);
	wait1Msec(1500);
	addTarget(800,-0);
	wait1Msec(700);
	stopTask(intakeControl);
	motor[intake] = -127;
	motor[indexer] = -127;
	addTarget(900, 900, 90);
	wait1Msec(1300);
	stopTask(drivebasePID);
	setWheelSpeed(0);
	startTask(intakeControl);
}

void leftAutonomous1 () {
	autonomousInit();
	startFlywheel(pipeShot);
	addTarget(800,800,90);
	wait1Msec(900);
	addTargetNoIntegral(1000,600,100);
	wait1Msec(800);
	addTargetNoIntegral(600,1000,100);
	intakeAutonomousIntake = true;
	wait1Msec(800);
	addTargetNoIntegral(1200, 1200, 30);
	wait1Msec(500);
	intakeAutonomousIndexer = true;
	intakeAutonomousShoot = true;
	wait1Msec(500);
	addTarget(0);
	wait1Msec(2000);

	resetEncoders();
	addTarget(-500,0);
	stopFlywheel();
	intakeAutonomousIntake = false;
	intakeAutonomousIndexer = false;
	intakeAutonomousShoot = false;
	wait1Msec(2000);
	addTarget(0);
	wait1Msec(200);
	addTarget(400);
	wait1Msec(1500);
	addTarget(0,800);
	wait1Msec(700);
	stopTask(intakeControl);
	motor[intake] = -127;
	motor[indexer] = -127;
	addTarget(900, 900, 90);
	wait1Msec(1300);
	stopTask(drivebasePID);
	setWheelSpeed(0);
	startTask(intakeControl);
}

void rightAutonomous2 () {
	autonomousInit();
	startFlywheel(midShot);

	addTargetNoIntegral(1000, 1000, 70);
	intakeAutonomousIntake = true;
	wait1Msec(2000);
	addTarget(0,400);
	wait1Msec(600);
	intakeAutonomousShoot = true;
	intakeAutonomousIndexer = true;
	addTarget(0);
	wait1Msec(2000);
	intakeAutonomousShoot = false;
	intakeAutonomousIndexer = false;
	stopFlywheel();
	addTarget(-250,250);
	wait1Msec(1000);
	addTarget(1100,1100);
	wait1Msec(2000);
	addTarget(500, 500, 20);
	wait1Msec(3000);
	addTarget(0);
	setWheelSpeed(0);
	intakeAutonomousIndexer = false;
	intakeAutonomousShoot = false;
	intakeAutonomousIntake = false;
}

void leftAutonomous2 () {
	autonomousInit();
	startFlywheel(midShot);

	addTargetNoIntegral(1000, 1000, 70);
	intakeAutonomousIntake = true;
	wait1Msec(2000);
	addTarget(400,0);
	wait1Msec(600);
	intakeAutonomousShoot = true;
	intakeAutonomousIndexer = true;
	addTarget(0);
	wait1Msec(2000);
	intakeAutonomousShoot = false;
	intakeAutonomousIndexer = false;
	stopFlywheel();
	addTarget(250,-250);
	wait1Msec(1000);
	addTarget(1100,1100);
	wait1Msec(2000);
	addTarget(500, 500, 20);
	wait1Msec(3000);
	addTarget(0);
	setWheelSpeed(0);
	intakeAutonomousIndexer = false;
	intakeAutonomousShoot = false;
	intakeAutonomousIntake = false;
}
