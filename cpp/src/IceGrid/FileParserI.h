//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_FILE_PARSERI_H
#define ICE_GRID_FILE_PARSERI_H

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
