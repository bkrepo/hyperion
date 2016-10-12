
// STL includes
#include <cstring>
#include <cstdio>
#include <iostream>

// Linux includes
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

// hyperion local includes
#include "LedDeviceOdlight.h"

LedDeviceOdlight::LedDeviceOdlight(const std::string& outputDevice, const unsigned baudrate, int delayAfterConnect_ms) :
	LedRs232Device(outputDevice, baudrate, delayAfterConnect_ms),
	_ledBuffer(0),
	_timer()
{
	// setup the timer
	_timer.setSingleShot(false);
	_timer.setInterval(5000);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(rewriteLeds()));

	// start the timer
	_timer.start();
}

int LedDeviceOdlight::write(const std::vector<ColorRgb> & ledValues)
{
	int ret = 0;

	if (_ledBuffer.size() == 0)
	{
		_ledBuffer.resize(4 + 3*ledValues.size());
		_ledBuffer[0] = 'o';
		_ledBuffer[1] = 'd';
		_ledBuffer[2] = ledValues.size(); // LED count
		_ledBuffer[3] = 0;
		_ledBuffer[3] ^= _ledBuffer[0];
		_ledBuffer[3] ^= _ledBuffer[1];
		_ledBuffer[3] ^= _ledBuffer[2]; // Checksum
	}

	// restart the timer
	_timer.start();

	// write data
	memcpy(4 + _ledBuffer.data(), ledValues.data(), ledValues.size() * 3);
	ret = writeBytes(_ledBuffer.size(), _ledBuffer.data());
	if (ret >= 0 && !waitForAck())
		return ret;
	return -1;
}

int LedDeviceOdlight::waitForAck()
{
	uint8_t ack = 0;
	int timeout_count = 2000;

	// Wait for ACK
	while (timeout_count > 0)
	{
		readBytes(1, &ack);
		if (ack == 'N')
			return 0;
		usleep(1000);
		timeout_count--;
	}
	std::cerr << "Invalid ACK:" << ack << std::endl;
	return -1;
}

int LedDeviceOdlight::switchOff()
{
	int ret = 0;

	// restart the timer
	_timer.start();

	// write data
	memset(4 + _ledBuffer.data(), 0, _ledBuffer.size()-4);

	ret = writeBytes(_ledBuffer.size(), _ledBuffer.data());
	if (ret >= 0 && !waitForAck())
		return ret;
	return -1;
}

void LedDeviceOdlight::rewriteLeds()
{
	writeBytes(_ledBuffer.size(), _ledBuffer.data());
	waitForAck();
}
