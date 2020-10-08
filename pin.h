#ifndef PIN_H
#define PIN_H

#include <QObject>

class Pin
{
public:
    enum ScanType{IN,OUT,NONE};
    enum Direction{INPUT,OUTPUT,IO,NOTSET};
    enum Format{NR,RZ,RO,ST,NOFORMAT};

    Pin();

    Pin(QString name,Direction direction, int position);
    Pin(QString name,Direction direction ,Format format,int rise, int fall, int strobe,int position);

    void setName(QString name);
    void setDirection(Direction direction);
    void setPosition(int position);
    void setRise(int value);
    void setFall(int value);
    void setFormat(Format format);
    void setStrobe(int value);
    void setScanType(ScanType type);
    Pin* clone();

    QString getName();
    Direction getDirection();
    int getPosition();
    int getRise();
    int getFall();
    Format getFormat();
    int getStrobe();
    ScanType getScanType();

private:

    QString m_name;
    Direction m_direction;
    int     m_position;
    int     m_rise;
    int     m_fall;
    Format m_format;
    int     m_strobe;
    ScanType m_scanType;

};


#endif // PIN_H
