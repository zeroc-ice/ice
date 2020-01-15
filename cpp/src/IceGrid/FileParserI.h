//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_FILE_PARSERI_H
#define ICE_GRID_FILE_PARSERI_H

#include <IceGrid/FileParser.h>

namespace IceGrid
{

class FileParserI : public FileParser
{
public:

    ApplicationDescriptor
    parse(std::string file, std::shared_ptr<AdminPrx> admin, const Ice::Current&) override;
};

}

#endif
