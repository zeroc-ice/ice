// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

Ice::LoggerI::LoggerI(const string& prefix, const string& dateFormat) :
    _dateFormat(dateFormat)
{
    if(!prefix.empty())
    {
	_prefix = prefix + ": ";
    }

    if(_dateFormat == "0")
    {
        _dateFormat = "";
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
    string s = "[ ";
    if(!_dateFormat.empty())
    {
        s += IceUtil::Time::now().toDateTime(_dateFormat) + " ";
    }
    s += _prefix;
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
    string s;
    if(!_dateFormat.empty())
    {
        s += IceUtil::Time::now().toDateTime(_dateFormat) + " ";
    }
    s += _prefix + "warning: " + message;

    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << s << endl;
}

void
Ice::LoggerI::error(const string& message)
{
    string s;
    if(!_dateFormat.empty())
    {
        s += IceUtil::Time::now().toDateTime(_dateFormat) + " ";
    }
    s += _prefix + "error: " + message;

    IceUtil::StaticMutex::Lock sync(outputMutex);

    cerr << s << endl;
}
