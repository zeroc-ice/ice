// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SLICE_CHECKSUM_H
#define SLICE_CHECKSUM_H

#include <Slice/Parser.h>
#include <map>
#include <vector>

namespace Slice
{

typedef std::map<std::string, std::vector<unsigned char> > ChecksumMap;

SLICE_API ChecksumMap createChecksums(const UnitPtr&);

}

#endif
