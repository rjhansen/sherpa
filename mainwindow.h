#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <exception>
#include <array>
#include "gpgme.h"

namespace Ui {
class MainWindow;
}

extern gpgme_ctx_t ctx;

class GnuPGNotFound : public std::exception {
    const char* what() const noexcept { return "GnuPG not found"; }
};

class GpgmeException : public std::exception {
public:
    GpgmeException(gpgme_error_t e) : error { e } {}
    const char* what() const noexcept { return "gpgme error"; }
    gpgme_error_t error;
};

class UncompressError : public std::exception {
    const char* what() const noexcept { return "corrupt sherpa file"; }
};

class NotASherpa : public std::exception {
    const char* what() const noexcept { return "not a sherpa file"; }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    enum struct GpgType
    {
        classic = 0,
        stable = 1,
        modern = 2
    };
    void backupTo();
    void restoreFrom();
    void updateUI();
    Ui::MainWindow *ui;
    QString gnupgDir;
    std::array<QString, 2> filenames;
    GpgType gpgType;

private slots:
    void changeFileClicked();
    void comboChanged(int idx);
    void go();
};

#endif // MAINWINDOW_H
