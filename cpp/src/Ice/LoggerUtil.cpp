
// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerUtil.h>
#include <Ice/Logger.h>
#include <Ice/Plugin.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>

using namespace std;

namespace IceInternal
{

bool printStackTraces = false;

}

ostringstream&
Ice::LoggerOutputBase::__str()
{
    return _str;
}

Ice::LoggerOutputBase&
Ice::operator<<(Ice::LoggerOutputBase& out, ios_base& (*val)(ios_base&))
{
    out.__str() << val;
    return out;
}

Ice::LoggerOutputBase&
Ice::operator<<(Ice::LoggerOutputBase& out, const std::exception& ex)
{
#ifdef __GNUC__
    if(IceInternal::printStackTraces)
    {
        const ::IceUtil::Exception* exception = dynamic_cast<const ::IceUtil::Exception*>(&ex);
        if(exception)
        {
            out.__str() << exception->what() << '\n' << exception->ice_stackTrace();
            return out;
        }
    }
#endif
    out.__str() << ex.what();
    return out;
}

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
    string s = __str().str();
    if(!s.empty())
    {
        _logger->print(s);
    }
    __str().str("");
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
    string s = __str().str();
    if(!s.empty())
    {
        _logger->warning(s);
    }
    __str().str("");
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
    string s = __str().str();
    if(!s.empty())
    {
        _logger->error(s);
    }
    __str().str("");
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
    string s = __str().str();
    if(!s.empty())
    {
        _logger->trace(_category, s);
    }
    __str().str("");
}

Ice::LoggerPlugin::LoggerPlugin(const CommunicatorPtr& communicator, const LoggerPtr& logger)
{
    if(communicator == 0)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "Communicator cannot be null");
    }

    if(logger == 0)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "Logger cannot be null");
    }

    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    instance->setLogger(logger);
}

void
Ice::LoggerPlugin::initialize()
{
}

void
Ice::LoggerPlugin::destroy()
{
}
