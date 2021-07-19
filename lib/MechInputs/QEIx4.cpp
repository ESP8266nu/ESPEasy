/*-------------------------------------------------------------------------
  Arduino library to ...

  Written by Jochen Krapf,
  contributions by ... and other members of the open
  source community.

  -------------------------------------------------------------------------
  This file is part of the MechInputs library.

  MechInputs is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  MechInputs is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with MechInputs.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

#include "QEIx4.h"
#include <limits.h>


#ifndef CORE_POST_3_0_0
  #ifdef ESP8266
    #define IRAM_ATTR ICACHE_RAM_ATTR
  #endif
#endif


// bit masks for state machine - don't change!!!
#define QEIx4_STATE   0xC
#define QEIx4_MASK    0x1C
#define QEIx4_1x_INC  0x0100
#define QEIx4_2x_INC  0x0200
#define QEIx4_4x_INC  0x0400
#define QEIx4_1x_DEC  0x1000
#define QEIx4_2x_DEC  0x2000
#define QEIx4_4x_DEC  0x4000
#define QEIx4_1x_MASK 0x11FF
#define QEIx4_2x_MASK 0x33FF
#define QEIx4_4x_MASK 0x77FF
#define QEIx4_DIR     0x20
#define QEIx4_A       1
#define QEIx4_B       2
#define QEIx4_AB      3
#define QEIx4_S0      0x0
#define QEIx4_S1      0x4
#define QEIx4_S2      0x8
#define QEIx4_S3      0xC
#define QEIx4_CCW     0
#define QEIx4_CW      0x10
#define QEIx4_IS_CHG  0x7700
#define QEIx4_IS_INC  0x0700
#define QEIx4_IS_DEC  0x7000
#define QEIx4_IS_DIR  0x20

// state machine for decoting - don't change!!!
uint16_t QEIx4::__stateLUT[32] = {
	// act state S0 in CCW direction
	QEIx4_CCW | QEIx4_S0,
	QEIx4_CW  | QEIx4_S1 | QEIx4_A  | QEIx4_4x_INC | QEIx4_DIR,
	QEIx4_CCW | QEIx4_S0 | QEIx4_B,
	QEIx4_CCW | QEIx4_S3 | QEIx4_AB | QEIx4_1x_DEC,
	// act state S1 in CCW direction
	QEIx4_CCW | QEIx4_S1,
	QEIx4_CCW | QEIx4_S1 | QEIx4_A,
	QEIx4_CCW | QEIx4_S0 | QEIx4_B  | QEIx4_4x_DEC,
	QEIx4_CW  | QEIx4_S2 | QEIx4_AB | QEIx4_1x_INC | QEIx4_DIR,
	// act state S2 in CCW direction
	QEIx4_CCW | QEIx4_S1 |            QEIx4_2x_DEC,
	QEIx4_CCW | QEIx4_S2 | QEIx4_A,
	QEIx4_CW  | QEIx4_S3 | QEIx4_B  | QEIx4_4x_INC | QEIx4_DIR,
	QEIx4_CCW | QEIx4_S2 | QEIx4_AB,
	// act state S3 in CCW direction
	QEIx4_CW  | QEIx4_S0 |            QEIx4_2x_INC | QEIx4_DIR,
	QEIx4_CCW | QEIx4_S2 | QEIx4_A  | QEIx4_4x_DEC,
	QEIx4_CCW | QEIx4_S3 | QEIx4_B,
	QEIx4_CCW | QEIx4_S3 | QEIx4_AB,

	// act state S0 in CW direction
	QEIx4_CW  | QEIx4_S0,
	QEIx4_CW  | QEIx4_S1 | QEIx4_A  | QEIx4_4x_INC,
	QEIx4_CW  | QEIx4_S0 | QEIx4_B,
	QEIx4_CCW | QEIx4_S3 | QEIx4_AB | QEIx4_1x_DEC | QEIx4_DIR,
	// act state S1 in CW direction
	QEIx4_CW  | QEIx4_S1,
	QEIx4_CW  | QEIx4_S1 | QEIx4_A,
	QEIx4_CCW | QEIx4_S0 | QEIx4_B  | QEIx4_4x_DEC | QEIx4_DIR,
	QEIx4_CW  | QEIx4_S2 | QEIx4_AB | QEIx4_1x_INC,
	// act state S2 in CW direction
	QEIx4_CCW | QEIx4_S1 |            QEIx4_2x_DEC | QEIx4_DIR,
	QEIx4_CW  | QEIx4_S2 | QEIx4_A,
	QEIx4_CW  | QEIx4_S3 | QEIx4_B  | QEIx4_4x_INC,
	QEIx4_CW  | QEIx4_S2 | QEIx4_AB,
	// act state S3 in CW direction
	QEIx4_CW  | QEIx4_S0 |            QEIx4_2x_INC,
	QEIx4_CCW | QEIx4_S2 | QEIx4_A  | QEIx4_4x_DEC | QEIx4_DIR,
	QEIx4_CW  | QEIx4_S3 | QEIx4_B,
	QEIx4_CW  | QEIx4_S3 | QEIx4_AB
};

// Helper for ISR call
QEIx4* QEIx4::__instance[4] = { 0 };


//#define DEB(x) printf (x)
#define DEB(x)

///////////////////////////////////////////////////////////////////////////////

QEIx4::QEIx4()
{
	for (uint8_t i=0; i<4; i++)
		if (__instance[i] == 0)
		{
			__instance[i] = this;
			DEB("::");
			break;
		}

	_pinA = -1;
	_pinB = -1;
	_pinI = -1;
	_state = 0;
	_limitMin = LONG_MIN;
	_limitMax = LONG_MAX;
}

///////////////////////////////////////////////////////////////////////////////

QEIx4::~QEIx4()
{
	for (uint8_t i=0; i<4; i++)
		if (__instance[i] == this)
		{
			__instance[i] = 0;
		}
}

///////////////////////////////////////////////////////////////////////////////

void QEIx4::begin(int16_t pinA, int16_t pinB, int16_t pinI, uint8_t mode)
{
	if (_pinA >= 0)
		detachInterrupt(digitalPinToInterrupt(_pinA));
	if (_pinB >= 0)
		detachInterrupt(digitalPinToInterrupt(_pinB));

	_pinA = pinA;
	_pinB = pinB;
	_pinI = pinI;

	_counter = 0;
	_bHasChanged = false;

	if (mode == 1)
		_eventMask = QEIx4_1x_MASK;
	else if (mode == 2)
		_eventMask = QEIx4_2x_MASK;
	else
		_eventMask = QEIx4_4x_MASK;

	pinMode(_pinA, INPUT_PULLUP);
	pinMode(_pinB, INPUT_PULLUP);
	pinMode(_pinI, INPUT_PULLUP);

	if (_pinA >= 0)
		attachInterrupt(digitalPinToInterrupt(_pinA), ISR, CHANGE);
	if (_pinB >= 0)
		attachInterrupt(digitalPinToInterrupt(_pinB), ISR, CHANGE);
}

///////////////////////////////////////////////////////////////////////////////

long QEIx4::read()
{
	noInterrupts();
	_bHasChanged = false;
	long ret = _counter;
	interrupts();
	return ret;
}

void QEIx4::loop()
{
	noInterrupts();
	processStateMachine();
	interrupts();
}

///////////////////////////////////////////////////////////////////////////////

void IRAM_ATTR QEIx4::processStateMachine()
{
	DEB(".");

	_state &= QEIx4_MASK;
	if (digitalRead(_pinA)) _state |= QEIx4_A;
	if (digitalRead(_pinB)) _state |= QEIx4_B;

	_state = __stateLUT[_state];   // magic is done by lookup-table
	_state &= _eventMask;

	if (_state & QEIx4_IS_CHG) {   // is any change?
		bool bCounterChange = false;

		if ((_state & QEIx4_IS_INC) && (_counter < _limitMax)) {   // has moved foreward?
			_counter++;
			bCounterChange = true;
			DEB("+");
		}
		if ((_state & QEIx4_IS_DEC) && (_counter > _limitMin)) {   // has moved backward?
			_counter--;
			bCounterChange = true;
			DEB("-");
		}

		if (_pinI >= 0 && _bIndexTrigger && bCounterChange && digitalRead(_pinI)) {   // is index pin triggered?
			_bIndexTrigger = false;
			_counter = 0;
			DEB("I");
		}

		if (bCounterChange) {   // has counter changed?
			_bHasChanged = true;
		}
	}
}

void IRAM_ATTR QEIx4::ISR()
{
	for (uint8_t i=0; i<4; i++)
		if (__instance[i])
		{
			__instance[i]->processStateMachine();
		}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
