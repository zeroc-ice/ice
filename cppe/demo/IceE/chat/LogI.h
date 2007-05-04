// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LOG_I_H
#define LOG_I_H

#include <IceE/IceE.h>

class LogI : public Ice::Logger
{
public:

    LogI();

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

    void message(const std::string&);
    void setHandle(HWND);

private:

    void post(const std::string&);

    std::string _buffer;
    HWND _hwnd;
};

typedef IceUtil::Handle<LogI> LogIPtr;

#endif
