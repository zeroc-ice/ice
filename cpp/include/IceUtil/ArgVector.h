// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

namespace IceUtil
{

class ICE_UTIL_API ArgVector
{
public:

    ArgVector(int argc, char* argv[]);
    ArgVector(const ::std::vector< ::std::string>&);
    ArgVector(const ArgVector&);
    ArgVector& operator=(const ArgVector&);
    ~ArgVector();

    void setNoDelete();

    int argc;
    char** argv;

private:

    int _origArgc;
    bool _noDelete;

    void copyVec(int, int, char**);
    void copyVec(const ::std::vector< ::std::string>&);
    void swap(ArgVector&) throw();
};

}

#endif
