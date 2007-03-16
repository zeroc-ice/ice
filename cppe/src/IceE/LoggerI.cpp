// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LoggerI.h>
#include <IceE/StaticMutex.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

static IceUtil::StaticMutex globalMutex = ICE_STATIC_MUTEX_INITIALIZER;

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
    IceUtil::StaticMutex::Lock sync(globalMutex);
    fprintf(stderr, "%s\n", message.c_str());
}

void
Ice::LoggerI::trace(const string& category, const string& message)
{
    string s = "[ ";
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

    IceUtil::StaticMutex::Lock sync(globalMutex);
    fprintf(stderr, "%s\n", s.c_str());
}

void
Ice::LoggerI::warning(const string& message)
{
    IceUtil::StaticMutex::Lock sync(globalMutex);
    fprintf(stderr, "%swarning: %s\n", _prefix.c_str(), message.c_str());
}

void
Ice::LoggerI::error(const string& message)
{
    IceUtil::StaticMutex::Lock sync(globalMutex);
    fprintf(stderr, "%serror: %s\n", _prefix.c_str(), message.c_str());
}
