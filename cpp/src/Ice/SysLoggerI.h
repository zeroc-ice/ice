// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_SYS_LOGGER_I_H
#define ICE_SYS_LOGGER_I_H

#include <IceUtil/Mutex.h>
#include <Ice/Logger.h>

namespace Ice
{

class SysLoggerI : public Logger, public ::IceUtil::Mutex
{
public:

    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
};

}

#endif
