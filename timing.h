#ifndef TIMING_H
#define TIMING_H
#include "pin.h"
#include <QObject>

class Timing
{
public:
    Timing();

    void addPin(Pin* pin);
    void setTimingName(QString name);
    void setPeriod(int value);
    void setScale (QString scale);

    QString getTimingName();
    int getTimingPeriod();
    QString getScale();
    Pin* getPin(int pos);

private:
    QString m_timingSetName;
    int     m_period;
    QString m_scale;

    QList<Pin*> m_pins;

};

#endif // TIMING_H
