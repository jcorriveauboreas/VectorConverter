#include "pin.h"

Pin::Pin()
{
    m_name = "";
    m_position = 0;
    m_direction=Direction::IO;
}

Pin::Pin(QString name, Direction direction,int position)
{
    m_name = name;
    m_direction = direction;
    m_position = position;
    m_fall=0;
    m_rise=0;
    m_format=Pin::NOFORMAT;
    m_strobe=0;
    m_scanType = ScanType::NONE;
}

Pin::Pin(QString name,Direction direction, Format format,int rise,int fall, int strobe,int position)
{
    m_name = name;
    m_direction = direction;
    m_position = position;
    m_fall=fall;
    m_rise=rise;
    m_format=format;
    m_strobe=strobe;
    m_scanType = ScanType::NONE;
}

void Pin::setFall(int value){ m_fall = value;}
int Pin::getFall(){    return m_fall ;}

void Pin::setRise(int value){   this-> m_rise = value;}
int Pin::getRise(){ return this->m_rise ;}

void Pin::setFormat(Format format){this->m_format  = format;}
Pin::Format Pin::getFormat(){ return this->m_format;}

void Pin::setStrobe(int value){    m_strobe=value;}
int Pin::getStrobe(){ return   m_strobe;}

void Pin::setName(QString name){    m_name = name;}
QString Pin::getName(){ return m_name;}

void Pin::setDirection(Direction direction){    m_direction = direction;}
Pin::Direction Pin::getDirection(){  return m_direction; }

void Pin::setPosition(int position){    m_position=position;}
int Pin::getPosition(){ return   m_position;}

void Pin::setScanType(ScanType type){    m_scanType = type;    }
Pin::ScanType Pin::getScanType(){ return m_scanType;    }

Pin* Pin::clone()
{
   return new Pin(m_name,m_direction ,m_format,m_rise,m_fall, m_strobe,m_position);
}



