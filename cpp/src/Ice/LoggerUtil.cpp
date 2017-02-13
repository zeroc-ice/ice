
// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

namespace IceUtilInternal
{

extern bool printStackTraces;

}

string
Ice::LoggerOutputBase::str() const
{
    return _os.str();
}

ostringstream&
Ice::LoggerOutputBase::_stream()
{
    return _os;
}

Ice::LoggerOutputBase&
Ice::operator<<(Ice::LoggerOutputBase& out, ios_base& (*val)(ios_base&))
{
    out._stream() << val;
    return out;
}

Ice::LoggerOutputBase&
Ice::loggerInsert(Ice::LoggerOutputBase& out, const IceUtil::Exception& ex)
{
    if(IceUtilInternal::printStackTraces)
    {
        out._stream() << ex.what() << '\n' << ex.ice_stackTrace();
    }
    else
    {
        out._stream() << ex.what();
    }
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
    string s = _stream().str();
    if(!s.empty())
    {
        _logger->trace(_category, s);
    }
    _stream().str("");
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
