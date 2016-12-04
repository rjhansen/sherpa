#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "gpgme.h"
#include <locale.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow { parent },
    ui { new Ui::MainWindow }
{
    ui->setupUi(this);
    const char* homedir = gpgme_get_dirinfo("homedir");
    gnupgDir = homedir ? QString(homedir) : QString::null;

    ui->statusBar->showMessage(tr("Click “Choose file” to proceed."));
    ui->goBtn->setEnabled(false);

    ui->comboBox->setCurrentIndex(QDir(gnupgDir).exists() ? 0 : 1);

    connect(ui->fileBtn, &QPushButton::clicked, this, &MainWindow::changeFileClicked);
    connect(ui->comboBox,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this,
            &MainWindow::comboChanged);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QApplication::quit);
    connect(ui->quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(ui->goBtn, &QPushButton::clicked, this, &MainWindow::go);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::comboChanged(int idx)
{
    switch (idx)
    {
    case 0:
        if (! QDir(gnupgDir).exists()) {
            QMessageBox::warning(this,
                                 tr("No profile found"),
                                 tr("There is no profile to back up."),
                                 QMessageBox::Ok,
                                 QMessageBox::Ok);
            ui->comboBox->setCurrentIndex(1);
            return;
        }
        ui->fileBtn->setEnabled(true);
        break;
    case 1:
        ui->fileBtn->setEnabled(true);
        break;
    }

    ui->statusBar->clearMessage();
    ui->goBtn->setEnabled(false);
    ui->lineEdit->setText("");
    ui->statusBar->clearMessage();
    ui->statusBar->showMessage(tr("Click “Choose file” to proceed."));
}

void MainWindow::changeFileClicked()
{
    ui->statusBar->clearMessage();
    QString filename = QString::null;

    switch (ui->comboBox->currentIndex())
    {
    case 0: // backup
        filename = QFileDialog::getSaveFileName(this,
                                                tr("Backup to…"),
                                                QDir::homePath(),
                                                tr("GnuPG backups (*.zip)"));
        if (filename != "")
        {
            filename += filename.endsWith(".zip") ? "" : ".zip";
            ui->lineEdit->setText(filename);
            ui->statusBar->showMessage("Click “Go” to back up the GnuPG data directory.");
            ui->goBtn->setEnabled(true);
            return;
        }
        ui->lineEdit->setText("");
        ui->goBtn->setEnabled(false);
        ui->statusBar->showMessage(tr("Click “Choose file” to proceed."));
        return;

    case 1: // restore
        filename = QFileDialog::getOpenFileName(this,
                                                tr("Restore from…"),
                                                QDir::homePath(),
                                                tr("GnuPG backups (*.zip)"));
        if (filename != "")
        {
            filename += filename.endsWith(".zip") ? "" : ".zip";
            if (QFileInfo(filename).exists() && QFileInfo(filename).isReadable()) {
                ui->goBtn->setEnabled(true);
                ui->lineEdit->setText(filename);
                ui->statusBar->showMessage("Click “Go” to restore from this backup.");
                return;
            }
            QMessageBox::warning(this,
                                 tr("File unreadable"),
                                 tr("The file could not be read."),
                                 QMessageBox::Ok,
                                 QMessageBox::Ok);
            ui->goBtn->setEnabled(false);
            ui->lineEdit->setText("");
            ui->statusBar->showMessage(tr("Click “Choose file” to proceed."));
            return;
        }

        ui->goBtn->setEnabled(false);
        ui->lineEdit->setText("");
        ui->statusBar->showMessage(tr("Click “Choose file” to proceed."));
        return;
    }
}

void MainWindow::go()
{
    switch (ui->comboBox->currentIndex())
    {
    case 0: // backup
        backupTo();
        break;
    case 1: // restore
        restoreFrom();
        break;
    }
}
