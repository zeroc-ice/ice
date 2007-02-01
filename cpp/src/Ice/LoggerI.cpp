// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Time.h>
#include <Ice/LoggerI.h>
#include <IceUtil/StaticMutex.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

static IceUtil::StaticMutex outputMutex = ICE_STATIC_MUTEX_INITIALIZER;

Ice::LoggerI::LoggerI(const string& prefix)
{
    if(!prefix.empty())
    {
        _prefix = prefix + ": ";
    }
}

void
Ice::LoggerI::print(const string& message)
{
    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << message << endl;
}

void
Ice::LoggerI::trace(const string& category, const string& message)
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
Ice::LoggerI::warning(const string& message)
{
    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << IceUtil::Time::now().toDateTime() << " " << _prefix << "warning: " << message << endl;
}

void
Ice::LoggerI::error(const string& message)
{
    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << IceUtil::Time::now().toDateTime() << " " << _prefix << "error: " << message << endl;
}
