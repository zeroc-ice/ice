// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LoggerI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::Mutex Ice::LoggerI::_globalMutex;

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
    Ice::Mutex::Lock sync(_globalMutex);

    fprintf(stderr, "%s\n", message.c_str());
}

void
Ice::LoggerI::trace(const string& category, const string& message)
{
    Ice::Mutex::Lock sync(_globalMutex);

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
    fprintf(stderr, "%s\n", s.c_str());
}

void
Ice::LoggerI::warning(const string& message)
{
    Ice::Mutex::Lock sync(_globalMutex);
    fprintf(stderr, "%s warning: %s\n", _prefix.c_str(), message.c_str());
}

void
Ice::LoggerI::error(const string& message)
{
    Ice::Mutex::Lock sync(_globalMutex);
    fprintf(stderr, "%s error: %s\n", _prefix.c_str(), message.c_str());
}
