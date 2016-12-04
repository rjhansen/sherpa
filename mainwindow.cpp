#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow { parent },
    ui { new Ui::MainWindow }
{
    ui->setupUi(this);
#ifdef __APPLE__
    gnupgDir = QDir::homePath() + QDir::separator() + QString(".gnupg");
#endif

    ui->statusBar->showMessage(
                QDir(gnupgDir).exists() ?
                (QString("GnuPG directory found at ") + gnupgDir) :
                    QString("No GnuPG installation was found"), 5000);
    ui->goBtn->setEnabled(false);

    connect(ui->fileBtn, &QPushButton::clicked, this, &MainWindow::changeFileClicked);
    connect(ui->comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::comboChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::comboChanged(int idx)
{
    ui->statusBar->clearMessage();
    ui->goBtn->setEnabled(false);
    ui->filename->setText("(no filename entered)");
}

void MainWindow::changeFileClicked()
{
    ui->statusBar->clearMessage();
    if (ui->comboBox->currentIndex() == 1) {
        auto filename = QFileDialog::getOpenFileName(this, tr("Restore from…"), QDir::homePath(), tr("GnuPG backups (*.zip)"));
        QFileInfo qfi(filename);
        if (qfi.exists() && qfi.isReadable() && filename.endsWith(".zip")) {
            ui->goBtn->setEnabled(true);
            ui->filename->setText(filename);
            ui->statusBar->showMessage("Click “Go” to restore from this backup.");
        } else {
            ui->goBtn->setEnabled(false);
            ui->filename->setText("(no filename entered)");
            if (filename != "")
                ui->statusBar->showMessage("This file is not readable.");
        }
    }
    else {
        QFileInfo gpgdirinfo(gnupgDir);
        if (! gpgdirinfo.isDir()) {
            ui->statusBar->showMessage("Could not find the GnuPG data directory.");
            ui->filename->setText("(no filename entered)");
            ui->goBtn->setEnabled(false);
        } else {
            auto filename = QFileDialog::getSaveFileName(this, tr("Backup to…"), QDir::homePath(), tr("GnuPG backups (*.zip)"));
            if (! filename.endsWith(".zip")) {
                ui->filename->setText("(no filename entered)");
                ui->goBtn->setEnabled(false);
            }
            else {
                ui->filename->setText(filename);
                ui->statusBar->showMessage("Click “Go” to back up the GnuPG data directory.");
                ui->goBtn->setEnabled(true);
            }
        }
    }
}
