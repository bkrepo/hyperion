#pragma once

// STL includes
#include <string>

// Qt includes
#include <QTimer>

// hyperion include
#include "LedRs232Device.h"

///
/// Implementation of the LedDevice interface for writing to an Odlight led device.
///
class LedDeviceOdlight : public LedRs232Device
{
	Q_OBJECT

public:
	///
	/// Constructs the LedDevice for attached Odlight device
	///
	/// @param outputDevice The name of the output device (eg '/dev/ttyS0')
	/// @param baudrate The used baudrate for writing to the output device
	///
	LedDeviceOdlight(const std::string& outputDevice, const unsigned baudrate, int delayAfterConnect_ms);

	///
	/// Writes the led color values to the led-device
	///
	/// @param ledValues The color-value per led
	/// @return Zero on succes else negative
	///
	virtual int write(const std::vector<ColorRgb> & ledValues);

	/// Switch the leds off
	virtual int switchOff();

private slots:
	/// Write the last data to the leds again
	void rewriteLeds();

protected:
	/// The buffer containing the packed RGB values
	std::vector<uint8_t> _ledBuffer;

	/// Timer object which makes sure that led data is written at a minimum rate
	/// The Odlight device will switch off when it does not receive data at least
	/// every 15 seconds
	QTimer _timer;

private:
	int waitForAck();

};
