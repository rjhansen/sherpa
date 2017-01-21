#include "zipper.h"

using std::vector;
using std::string;
using std::array;
using std::fill;
using std::find;
using std::map;
using std::back_inserter;
using std::make_pair;

namespace {
string discoverGlobalComment(unzFile fh)
{
    array<char, 1024> cmt;
    fill(cmt.begin(), cmt.end(), 0);
    unzGetGlobalComment(fh, &cmt[0], cmt.size());
    return string(cmt.cbegin(),
        find(cmt.cbegin(), cmt.cend(), '\0'));
}

map<const string, const size_t> makeCatalog(unzFile fh)
{
    map<const string, const size_t> catalog;
    array<char, PATH_MAX> filename;
    array<char, 1024> comment_buf;
    auto err = unzGoToFirstFile(fh);
    unz_file_info info;

    if (UNZ_OK != err) {
        unzClose(fh);
        throw ZipError();
    }
    while (UNZ_OK == err) {
        fill(filename.begin(), filename.end(), '\0');
        unzGetCurrentFileInfo(fh,
            &info,
            &filename[0],
            filename.size(),
            nullptr,
            0,
            &comment_buf[0],
            comment_buf.size());
        string fn(&filename[0]);
        if (catalog.find(fn) == catalog.end())
            catalog.insert(make_pair(fn, info.uncompressed_size));
        err = unzGoToNextFile(fh);
    }
    if (UNZ_END_OF_LIST_OF_FILE != err) {
        unzClose(fh);
        throw ZipError();
    }
    return catalog;
}
}

IZipFile::IZipFile(const std::string fn)
    : fh{ unzOpen(fn.c_str()) }
    , comment{ discoverGlobalComment(fh) }
    , catalog{ makeCatalog(fh) }
{
}

IZipFile::IZipFile(const char* fn)
    : fh{ unzOpen(fn) }
    , comment{ discoverGlobalComment(fh) }
    , catalog{ makeCatalog(fh) }
{
}

IZipFile::~IZipFile()
{
    unzClose(fh);
}

const string& IZipFile::getGlobalComment() const
{
    return comment;
}

const map<const string, const size_t>& IZipFile::getCatalog() const
{
    return catalog;
}

map<string, vector<uint8_t> > IZipFile::extractAll()
{
    map<string, vector<uint8_t> > all_files;
    array<char, PATH_MAX> filename;
    array<char, 1024> comment_buf;
    array<char, 8192> buffer;
    auto err{ unzGoToFirstFile(fh) };
    unz_file_info info;
    int bytesRead{ 0 };

    if (UNZ_OK != err) {
        unzClose(fh);
        throw ZipError();
    }
    while (UNZ_OK == err) {
        fill(filename.begin(), filename.end(), '\0');
        unzGetCurrentFileInfo(fh,
            &info,
            &filename[0],
            filename.size(),
            nullptr,
            0,
            &comment_buf[0],
            comment_buf.size());
        string fn(&filename[0]);

        vector<uint8_t> data;
        auto inserter = back_inserter(data);

        if (UNZ_OK != unzOpenCurrentFile(fh))
            throw ZipError();

        while (0 > (bytesRead = unzReadCurrentFile(fh,
                        &buffer[0], static_cast<uint32_t>(buffer.size()))))
            copy(buffer.cbegin(), buffer.cbegin() + bytesRead, inserter);

        if (bytesRead < 0)
            throw ZipError();
        if (UNZ_CRCERROR == unzCloseCurrentFile(fh))
            throw CRCError();

        all_files.insert(make_pair(fn, data));

        err = unzGoToNextFile(fh);
    }
    if (UNZ_END_OF_LIST_OF_FILE != err) {
        unzClose(fh);
        throw ZipError();
    }
    return all_files;
}

OZipFile::OZipFile(string fn)
    : fh{ zipOpen(fn.c_str(), APPEND_STATUS_CREATE) }
    , comment{ "" }
{
    if (NULL == fh)
        throw ZipError();
}

OZipFile::OZipFile(const char* fn)
    : fh{ zipOpen(fn, APPEND_STATUS_CREATE) }
    , comment{ "" }
{
    if (NULL == fh)
        throw ZipError();
}

OZipFile::~OZipFile()
{
    zipClose(fh, comment.c_str());
}

void OZipFile::setGlobalComment(string cmt)
{
    comment = cmt;
}
