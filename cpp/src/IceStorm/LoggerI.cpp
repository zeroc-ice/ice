// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Time.h>
#include <Ice/Ice.h>
#include <Ice/Plugin.h>
#include <IceStorm/LoggerI.h>
#include <IceUtil/StaticMutex.h>

#include <IceStorm/Service.h> // For ICE_STORM_API

using namespace std;
using namespace IceStorm;

static IceUtil::StaticMutex outputMutex = ICE_STATIC_MUTEX_INITIALIZER;

LoggerI::LoggerI()
{
}

void
LoggerI::setPrefix(const string& prefix)
{
    _prefix = prefix + ": ";
}

void
LoggerI::print(const string& message)
{
    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << message << endl;
}

void
LoggerI::trace(const string& category, const string& message)
{
    string s = "[ " + IceUtil::Time::now().toDateTime() + " " + _prefix;
    if(!category.empty())
    {
        s += category + ": ";
    }
    s += message + " ]";

    string::size_type idx = 0;
    while((idx = s.find("\n", idx)) != string::npos)
    {
        s.insert(idx + 1, "  ");
        ++idx;
    }

    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << s << endl;
}

void
LoggerI::warning(const string& message)
{
    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << IceUtil::Time::now().toDateTime() << " " << _prefix << "warning: " << message << endl;
}

void
LoggerI::error(const string& message)
{
    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << IceUtil::Time::now().toDateTime() << " " << _prefix << "error: " << message << endl;
}

namespace
{

class LoggerPluginI : public Ice::LoggerPlugin
{
public:

    virtual void initialize() { }

    virtual void destroy() { }

    virtual ::Ice::LoggerPtr getLogger()
    {
        return new LoggerI();
    }
};

}

extern "C"
{

ICE_STORM_API ::Ice::Plugin*
createLogger(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new LoggerPluginI();
}

}
