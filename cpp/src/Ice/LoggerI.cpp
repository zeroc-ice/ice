// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Time.h>
#include <Ice/LoggerI.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <Ice/StringConverter.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

IceUtil::Mutex* outputMutex = 0;

class Init
{
public:

    Init()
    {
        outputMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete outputMutex;
        outputMutex = 0;
    }
};

Init init;

}

Ice::LoggerI::LoggerI(const string& prefix, const string& file)
{
    if(!prefix.empty())
    {
        _prefix = prefix + ": ";
    }

    if(!file.empty())
    {
        //
        // The given file string is execpted to be encoded as UTF8 by
        // the caller, so no need to convert it here.
        //
        _file = file;
        _out.open(file, fstream::out | fstream::app);
        if(!_out.is_open())
        {
            throw InitializationException(__FILE__, __LINE__, "FileLogger: cannot open " + _file);
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
    write(message, false);
}

void
Ice::LoggerI::trace(const string& category, const string& message)
{
    string s = "-- " + IceUtil::Time::now().toDateTime() + " " + _prefix;
    if(!category.empty())
    {
        s += category + ": ";
    }
    s += message;

    write(s, true);
}

void
Ice::LoggerI::warning(const string& message)
{
    write("-! " + IceUtil::Time::now().toDateTime() + " " + _prefix + "warning: " + message, true);
}

void
Ice::LoggerI::error(const string& message)
{
    write("!! " + IceUtil::Time::now().toDateTime() + " " + _prefix + "error: " + message, true);
}

LoggerPtr
Ice::LoggerI::cloneWithPrefix(const std::string& prefix)
{
    return new LoggerI(prefix, _file);
}

void
Ice::LoggerI::write(const string& message, bool indent)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(outputMutex);

    string s = message;

    if(indent)
    {
        string::size_type idx = 0;
        while((idx = s.find("\n", idx)) != string::npos)
        {
            s.insert(idx + 1, "   ");
            ++idx;
        }
    }

    if(_out.is_open())
    {
        _out << s << endl;
    }
    else
    {
        cerr << s << endl;
    }
}
