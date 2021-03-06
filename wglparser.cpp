#include "wglparser.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include <QStringList>
#include <QList>
#include "pin.h"
#include "mainwindow.h"

WGLParser::WGLParser( QWidget *parent) : QWidget(parent)
{
}

bool WGLParser::StartParsing(MainWindow::options opt)
{
    try
    {
        mOptions = opt;

        m_inFile.setFileName(mOptions.fileInInfo.absoluteFilePath());
        QFileInfo* info = new QFileInfo(m_inFile);
        QString line;
        QString returnCode;

        m_signals.clear();
        m_timingSets.clear();
        m_vectorCounter=0;
        m_loadCounter=0;
        m_totalCycles=0;

        m_maxByte = info->size();

        emit sigNewMessage("    Size of the input file\t: " + QString::number(info->size()) + " Bytes");

        //==================================================
        //     Opening the WGL file
        //==================================================
        if(!m_inFile.open(QFile::ReadOnly |QFile::Text))
        {
            emit sigNewMessage("    ERROR opening file \n" + QString::number(m_inFile.error()));
            return 0;
        }

        m_endOfFile=false;
        m_include = false;
        m_byteRead=0;
        m_lineCounter = 0;
        m_outFileCount = 0;

        //==================================================
        //     Reading the WGL file
        //==================================================
        while (!m_endOfFile)
        {
            line = getNextLine();    //line = fileIn.readLine();

            if(!line.isEmpty() && !line.startsWith("#") && !line.startsWith("end"))
            {
                //====== VECTOR ======
                if (line.startsWith("vector"))
                {
                    QString parVector;

                    parVector = line;

                    while (!parVector.endsWith("];"))
                    {
                        parVector += getNextLine();
                    }

                    if(parseVectorLine(parVector)){
                        emit sigNewMessage("\tERROR parsing a vector line");
                        return 1;
                    }
                }
                //====== SCAN ======
                else if (line.startsWith("scan "))
                {
                    QString scanVector;

                    scanVector = line;

                    while (!scanVector.endsWith("];"))
                    {
                        scanVector += getNextLine();
                    }

                    //Need to split the ouput file
                    if(m_totalCycles >= 9500000 && opt.splitLimit)
                    {
                        emit sigNewMessage("\tReached the 10M limit, need to split the ouput file");

                        m_outFileCount ++;

                        if (!createHeader())
                        {
                            emit sigNewMessage("\tERROR creating the header");
                            return false;
                        }

                        m_totalCycles = 0;
                    }

                    if(parseVectorLine(scanVector)){
                        emit sigNewMessage("\tERROR parsing a scan vector line");
                        return 1;
                    }                }
                //====== INCLUDE ======
                else if (line.startsWith("include"))
                {
                    if(parseInclude(line, info->absolutePath())){
                        emit sigNewMessage("\tERROR in the Include parsing");
                        return 1;
                    }
                }
                //====== TIMEPLATE ======
                else if (line.startsWith("timeplate"))
                {
                    if(parseTimeplate(line)){
                        emit sigNewMessage("\tERROR in the TimePlate parsing");
                        return 1;
                    }
                 }
                //====== SCANCELL ======
                else if (line.startsWith("scancell"))
                {
                    if(parseScanCell(line)){
                        emit sigNewMessage("\tERROR in the ScanCell parsing");
                        return 1;
                    }
                }
                //====== SCANCHAIN ======
                else if (line.startsWith("scanchain"))
                {
                    if(parseScanChain(line)){
                        emit sigNewMessage("\tERROR in the ScanChain parsing");
                        return 1;
                    }
                }
                //====== SIGNAL ======
                else if (line.startsWith("signal"))
                {
                    if(parseSignals(line)){
                        emit sigNewMessage("\tERROR in the Signals parsing");
                        return 1;
                    }
                }
                //====== SCANSTATE ======
                else if (line.startsWith("scanstate"))
                {
                    if(parseScanState(line))
                    {
                        emit sigNewMessage("\tERROR in the ScanState parsing");
                        return 1;
                    }
                }
                else if (line.startsWith("waveform"))
                {
                    if(parseWaveForm(line)){
                        emit sigNewMessage("\tERROR in the WaveForm parsing");
                        return 1;
                    }
                }
                //====== PATTERN ======
                else if (line.startsWith("pattern"))
                {
                    while (!line.endsWith(")"))
                    {
                        line += getNextLine();
                    }
                    emit sigNewMessage("\tFound pattern");

                    if (!createHeader())
                    {
                        emit sigNewMessage("\tERROR : unable to create the header" );
                        return 1;
                    }
                    m_cycleBit = true  ;
                }
                else
                {
                     emit sigNewMessage("\tERROR : unable to decode the input line : \n " + line);
                     return 1;
                }
            }

            float completed = float(m_byteRead)/m_maxByte;
            emit sigChangeProgressStatus(completed*100,m_lineCounter);
        }
        emit sigNewMessage("\n    End of file found");

        emit sigNewMessage("\tNumber of vector line\t : " + QString::number(m_vectorCounter));
        emit sigNewMessage("\tNumber of scan load\t : " + QString::number(m_loadCounter));

        closeAllFile();

        return 0;

    } catch (...)
    {
        return 1;
    }

}

