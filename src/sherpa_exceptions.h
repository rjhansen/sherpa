#ifndef SHERPA_EXCEPTIONS_H
#define SHERPA_EXCEPTIONS_H

#include <exception>
#include <gpgme.h>

class ZipError : public std::exception {
public:
    const char* what() const noexcept { return "zip error"; }
};

class FileNotInZip : public ZipError {
public:
    const char* what() const noexcept { return "file not in zip"; }
};

class CRCError : public ZipError {
public:
    const char* what() const noexcept { return "bad crc"; }
};

class GnuPGNotFound : public std::exception {
    const char* what() const noexcept { return "GnuPG not found"; }
};

class GpgmeException : public std::exception {
public:
    GpgmeException(gpgme_error_t e)
        : error{ e }
    {
    }
    const char* what() const noexcept { return "gpgme error"; }
    gpgme_error_t error;
};

class NotASherpa : public std::exception {
    const char* what() const noexcept { return "not a sherpa file"; }
};

#endif
