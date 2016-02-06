#warning "clearLCD"
void clearLCD () {
	clearLCDLine(0);
	clearLCDLine(1);
}

#warning "waitForRelease"
void waitForRelease () {
	while(nLCDButtons != 0)
		delay(25);
}

#warning "waitForPress"
void waitForPress () {
	while (nLCDButtons == 0)
		delay(25);
}