//======================================
//   GETNEXTLINE
//======================================
QString WGLParser::getNextLine()
{
    try {
        QString line;

        if(m_include)
        {
            if(!m_includeFile.atEnd())
                line = m_includeFile.readLine();
            else
            {
                line = "";
                m_include = false;
                m_includeFile.close();
            }
        }
        else
        {
            if(!m_inFile.atEnd())
                line = m_inFile.readLine();
            else
            {
                line="";
                m_endOfFile=true;
                m_inFile.close();
            }
        }

        m_byteRead += line.length();
        m_lineCounter++;

        line = line.trimmed();

        return line;

    } catch (...) {
        emit sigNewMessage("ERROR in the getNextLine function" );
        return "";
    }
}

//======================================
//   GETSCANSTATEDATA
//======================================
QString WGLParser::getScanStateData(QString dataName)
{
    try
    {
        QString scanData;

        if(!m_scanStateFile.isOpen())
        {
            if (!m_scanStateFile.open(QFile::ReadOnly |QFile::Text ))
            {
                emit sigNewMessage("ERROR : Unable to open the ScanState File");
                return "";
            }
        }

        while(!scanData.startsWith(dataName) && !m_scanStateFile.atEnd())
        {
            scanData = m_scanStateFile.readLine();
        }

        if (scanData.isEmpty())
        {
            emit sigNewMessage("ERROR : The scan data was not found -> " + dataName);
            return "";
        }
        else
        {
            scanData = scanData.replace(");","");
            scanData = scanData.remove(0,scanData.indexOf("(")+1);
            scanData = scanData.trimmed();
        }

        return scanData;

    } catch (...)
    {
        emit sigNewMessage("ERROR in te getScanData function");
        return "";
    }
}

//======================================
//   PARSE INCLUDE
//======================================
bool WGLParser::parseInclude(QString line, QString path)
{
    try
    {
        QString tmpInclude;

        emit sigNewMessage("\tFound include");
        tmpInclude = line.mid(line.indexOf("\"")+1,line.lastIndexOf("\"")-line.indexOf("\"")-1);

        tmpInclude  = path + "/" + tmpInclude.trimmed();
        emit sigNewMessage("\t\tFile : " +tmpInclude);

        m_includeFile.setFileName(tmpInclude);

        if(!m_includeFile.exists())
        {
            emit sigNewMessage("\tERROR : include file not found");
            return 0;
        }
        if(!m_includeFile.open(QFile::ReadOnly |QFile::Text))
        {
            emit sigNewMessage("    ERROR opening file \n" + QString::number(m_includeFile.error()));
            return 0;
        }

        QFileInfo* infoInclude = new QFileInfo(m_includeFile);

        m_maxByte += infoInclude->size();

        m_include = true;
        return 0;

    } catch (...)
    {
        return 1;
    }

}

