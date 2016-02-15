#include "JonLib/LCD.h"

int tuneValue ( double value ) {
	string valuetext;
	float place = 0;
	while(true) {
		clearLCD();
		sprintf(valuetext, "%f", value);
		centerLine(0,valuetext);

		switch(place) {
			case -5: 	centerLine(1,"hund thths"); break;
			case -4: 	centerLine(1,"ten thths"); break;
			case -3: 	centerLine(1,"thousandths"); break;
			case -2: 	centerLine(1,"hundredths"); break;
			case -1: 	centerLine(1,"tenths"); break;
			case 0: 	centerLine(1,"ones"); break;
			case 1: 	centerLine(1,"tens"); break;
			case 2: 	centerLine(1,"hundreds"); break;
		}

		if(nLCDButtons == LCD_LEFT_BUTTON) {
			waitForRelease();
			value = value-pow(10,place);
		} else if (nLCDButtons == LCD_RIGHT_BUTTON) {
			waitForRelease();
			value = value+pow(10,place);
		} else if (nLCDButtons == LCD_CENTRE_BUTTON) {
			waitForRelease();
			if(place==2)
				place = -5;
			else
				place++;
		}
		else if(nLCDButtons==LCD_LEFT_RIGHT_BUTTON) {
			waitForRelease();
			writeDebugStream("kP: %f kI: %f kD: %f", gyroscope.kP, gyroscope.kI, gyroscope.kD);
			return(value);
		}

		delay(50);
	}
}

task LCD () {
	clearLCD();
	string currentValues;
	while(true) {
		sprintf(currentValues, "%f %f %f", gyroscope.kP, gyroscope.kI, gyroscope.kD);
		centerDisplay(currentValues, "kP  kI  kD");

		if(nLCDButtons == LCD_LEFT_BUTTON) {
			waitForRelease();
			gyroscope.kP = tuneValue(gyroscope.kP);
		}
		if(nLCDButtons == LCD_CENTRE_BUTTON) {
			waitForRelease();
			gyroscope.kI = tuneValue(gyroscope.kI);
		}
		if(nLCDButtons == LCD_RIGHT_BUTTON) {
			waitForRelease();
			gyroscope.kD = tuneValue(gyroscope.kD);
		}

		delay(60);
	}
}
