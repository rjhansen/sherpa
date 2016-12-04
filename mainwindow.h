#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private slots:
    void changeFileClicked();
    void comboChanged(int idx);
};

#endif // MAINWINDOW_H
