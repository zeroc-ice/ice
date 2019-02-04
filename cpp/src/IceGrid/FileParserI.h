//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_FILE_PARSERI_H
#define ICE_GRID_FILE_PARSERI_H

#include <IceGrid/FileParser.h>

class FileParserI : public IceGrid::FileParser
{
public:

    IceGrid::ApplicationDescriptor
    parse(const std::string& file, const IceGrid::AdminPrx& admin, const Ice::Current&);
};

#endif
