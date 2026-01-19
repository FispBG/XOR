#include "processfile.h"
#include <QFile.h>
#include <QString>
#include <vector>
#include <QDir>
#include <QThread>

ProcessFile::ProcessFile(QObject *parent) : QObject(parent), stopProcess(false){}

void ProcessFile::setParams(QString give_inputDir,
                            QString give_outputDir,
                            bool give_deleteInputFiles,
                            existMode give_isFileExists,
                            QString give_fileFormat,
                            bool give_timer,
                            quint64 give_keyProcess,
                            quint32 give_millisecond){
    inputDir = give_inputDir;
    outputDir= give_outputDir;
    deleteInputFiles = give_deleteInputFiles;
    isFileExists = give_isFileExists;
    timer = give_timer;
    fileFormat = give_fileFormat;
    keyProcess = give_keyProcess;
    millisecond = give_millisecond;
    stopProcess = false;
}

void ProcessFile::XOR(char* buff, quint64 countBytes, quint64 keyProcess){
    // Преобразование указаьеля quint64 -> char (8 char = 1 quint64).
    // Сравнение по 1 char.
    char* keyChar = reinterpret_cast<char*>(&keyProcess);

    for (qint64 i = 0; i < countBytes; ++i){
        buff[i] ^= keyChar[i % 8];
    }
}

bool ProcessFile::processFile(QString enterPath, QString outputPath, quint64 keyProcess) {
    // Открытие файлов.
    QFile enterFile(enterPath);
    QFile outputFile(outputPath);

    // Progress bar.
    const quint64 fileSize = enterFile.size();
    quint64 bytesNow = 0;
    int procentProgress = 0;

    // Проверка.
    if (!enterFile.open(QIODevice::ReadOnly)){
        return 0;
    }

    if (!outputFile.open(QIODevice::WriteOnly)){
        return 0;
    }
    emit logAppend("Файлы открыты.");

    // Размер буфера.
    const quint64 buffSize = 1024 * 1024;

    // Буфер.
    std::unique_ptr<std::vector<char>> buff = std::make_unique<std::vector<char>>(buffSize);

    // Обработка файла.
    emit logAppend("Начало обработки файла: " + enterFile.fileName());
    while (!enterFile.atEnd()){
        //Чтение буфера.
        qint64 readBytes = enterFile.read(buff->data(), buffSize);

        if (readBytes == -1){
            break;
        }

        if (readBytes > 0){
            // XOR для буфера.
            XOR(buff->data(), readBytes, keyProcess);

            //Запись в файл output.
            qint64 bytesWritten = outputFile.write(buff->data(), readBytes);

            if (readBytes != bytesWritten){
                break;
            }

            bytesNow += readBytes;

            // Вычисление процента выполнения.
            if (fileSize > 0){
                int processNow = static_cast<int>((bytesNow * 100) / fileSize);

                if (processNow != procentProgress){
                    // Отправление сигнала в progress bar.
                    emit progressChanged(processNow);
                    procentProgress = processNow;
                }
            }
        }
    }

    emit progressChanged(100);
    emit logAppend("Конец обработки файла.");

    return bytesNow == fileSize;
}

void ProcessFile::runOnFiles(QDir &dirIn, QDir &dirOut){

    // Читаем форматы файлов и разделяем по пробелам.
    QString format = fileFormat;
    QStringList formatList = format.split(" ", Qt::SkipEmptyParts);
    dirIn.setNameFilters(formatList);

    // Найденные файлы.
    QFileInfoList files = dirIn.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    emit logAppend("------------------------");
    emit logAppend("Найдено " + QString::number(files.count()) + " файл(ов).");

    // Пробегаемся по файлам.
    for (auto &file : files){
        // Предпологаемый путь сохранения.
        QString pathWrite = outputDir + "/" + file.fileName();

        // Если у нас режим нумерации (test_1.txt).
        if (QFile::exists(pathWrite) && isFileExists == NUMERIC){
            QString format = file.suffix();
            QString name = file.completeBaseName();
            int counter = 1;

            while (QFile::exists(pathWrite)){
                pathWrite = QString("%1/%2_%3.%4").arg(outputDir).arg(name).arg(counter).arg(format);
                counter++;
            }
        }

        bool done = processFile(file.absoluteFilePath(), pathWrite, keyProcess);

        // Если режим удаления при удалении.
        if (done && deleteInputFiles){
            dirIn.remove(file.fileName());
        }
    }
    emit logAppend("------------------------");
}

void ProcessFile::start(){
    // Открываем папки.
    QDir dirIn(inputDir);
    QDir dirOut(outputDir);

    // Проверяем открытие папок.
    if (!dirIn.exists()){
        emit logAppend("Не открылась папка(входная).");
        emit finished();
    }

    if (!dirOut.exists()){
        emit logAppend("Не открылась папка(сохранения).");
        emit finished();
    }

    // Режим таймер/один раз.
    if (timer){
        while (!stopProcess){
            runOnFiles(dirIn, dirOut);
            QThread::msleep(millisecond);
        }
    }else{
        runOnFiles(dirIn, dirOut);
    }

    emit logAppend("Поток закончил работу.");
    emit logAppend("------------------------");
    emit finished();
}

// Инкапсуляция состояния стоп.
void ProcessFile::stop(){
    stopProcess = true;
}
