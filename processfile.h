#ifndef PROCESSFILE_H
#define PROCESSFILE_H

#include <QObject>
#include <QDir>

enum existMode{
    REWRITE,
    NUMERIC
};

class ProcessFile : public QObject{
    Q_OBJECT

private:
    QString inputDir;
    QString outputDir;
    bool deleteInputFiles;
    existMode isFileExists;
    bool timer;
    quint64 keyProcess;
    quint32 millisecond;
    QString fileFormat;

    bool stopProcess;
public:
    ProcessFile(QObject *parent = nullptr);
    void setParams(QString give_inputDir,
              QString give_outputDir,
              bool give_deleteInputFiles,
              existMode give_isFileExists,
              QString fileFormat,
              bool give_timer,
              quint64 give_keyProcess,
              quint32 give_millisecond = 1000);
    void XOR(char* buff, quint64 countBytes, quint64 keyProcess);
    void start();
    void runOnFiles(QDir &dirIn, QDir &dirOut);
    void stop();

public slots:
    bool processFile(QString enterPath, QString outputPath, quint64 keyProcess);

signals:
    void progressChanged(int value);
    void finished();
    void logAppend(QString message);

};

#endif // PROCESSFILE_H
