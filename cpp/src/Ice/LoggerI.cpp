// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StaticMutex.h>
#include <IceUtil/Time.h>
#include <Ice/LoggerI.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

static IceUtil::StaticMutex outputMutex = ICE_STATIC_MUTEX_INITIALIZER;

Ice::LoggerI::LoggerI(const string& prefix, const string& file)
{
    if(!prefix.empty())
    {
        _prefix = prefix + ": ";
    }

    if(!file.empty())
    {
        _out.open(file.c_str(), fstream::out | fstream::app);
        if(!_out.is_open())
        {
            throw InitializationException(__FILE__, __LINE__, "FileLogger: cannot open " + file);
        }
    }
}


Ice::LoggerI::~LoggerI()
{
    if(_out.is_open())
    {
        _out.close();
    }
}

void
Ice::LoggerI::print(const string& message)
{
    write(message);
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

    write(s);
}

void
Ice::LoggerI::warning(const string& message)
{
    write(IceUtil::Time::now().toDateTime() + " " + _prefix + "warning: " + message);
}

void
Ice::LoggerI::error(const string& message)
{
    write(IceUtil::Time::now().toDateTime() + " " + _prefix + "error: " + message);
}

void
Ice::LoggerI::write(const string& message)
{
    IceUtil::StaticMutex::Lock sync(outputMutex);

    if(_out.is_open())
    {
        _out << message << endl;
    }
    else
    {
        cerr << message << endl;
    }
}
