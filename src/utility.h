#ifndef UTILITY_H
#define UTILITY_H

#include "sherpa_exceptions.h"
#include <QByteArray>
#include <QString>
#include <cstdint>
#include <map>
#include <vector>

std::map<QString, std::vector<uint8_t> > getFilesAndContents(QString basedir);

std::map<QString, QByteArray> readSherpaFile(QString zipfilename);

#endif
