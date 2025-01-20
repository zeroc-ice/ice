// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_FILE_CACHE_H
#define ICEGRID_FILE_CACHE_H

#include "Ice/BuiltinSequences.h"
#include "Ice/CommunicatorF.h"

namespace IceGrid
{
    class FileCache
    {
    public:
        FileCache(const Ice::CommunicatorPtr&);

        std::int64_t getOffsetFromEnd(const std::string&, int);
        bool read(const std::string&, std::int64_t, int, std::int64_t&, Ice::StringSeq&);

    private:
        const int _messageSizeMax;
    };

};

#endif
