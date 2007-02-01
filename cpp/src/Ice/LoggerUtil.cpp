// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerUtil.h>
#include <Ice/Logger.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::Print::Print(const LoggerPtr& logger) :
    _logger(logger)
{
}

Ice::Print::~Print()
{
    flush();
}

void
Ice::Print::flush()
{
    string s = _str.str();
    if(!s.empty())
    {
        _logger->print(s);
    }
    _str.str("");
}

ostringstream&
Ice::Print::__str()
{
    return _str;
}

Print&
Ice::operator<<(Print& out, ios_base& (*val)(ios_base&))
{
    out.__str() << val;
    return out;
}

Ice::Warning::Warning(const LoggerPtr& logger) :
    _logger(logger)
{
}

Ice::Warning::~Warning()
{
    flush();
}

void
Ice::Warning::flush()
{
    string s = _str.str();
    if(!s.empty())
    {
        _logger->warning(s);
    }
    _str.str("");
}

ostringstream&
Ice::Warning::__str()
{
    return _str;
}

Warning&
Ice::operator<<(Warning& out, ios_base& (*val)(ios_base&))
{
    out.__str() << val;
    return out;
}

Ice::Error::Error(const LoggerPtr& logger) :
    _logger(logger)
{
}

Ice::Error::~Error()
{
    flush();
}

void
Ice::Error::flush()
{
    string s = _str.str();
    if(!s.empty())
    {
        _logger->error(s);
    }
    _str.str("");
}

ostringstream&
Ice::Error::__str()
{
    return _str;
}

Error&
Ice::operator<<(Error& out, ios_base& (*val)(ios_base&))
{
    out.__str() << val;
    return out;
}

Ice::Trace::Trace(const LoggerPtr& logger, const string& category) :
    _logger(logger),
    _category(category)
{
}

Ice::Trace::~Trace()
{
    flush();
}

void
Ice::Trace::flush()
{
    string s = _str.str();
    if(!s.empty())
    {
        _logger->trace(_category, s);
    }
    _str.str("");
}

ostringstream&
Ice::Trace::__str()
{
    return _str;
}

Trace&
Ice::operator<<(Trace& out, ios_base& (*val)(ios_base&))
{
    out.__str() << val;
    return out;
}
