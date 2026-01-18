#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include "processfile.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QLineEdit *inputFolder;
    QPushButton *selectInputBtn;

    // a.
    QLineEdit *fileFormat;
    // b.
    QCheckBox *deleteEnterFile;
    // c.
    QLineEdit *folderSave;
    QPushButton *selectSaveBtn;
    // d.
    QComboBox *rewriteMode;
    // e.
    QRadioButton *radioOneTime;
    QRadioButton *radioTimer;
    // f.
    QSpinBox *intervalTimer;
    // g.
    QLineEdit *xorKey;

    QPushButton *Start;
    QPushButton *Stop;
    QProgressBar *processBar;
    QTextEdit *log;

    void setupUi();

    QThread *thread = nullptr;
    ProcessFile *processor = nullptr;

private slots:
    void onStartClicked();
    void onStopClicked();
    void updateTimerState();
};
#endif // MAINWINDOW_H
