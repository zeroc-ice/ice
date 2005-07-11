// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LOG_I_H
#define LOG_I_H

class LogI : public Ice::Logger
{
public:

    LogI();

    virtual void print(const std::string&);
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
