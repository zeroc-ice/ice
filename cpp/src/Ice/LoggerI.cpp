// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/LoggerI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::LoggerI::trace(const string& category, const string& message)
{
    IceUtil::Mutex::Lock sync(*this);
    ostringstream thread;
    thread << dec << getpid();
    string s = "[ " + category + ": Thread(" + thread.str() + ") "+ message + " ]";
    string::size_type idx = 0;
    while ((idx = s.find("\n", idx)) != string::npos)
    {
	s.insert(idx + 1, "  ");
	++idx;
    }
    cerr << s << endl;
}

void
Ice::LoggerI::warning(const string& message)
{
    IceUtil::Mutex::Lock sync(*this);
    cerr << "warning: " << message << endl;
}

void
Ice::LoggerI::error(const string& message)
{
    IceUtil::Mutex::Lock sync(*this);
    cerr << "error: " << message << endl;
}

void
Ice::LoggerI::destroy()
{
    // Nothing to do
}
