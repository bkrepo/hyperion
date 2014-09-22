// Qt includes
#include <QDateTime>

#include "LinearColorSmoothing.h"

LinearColorSmoothing::LinearColorSmoothing(
		LedDevice *ledDevice,
		double ledUpdateFrequency_hz,
		int settlingTime_ms,
		unsigned updateDelay) :
	QObject(),
	LedDevice(),
	_ledDevice(ledDevice),
	_updateInterval(1000 / ledUpdateFrequency_hz),
	_settlingTime(settlingTime_ms),
	_timer(),
	_outputDelay(updateDelay)
{
	_timer.setSingleShot(false);
	_timer.setInterval(_updateInterval);

	connect(&_timer, SIGNAL(timeout()), this, SLOT(updateLeds()));
}

LinearColorSmoothing::~LinearColorSmoothing()
{
	delete _ledDevice;
}

int LinearColorSmoothing::write(const std::vector<ColorRgb> &ledValues)
{
	// received a new target color
	if (_previousValues.empty())
	{
		// not initialized yet
		_targetTime = QDateTime::currentMSecsSinceEpoch() + _settlingTime;
		_targetValues = ledValues;

		_previousTime = QDateTime::currentMSecsSinceEpoch();
		_previousValues = ledValues;
		_timer.start();
	}
	else
	{
		_targetTime = QDateTime::currentMSecsSinceEpoch() + _settlingTime;
		memcpy(_targetValues.data(), ledValues.data(), ledValues.size() * sizeof(ColorRgb));
	}

	return 0;
}

int LinearColorSmoothing::switchOff()
{
	// Clear the smoothing parameters
	std::fill(_targetValues.begin(), _targetValues.end(), ColorRgb::BLACK);
	_targetTime = 0;

	// Erase the output-queue
	for (unsigned i=0; i<_outputQueue.size(); ++i)
	{
		_outputQueue.push_back(_targetValues);
		_outputQueue.pop_front();
	}


	return 0;
}

void LinearColorSmoothing::updateLeds()
{
	int64_t now = QDateTime::currentMSecsSinceEpoch();
	int deltaTime = _targetTime - now;

	if (deltaTime < 0)
	{
		memcpy(_previousValues.data(), _targetValues.data(), _targetValues.size() * sizeof(ColorRgb));
		_previousTime = now;

		queueColors(_previousValues);
	}
	else
	{
		float k = 1.0f - 1.0f * deltaTime / (_targetTime - _previousTime);

		for (size_t i = 0; i < _previousValues.size(); ++i)
		{
			ColorRgb & prev = _previousValues[i];
			ColorRgb & target = _targetValues[i];

			prev.red   += k * (target.red   - prev.red);
			prev.green += k * (target.green - prev.green);
			prev.blue  += k * (target.blue  - prev.blue);
		}
		_previousTime = now;

		queueColors(_previousValues);
	}
}

void LinearColorSmoothing::queueColors(const std::vector<ColorRgb> & ledColors)
{
	if (_outputDelay == 0)
	{
		_ledDevice->write(ledColors);
	}
	else
	{
		_outputQueue.push_back(ledColors);
		if (_outputQueue.size() > _outputDelay)
		{
			_ledDevice->write(_outputQueue.front());
			_outputQueue.pop_front();
		}
	}
}
