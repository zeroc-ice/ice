
// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    return _str.str();
}

ostringstream&
Ice::LoggerOutputBase::iceStr()
{
    return _str;
}

Ice::LoggerOutputBase&
Ice::operator<<(Ice::LoggerOutputBase& out, ios_base& (*val)(ios_base&))
{
    out.iceStr() << val;
    return out;
}

Ice::LoggerOutputBase&
Ice::loggerInsert(Ice::LoggerOutputBase& out, const IceUtil::Exception& ex)
{
    if(IceUtilInternal::printStackTraces)
    {
        out.iceStr() << ex.what() << '\n' << ex.ice_stackTrace();
    }
    else
    {
        out.iceStr() << ex.what();
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
    string s = iceStr().str();
    if(!s.empty())
    {
        _logger->trace(_category, s);
    }
    iceStr().str("");
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
