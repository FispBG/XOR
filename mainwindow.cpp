#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    layout->addWidget(new QLabel("Папка с файлами:"));
    inputFolder = new QLineEdit();
    selectInputBtn = new QPushButton("Выбрать папку");
    layout->addWidget(inputFolder);
    layout->addWidget(selectInputBtn);

    // a.
    layout->addWidget(new QLabel("Маска (например *.txt):"));
    fileFormat = new QLineEdit("*.txt *.bin");
    layout->addWidget(fileFormat);

    // b.
    deleteEnterFile = new QCheckBox("Удалять входные файлы");
    layout->addWidget(deleteEnterFile);

    // c.
    layout->addWidget(new QLabel("Куда сохранять:"));
    folderSave = new QLineEdit();
    selectSaveBtn = new QPushButton("Выбрать папку");
    layout->addWidget(folderSave);
    layout->addWidget(selectSaveBtn);

    // d.
    layout->addWidget(new QLabel("Если файл уже есть:"));
    rewriteMode = new QComboBox();
    rewriteMode->addItem("Перезапись");
    rewriteMode->addItem("Счетчик");
    layout->addWidget(rewriteMode);

    // e.
    layout->addWidget(new QLabel("Режим работы:"));
    radioOneTime = new QRadioButton("Разовый");
    radioTimer = new QRadioButton("Таймер");
    radioOneTime->setChecked(true);
    layout->addWidget(radioOneTime);
    layout->addWidget(radioTimer);

    // f.
    layout->addWidget(new QLabel("Интервал (мс):"));
    intervalTimer = new QSpinBox();
    intervalTimer->setRange(100, 999999);
    intervalTimer->setValue(1000);
    intervalTimer->setEnabled(false);
    layout->addWidget(intervalTimer);

    // e.
    layout->addWidget(new QLabel("Ключ (8 байт число quint64):"));
    xorKey = new QLineEdit();
    xorKey->setInputMask("345345");
    layout->addWidget(xorKey);

    Start = new QPushButton("СТАРТ");
    Stop = new QPushButton("СТОП");
    Stop->setEnabled(false);
    layout->addWidget(Start);
    layout->addWidget(Stop);

    processBar = new QProgressBar();
    processBar->setValue(0);
    layout->addWidget(processBar);

    log = new QTextEdit();
    log->setReadOnly(true);
    layout->addWidget(log);

    connect(selectInputBtn, &QPushButton::clicked, this, [this](){
        QString dir = QFileDialog::getExistingDirectory(this, "Выбор папки");
        if (!dir.isEmpty())
            inputFolder->setText(dir);
    });

    connect(selectSaveBtn, &QPushButton::clicked, this, [this](){
        QString dir = QFileDialog::getExistingDirectory(this, "Выбор папки");
        if (!dir.isEmpty())
            folderSave->setText(dir);
    });
    connect(Start, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(Stop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(radioTimer, &QRadioButton::toggled, this, &MainWindow::updateTimerState);
    connect(radioOneTime, &QRadioButton::toggled, this, &MainWindow::updateTimerState);
}

void MainWindow::onStartClicked(){
    // Создаем поток и обработчик файлов.
    thread = new QThread;
    processor = new ProcessFile;

    // Что делать, если нашли файл с таким же названием.
    existMode ifExist;

    if (rewriteMode->currentText() == "Счетчик"){
        ifExist = NUMERIC;
    }else{
        ifExist = REWRITE;
    }

    // Задаем парамерты обработчика.
    processor->setParams(inputFolder->text(),
                         folderSave->text(),
                         deleteEnterFile->isChecked(),
                         ifExist,
                         fileFormat->text(),
                         radioTimer->isChecked(),
                         xorKey->text().toULongLong(nullptr, 10),
                         intervalTimer->value());

    // Обработчик в поток.
    processor->moveToThread(thread);

    // При старте вызываем ProcessFile::start().
    connect(thread, &QThread::started, processor, &ProcessFile::start);

    // Удаление объектов при завершении процесса.
    connect(processor, &ProcessFile::finished, thread, &QThread::quit);
    connect(processor, &ProcessFile::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // Разблокируем старт, блокируем стоп
    connect(thread, &QThread::finished, this, [this](){
        Start->setEnabled(true);
        Stop->setEnabled(false);
    });

    // Добавление в лог для сигнала процессора.
    connect(processor, &ProcessFile::logAppend, this, [this](QString message){
        log->append(message);
    });

    // Обновление progress bar от сигнала процессора.
    connect(processor, &ProcessFile::progressChanged, this, [this](int value){
        processBar->setValue(value);
    });

    thread->start();

    // Блокируем старт, разблокируем стоп.
    Start->setEnabled(false);
    Stop->setEnabled(true);
}

void MainWindow::onStopClicked(){
    if (processor){
        processor->stop();
    }
}

void MainWindow::updateTimerState(){
    intervalTimer->setEnabled(radioTimer->isChecked());
}


MainWindow::~MainWindow()
{
}
