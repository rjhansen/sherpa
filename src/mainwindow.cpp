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

#include "mainwindow.h"
#include "aboutdialog.h"
#include "sherpa_exceptions.h"
#include "ui_mainwindow.h"
#include "utility.h"
#include "zipper.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegExp>
#include <QStandardPaths>
#include <algorithm>
#include <array>
#include <list>
#include <locale.h>
#include <map>
#include <string>
#include <vector>

#ifdef WIN32
#elif __APPLE__ || __UNIX__
#include <sys/stat.h>
#include <sys/types.h>
#endif

using std::vector;
using std::list;
using std::remove_if;
using std::transform;
using std::map;
using std::array;

namespace {
QString clickFile = QCoreApplication::translate("sherpa", "Click “Choose file” to continue.");
QString goBackup = QCoreApplication::translate("sherpa", "Click “Go” to back up your GnuPG folder.");
QString goRestore = QCoreApplication::translate("sherpa", "Click “Go” to restore your GnuPG folder.");
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow{ parent }
    , ui{ new Ui::MainWindow }
    , gnupgDir{ gpgme_get_dirinfo("homedir") }
{
    gpgme_engine_info_t info;

    if (gpg_err_code(gpgme_get_engine_info(&info)) == GPG_ERR_NO_ERROR) {
        while (info && info->protocol != GPGME_PROTOCOL_OpenPGP)
            info = info->next;
        if (!(info && info->version))
            throw GnuPGNotFound();
        auto elements = QString(info->version).split(".").toVector();
        uint32_t version = static_cast<uint8_t>(elements.at(0).toUInt());
        version = (version << 8) + static_cast<uint8_t>(elements.at(1).toUInt());
        version = (version << 8) + static_cast<uint8_t>(elements.at(2).toUInt());
        gpgType = version < 0x00020000 ? GpgType::classic : version < 0x00020100 ? GpgType::stable : GpgType::modern;
    }

    ui->setupUi(this);
    setWindowTitle("Sherpa " FULL_VERSION);
    ui->comboBox->setCurrentIndex(QDir(gnupgDir).exists() ? 0 : 1);
    filenames.at(0) = "";
    filenames.at(1) = "";

    connect(ui->fileBtn, &QPushButton::clicked, this, &MainWindow::changeFileClicked);
    connect(ui->comboBox,
        static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
        this,
        &MainWindow::comboChanged);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QApplication::quit);
    connect(ui->actionAbout, &QAction::triggered, [=]() { auto x = new AboutDialog(this); x->show(); });
    connect(ui->quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(ui->goBtn, &QPushButton::clicked, this, &MainWindow::go);

    updateUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::comboChanged(int idx)
{
    ui->lineEdit->setText(filenames.at(static_cast<size_t>(idx)));
    updateUI();
}

void MainWindow::changeFileClicked()
{
    QString filename = "";

    switch (ui->comboBox->currentIndex()) {
    case 0: // backup
        filename = QFileDialog::getSaveFileName(this,
            tr("Backup to…"),
            QDir::homePath(),
            tr("GnuPG backups (*.zip)"));
        filename = (filename == "" || filename.endsWith(".zip"))
            ? filename
            : (filename + ".zip");
        filenames.at(0) = filename;
        break;

    case 1: // restore
        filename = QFileDialog::getOpenFileName(this,
            tr("Restore from…"),
            QDir::homePath(),
            tr("GnuPG backups (*.zip)"));
        filename = (filename == "" || filename.endsWith(".zip"))
            ? filename
            : (filename + ".zip");
        filenames.at(1) = filename;
        break;
    }
    ui->lineEdit->setText(filename);
    updateUI();
}

void MainWindow::go()
{
    ui->goBtn->setEnabled(false);
    switch (ui->comboBox->currentIndex()) {
    case 0: // backup
        backupTo();
        break;
    case 1: // restore
        restoreFrom();
        break;
    }
    ui->goBtn->setEnabled(true);
}

void MainWindow::backupTo()
{
    map<QString, vector<uint8_t> > data;
    try {
        data = getFilesAndContents(gnupgDir);
    } catch (const GpgmeException&) {
        QMessageBox::critical(this,
            tr("A haiku for you!"),
            tr("Good days and bad days.\n"
               "GPGME’s having\n"
               "The latter.  Error!\n\n"),
            QMessageBox::Abort,
            QMessageBox::Abort);
        qApp->exit(1);
    }

    QString version = "Sherpa 1.0 backing up GnuPG ";
    switch (gpgType) {
    case GpgType::classic:
        version += "Classic";
        break;
    case GpgType::stable:
        version += "Stable";
        break;
    case GpgType::modern:
        version += "Modern";
        break;
    }

    QFile file(ui->lineEdit->text());
    if (file.exists())
        file.remove();

    auto fn_qba = ui->lineEdit->text().toUtf8();

    {
        OZipFile ozf{ fn_qba.data() };
        ozf.setGlobalComment(version.toUtf8().data());
        for (auto const& elem : data) {
            const char* name = elem.first.toUtf8().data();
            const auto& data = elem.second;
            ozf.insert(name, data.cbegin(), data.cend());
        }
    }

    QMessageBox::information(this,
        tr("Success"),
        tr("You have successfully backed up your GnuPG user data."
           "\n\n"
           "Click ‘Ok’ to exit."),
        QMessageBox::Ok,
        QMessageBox::Ok);
    qApp->exit(0);
}

void MainWindow::restoreFrom()
{
    map<QString, QByteArray> files;

    try {
        files = readSherpaFile(ui->lineEdit->text());
    } catch (const ZipError&) {
        QMessageBox::information(this,
            tr("File corruption"),
            tr("An error occurred while uncompressing this file."),
            QMessageBox::Ok,
            QMessageBox::Ok);
        return;
    } catch (const NotASherpa&) {
        QMessageBox::information(this,
            tr("Not a GnuPG backup"),
            tr("This file does not appear to hold a GnuPG backup profile."),
            QMessageBox::Ok,
            QMessageBox::Ok);
        return;
    }

    QFileDevice::Permissions privatePerm = QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser;
    for (auto const& elem : files) {
        if (elem.first.endsWith(".bin"))
            continue;
        QString fn(gnupgDir + QDir::separator() + elem.first);
        auto contents = elem.second;
        QFileInfo qfi(fn);
        QDir dir = qfi.absoluteDir();
        if (!dir.exists())
            dir.mkpath(".");
        QFile fh(fn);
        if (fh.exists())
            fh.remove();
        fh.open(QFile::WriteOnly);
        fh.write(contents);
        fh.close();
        fh.setPermissions(privatePerm);
    }

#ifdef WIN32
#elif __APPLE__ || __UNIX__
    array<char, PATH_MAX> buffer;
    if (QDir(gnupgDir + "/openpgp-revocs.d").exists()) {
        std::fill(buffer.begin(), buffer.end(), 0);
        strncpy(&buffer[0], gpgme_get_dirinfo("homedir"), buffer.size());
        strcat(&buffer[0], "/openpgp-revocs.d");
        chmod(&buffer[0], 0700);
    }
    if (QDir(gnupgDir + "/private-keys-v1.d").exists()) {
        std::fill(buffer.begin(), buffer.end(), 0);
        strncpy(&buffer[0], gpgme_get_dirinfo("homedir"), buffer.size());
        strcat(&buffer[0], "/private-keys-v1.d");
        chmod(&buffer[0], 0700);
    }
#endif

    auto fn_qba = ui->lineEdit->text().toUtf8();
    auto zipfile = unzOpen(fn_qba.data());
    std::array<char, 80> comment_buf;
    unzGetGlobalComment(zipfile, &comment_buf[0], comment_buf.size());
    unzClose(zipfile);

    QString version(&comment_buf[0]);
    if ((gpgType == GpgType::modern && !version.endsWith("Modern")) || (gpgType != GpgType::modern && version.endsWith("Modern"))) {
        gpgme_data_t foo;
        const QByteArray& bar = files["public_keys.bin"];
        gpgme_data_new_from_mem(&foo, bar.data(), bar.size(), 0);
        gpgme_op_import(ctx, foo);
        gpgme_data_release(foo);

        const QByteArray& baz = files["private_keys.bin"];
        gpgme_data_new_from_mem(&foo, baz.data(), baz.size(), 0);
        gpgme_op_import(ctx, foo);
        gpgme_data_release(foo);
    }

    QMessageBox::information(this,
        tr("Success"),
        tr("You have successfully restored your GnuPG user data."
           "\n\n"
           "Click ‘Ok’ to exit."),
        QMessageBox::Ok,
        QMessageBox::Ok);
    qApp->exit(0);
}

void MainWindow::updateUI()
{
    switch (ui->comboBox->currentIndex()) {
    case 0:
        if (!QDir(gnupgDir).exists()) {
            QMessageBox::warning(this,
                tr("No profile found"),
                tr("There is no profile to back up."),
                QMessageBox::Ok,
                QMessageBox::Ok);
            ui->comboBox->setCurrentIndex(1);
            ui->lineEdit->setText("");
            ui->goBtn->setEnabled(false);
            ui->fileBtn->setEnabled(true);
            ui->statusBar->showMessage(clickFile);
            return;
        }

        if (ui->lineEdit->text() != "") {
            auto qfi = QFileInfo(ui->lineEdit->text());
            auto dir = qfi.absoluteDir();
            if (!(dir.exists() && dir.Writable)) {
                QMessageBox::information(this,
                    tr("Directory not writeable"),
                    tr("This directory is not writeable."),
                    QMessageBox::Ok,
                    QMessageBox::Ok);
                ui->goBtn->setEnabled(false);
                ui->statusBar->showMessage(clickFile);
                return;
            }
            if (qfi.isDir()) {
                QMessageBox::information(this,
                    tr("Not a file"),
                    tr("Profiles cannot be backed up to a directory."),
                    QMessageBox::Ok,
                    QMessageBox::Ok);
                ui->goBtn->setEnabled(false);
                ui->statusBar->showMessage(clickFile);
                return;
            }
            if (qfi.exists() && QMessageBox::Cancel == QMessageBox::warning(this, tr("File already exists"), tr("This file already exists.  "
                                                                                                                "If you continue, it will be overwritten."),
                                                           QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel)) {
                ui->lineEdit->setText("");
                ui->goBtn->setEnabled(false);
                ui->statusBar->showMessage(clickFile);
                return;
            }
            ui->goBtn->setEnabled(true);
            ui->statusBar->showMessage(goBackup);
        } else {
            ui->goBtn->setEnabled(false);
            ui->statusBar->showMessage(clickFile);
        }
        break;
    case 1:
        if (!QDir(gnupgDir).exists()) {
            QMessageBox::warning(this,
                tr("No profile found"),
                tr("There is no GnuPG profile folder to restore into."),
                QMessageBox::Ok,
                QMessageBox::Ok);
            ui->lineEdit->setText("");
            ui->goBtn->setEnabled(false);
            ui->statusBar->showMessage("");
            return;
        }
        if (ui->lineEdit->text() != "") {
            auto qfi = QFileInfo(ui->lineEdit->text());
            if (!qfi.isFile()) {
                QMessageBox::information(this,
                    tr("Not a file"),
                    tr("This is not a file."),
                    QMessageBox::Ok,
                    QMessageBox::Ok);
                ui->goBtn->setEnabled(false);
                ui->statusBar->showMessage(clickFile);
                return;
            }
            if (!qfi.isReadable()) {
                QMessageBox::information(this,
                    tr("Not readable"),
                    tr("This file cannot be read."),
                    QMessageBox::Ok,
                    QMessageBox::Ok);
                ui->goBtn->setEnabled(false);
                ui->statusBar->showMessage(clickFile);
                return;
            }

            auto fn_qba = ui->lineEdit->text().toUtf8();
            auto zipfile = unzOpen(fn_qba.data());
            if (NULL == zipfile) {
                QMessageBox::information(this,
                    tr("Not a backup file"),
                    tr("This file does not contain a GnuPG backup."),
                    QMessageBox::Ok,
                    QMessageBox::Ok);
                ui->goBtn->setEnabled(false);
                ui->statusBar->showMessage(clickFile);
                return;
            }

            std::array<char, 80> comment_buf;
            unzGetGlobalComment(zipfile, &comment_buf[0], comment_buf.size());
            unzClose(zipfile);

            QString version(&comment_buf[0]);
            if (!version.startsWith("Sherpa")) {
                QMessageBox::information(this,
                    tr("Not a backup file"),
                    tr("This file does not contain a GnuPG backup."),
                    QMessageBox::Ok,
                    QMessageBox::Ok);
                ui->goBtn->setEnabled(false);
                ui->statusBar->showMessage(clickFile);
                return;
            }

            ui->goBtn->setEnabled(true);
            ui->statusBar->showMessage(goRestore);
            return;
        }
        ui->goBtn->setEnabled(false);
        ui->statusBar->showMessage(clickFile);
    }
}
