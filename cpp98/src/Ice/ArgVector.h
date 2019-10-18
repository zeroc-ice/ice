//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ARGVECTOR_H
#define ICE_ARGVECTOR_H

#include <Ice/Config.h>
#include <vector>
#include <string>

namespace IceInternal
{

class ICE_API ArgVector
{
public:

    ArgVector(int argc, const char* const argv[]);
    ArgVector(const ::std::vector< ::std::string>&);
    ArgVector(const ArgVector&);
    ArgVector& operator=(const ArgVector&);
    ~ArgVector();

    int argc;
    char** argv;

private:

    ::std::vector< ::std::string> _args;
    void setupArgcArgv();
};

}

#endif
