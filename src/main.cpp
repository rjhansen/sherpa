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
#include "sherpa_exceptions.h"
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <array>
#include <locale.h>
#include <set>

#ifdef WIN32
#include <QSettings>
#endif

gpgme_ctx_t ctx;

namespace {

void init_gpgme()
{
    gpgme_engine_info_t info;
    gpgme_error_t err;

    std::set<QString> pathset;
    QStringList newpath;
    auto path = QString::fromUtf8(qgetenv("PATH"));
    for (auto entry : path.split(QDir::listSeparator()))
        if (pathset.find(entry) == pathset.end()) {
            pathset.insert(entry);
            newpath << entry;
        }

#ifdef WIN32
    QSettings settings("HKEY_LOCAL_MACHINE\\Software\\GnuPG",
        QSettings::NativeFormat);
    auto installDir = settings.value("Install Directory").toString();
    std::array<QString, 2> addpaths{ { installDir,
        installDir + QDir::separator() + "bin" } };
#elif __APPLE__ || __UNIX__ || __unix__
    std::array<QString, 5> addpaths{ { QDir::homePath() + QDir::separator() + "bin",
        "/usr/local/gnupg-2.1/bin",
        "/usr/local/bin",
        "/opt/local/bin",
        "/opt/bin" } };
#endif
    for (auto entry : addpaths)
        if (pathset.find(entry) == pathset.end() && QDir(entry).exists()) {
            pathset.insert(entry);
            newpath << entry;
        }

    qputenv("PATH", newpath.join(QDir::listSeparator()).toUtf8());

    setlocale(LC_ALL, "");
    gpgme_check_version(nullptr);
    gpgme_set_locale(nullptr, LC_CTYPE, setlocale(LC_CTYPE, nullptr));
#ifdef LC_MESSAGES
    gpgme_set_locale(nullptr, LC_MESSAGES, setlocale(LC_MESSAGES, nullptr));
#endif

#define NOERR(x) (GPG_ERR_NO_ERROR == gpg_err_code(x))
    bool cond1 = NOERR(gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP));
    bool cond2 = NOERR(gpgme_get_engine_info(&info));
#undef NOERR
    bool cond3 = ((nullptr != gpgme_get_dirinfo("homedir")
        && QDir(QString(gpgme_get_dirinfo("homedir"))).exists()));

    if (!(cond1 && cond2 && cond3))
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

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    try {
        init_gpgme();
        MainWindow w;
        w.show();
        int rv = a.exec();
        gpgme_release(ctx);
        return rv;
    } catch (const GnuPGNotFound&) {
        QMessageBox::critical(nullptr,
            "Could not find GnuPG",
            "GnuPG doesn't appear to be installed.  Install it and run "
            "it once from the command line to initialize it, then re-run "
            "Sherpa.",
            QMessageBox::Abort,
            QMessageBox::Abort);
        return 1;
    } catch (const GpgmeException&) {
        QMessageBox::critical(nullptr,
            "GPGME error",
            "GPGME could not be initialized.",
            QMessageBox::Abort,
            QMessageBox::Abort);
        return 2;
    }
}
