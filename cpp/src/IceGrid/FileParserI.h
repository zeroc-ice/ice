// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
