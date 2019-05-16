/******************************************************************************

Modified by Jean Simonet, Systemic Games

******************************************************************************/

/******************************************************************************
SparkFun_MMA8452Q.cpp
SparkFun_MMA8452Q Library Source File
Jim Lindblom @ SparkFun Electronics
Original Creation Date: June 3, 2014
https://github.com/sparkfun/MMA8452_LIS2DE12

This file implements all functions of the MMA8452Q class. Functions here range
from higher level stuff, like reading/writing MMA8452Q registers to low-level,
hardware I2C reads and writes.

Development environment specifics:
IDE: Arduino 1.0.5
Hardware Platform: Arduino Uno

**Updated for Arduino 1.6.4 5/2015**

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include "lis2de12.h"
#include "drivers_nrf/i2c.h"
#include "nrf_log.h"

using namespace DriversNRF;

#define DEV_ADDRESS 0x18

///////////////////////////////////
// LIS2DE12 Register Definitions //
///////////////////////////////////
enum LIS2DE12_Register {
	STATUS_REG_AUX = 0x07,
	OUT_TEMP_L = 0x0C,
	OUT_TEMP_H = 0x0D,
	WHO_AM_I = 0x0F,
	CTRL_REG0 = 0x1E,
	TEMP_CFG_REG = 0x1F,
	CTRL_REG1 = 0x20,
	CTRL_REG2 = 0x21,
	CTRL_REG3 = 0x22,
	CTRL_REG4 = 0x23,
	CTRL_REG5 = 0x24,
	CTRL_REG6 = 0x25,
	REFERENCE = 0x26,
	STATUS_REG = 0x27,
	FIFO_READ_START = 0x28,
	OUT_X_H = 0x29,
	OUT_Y_H = 0x2B,
	OUT_Z_H = 0x2D,
	FIFO_CTRL_REG = 0x2E,
	FIFO_SRC_REG = 0x2F,
	INT1_CFG = 0x30,
	INT1_SRC = 0x31,
	INT1_THS = 0x32,
	INT1_DURATION = 0x33,
	INT2_CFG = 0x34,
	INT2_SRC = 0x35,
	INT2_THS = 0x36,
	INT2_DURATION = 0x37,
	CLICK_CFG = 0x38,
	CLICK_SRC = 0x39,
	CLICK_THS = 0x3A,
	TIME_LIMIT = 0x3B,
	TIME_LATENCY = 0x3C,
	TIME_WINDOW = 0x3D,
	ACT_THS = 0x3E,
	ACT_DUR = 0x3F,
};

namespace DriversHW
{
namespace LIS2DE12
{
	short x, y, z;
	float cx, cy, cz;
	LIS2DE12_Scale scale;

	void writeRegister(LIS2DE12_Register reg, uint8_t data);
	uint8_t readRegister(LIS2DE12_Register reg);
	void readRegisters(LIS2DE12_Register reg, uint8_t *buffer, uint8_t len);

	/// <summary>
	///	This function initializes the LIS2DE12. It sets up the scale (either 2, 4,
	///	or 8g), output data rate, portrait/landscape detection and tap detection.
	///	It also checks the WHO_AM_I register to make sure we can communicate with
	///	the sensor. Returns a 0 if communication failed, 1 if successful.
	/// </summary>
	/// <param name="fsr"></param>
	/// <param name="odr"></param>
	/// <returns></returns>
	void init(LIS2DE12_Scale fsr, LIS2DE12_ODR odr)
	{
		scale = fsr; // Haul fsr into our class variable, scale

		uint8_t c = readRegister(WHO_AM_I);  // Read WHO_AM_I register

		if (c != 0x33) // WHO_AM_I should always be 0x2A
		{
			NRF_LOG_ERROR("Bad WHOAMI");
			return;
		}

		standby();  // Must be in standby to change registers

		setScale(scale);  // Set up accelerometer scale
		setODR(odr);  // Set up output data rate
		active();  // Set to active to start reading
	}

	/// <summary>
	/// READ ACCELERATION DATA
	///  This function will read the acceleration values from the MMA8452Q. After
	///	reading, it will update two triplets of variables:
	///		* int's x, y, and z will store the signed 12-bit values read out
	///		  of the acceleromter.
	///		* floats cx, cy, and cz will store the calculated acceleration from
	///		  those 12-bit values. These variables are in units of g's.
	/// </summary>
	void read()
	{
		x = readRegister(OUT_X_H);
		y = readRegister(OUT_Y_H);
		z = readRegister(OUT_Z_H);
		cx = (float)x / (float)(1 << 11) * (float)(scale);
		cy = (float)y / (float)(1 << 11) * (float)(scale);
		cz = (float)z / (float)(1 << 11) * (float)(scale);
	}

	/// <summary>
	/// Helper that converts a raw reading value to a float
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	float convert(short value)
	{
		return (float)value / (float)(1 << 11) * (float)(scale);
	}

	/// <summary>
	/// CHECK IF NEW DATA IS AVAILABLE
	///	This function checks the status of the MMA8452Q to see if new data is availble.
	///	returns 0 if no new data is present, or a 1 if new data is available.
	/// </summary>
	uint8_t available()
	{
		return (readRegister(FIFO_SRC_REG) & 0x1F);
	}

	/// <summary>
	/// SET FULL-SCALE RANGE
	///	This function sets the full-scale range of the x, y, and z axis accelerometers.
	/// </summary>
	/// <param name="fsr">
	///	Possible values for the fsr variable are SCALE_2G, SCALE_4G, or SCALE_8G.
	/// </param>
	void setScale(LIS2DE12_Scale fsr)
	{
		// Must be in standby mode to make changes!!!
		uint8_t cfg = readRegister(CTRL_REG4);
		cfg &= 0b11001111; // Mask out scale bits
		cfg |= (fsr << 4);  // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
		writeRegister(CTRL_REG4, cfg);
	}

	/// <summary>
	/// SET THE OUTPUT DATA RATE
	/// </summary>
	void setODR(LIS2DE12_ODR odr)
	{
		// Must be in standby mode to make changes!!!
		uint8_t ctrl = readRegister(CTRL_REG1);
		ctrl &= 0x0F; // Mask out data rate bits
		ctrl |= (odr << 4);
		writeRegister(CTRL_REG1, ctrl);
	}


	/// <summary>
	/// ENABLE INTERRUPT ON TRANSIENT MOTION DETECTION
	/// This function sets up the MMA8452Q to trigger an interrupt on pin 1
	/// when it detects any motion (lowest detectable threshold).
	/// </summary>
	void enableTransientInterrupt()
	{
		standby();

		// // Tell the accelerometer that we want transient interrupts!
		// writeRegister(TRANSIENT_CFG, 0b00011110); // enable latch, xyz and hi-pass filter

		// // Setup the threshold
		// writeRegister(TRANSIENT_THS, 16); // Minimum threshold

		// // Set detection count
		// writeRegister(TRANSIENT_COUNT, 1); // Shortest detection period

		// // Route the transient interrupt to interrupt pin 1
		// writeRegister(CTRL_REG5, 0b00100000);

		// // Enable the transient interrupt
		// writeRegister(CTRL_REG4, 0b00100000);

		active();
	}

	/// <summary>
	/// CLEARS TRANSIENT INTERRUPT
	/// This function will 'aknowledge' the transient interrupt from the device
	/// </summary>
	void clearTransientInterrupt()
	{
		// standby();
		// uint8_t dontCare = readRegister(TRANSIENT_SRC);
		// // maybe log to console->..
		// active();
	}

	/// <summary>
	/// DISABLE TRANSIENT INTERRUPT
	/// </summary>
	void disableTransientInterrupt()
	{
		// standby();

		// writeRegister(TRANSIENT_CFG, 0b000000000);

		// active();
	}

	/// <summary>
	/// SET STANDBY MODE
	///	Sets the MMA8452 to standby mode. It must be in standby to change most register settings
	/// </summary>
	void standby()
	{
		uint8_t c = readRegister(CTRL_REG1);
		writeRegister(CTRL_REG1, c & ~(0x08)); //Clear the active bit to go into standby
	}

	/// <summary>
	/// SET ACTIVE MODE
	///	Sets the MMA8452 to active mode. Needs to be in this mode to output data
	/// </summary>
	void active()
	{
		uint8_t c = readRegister(CTRL_REG1);
		writeRegister(CTRL_REG1, c | 0x08); //Set the active bit to begin detection
	}

	/// <summary>
	/// WRITE A SINGLE REGISTER
	/// 	Write a single uint8_t of data to a register in the MMA8452Q.
	/// </summary>
	void writeRegister(LIS2DE12_Register reg, uint8_t data)
	{
		uint8_t write[2];
		write[0] = reg;
		write[1] = data;
		I2C::write(DEV_ADDRESS, write, 2);
	}

	/// <summary>
	/// READ A SINGLE REGISTER
	///	Read a uint8_t from the MMA8452Q register "reg".
	/// </summary>
	uint8_t readRegister(LIS2DE12_Register reg)
	{
		I2C::write(DEV_ADDRESS, reg, true);
		uint8_t ret = 0;
		I2C::read(DEV_ADDRESS, &ret, 1);
		return ret;
	}

	/// <summary>
	/// READ MULTIPLE REGISTERS
	///	Read "len" bytes from the MMA8452Q, starting at register "reg". Bytes are stored
	///	in "buffer" on exit.
	/// </summary>
	void readRegisters(LIS2DE12_Register reg, uint8_t *buffer, uint8_t len)
	{
		I2C::write(DEV_ADDRESS & 0x8000, reg, true);
		I2C::read(DEV_ADDRESS, buffer, len);
	}
}
}

