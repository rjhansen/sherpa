#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString gnupgDir;
    QString filename;
    uint8_t gpgMajor, gpgMinor, gpgBugfix;

private slots:
    void changeFileClicked();
    void comboChanged(int idx);
    void go();
};

#endif // MAINWINDOW_H
