#include "utility.h"
#include "zipper.h"
#include <QDir>
#include <QRegExp>
#include <array>
#include <errno.h>
#include <iterator>
#include <list>

using std::array;
using std::map;
using std::list;
using std::back_inserter;
using std::vector;

extern gpgme_ctx_t ctx;

template <typename T>
void extractKeyring(T inserter, bool type = false)
{
    gpgme_error_t err;
    gpgme_data_t data;
    ssize_t size;
    array<uint8_t, 8192> buffer;

    err = gpgme_data_new(&data);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        throw GpgmeException(err);
    }

    err = gpgme_op_export(ctx,
        NULL, type ? GPGME_EXPORT_MODE_SECRET : 0, data);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        gpgme_data_release(data);
        throw GpgmeException(err);
    }

    gpgme_data_seek(data, 0, SEEK_SET);
    while (0 < (size = gpgme_data_read(data, &buffer[0], buffer.size())))
        std::copy(buffer.cbegin(), buffer.cbegin() + size, inserter);

    gpgme_data_release(data);
    if (0 > size)
        throw GpgmeException(gpgme_error_from_errno(errno));
}

template <typename T>
void getSubdirsAndFiles(QString basedir, T inserter)
{
    for (auto const& fn : {
             "gpg-agent.conf", "gpg.conf", "pubring.gpg", "secring.gpg",
             "trustdb.gpg", "pubring.kbx", "sshcontrol", "dirmngr.conf",
             "gpa.conf", "scdaemon.conf", "gpgsm.conf", "policies.txt",
             "trustlist.txt", "scd-event", "tofu.db", "gpg.conf-2.1",
             "gpg.conf-2.0", "gpg.conf-2", "gpg.conf-1.4", "gpg.conf-1" }) {
        QFileInfo qfi(basedir + QDir::separator() + fn);
        if (qfi.exists() && qfi.isFile() && qfi.isReadable())
            *inserter++ = qfi.fileName();
    }

    QString dirname = "openpgp-revocs.d";
    auto qd = QDir(basedir + QDir::separator() + dirname);
    QRegExp revocs("^[A-Fa-f0-9]{40}\\.rev$");
    if (qd.exists() && qd.isReadable()) {
        qd.setFilter(QDir::Files);
        auto files = qd.entryInfoList();
        for (const auto& qfi : files)
            if (qfi.isReadable() && revocs.exactMatch(qfi.fileName()))
                *inserter++ = dirname + QDir::separator() + qfi.fileName();
    }

    dirname = "private-keys-v1.d";
    qd = QDir(basedir + QDir::separator() + dirname);
    QRegExp privkeys("^[A-Fa-f0-9]{40}\\.key$");
    if (qd.exists() && qd.isReadable()) {
        qd.setFilter(QDir::Files);
        auto files = qd.entryInfoList();
        for (const auto& qfi : files)
            if (qfi.isReadable() && privkeys.exactMatch(qfi.fileName()))
                *inserter++ = dirname + QDir::separator() + qfi.fileName();
    }
}

map<QString, vector<uint8_t> > getFilesAndContents(QString basedir)
{
    map<QString, vector<uint8_t> > rv;
    vector<uint8_t> pub, priv;
    list<QString> files;

    getSubdirsAndFiles(basedir, back_inserter(files));
    extractKeyring(back_inserter(pub), false);
    extractKeyring(back_inserter(priv), true);

    for (auto const& fn : files) {
        QFile fh(basedir + QDir::separator() + fn);
        if (!fh.open(QIODevice::ReadOnly))
            continue;
        auto qba = fh.readAll();
        rv[fn] = vector<uint8_t>(qba.cbegin(), qba.cend());
    }
    rv["public_keys.bin"] = pub;
    rv["private_keys.bin"] = priv;
    return rv;
}

map<QString, QByteArray> readSherpaFile(QString zipfilename)
{
    map<QString, QByteArray> rv;
    IZipFile izf{ zipfilename.toUtf8() };
    QString version(izf.getGlobalComment().c_str());
    if (!version.startsWith("Sherpa")) {
        throw NotASherpa();
    }

    auto entries = izf.extractAll();

    for (const auto entry : entries) {
        const char* ptr = reinterpret_cast<const char*>(&entry.second[0]);
        uint32_t sz = entry.second.size();
        QString key(entry.first.c_str());
        QByteArray value(ptr, sz);
        rv[key] = value;
    }
    return rv;
}