//======================================
//   PARSE TIMEPLATE
//======================================
bool WGLParser::parseTimeplate(QString line)
{
    try
    {
         QString pinName;
         Pin::Direction dir = Pin::Direction::NOTSET;
         QString timingName;
         QString per;
         QString scale;
         QStringList words;
         QString states;
         Pin::Format format;
         int rise;
         int fall;
         int strobe;
         int i;
         int period;

         Timing* tmpTiming = new Timing();
         Pin* tmpPin = new Pin();
         //===========================================
         //  Info sur la periode    timeplate "test_cycle" period 80.000000ns
         //===========================================
         timingName = line.mid(line.indexOf("\"")+1, (line.lastIndexOf("\"")- line.indexOf("\""))-1).trimmed();
         per = line.mid(line.lastIndexOf(" "),line.length()).trimmed();
         scale = per.rightRef(2).toString();
         per = per.remove((QRegExp("[\\n\\r\\t\"ns]")));

         period = per.toDouble();

         tmpTiming->setTimingName(timingName);
         tmpTiming->setScale(scale);
         tmpTiming->setPeriod(period);

         //===========================================
         //    Lecture des signaux   "SCLK" := input  [ 0ns:P, 16.000000ns:S, 32.000000ns:D ];
         //===========================================
         line =getNextLine();

         while(!line.startsWith("end"))
         {
             states ="";

             line = line.remove((QRegExp("[\n\r\t\"; =']")));
             line = line.remove("]");
             line = line.remove("edge");             line = line.replace(",",":");
             line = line.replace("[",":");
             line=line.remove(scale);

             words = line.split(":");

             pinName = words[0];

             if(words[1].contains("input"))
                 dir = Pin::Direction::INPUT;
             if(words[1].contains("output"))
                 dir = Pin::Direction::OUTPUT;

             for (i=2;i<=words.length();i ++)
             {
                if(i%2)
                    states += words[i];
             }

             //===========================================================================
             // s				NR          Rise = words[2]
             // dsd				RZ			Rise = words[4]      Fall = words[6]
             // psd				RZ			Rise = words[4]      Fall = words[6]
             // psu				RO			Rise = words[4]      Fall = words[6]
             // usu				RO			Rise = words[4]      Fall = words[6]
             // XQ              ST          Strobe = words[4]
             // XQX             ST          Strobe = words[4]
             //============================================================================

             if (states.toUpper()=="S")
             {
                 format = Pin::NR;
                 rise =  static_cast<int>((100 * words[2].toDouble())/period);
                 fall = 0;
                 strobe=0;
             }
             else if(states.toUpper()=="XQ" ||states.toUpper()=="XQX" )
             {
                 format = Pin::ST;
                 strobe = static_cast<int>((100 * words[4].toDouble())/period);
                 rise = 0;
                 fall = 0;
             }
             else if(states.toUpper()=="DSD" ||states.toUpper()=="PSD" )
             {
                 format = Pin::RZ;
                 rise = static_cast<int>((100 * words[4].toDouble())/period);
                 fall = static_cast<int>((100 * words[6].toDouble())/period);
                 strobe=0;
             }
             else if(states.toUpper()=="PSU" ||states.toUpper()=="USU" )
             {
                 format = Pin::RO;
                 rise = static_cast<int>((100 * words[4].toDouble())/period);
                 fall = static_cast<int>((100 * words[6].toDouble())/period);
                 strobe=0;
             }
             else
             {
                 emit sigNewMessage("ERROR : unable to decode the STATES :" + states);
                 return 1;
             }

             //====================================================
             // On trouve le signals, modifie et ajoute au timing
             //====================================================
             Pin* temp = new Pin();
             foreach(tmpPin,m_signals){
                if(tmpPin->getName()==pinName){
                    temp = tmpPin->clone();
                    temp->setDirection(dir);
                    temp->setFormat(format);
                    temp->setRise(rise);
                    temp->setFall(fall);
                    temp->setStrobe(strobe);
                    break;
                }
             }

             tmpTiming->addPin(temp);

             line = getNextLine();
         }

         m_timingSets.append(tmpTiming);

        emit sigNewMessage("\tFound timeplate  \n\t\tTiming Set Name : " + timingName + "\n\t\tPeriod :\t" + per);
        return 0;

    } catch (...)
    {
        return 1;
    }
}

