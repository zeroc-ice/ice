// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/LoggerI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Mutex Ice::LoggerI::_globalMutex;

Ice::LoggerI::LoggerI(const string& prefix)
{
    if(!prefix.empty())
    {
	_prefix = prefix + ": ";
    }
}

void
Ice::LoggerI::trace(const string& category, const string& message)
{
    IceUtil::Mutex::Lock sync(_globalMutex);
    string s = "[ " + _prefix + category + ": " + message + " ]";
    string::size_type idx = 0;
    while((idx = s.find("\n", idx)) != string::npos)
    {
	s.insert(idx + 1, "  ");
	++idx;
    }
    cerr << s << endl;
}

void
Ice::LoggerI::warning(const string& message)
{
    IceUtil::Mutex::Lock sync(_globalMutex);
    cerr << _prefix  << "warning: " << message << endl;
}

void
Ice::LoggerI::error(const string& message)
{
    IceUtil::Mutex::Lock sync(_globalMutex);
    cerr  << _prefix << "error: " << message << endl;
}
