// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOGGER_H
#define ICEE_LOGGER_H

#include <IceE/LoggerF.h>

namespace Ice
{

class ICE_API Logger : public ::IceUtil::Shared
{
public:

    virtual void print(const ::std::string&) = 0;
    virtual void trace(const ::std::string&, const ::std::string&) = 0;
    virtual void warning(const ::std::string&) = 0;
    virtual void error(const ::std::string&) = 0;
};

}

#endif