//======================================
//   PARSE SCANCHAIN
//======================================
bool WGLParser::parseScanChain(QString line)
{
    QString scanchain;
    QString invMaskString = "";
    QString inverted = "0";

    try
    {
        emit sigNewMessage("\tFound scanchain");
        m_inverterFound=false;
        scanchain = getNextLine();
        scanchain = getNextLine();

        while (!line.contains("];")){
            line = getNextLine();

            if(line.contains("!"))
            {
                m_inverterFound = true;
                emit sigNewMessage("\t\tInverter found \n\t\t\t" + line  );

                if (inverted == "0")
                    inverted = "1";
                else
                    inverted ="0";
            }
            if(line.contains(","))
                invMaskString += inverted;
        }
        scanchain +=line;
        scanchain = scanchain.replace("];","");
        scanchain = scanchain.trimmed();
        scanchain = scanchain.remove((QRegExp("[\\n\\t\\r\" ]")));
        // Ici on a le scin et scout
        QStringList scanpins = scanchain.split(",");

        while (!line.startsWith("end"))
        {
            line = getNextLine();
        }

        //=====================================
        //  Add info to signals
        //=====================================
        Pin* tmp;
        QString scanin = scanpins[0];
        QString scanout = scanpins[1];

        foreach (tmp, m_signals) {
            if(tmp->getName()== scanin)
                tmp->setScanType(Pin::ScanType::IN);

            if(tmp->getName()== scanout)
                tmp->setScanType(Pin::ScanType::OUT);
        }

        m_scanInName= scanpins[0];
        m_scanOutName= scanpins[1];

        emit sigNewMessage("\t\tSCAN IN \t: " + m_scanInName + "\n\t\tSCAN OUT \t: " + m_scanOutName);

        //=====================================
        //  Add inversion Mask
        //=====================================

        m_inpInversionMask = invMaskString;
        m_outInversionMask = invMaskString;

        // Need to invert the mask for output signal if inverter found
        if(m_inverterFound)
        {
            m_outInversionMask = m_outInversionMask.replace("0","*");
            m_outInversionMask = m_outInversionMask.replace("1","0");
            m_outInversionMask = m_outInversionMask.replace("*","1");
        }
        return 0;

    } catch (...)
    {
        return 1;
    }
}

//======================================
//   PARSE SCANCELL
//======================================
bool WGLParser::parseScanCell(QString line)
{

    try
    {
        emit sigNewMessage("\tFound scancell");
        int compteur = -1;

        while(!line.startsWith("end"))
        {
            line = getNextLine();

            compteur++;
        }

        emit sigNewMessage("\t\tNumber of cells :  " + QString::number(compteur));
        m_scanLength = compteur;

        return 0;
    }
    catch (...)
    {
        return 1;
    }
}

//======================================
//   PARSE SIGNALS
//======================================
bool WGLParser::parseSignals(QString line)
{
    try {
        emit sigNewMessage("\tFound signal");

        QString lineSignal;
        QStringList words;
        Pin::Direction dir = Pin::Direction::NOTSET;
        int index =0;

        line =getNextLine();

        while (!line.startsWith("end"))
        {
            index++;
            line = line.remove(line.indexOf(";"),line.length());
            line = line.remove((QRegExp("[\\n\\r\\t\" ]")));
            words = line.split(":");
            if(words[1].contains("input"))
                dir = Pin::Direction::INPUT;
            if(words[1].contains("output"))
                dir = Pin::Direction::OUTPUT;

            Pin* tmp = new Pin(words[0],dir,index);

            m_signals.append(tmp);

            line = getNextLine();
        }

        emit sigNewMessage("\t\tNumber of signals found : " + QString::number(m_signals.length()));

        return 0;

    } catch (...)
    {
        return 1;
    }
}

//======================================
//   PARSE SCANSTATE
//======================================
bool WGLParser::parseScanState(QString line)
{
    try
    {
        emit sigNewMessage("\tFound scanstate... \n\t\tCreating : " + mOptions.fileScanStateInfo.absoluteFilePath());

        m_scanStateFile.setFileName(mOptions.fileScanStateInfo.absoluteFilePath());

        if(m_scanStateFile.exists())
            m_scanStateFile.remove();

        if(!m_scanStateFile.open(QFile::WriteOnly |QIODevice::Append |QFile::Text))
        {
            emit sigNewMessage("\tERROR : unable to open the ScanState.tmp file");
            return 1;
        }
        else
        {
            line = getNextLine();

            QTextStream stream(&m_scanStateFile);
            QString tmpLine = "";

            while(!line.startsWith("end"))
            {
                tmpLine += line;
                if (tmpLine.endsWith(";"))
                {
                    tmpLine = tmpLine.trimmed();
                    tmpLine = tmpLine.remove((QRegExp("[\\n\\r\\t\" ]")));
                    stream << tmpLine << "\n";
                    tmpLine="";
                }

                line = getNextLine();
            }
            m_scanStateFile.close();
        }
        return 0;
    } catch (...)
    {
        return 1;
    }
}

