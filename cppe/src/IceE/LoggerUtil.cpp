// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LoggerUtil.h>
#include <IceE/Logger.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

IceE::Print::Print(const LoggerPtr& logger) :
    _logger(logger)
{
}

IceE::Print::~Print()
{
    flush();
}

void
IceE::Print::flush()
{
    if(!_str.empty())
    {
	_logger->print(_str);
    }
    _str = "";;
}

string&
IceE::Print::__str()
{
    return _str;
}

IceE::Warning::Warning(const LoggerPtr& logger) :
    _logger(logger)
{
}

IceE::Warning::~Warning()
{
    flush();
}

void
IceE::Warning::flush()
{
    if(!_str.empty())
    {
	_logger->warning(_str);
    }
    _str = "";
}

string&
IceE::Warning::__str()
{
    return _str;
}

IceE::Error::Error(const LoggerPtr& logger) :
    _logger(logger)
{
}

IceE::Error::~Error()
{
    flush();
}

void
IceE::Error::flush()
{
    if(!_str.empty())
    {
	_logger->error(_str);
    }
    _str = "";
}

string&
IceE::Error::__str()
{
    return _str;
}

IceE::Trace::Trace(const LoggerPtr& logger, const string& category) :
    _logger(logger),
    _category(category)
{
}

IceE::Trace::~Trace()
{
    flush();
}

void
IceE::Trace::flush()
{
    if(!_str.empty())
    {
	_logger->trace(_category, _str);
    }
    _str = "";
}

string&
IceE::Trace::__str()
{
    return _str;
}
