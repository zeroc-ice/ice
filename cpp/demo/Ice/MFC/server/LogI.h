// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef LOG_I_H
#define LOG_I_H

class LogI : public Ice::Logger
{
public:

    LogI();

    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

    void message(const std::string&);
    void setControl(CEdit*);

private:

    std::string _buffer;
    CEdit* _log;
};

typedef IceUtil::Handle<LogI> LogIPtr;

#endif
