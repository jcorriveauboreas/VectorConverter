#include "splitter.h"
#include <QFile>
#include <QTextStream>

Splitter::Splitter()
{
}

bool Splitter::splitFile(MainWindow::options options)
{
    try {
        QString splitInFileName;
        QString splitBaseFileName;
        QStringList signalNames;
        QStringList signalData;

        QString line;
        QString tmp;
        QFile tmpFile[100];
        int index=0;

        QTextStream streamOut;
        QFile splitInFile;

        splitInFileName = options.fileOutInfo.absoluteFilePath();
        splitBaseFileName = splitInFileName;
        splitBaseFileName.chop(4);

        splitInFile.setFileName(splitInFileName);
        splitInFile.open(QIODevice::ReadOnly | QIODevice::Text);

        line = splitInFile.readLine();
        line.chop(1);
        signalNames = line.split(",", QString::SkipEmptyParts);

        //generate files

        foreach(tmp,signalNames)
        {
            QString fileName = splitBaseFileName + "_" + tmp.trimmed() + ".csv";
            tmpFile[index].setFileName(fileName);
            tmpFile[index].open(QIODevice::WriteOnly | QIODevice::Text);
            tmpFile[index].write(tmp.trimmed().toUtf8() + "\n");
            index++;
        }

        while(!splitInFile.atEnd())
        {
            line = splitInFile.readLine();
            line.chop(1);
            signalData = line.split(",", QString::SkipEmptyParts);

            for(index=0;index < signalNames.length();index++)
            {
                QString data = signalData[index].trimmed();
                QTextStream tmpStream(&tmpFile[index]);
                tmpStream << data << endl;
            }
        }

        for(index=0;index < signalNames.length();index++)
            tmpFile[index].close();

        splitInFile.close();

        return false;

    } catch (...) {
        return true;
    }
}

