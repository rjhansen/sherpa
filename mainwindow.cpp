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
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "minizip/unzip.h"
#include "minizip/zip.h"

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
#include <sys/types.h>
#include <sys/stat.h>
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

vector<uint8_t> extractKeyring(bool type = false)
{
    gpgme_error_t err;
    gpgme_data_t data;
    vector<uint8_t> buffer;
    off_t sz = 0;
    QString keyring;

    err = gpgme_data_new(&data);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        gpgme_release(ctx);
        throw GpgmeException(err);
    }

    err = gpgme_op_export(ctx, NULL, type ? GPGME_EXPORT_MODE_SECRET : 0, data);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        gpgme_data_release(data);
        gpgme_release(ctx);
        throw GpgmeException(err);
    }

    sz = gpgme_data_seek(data, 0, SEEK_END);
    buffer.resize(static_cast<size_t>(sz));
    gpgme_data_seek(data, 0, SEEK_SET);
    sz = gpgme_data_read(data, &buffer[0], buffer.size() - 1);

    gpgme_data_release(data);

    return buffer;
}

list<QString> getSubdirsAndFiles(QString basedir)
{
    list<QString> rv;

    for (auto const& fn : {
             "gpg-agent.conf", "gpg.conf", "pubring.gpg", "secring.gpg",
             "trustdb.gpg", "pubring.kbx", "sshcontrol", "dirmngr.conf",
             "gpa.conf", "scdaemon.conf", "gpgsm.conf", "policies.txt",
             "trustlist.txt", "scd-event", "tofu.db", "gpg.conf-2.1",
             "gpg.conf-2.0", "gpg.conf-2", "gpg.conf-1.4", "gpg.conf-1" }) {
        QFileInfo qfi(basedir + QDir::separator() + fn);
        if (qfi.exists() && qfi.isFile() && qfi.isReadable())
            rv.push_back(qfi.fileName());
    }

    QString dirname = "openpgp-revocs.d";
    auto qd = QDir(basedir + QDir::separator() + dirname);
    QRegExp revocs("^[A-Fa-f0-9]{40}\\.rev$");
    if (qd.exists() && qd.isReadable()) {
        qd.setFilter(QDir::Files);
        auto files = qd.entryInfoList();
        for (const auto& qfi : files)
            if (qfi.isReadable() && revocs.exactMatch(qfi.fileName()))
                rv.push_back(dirname + QDir::separator() + qfi.fileName());
    }

    dirname = "private-keys-v1.d";
    qd = QDir(basedir + QDir::separator() + dirname);
    QRegExp privkeys("^[A-Fa-f0-9]{40}\\.key$");
    if (qd.exists() && qd.isReadable()) {
        qd.setFilter(QDir::Files);
        auto files = qd.entryInfoList();
        for (const auto& qfi : files)
            if (qfi.isReadable() && privkeys.exactMatch(qfi.fileName()))
                rv.push_back(dirname + QDir::separator() + qfi.fileName());
    }

    return rv;
}

map<QString, vector<uint8_t> > getFilesAndContents(QString basedir)
{
    map<QString, vector<uint8_t> > rv;

    for (auto const& fn : getSubdirsAndFiles(basedir)) {
        QFile fh(basedir + QDir::separator() + fn);
        if (!fh.open(QIODevice::ReadOnly))
            continue;
        auto qba = fh.readAll();
        rv[fn] = vector<uint8_t>(qba.cbegin(), qba.cend());
    }
    rv["public_keys.bin"] = extractKeyring(false);
    rv["private_keys.bin"] = extractKeyring(true);
    return rv;
}

