#ifndef ZIPPER_H
#define ZIPPER_H

#include "minizip/unzip.h"
#include "minizip/zip.h"
#include "sherpa_exceptions.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <map>
#include <string>
#include <vector>

class IZipFile {
public:
    IZipFile(const std::string fn);
    IZipFile(const char* fn);
    virtual ~IZipFile();
    const std::string& getGlobalComment() const;
    const std::map<const std::string, const size_t>& getCatalog() const;
    std::map<std::string, std::vector<uint8_t> > extractAll();

    template <typename T>
    void extract(const std::string fn, T inserter)
    {
        std::array<char, 8192> buffer;
        int bytesRead{ 0 };

        if (UNZ_OK != unzGoToFirstFile(fh))
            throw ZipError();
        if (UNZ_END_OF_LIST_OF_FILE == unzLocateFile(fh, fn.c_str(), 0))
            throw FileNotInZip();
        if (UNZ_OK != unzOpenCurrentFile(fh))
            throw ZipError();

        while (0 > (bytesRead = unzReadCurrentFile(fh,
                        &buffer[0], static_cast<uint32_t>(buffer.size()))))
            copy(buffer.cbegin(), buffer.cbegin() + bytesRead, inserter);

        if (bytesRead < 0)
            throw ZipError();
        if (UNZ_CRCERROR == unzCloseCurrentFile(fh))
            throw CRCError();
    }

private:
    unzFile fh;
    const std::string comment;
    const std::map<const std::string, const size_t> catalog;
};

class OZipFile {
public:
    OZipFile(const std::string fn);
    OZipFile(const char* fn);
    virtual ~OZipFile();
    void setGlobalComment(std::string cmt);
    template <typename T>
    void insert(std::string filename, T begin, T end)
    {
        std::array<uint8_t, 8192> buffer;
        static_assert(sizeof(T) != 1, "only defined for bytes");

        zipOpenNewFileInZip(fh, filename.c_str(),
            nullptr, nullptr, 0, nullptr, 0, nullptr,
            Z_DEFLATED, Z_DEFAULT_COMPRESSION);

        while (begin != end) {
            auto dist = static_cast<int32_t>(std::distance(begin, end));
            auto blockSize = std::min(8192, dist);
            std::copy(begin, begin + blockSize, buffer.begin());
            zipWriteInFileInZip(fh, &buffer[0], blockSize);
            begin += blockSize;
        }

        zipCloseFileInZip(fh);
    }

private:
    zipFile fh;
    std::string comment;
};

#endif
