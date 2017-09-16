// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include <Ice/Ice.h>
#include <Ice/RegisterPlugins.h>
#include <IceUtil/Time.h>
#include "icematlab.h"
#include "Util.h"

using namespace std;
using namespace IceMatlab;

namespace
{

class Logger : public Ice::Logger
{
public:

    Logger(const string&);

    virtual void print(const string&);
    virtual void trace(const string&, const string&);
    virtual void warning(const string&);
    virtual void error(const string&);
    virtual string getPrefix();
    virtual shared_ptr<Ice::Logger> cloneWithPrefix(const string&);

private:

    void write(const string&, bool);

    const string _prefix;
    string _formattedPrefix;
};

Logger::Logger(const string& prefix) :
    _prefix(prefix)
{
    if(!prefix.empty())
    {
        _formattedPrefix = prefix + ": ";
    }
}

void
Logger::print(const string& message)
{
    write(message, false);
}

void
Logger::trace(const string& category, const string& message)
{
    string s = "-- " + IceUtil::Time::now().toDateTime() + " " + _formattedPrefix;
    if(!category.empty())
    {
        s += category + ": ";
    }
    s += message;

    write(s, true);
}

void
Logger::warning(const string& message)
{
    write("-! " + IceUtil::Time::now().toDateTime() + " " + _formattedPrefix + "warning: " + message, true);
}

void
Logger::error(const string& message)
{
    write("!! " + IceUtil::Time::now().toDateTime() + " " + _formattedPrefix + "error: " + message, true);
}

string
Logger::getPrefix()
{
    return _prefix;
}

shared_ptr<Ice::Logger>
Logger::cloneWithPrefix(const string& prefix)
{
    return make_shared<Logger>(prefix);
}

void
Logger::write(const string& message, bool indent)
{
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

    mexPrintf("%s\n", message.c_str());
    mexEvalString("pause(.001);"); // Hack to flush output.
}

}

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_initialize(mxArray* args, void* propsImpl, void** r)
{
    try
    {
        Ice::StringSeq a;
        getStringList(args, a);

        //
        // Collect InitializationData members.
        //
        Ice::InitializationData id;

        //
        // Properties
        //
        if(propsImpl)
        {
            id.properties = *(reinterpret_cast<shared_ptr<Ice::Properties>*>(propsImpl));
        }

        //id.logger = make_shared<Logger>(""); // TODO

        shared_ptr<Ice::Communicator> c = Ice::initialize(a, id);
        *r = new shared_ptr<Ice::Communicator>(c);
        return createResultValue(createStringList(a));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_stringToIdentity(mxArray* s)
{
    try
    {
        Ice::Identity id = Ice::stringToIdentity(getStringFromUTF16(s));
        return createResultValue(createIdentity(id));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_identityToString(mxArray* id, int mode)
{
    try
    {
        Ice::ToStringMode m = static_cast<Ice::ToStringMode>(mode);
        Ice::Identity ident;
        getIdentity(id, ident);
        return createResultValue(createStringFromUTF8(Ice::identityToString(ident, m)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_stringVersion()
{
    return createResultValue(createStringFromUTF8(ICE_STRING_VERSION));
}

EXPORTED_FUNCTION mxArray*
Ice_intVersion(int* v)
{
    *v = ICE_INT_VERSION;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_currentEncoding()
{
    return createResultValue(createEncodingVersion(Ice::currentEncoding));
}

EXPORTED_FUNCTION mxArray*
Ice_currentProtocol()
{
    return createResultValue(createProtocolVersion(Ice::currentProtocol));
}

EXPORTED_FUNCTION mxArray*
Ice_currentProtocolEncoding()
{
    return createResultValue(createEncodingVersion(Ice::currentProtocolEncoding));
}

//
// This function exists so that mex may be used to compile the library. It is not otherwise needed.
//
void
mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
}

}