map<QString, QByteArray> readSherpaFile(QString zipfilename)
{
    map<QString, QByteArray> rv;
    std::array<char, 80> comment_buf;
    std::array<char, 1024> filename;

    std::fill(comment_buf.begin(), comment_buf.end(), '\0');
    std::fill(filename.begin(), filename.end(), '\0');

    auto fn_qba = zipfilename.toUtf8();
    auto zipfile = unzOpen(fn_qba.data());
    int zip_err;

    unzGetGlobalComment(zipfile, &comment_buf[0], comment_buf.size());
    QString version(&comment_buf[0]);
    if (!version.startsWith("Sherpa")) {
        unzClose(zipfile);
        throw NotASherpa();
    }

    zip_err = unzGoToFirstFile(zipfile);
    if (zip_err != UNZ_OK) {
        unzClose(zipfile);
        throw UncompressError();
    }

    while (zip_err == UNZ_OK) {
        unz_file_info info;
        std::fill(filename.begin(), filename.end(), '\0');

        unzGetCurrentFileInfo(zipfile,
            &info,
            &filename[0],
            filename.size(),
            NULL,
            0,
            &comment_buf[0],
            comment_buf.size());

        if (info.uncompressed_size > (256 * 1048576)) {
            unzClose(zipfile);
            throw UncompressError();
        }

        QByteArray uncompressed(static_cast<int>(info.uncompressed_size), 0);
        if (UNZ_OK != unzOpenCurrentFile(zipfile)) {
            unzClose(zipfile);
            throw UncompressError();
        }
        if (0 > unzReadCurrentFile(zipfile,
                    uncompressed.data(),
                    static_cast<uint32_t>(uncompressed.size()))) {
            unzCloseCurrentFile(zipfile);
            unzClose(zipfile);
            throw UncompressError();
        }
        if (UNZ_CRCERROR == unzCloseCurrentFile(zipfile)) {
            unzClose(zipfile);
            throw UncompressError();
        }

        rv[QString(&filename[0])] = uncompressed;
        zip_err = unzGoToNextFile(zipfile);
    }
    unzClose(zipfile);
    return rv;
}
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
    auto zipfile = zipOpen(fn_qba.data(), APPEND_STATUS_CREATE);
    for (auto const& elem : data) {
        auto this_qba = elem.first.toUtf8();
        zipOpenNewFileInZip(zipfile, this_qba.data(), NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
        zipWriteInFileInZip(zipfile, &elem.second[0], static_cast<uint32_t>(elem.second.size()));
        zipCloseFileInZip(zipfile);
    }
    auto comment = version.toUtf8();
    zipClose(zipfile, comment.data());

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
    } catch (const UncompressError&) {
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

    QFileDevice::Permissions publicPerm =
            QFileDevice::ReadOwner | QFileDevice::WriteOwner |
            QFileDevice::ReadUser  | QFileDevice::WriteUser  |
            QFileDevice::ReadGroup | QFileDevice::ReadOther;
    QFileDevice::Permissions privatePerm =
            QFileDevice::ReadOwner | QFileDevice::WriteOwner |
            QFileDevice::ReadUser  | QFileDevice::WriteUser;
    for (auto const& elem : files) {
        if (elem.first.endsWith(".bin"))
            continue;
        QString fn(gnupgDir + QDir::separator() + elem.first);
        auto contents = elem.second;
        QFileInfo qfi(fn);
        QDir dir = qfi.absoluteDir();
        if (! dir.exists())
            dir.mkpath(".");
        QFile fh(fn);
        if (fh.exists())
            fh.remove();
        fh.open(QFile::WriteOnly);
        fh.write(contents);
        fh.close();
        if (QFileInfo(fn).absolutePath().endsWith("private-keys-v1.d"))
            fh.setPermissions(privatePerm);
        else
            fh.setPermissions(publicPerm);
    }

#ifdef WIN32
#elif __APPLE__ || __UNIX__
    array<char, PATH_MAX> buffer;
    if (QDir(gnupgDir + "/openpgp-revocs.d").exists()) {
        std::fill(buffer.begin(), buffer.end(), 0);
        strncpy(&buffer[0], gpgme_get_dirinfo("homedir"), buffer.size());
        strcat(&buffer[0], "/openpgp-revocs.d");
        chmod(&buffer[0], 0755);
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
    if ((gpgType == GpgType::modern && !version.endsWith("Modern")) ||
            (gpgType != GpgType::modern && version.endsWith("Modern")))
    {
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
