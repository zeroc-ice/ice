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

        std::int64_t getOffsetFromEnd(const std::string&, int);
        bool read(const std::string&, std::int64_t, int, std::int64_t&, Ice::StringSeq&);

    private:
        const int _messageSizeMax;
    };

};

#endif
