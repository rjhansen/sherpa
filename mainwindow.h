/*
 * Copyright (c) 2016, Robert J. Hansen <rjh@sixdemonbag.org>
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

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