//======================================
//   PARSE WAVEFORM
//======================================
bool WGLParser::parseWaveForm(QString line)
{
    try
    {
        line = line.remove(0,line.indexOf(" "));
        line = line.remove((QRegExp("[\\n\\r\\t\" ]")));
        line = line.trimmed();
        emit sigNewMessage("\tFound waveform -> " + line);

        return 0;
    } catch (...)
    {
        return 1;
    }
}

//================================================================
//   PARSE VECTOR
//================================================================
bool WGLParser::parseVectorLine(QString vector)
{
    //================================================================
    //  vector ( +, "test_cycle" ) := [10011X];
    //  scan   ( +, "scan_cycle_TM_1" ) := [11-11X],input  [ "OR_1_TM_1" : "CR.2.1.1.2.1.1.1" ];
    //
    //  data[0] = type  vector /scan
    //  data[1] = timingSet
    //  data[2] = vector data
    //  data[3] = scanData
    //  data[4] = scanData
    //  data[x] .... mode scan chain
    //================================================================
    QStringList data;
    QString scanDataIn = "";
    QString scanDataOut = "";
    bool dataIn = false;
    bool dataOut = false;

    try
    {
        vector = vector.remove((QRegExp("[\\n\\r\\t\" +();]")));
        vector = vector.replace(":=","!");
        vector = vector.replace(",","!");
        vector = vector.trimmed();

        data = vector.split("!");

        if (data[0] == "scan")
        {
            // splitter le data pour aller chercher le CR....
            // dans getscan data il faut s<assurer de la bonne longueur, il y a un " au debut
            int i;
            QString tmpData;

            for (i=3; i < data.length();i++) {
                tmpData  = data[i].remove("]");
                tmpData = tmpData.mid(tmpData.indexOf(":")+1,tmpData.length());

                if(data[i].startsWith("input")){
                    scanDataIn = getScanStateData(tmpData);
                    dataIn=true;
                }
                if(data[i].startsWith("output")){
                     scanDataOut = getScanStateData(tmpData);
                     dataOut = true;
                }
            }

            // A modifier car cause une erreur si il n'y a pas de data de scanin
            if ((m_scanLength != scanDataIn.length() && dataIn) || (m_scanLength != scanDataOut.length() && dataOut) ){
                emit sigNewMessage("ERROR : Length of scan data does not fit the scancell count");
                emit sigNewMessage("\tExpected DataIn length : " + QString::number(m_scanLength) + "      Data Length : " + QString::number(scanDataIn.length()));
                emit sigNewMessage("\tExpected DataOut length : " + QString::number(m_scanLength) + "      Data Length : " + QString::number(scanDataOut.length()));
                return 1;
            }
       }

       if(outputLine(data,scanDataIn,scanDataOut))
       {
           emit sigNewMessage("ERROR : In the OutputLine function");
           return 1;
       }
        return 0;
    } catch (...)
    {
        return 1;
    }
}

//======================================
//  CLOSING ALL FILE
//======================================
void WGLParser::closeAllFile()
{
    m_inFile.close();
    m_outFile.close();
    m_scanStateFile.close();
}

