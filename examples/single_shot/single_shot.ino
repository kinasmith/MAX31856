#include <SPI.h>
#include "MAX31856.h"

int Tc_cs = 5;

MAX31856 tc(Tc_cs, T_TYPE, CUTOFF_60HZ, AVG_SEL_16SAMP, CMODE_OFF, ONESHOT_ON); //one shot mode
// MAX31856 tc(Tc_cs, T_TYPE, CUTOFF_60HZ, AVG_SEL_1SAMP, CMODE_AUTO, ONESHOT_OFF); //constant conversion mode

void setup() {
	Serial.begin(9600);
}

void loop() {
	tc.prime(); //write registers for single shot read
	tc.read();
	// Print temperature in the serial port, checking for errors
	if (tc.hasError()) {
		printErrors();
	} else {
		Serial.print("Internal: ");
		Serial.print(tc.getInternal());
		Serial.print(" | External: ");
		Serial.print(tc.getExternal());
	}
	Serial.println();
	// Wait for next reading For One Shot Mode, it must be greater than 150ms.
	delay(1000); 
}

void printErrors() {
	if (tc.isOpen()) {
		Serial.print("Open circuit");
	} else if (tc.isOverUnderVoltage()) {
		Serial.print("Overvoltage/Undervoltage");
	} else if (tc.isInternalOutOfRange()) {
		Serial.print("Internal temperature (cold junction) out of range)");
	} else if (tc.isExternalOutOfRange()) {
		Serial.print("External temperature (hot junction) out of range");
	}
}