#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>

class WGLParser;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void saveLog();
    void updateProgress(int percent,int count);
    void updateMessage(QString value);

    struct options {
        QString mode= "NORMAL";
        bool ScanState = true;
        bool Comments = false;
        bool Split = false;
        bool Strobe = false;
        bool splitLimit = false;
        QFileInfo fileInInfo;
        QFileInfo fileOutInfo;
        QFileInfo fileConfigInfo;
        QFileInfo fileScanStateInfo;
        QFileInfo fileIncludeInfo;
    };

   static QString versionNumber;

private slots:
    void connectUi();
    void on_btnWglFile_clicked();
    void on_btnConfigFile_clicked();
    void on_btnStart_clicked();
    void on_actionSave_the_Log_triggered();
    void on_actionExit_triggered();
    void validateStart();
    void on_rdbExpand_clicked();
    void on_rdbNormal_clicked();


    void on_chkSplitFiles_stateChanged(int arg1);
    void on_chkStrobe_stateChanged(int arg1);
    void on_chkComments_stateChanged(int arg1);
    void on_chkScanState_stateChanged(int arg1);

    void on_chkFileLimit_stateChanged(int arg1);

signals:
    void sigLockUi(bool set);
    void sigNewMessage(QString msg);
    void sigChangeProgressStatus(int percent,int count);
    void sigStartEnabled(int value);
    void sigValidateStart();
    void sigPeriodEnabled(int value);

private:
    Ui::MainWindow *ui;
    options Options;
    WGLParser* parser;
    QString lastDirPath;


};
#endif // MAINWINDOW_H