//======================================
//  OUTPUT LINE
//======================================
bool WGLParser::outputLine(QStringList data, QString scanIn, QString scanOut)
{
    try
    {
        QString vectorType = data[0].trimmed();
        QString tsName = data[1].trimmed();
        QString vectorData = data[2].trimmed();
        int x;
        QString lineOut="";
        QString tmpLine;
        QTextStream streamOut(&m_outFile);
        QStringList scaninData;
        QStringList scanoutData;

        QString maskBit;
        QString strobeBit;
        QString cycleBit;
        QString vectorOut;

        vectorData = vectorData.remove("[");
        vectorData = vectorData.remove("]");

        if(vectorType == "scan")
        {
            m_loadCounter++;

            //======================================================
            // On applique le masque d'inversion sur le scan data
            //======================================================
            QString outTmp;
            QString inpTmp;
            QString tmpData;
            for (int i=0; i<m_scanLength ;i++)
            {
                if (m_inpInversionMask.mid(i,1) == "0")
                    inpTmp += scanIn.mid(i,1);
                else {
                    tmpData = scanIn.mid(i,1);
                    if(tmpData == "0")
                        inpTmp += "1";
                    else if (tmpData == "1")
                        inpTmp += "0";
                    else
                        inpTmp += tmpData;
                }

                if (m_outInversionMask.mid(i,1) == "0")
                    outTmp += scanOut.mid(i,1);
                else {
                    tmpData = scanOut.mid(i,1);
                    if(tmpData == "0")
                        outTmp += "1";
                    else if (tmpData == "1")
                        outTmp += "0";
                    else
                        outTmp += tmpData;
                }
            }

//            if(scanIn.length() == inpTmp.length() && scanOut.length() == outTmp.length()){
                scanIn = inpTmp;
                scanOut = outTmp;
//            }
//            else {
//                emit sigNewMessage("ERROR : Length of scan data does not fit inversion");
//                return 1;
//            }


            // On doit commencer à regarder/loader le data par la fin, on inverse la chaine
            std::reverse(scanOut.begin(), scanOut.end());
            std::reverse(scanIn.begin(), scanIn.end());

            scaninData = scanIn.split("", Qt::SplitBehaviorFlags::SkipEmptyParts);
            scanoutData = scanOut.split("", Qt::SplitBehaviorFlags::SkipEmptyParts);

            for (x=0;x<m_scanLength;x++) {
                maskBit="0";
                strobeBit="1";
                cycleBit="1";

                vectorOut = vectorData;

                if(vectorData.mid(m_scanInPos-1,1)=="-")
                    vectorOut = vectorOut.replace(m_scanInPos-1,1,scaninData[x]);

                if(vectorData.mid(m_scanOutPos-1,1)=="-")
                    vectorOut = vectorOut.replace(m_scanOutPos-1,1,scanoutData[x]);

                if(mOptions.mode =="NORMAL")
                {
                    if (vectorOut.count("X") != 0){
                        vectorOut= vectorOut.replace("X","0");
                        maskBit="1";
                    }

                    if(mOptions.Strobe)
                        vectorOut += maskBit + strobeBit + cycleBit;

                    lineOut = vectorOut.split("", Qt::SplitBehaviorFlags::SkipEmptyParts).join(',');

                    if(mOptions.Comments)
                        streamOut << lineOut << " ,   Load :" << QString::number(m_loadCounter) << "." << QString::number(x) <<  Qt::endl;
                    else
                        streamOut << lineOut << Qt::endl;

                    m_totalCycles ++;
                }
                else
                {
                    lineOut = expandLine(vectorOut,tsName);

                    if(mOptions.Comments)
                        streamOut << lineOut << " ,   Load :" << QString::number(m_loadCounter) << "." << QString::number(x) <<  Qt::endl;
                    else
                        streamOut << lineOut << Qt::endl;

                    m_totalCycles += 10;
                }
            }
        }
        else
        {
            maskBit="0";
            strobeBit="1";
            cycleBit="1";

            m_vectorCounter++;

            if (mOptions.mode == "NORMAL")
            {
                vectorOut = vectorData;

                if (vectorOut.count("X") != 0){
                    vectorOut= vectorOut.replace("X","0");
                    maskBit="1";
                }

                if(mOptions.Strobe)
                    vectorOut += maskBit + strobeBit + cycleBit;

                lineOut = vectorOut.split("", Qt::SplitBehaviorFlags::SkipEmptyParts).join(',');

                m_totalCycles ++;
            }
            else
            {
                lineOut = expandLine(vectorData,tsName);
                m_totalCycles += 10;
            }

            if(mOptions.Comments)
                streamOut << lineOut << " ,   Vector : " << QString::number(m_vectorCounter) <<  Qt::endl;
            else
                streamOut << lineOut << Qt::endl;
        }

    } catch (...)
    {
        return 1;
    }

    return 0;

}

