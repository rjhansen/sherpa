#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <locale.h>

gpgme_ctx_t ctx;

namespace {

void init_gpgme()
{
    gpgme_engine_info_t info;
    gpgme_error_t err;

    setlocale(LC_ALL, "");
    gpgme_check_version(nullptr);
    gpgme_set_locale(nullptr, LC_CTYPE, setlocale(LC_CTYPE, nullptr));
#ifdef LC_MESSAGES
    gpgme_set_locale(nullptr, LC_MESSAGES, setlocale(LC_MESSAGES, nullptr));
#endif

    auto err1 = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
    auto err2 = gpgme_get_engine_info(&info);

    if (! (gpg_err_code(err1) == GPG_ERR_NO_ERROR) &&
          (gpg_err_code(err2) == GPG_ERR_NO_ERROR))
        throw GnuPGNotFound();

    while (info && info->protocol != GPGME_PROTOCOL_OpenPGP)
        info = info->next;
    if (!(info && info->version))
        throw GnuPGNotFound();

    err = gpgme_new(&ctx);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR)
        throw GpgmeException(err);
    err = gpgme_set_protocol(ctx, GPGME_PROTOCOL_OpenPGP);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        gpgme_release(ctx);
        throw GpgmeException(err);
    }
    gpgme_set_armor(ctx, 0);
}
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    try
    {
        init_gpgme();
        MainWindow w;
        w.show();
        int rv = a.exec();
        gpgme_release(ctx);
        return rv;
    }
    catch (const GnuPGNotFound&)
    {
        QMessageBox::critical(nullptr,
                              "Could not find GnuPG",
                              "GnuPG doesn't appear to be installed.",
                              QMessageBox::Abort,
                              QMessageBox::Abort);
        return 1;
    }
    catch (const GpgmeException&)
    {
        QMessageBox::critical(nullptr,
                              "GPGME error",
                              "GPGME could not be initialized.",
                              QMessageBox::Abort,
                              QMessageBox::Abort);
        return 2;
    }
}
