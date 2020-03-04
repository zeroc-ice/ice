//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_FILE_CACHE_H
#define ICE_GRID_FILE_CACHE_H

#include <Ice/BuiltinSequences.h>
#include <Ice/CommunicatorF.h>

namespace IceGrid
{

class FileCache
{
public:

    FileCache(const std::shared_ptr<Ice::Communicator>&);

    long long  getOffsetFromEnd(const std::string&, int);
    bool read(const std::string&, long long , int, long long &, Ice::StringSeq&);

private:

    const int _messageSizeMax;
};

};

#endif
