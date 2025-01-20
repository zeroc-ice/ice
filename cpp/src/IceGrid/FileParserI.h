// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_FILE_PARSERI_H
#define ICEGRID_FILE_PARSERI_H

#include "IceGrid/FileParser.h"

namespace IceGrid
{
    class FileParserI final : public FileParser
    {
    public:
        ApplicationDescriptor parse(std::string file, std::optional<AdminPrx> admin, const Ice::Current&) final;
    };
}

#endif
