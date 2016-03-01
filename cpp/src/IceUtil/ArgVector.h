// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_ARGVECTOR_H
#define ICE_UTIL_ARGVECTOR_H

#include <IceUtil/Config.h>
#include <vector>
#include <string>

namespace IceUtilInternal
{

class ICE_UTIL_API ArgVector
{
public:

    ArgVector(int argc, char* const argv[]);
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
