
// Copyright (c) ZeroC, Inc.

#include "Ice/LoggerUtil.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Instance.h"

using namespace std;

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
Ice::loggerInsert(Ice::LoggerOutputBase& out, const Ice::Exception& ex)
{
    out._stream() << ex;
    return out;
}

Ice::Trace::Trace(LoggerPtr logger, string category) : _logger(std::move(logger)), _category(std::move(category)) {}

Ice::Trace::~Trace() { flush(); }

void
Ice::Trace::flush()
{
    string s = _stream().str();
    if (!s.empty())
    {
        _logger->trace(_category, s);
    }
    _stream().str("");
}

Ice::LoggerPlugin::LoggerPlugin(const CommunicatorPtr& communicator, const LoggerPtr& logger)
{
    if (communicator == nullptr)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "communicator cannot be null");
    }

    if (logger == nullptr)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "logger cannot be null");
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
