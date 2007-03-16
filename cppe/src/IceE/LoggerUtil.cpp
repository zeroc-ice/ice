// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LoggerUtil.h>
#include <IceE/Logger.h>

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
    if(!_str.empty())
    {
	_logger->print(_str);
    }
    _str = "";;
}

string&
Ice::Print::__str()
{
    return _str;
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
    if(!_str.empty())
    {
	_logger->warning(_str);
    }
    _str = "";
}

string&
Ice::Warning::__str()
{
    return _str;
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
    if(!_str.empty())
    {
	_logger->error(_str);
    }
    _str = "";
}

string&
Ice::Error::__str()
{
    return _str;
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
    if(!_str.empty())
    {
	_logger->trace(_category, _str);
    }
    _str = "";
}

string&
Ice::Trace::__str()
{
    return _str;
}
