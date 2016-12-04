#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "gpgme.h"
#include <locale.h>

namespace {
bool bomb_early()
{
    QMessageBox::critical(nullptr,
                          "Could not find GnuPG",
                          "GnuPG doesn't appear to be installed.",
                          QMessageBox::Abort,
                          QMessageBox::Abort);
    return false;
}


bool init_gpgme()
{
    gpgme_engine_info_t info;

    setlocale(LC_ALL, "");
    gpgme_check_version(nullptr);
    gpgme_set_locale(nullptr, LC_CTYPE, setlocale(LC_CTYPE, nullptr));
#ifdef LC_MESSAGES
    gpgme_set_locale(nullptr, LC_MESSAGES, setlocale(LC_MESSAGES, nullptr));
#endif

    auto err1 { gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP) };
    auto err2 { gpgme_get_engine_info(&info) };

    if (! (gpg_err_code(err1) == GPG_ERR_NO_ERROR) &&
          (gpg_err_code(err2) == GPG_ERR_NO_ERROR))
        return bomb_early();

    while (info && info->protocol != GPGME_PROTOCOL_OpenPGP)
        info = info->next;
    if (!(info && info->version))
        return bomb_early();

    return true;
}
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (init_gpgme())
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
}
