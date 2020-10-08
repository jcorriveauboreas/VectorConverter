#include "timing.h"
#include "pin.h"

Timing::Timing()
{
    this->m_timingSetName="";
    this->m_pins.clear();
}

void Timing::setScale(QString scale)
{
    this->m_scale = scale;
}
void Timing::setPeriod(int period)
{
    this->m_period=period;
}
void Timing::setTimingName(QString name)
{
    this->m_timingSetName=name;
}
void Timing::addPin(Pin* pin)
{
    m_pins.append(pin);
}

QString Timing::getScale()
{
    return this->m_scale;
}
int Timing::getTimingPeriod()
{
    return this->m_period;
}
QString Timing::getTimingName()
{
    return this->m_timingSetName;
}

Pin* Timing::getPin(int pos)
{
    Pin* pin;
    foreach(pin,m_pins)
    {
        int pinPos = pin->getPosition();

        if (pinPos == pos)
            return pin;
    }
    return nullptr;
}

