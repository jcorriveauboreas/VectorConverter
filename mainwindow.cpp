#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QThread>
#include "wglparser.h"
#include "splitter.h"
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    parser  = new WGLParser();

    this->connectUi();

    ui->rdbNormal->click();
    ui->btnStart->setEnabled(false);
    ui->lcdCounter->display("0");
    ui->barProgress->reset();
    ui->txtMsg->clear();
    ui->txtMsg->setText("Welcome to the WGL Converter");

    ui->btnConfigFile->setVisible(false);
    ui->lblConfigFile->setVisible(false);
    ui->txtConfigFile->setVisible(false);
}

void MainWindow::connectUi()
{
    connect(parser, &WGLParser::sigChangeProgressStatus, this, &MainWindow::updateProgress);
    connect(parser,&WGLParser::sigNewMessage,ui->txtMsg, &QTextEdit::append);

    connect(this,&MainWindow::sigNewMessage,ui->txtMsg, &QTextEdit::append);
    connect(this,&MainWindow::sigChangeProgressStatus, this, &MainWindow::updateProgress);
    connect(this,&MainWindow::sigStartEnabled,ui->btnStart,&QPushButton::setEnabled);
    connect(this,&MainWindow::sigValidateStart,this,&MainWindow::validateStart);

    connect(this,&MainWindow::sigLockUi,ui->btnWglFile, &QPushButton::setEnabled);
    connect(this,&MainWindow::sigLockUi,ui->btnConfigFile, &QPushButton::setEnabled);
    connect(this,&MainWindow::sigLockUi,ui->btnStart, &QPushButton::setEnabled);
    connect(this,&MainWindow::sigLockUi,ui->grpMode, &QGroupBox::setEnabled);
    connect(this,&MainWindow::sigLockUi,ui->grpOption, &QGroupBox::setEnabled);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveLog()
{
   emit sigNewMessage("Test de save");
}

void MainWindow::validateStart()
{
    if (Options.fileInInfo.exists())
        emit sigStartEnabled(true);
}

void MainWindow::updateProgress(int percent,int count)
{
    ui->lcdCounter->display(count);
    ui->barProgress->setValue(percent);

    ui->lcdCounter->repaint();
    ui->barProgress->repaint();
}

void MainWindow::updateMessage(QString value)
{
    ui->txtMsg->append(value);
}

void MainWindow::on_btnWglFile_clicked()
{
    QString fileName;
    QFileDialog* fileDialog = new QFileDialog();

    fileName = fileDialog->getOpenFileName(this,tr("WGL File"), lastDirPath, tr("Wgl Files (*.wgl)"));

    if(!fileName.isNull())
    {
        Options.fileInInfo.setFile(fileName);
        ui->txtWglFile->setText(Options.fileInInfo.absoluteFilePath());

        Options.fileOutInfo.setFile(Options.fileInInfo.absoluteFilePath().remove(Options.fileInInfo.suffix())+ "csv");
        Options.fileScanStateInfo.setFile(Options.fileInInfo.absolutePath() + "/ScanState.tmp");

        ui->txtOutFile->setText(Options.fileOutInfo.absoluteFilePath());

        lastDirPath = Options.fileInInfo.absolutePath();
        emit sigValidateStart();
     }
 }

void MainWindow::on_btnConfigFile_clicked()
{
//    QString fileName;

//    fileName = QFileDialog::getOpenFileName(this,tr("Config File"), "C:/Users/jcorriveau/Documents/Produit", tr("Config File (*.txt)"));

//    if(!fileName.isNull())
//    {
//        Options.fileConfig.setFile(fileName);
//        ui->txtConfigFile->setText(Options.fileConfig.absoluteFilePath());
//        emit sigValidateStart();
//    }
}

void MainWindow::on_btnStart_clicked()
{
    QElapsedTimer conversionTime;
    conversionTime.start();

    emit sigLockUi(false);

    ui->txtMsg->clear();
    ui->txtMsg->setFocus();
    emit sigNewMessage("Starting the conversion....\n");
    emit sigNewMessage("    Input File\t\t: " + Options.fileInInfo.absoluteFilePath());
    emit sigNewMessage("    Output file\t\t: " + Options.fileOutInfo.absoluteFilePath());

    //===== Start the parser =====
    if(parser->StartParsing(Options))
    {
        parser->closeAllFile();
        emit sigNewMessage("\n    ERROR the parsing ended with errors");
    }

    ui->barProgress->setValue(100);

    emit sigNewMessage("    Number of line read\t: " + QString::number(ui->lcdCounter->value()));

    float timeElapsed = float( conversionTime.elapsed())/1000;
    emit sigNewMessage("\nConversion Time : " + QString::number( timeElapsed) + " sec");

    if(ui->chkScanState->checkState())
    {
        QFile tmpFile(Options.fileScanStateInfo.absoluteFilePath());
        tmpFile.remove();
        emit sigNewMessage("Erasing ScanState file");
    }

    emit sigNewMessage("\nConversion ended "  );

    //===============================================
    //       Spliting the file
    //===============================================

    if (Options.Split)
    {
        emit sigNewMessage("Starting File splitting");
        conversionTime.start();

        Splitter* tmpSplit = new Splitter();

        if (tmpSplit->splitFile(Options))
        {
            emit sigNewMessage("ERROR in the file splitter");
        }

        timeElapsed = float( conversionTime.elapsed())/1000;
        emit sigNewMessage("\nSplitting Time : " + QString::number( timeElapsed) + " sec");
    }

    emit sigLockUi(true);
    emit sigChangeProgressStatus(0,0);
}

void MainWindow::on_actionSave_the_Log_triggered()
{
    emit sigNewMessage("Test de save");
}

void MainWindow::on_actionExit_triggered()
{
    MainWindow::close();
}

void MainWindow::on_rdbExpand_clicked()
{
    Options.mode = "EXPAND";
    emit sigPeriodEnabled(true);
}

void MainWindow::on_rdbNormal_clicked()
{
    Options.mode = "NORMAL";
    emit sigPeriodEnabled(false);
}


void MainWindow::on_chkSplitFiles_stateChanged(int arg1)
{
    Options.Split = arg1;
}

void MainWindow::on_chkStrobe_stateChanged(int arg1)
{
    Options.Strobe = arg1;
}

void MainWindow::on_chkComments_stateChanged(int arg1)
{
    Options.Comments = arg1;
}

void MainWindow::on_chkScanState_stateChanged(int arg1)
{
    Options.ScanState = arg1;
}

void MainWindow::on_chkFileLimit_stateChanged(int arg1)
{
    Options.splitLimit = arg1;
}
