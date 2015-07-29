/*
  ShiftDisplay 2.2.0 (14/07/2015)
  https://github.com/Pyntoo/ShiftDisplay
*/


#include "Arduino.h"
#include "ShiftDisplayMini.h"
#include "ShiftDisplayConstants.h"


// CONSTRUCTORS ********************************************************************

// Creates lightweight ShiftDisplay object using 3 pins.
// latchPin, clockPin and dataPin are the shift register inputs connected to
// the Arduino digital outputs.
// commonCathode is true if the led type is common cathode, false if it's common anode.
// displayLength is the quantity of digits of all displays together, a maximum of 8.
ShiftDisplayMini::ShiftDisplayMini(int latchPin, int clockPin, int dataPin, bool commonCathode, int displayLength) {
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	_latchPin = latchPin;
	_clockPin = clockPin;
	_dataPin = dataPin;
	_commonCathode = commonCathode;
	_displayLength = displayLength;
}


// PRIVATE FUNCTIONS ***************************************************************

// Calculate power of number by exponent.
int ShiftDisplayMini::power(int value, int exponent) {
	return round(pow(value, exponent));
}

// Clear display.
void ShiftDisplayMini::clear() {
	digitalWrite(_latchPin, LOW);
	shiftOut(_dataPin, _clockPin, MSBFIRST, 0); // both ends of led with same value
	shiftOut(_dataPin, _clockPin, MSBFIRST, 0);
	digitalWrite(_latchPin, HIGH);
}

// Display byte array.
// Pre: characters array length = display number of digits
void ShiftDisplayMini::show(byte characters[], int time) {
	unsigned long start = millis();
	while (millis() - start < time) {
		for (int i = 0; i < _displayLength; i++) {
			digitalWrite(_latchPin, LOW);

			// shift data for display register
			byte out = _commonCathode ? ~DISPLAYS[i] : DISPLAYS[i];
			shiftOut(_dataPin, _clockPin, LSBFIRST, out);

			// shift data for character register
			shiftOut(_dataPin, _clockPin, MSBFIRST, characters[i]);

			digitalWrite(_latchPin, HIGH);
			delay(POV);
		}
	}
	clear();
}

// PUBLIC FUNCTIONS ****************************************************************

// Show an integer value, right aligned in the display,
// for the given time in milliseconds.
void ShiftDisplayMini::print(int value, int time) {
	bool negative = value < 0;
	byte characters[_displayLength];
	int i = _displayLength - 1;

	// tranform number into positive
	if (negative)
		value = value * -1;

	// store digits from number in array
	do { // if number is zero, prints single 0
		int digit = value % 10;
		characters[i--] = _commonCathode ? DIGITS[digit]: ~DIGITS[digit];
		value /= 10;
	} while (value && i >= 0);

	// place minus character on left of number
	if (negative && i >= 0)
		characters[i--] = _commonCathode ? MINUS : ~MINUS;

	// fill remaining array with empty
	while (i >= 0)
		characters[i--] = _commonCathode ? BLANK : ~BLANK;

	show(characters, time);
}

// Show a float value, rounded to specified decimal places,
// right aligned in the display, for the given time in milliseconds.
void ShiftDisplayMini::print(float value, int decimalPlaces, int time) {

	// if no decimal places, print int
	if (decimalPlaces == 0) {
		int n = round(value);
		print(n, time);
		return;
	}

	bool negative = value < 0;
	byte characters[_displayLength];
	int i = _displayLength - 1;

	// transform number in positive
	if (negative)
		value = value * -1;

	// get digits rounded without comma from value
	long newValue = round(value * power(10, decimalPlaces));

	// store digits in array
	while ((newValue || i > decimalPlaces) && i >= 0) {
		int digit = newValue % 10;
		characters[i--] = _commonCathode ? DIGITS[digit] : ~DIGITS[digit];
		newValue /= 10;
	}

	// place decimal point in unity value
	if (decimalPlaces < _displayLength) {
		int unity = _displayLength - decimalPlaces - 1;
		characters[unity] += DOT;
	}

	// place minus character on left of numbers
	if (negative && i >= 0)
		characters[i--] = _commonCathode ? MINUS : ~MINUS;

	// fill remaining characters with empty
	while (i >= 0)
		characters[i--] = _commonCathode ? BLANK : ~BLANK;

	show(characters, time);
}

// Show text, left aligned in the display, for the given time in milliseconds.
// Accepted characters are A-Z, a-z, 0-9, -, space.
void ShiftDisplayMini::print(String text, int time) {
	byte characters[_displayLength];
	int i = 0;

	// get characters from text
	while (i < text.length() && i < _displayLength) {
		char c = text[i];

		byte out;
		if (c >= 'A' && c <= 'Z')
			out = LETTERS[c - 'A'];
		else if (c >= 'a' && c <= 'z')
			out = LETTERS[c - 'a'];
		else if (c >= '0' && c <= '9')
			out = DIGITS[c - '0'];
		else if (c == '-')
			out = MINUS;
		else
			out = BLANK;
		characters[i++] = _commonCathode ? out : ~out;
	}

	// fill remaining right characters with empty
	while (i < _displayLength)
		characters[i++] = _commonCathode ? BLANK : ~BLANK;

	show(characters, time);
}
