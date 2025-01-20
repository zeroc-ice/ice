// Copyright (c) ZeroC, Inc.

#ifndef ICE_ARGVECTOR_H
#define ICE_ARGVECTOR_H

#include "Ice/Config.h"
#include <string>
#include <vector>

namespace IceInternal
{
    class ICE_API ArgVector
    {
    public:
        ArgVector(int argc, const char* const argv[]);
        ArgVector(const std::vector<std::string>&);
        ArgVector(const ArgVector&);
        ArgVector& operator=(const ArgVector&);
        ~ArgVector();

        int argc;
        char** argv;

    private:
        std::vector<std::string> _args;
        void setupArgcArgv();
    };
}

#endif