//======================================
//  EXPAND LINE
//======================================
QString WGLParser::expandLine(QString line, QString tsName)
{
    try
    {
        QString returnLine = "";
        int index = 0;
        int timing =0;
        QString tmp[10];
        QString tmpStrobe[10];
        QString tmpMask[10];
        QString tmpCycle[10];
        QStringList  tmpData = line.split("",Qt::SplitBehaviorFlags::SkipEmptyParts);

        QString bit;
        QString outBit;
        QString strobeBit;
        QString maskBit;

        int rise=0;
        int fall = 0;
        int strobe=0;
        Pin::Format format= Pin::NOFORMAT;
        Timing* tmpTiming;
        int position =1;

         foreach(bit,tmpData)
        {
            foreach(tmpTiming,m_timingSets)
            {
                if (tmpTiming->getTimingName() == tsName)
                {
                    Pin* tmpPin = new Pin();
                    tmpPin = tmpTiming->getPin(position);

                    rise = tmpPin->getRise();
                    fall = tmpPin->getFall();
                    strobe = tmpPin->getStrobe();
                    format = tmpPin->getFormat();
                    break;
                }
            }

            outBit = "0";
            strobeBit = "0";
            maskBit ="0";

            for (index=0;index<10;index++)
            {
                timing = index * 10;

                switch (format) {
                case  Pin::NR:
                    if(rise == 0 && bit == "0") outBit="0";
                    if(rise == 0 && bit == "1") outBit="1";
                    break;
                case Pin::RZ:
                    if(rise == timing && bit == "1") outBit="1";
                    if(fall == timing ) outBit="0";
                    break;
                case Pin::ST:
                    if( strobe == timing ) strobeBit="1"; else strobeBit= "0";
                    outBit = bit;
                    if (bit == "X") {
                        outBit="0";
                        maskBit="1";
                    }
                    break;
                case Pin::RO:
                    emit sigNewMessage("ERROR RO");

                    break;
                case Pin::NOFORMAT:
                    emit sigNewMessage("ERROR NOFORMAT");
                    break;
                }

                tmpMask[index]=maskBit;
                tmpStrobe[index] = strobeBit;
                tmp[index] += outBit;
                tmpCycle[index] = QString::number(m_cycleBit);
            }
            position++;
        }

        QString tmpLine;
        for (index=0;index<10;index++) {
            if (mOptions.Strobe)
                tmpLine = tmp[index] + tmpMask[index] + tmpStrobe[index] + tmpCycle[index] ;
            else
                tmpLine = tmp[index];

            if(mOptions.Comments){
                tmpLine = tmpLine.split("", Qt::SplitBehaviorFlags::SkipEmptyParts).join(',')+ "    ,"+QString::number(m_vectorCounter) + "." + QString::number(index) + "\n";
                returnLine += tmpLine;
            }else
                returnLine += tmpLine.split("", Qt::SplitBehaviorFlags::SkipEmptyParts).join(',') + "\n";
        }

        m_cycleBit = !m_cycleBit;
        returnLine.chop(1);
        return returnLine;

    } catch (...)
    {
        return "";
    }
}

//======================================
//  CREATE HEADER
//======================================
bool WGLParser::createHeader()
{
    try
    {
        QString lineOut = "";
        QString pinName;

        emit sigNewMessage("\t\tCreating header in the output file");

        if (m_outFileCount == 0)
            m_outFile.setFileName(mOptions.fileOutInfo.absoluteFilePath());
        else
        {
            m_outFile.close();                 // Need to close the file before we open the new one
            QString tempFile = mOptions.fileOutInfo.fileName();

            tempFile.chop(4);
            tempFile += "_" + QString::number(m_outFileCount) + ".csv";

            tempFile = mOptions.fileOutInfo.absolutePath() +"/" + tempFile;

            m_outFile.setFileName(tempFile);

        }

        if (m_outFile.exists())
            m_outFile.remove();
        if(m_outFile.exists())
        {
            emit sigNewMessage("\t\tOutput file already open, please close the file and run again");
            return false;
        }

        m_outFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QTextStream stream(&m_outFile);

        Pin* tmp = new Pin();

        foreach(tmp , m_signals)
        {
            pinName = tmp->getName();

            lineOut += pinName + " , ";

            if(pinName == m_scanInName)
                m_scanInPos = tmp->getPosition();

            if(pinName == m_scanOutName)
                m_scanOutPos = tmp->getPosition();
        }

        lineOut.chop(3);

        if(mOptions.Strobe)
            lineOut += " , MASK , STROBE , CYCLE";

        stream << lineOut << Qt::endl ;

        return true;

    }
    catch (...)
    {
        return false;
    }
}
