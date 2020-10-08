#ifndef WGLPARSER_H
#define WGLPARSER_H

#include <QWidget>
#include <iostream>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include "pin.h"
#include "timing.h"
#include "mainwindow.h"

class WGLParser : public QWidget
{
    Q_OBJECT
public:
    explicit WGLParser(QWidget *parent = nullptr);

    bool StartParsing(MainWindow::options options );
    void closeAllFile();

private:
    bool parseTimeplate(QString line);
    bool parseSignals(QString line);
    bool parseScanState(QString line);
    bool parseScanCell(QString line);
    bool parseScanChain(QString line);
    bool parseWaveForm(QString line);
    bool parseInclude(QString line, QString path);
    bool parseVectorLine(QString vector);
    bool outputLine(QStringList data,QString scanIn, QString scanOut);
    bool createHeader();
    QString expandLine(QString line,QString tsName);

    MainWindow::options mOptions;

    QString getNextLine();
    QString getScanStateData(QString dataName);

    QFile m_inFile;
    QFile m_outFile;
    QFile m_configFile;
    QFile m_scanStateFile;
    QFile m_includeFile;

    bool m_include;
    bool m_endOfFile;
    int m_scanLength;
    int m_outFileCount;

    qint64 m_maxByte;
    qint64 m_byteRead;
    int m_lineCounter;
    qint64 m_vectorCounter;
    qint64 m_loadCounter;
    qint64 m_totalCycles;

    QList<Pin*> m_signals;
    QList<Timing*> m_timingSets;

    int m_scanInPos;
    QString m_scanInName;
    int m_scanOutPos;
    QString m_scanOutName;
    bool m_cycleBit;

    QString m_inpInversionMask;
    QString m_outInversionMask;
    bool m_inverterFound;

signals:
    void sigChangeProgressStatus(int percent,int count);
    void sigNewMessage(QString msg);

public slots:
};



#endif // WGLPARSER_H
