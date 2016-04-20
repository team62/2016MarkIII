void resetEncoders() {
	nMotorEncoder(leftWheel13) = 0;
	nMotorEncoder(rightWheel13) = 0;
}

int autonomous2 = midShot-10;

void autonomousInit() {
	//failsafe - ensure robot is properly initialsied
  init();

  //clear the encoders before start of autonomous
	resetEncoders();


  //clear drivebase PID targets to prevent robot from running off anywhere
	l.target = 0;
	r.target = 0;

  //ensure intake won't do anything stupid
	intakeAutonomousIndexer = false;
	intakeAutonomousIntake = false;
	intakeAutonomousShoot = false;

	startTask(drivebasePID);
}

//Shoots four balls into net, can be called from other autonomous methods
void fourBalls() {
	startTask(intakeControl);
	startFlywheel(flywheelLongShot, flywheelLongPredictedDrive);
  //wait a bit to aviod shooting ball prematurely - flywheel can have tendancy to briefly load balls before flywheel has started spinning, jamming flywheel
	wait1Msec(1000);
	intakeAutonomousIndexer = true;
	intakeAutonomousIntake = true;
	intakeAutonomousShoot = true;

  //enough time to shoot ball
  wait1Msec(5000);

	intakeAutonomousIndexer = false;
	intakeAutonomousIntake = false;
	intakeAutonomousShoot = false;
	stopFlywheel();
}

//smack into bar and pipe shot
void rSCurveAuto () {
	autonomousInit();
	startFlywheel(flywheelPipeShot, 0.0); //start to spin up flywheel as we drive across
	addTarget(800,800,90); //begin to drive forward
	wait1Msec(900);

  //S Curve
	addTargetNoIntegral(600,1000,100);
	wait1Msec(800);
	addTargetNoIntegral(1000,600,100);
	intakeAutonomousIntake = true; //start spinning intake
	wait1Msec(800);
	addTargetNoIntegral(1200, 1200, 30); //finish driving straight
	wait1Msec(500);

  //shoot the balls
	intakeAutonomousIndexer = true;
	intakeAutonomousShoot = true;
	wait1Msec(500);
	addTarget(0); //stop the drivebase
	wait1Msec(2000);

  //return towards protected zone
	resetEncoders(); //for some reason, this helps
	addTarget(0,-450); //swing turn, align paralell with pipe
	stopFlywheel(); //we don't need the flywheel anymore

  //stop shooting
	intakeAutonomousIntake = false;
	intakeAutonomousIndexer = false;
	intakeAutonomousShoot = false;

	wait1Msec(2000);
	addTarget(0);
	wait1Msec(200);
	addTarget(500);
	wait1Msec(1500);
	addTarget(800,-0); //swing turn, align to push stacks towards our protected zone
	wait1Msec(700);

  //spit out any balls
	stopTask(intakeControl); //stop intake control task so that the intake can be controlled directly
	motor[intake] = -127;
	motor[indexer] = -127;

	addTarget(800, 800, 90); //drive back towards protected zone to push stax
	wait1Msec(1300);
	addTarget(0);
	wait1Msec(500);
	addTarget(-800); //reverse back to starting position
	wait1Msec(1300);

  //shutdown, stop movement
	stopTask(drivebasePID);
	setWheelSpeed(0);

	startTask(intakeControl); //failsafe - ensure that when usercontrol starts, we aren't without intake
}

//mirrors rSCurveAuto()
void lSCurveAuto () {
	autonomousInit();
	startFlywheel(flywheelPipeShot, 0.0);
	addTarget(800,800,90);
	wait1Msec(900);
	addTargetNoIntegral(1000,600,70);
	wait1Msec(800);
	addTargetNoIntegral(600,1000,70);
	intakeAutonomousIntake = true;
	wait1Msec(800);
	addTargetNoIntegral(1200, 1200, 30);
	wait1Msec(700);
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
	addTarget(450);
	wait1Msec(1500);
	addTarget(0,850);
	wait1Msec(700);
	stopTask(intakeControl);
	motor[intake] = -127;
	motor[indexer] = -127;
	addTarget(800, 800, 90);
	wait1Msec(1300);
	addTarget(0);
	wait1Msec(500);
	addTarget(-800);
	wait1Msec(1300);
	stopTask(drivebasePID);
	setWheelSpeed(0);
	startTask(intakeControl);
}

//get stack, shoot, go across field
void rAngleShotAuto () {
	autonomousInit();
	startFlywheel(autonomous2); //start flywheel to custom shot for angle

	addTargetNoIntegral(1000, 1000, 60); //drive forward to stack towards the bottom of the "box"
	intakeAutonomousIntake = true; //intake stax
	wait1Msec(2000);

	addTarget(0,380); //swing turn to align with goal
	wait1Msec(600);

  //shoot stack
  intakeAutonomousShoot = true;
	intakeAutonomousIndexer = true;
	addTarget(0);
	wait1Msec(2000); //amt of time to shoot four balls
	intakeAutonomousShoot = false;
	intakeAutonomousIndexer = false;
	stopFlywheel(); //dont waste

	addTarget(-250,250); //point turn to get contested stack on opponent's side, near the side wall
	wait1Msec(1000);
	addTarget(1100,1100); //drive to stack
	wait1Msec(2000);
	addTarget(500, 500, 20); //drive slowly to properly intake stack
	wait1Msec(3000);

  //stop movement
	addTarget(0);
	setWheelSpeed(0);
	intakeAutonomousIndexer = false;
	intakeAutonomousShoot = false;
	intakeAutonomousIntake = false;
}

//mirrors rAngleShotAuto()
void lAngleShotAuto () {
	autonomousInit();
	startFlywheel(autonomous2);

	addTargetNoIntegral(1000, 1000, 60);
	intakeAutonomousIntake = true;
	wait1Msec(2000);
	addTarget(380,0);
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
	addTargetNoIntegral(500, 520, 20);
	wait1Msec(3000);
	addTarget(0);
	setWheelSpeed(0);
	intakeAutonomousIndexer = false;
	intakeAutonomousShoot = false;
	intakeAutonomousIntake = false;
}

//shoot four and then go in front of other protected zone
void rFourCross () {
	autonomousInit();

	fourBalls(); //shoot four balls

	addTarget(0,265); //swing turn to go in front of opponent's protected zone
	wait1Msec(1000);
	intakeAutonomousIntake = true; //intake on our way
	addTarget(1500);
	wait1Msec(2000);

  //stop movement
	addTarget(0);
	setWheelSpeed(0);
}

//shoot four and then go in front of other protected zone
//mirrors rFourCross
void lFourCross () {
	autonomousInit();
	fourBalls();
	startTask(drivebasePID);
	addTarget(265,0);
	wait1Msec(1000);
	intakeAutonomousIntake = true;
	addTarget(1500);
	wait1Msec(2000);
	addTarget(0);
	setWheelSpeed(0);
}
