// Copyright (c) ZeroC, Inc.

#include "EndpointI.h"
#include "Ice/OutputStream.h"

#include <sstream>

using namespace std;

Ice::Endpoint::~Endpoint() = default; // Out of line to avoid weak vtable

Ice::EndpointInfo::~EndpointInfo() = default; // Out of line to avoid weak vtable

int16_t
Ice::EndpointInfo::type() const noexcept
{
    return underlying ? underlying->type() : -1;
}

bool
Ice::EndpointInfo::datagram() const noexcept
{
    return underlying ? underlying->datagram() : false;
}

bool
Ice::EndpointInfo::secure() const noexcept
{
    return underlying ? underlying->secure() : false;
}

Ice::IPEndpointInfo::~IPEndpointInfo() = default;
Ice::TCPEndpointInfo::~TCPEndpointInfo() = default;
Ice::UDPEndpointInfo::~UDPEndpointInfo() = default;
Ice::WSEndpointInfo::~WSEndpointInfo() = default;
Ice::IAPEndpointInfo::~IAPEndpointInfo() = default;
Ice::OpaqueEndpointInfo::~OpaqueEndpointInfo() = default;

void
IceInternal::EndpointI::streamWrite(Ice::OutputStream* s) const
{
    s->startEncapsulation();
    streamWriteImpl(s);
    s->endEncapsulation();
}

string
IceInternal::EndpointI::toString() const noexcept
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    return protocol() + options();
}

void
IceInternal::EndpointI::initWithOptions(vector<string>& args)
{
    vector<string> unknown;

    ostringstream ostr;
    ostr << '`' << protocol() << " ";
    for (const auto& arg : args)
    {
        if (arg.find_first_of(" \t\n\r") != string::npos)
        {
            ostr << " \"" << arg << "\"";
        }
        else
        {
            ostr << " " << arg;
        }
    }
    ostr << "'";
    const string str = ostr.str();

    for (vector<string>::size_type n = 0; n < args.size(); ++n)
    {
        const string& option = args[n];
        if (option.length() < 2 || option[0] != '-')
        {
            unknown.push_back(option);
            continue;
        }

        string argument;
        if (n + 1 < args.size() && args[n + 1][0] != '-')
        {
            argument = args[++n];
        }

        if (!checkOption(option, argument, str))
        {
            unknown.push_back(option);
            if (!argument.empty())
            {
                unknown.push_back(argument);
            }
        }
    }

    //
    // Replace argument vector with only those we didn't recognize.
    //
    args = unknown;
}

bool
IceInternal::EndpointI::checkOption(const string&, const string&, const string&)
{
    // Must be overridden to check for options.
    return false;
}
