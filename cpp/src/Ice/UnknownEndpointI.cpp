// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UnknownEndpointI.h>
#include <Ice/BasicStream.h>
#include <Ice/Exception.h>
#include <Ice/Instance.h>
#include <IceUtil/Base64.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::UnknownEndpointI::UnknownEndpointI(const string& str)
{
    const string delim = " \t\n\r";

    string::size_type beg;
    string::size_type end = 0;

    int topt = 0;
    int vopt = 0;

    while(true)
    {
        beg = str.find_first_not_of(delim, end);
        if(beg == string::npos)
        {
            break;
        }
        
        end = str.find_first_of(delim, beg);
        if(end == string::npos)
        {
            end = str.length();
        }

        string option = str.substr(beg, end - beg);
        if(option.length() != 2 || option[0] != '-')
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "opaque " + str;
            throw ex;
        }

        string argument;
        string::size_type argumentBeg = str.find_first_not_of(delim, end);
        if(argumentBeg != string::npos && str[argumentBeg] != '-')
        {
            beg = argumentBeg;
            end = str.find_first_of(delim, beg);
            if(end == string::npos)
            {
                end = str.length();
            }
            argument = str.substr(beg, end - beg);
        }

        switch(option[1])
        {
            case 't':
            {
                istringstream p(argument);
                Ice::Int t;
                if(!(p >> t) || !p.eof() || t < 0 || t > 65535)
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "opaque " + str;
                    throw ex;
                }
                _type = static_cast<Ice::Short>(t);
                ++topt;
                break;
            }

            case 'v':
            {
                if(argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "opaque " + str;
                    throw ex;
                }
                for(string::size_type i = 0; i < argument.size(); ++i)
                {
                    if(!IceUtil::Base64::isBase64(argument[i]))
                    {
                        EndpointParseException ex(__FILE__, __LINE__);
                        ex.str = "opaque " + str;
                        throw ex;
                    }
                }
                const_cast<vector<Byte>&>(_rawBytes) = IceUtil::Base64::decode(argument);
                ++vopt;
                break;
            }

            default:
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "opaque " + str;
                throw ex;
            }
        }
    }
    if(topt != 1 || vopt != 1)
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "opaque " + str;
        throw ex;
    }
}

IceInternal::UnknownEndpointI::UnknownEndpointI(Short type, BasicStream* s) :
    _type(type)
{
    s->startReadEncaps();
    Int sz = s->getReadEncapsSize();
    s->readBlob(const_cast<vector<Byte>&>(_rawBytes), sz);
    s->endReadEncaps();
}

void
IceInternal::UnknownEndpointI::streamWrite(BasicStream* s) const
{
    s->write(_type);
    s->startWriteEncaps();
    s->writeBlob(_rawBytes);
    s->endWriteEncaps();
}

string
IceInternal::UnknownEndpointI::toString() const
{

    ostringstream s;
    string val = IceUtil::Base64::encode(_rawBytes);
    s << "opaque -t " << _type << " -v " << val;
    return s.str();
}

Short
IceInternal::UnknownEndpointI::type() const
{
    return _type;
}

Int
IceInternal::UnknownEndpointI::timeout() const
{
    return -1;
}

EndpointIPtr
IceInternal::UnknownEndpointI::timeout(Int) const
{
    return const_cast<UnknownEndpointI*>(this);
}

EndpointIPtr
IceInternal::UnknownEndpointI::connectionId(const string&) const
{
    return const_cast<UnknownEndpointI*>(this);
}

bool
IceInternal::UnknownEndpointI::compress() const
{
    return false;
}

EndpointIPtr
IceInternal::UnknownEndpointI::compress(bool) const
{
    return const_cast<UnknownEndpointI*>(this);
}

bool
IceInternal::UnknownEndpointI::datagram() const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::secure() const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::unknown() const
{
    return true;
}

TransceiverPtr
IceInternal::UnknownEndpointI::transceiver(EndpointIPtr& endp) const
{
    endp = const_cast<UnknownEndpointI*>(this);
    return 0;
}

vector<ConnectorPtr>
IceInternal::UnknownEndpointI::connectors() const
{
    vector<ConnectorPtr> ret;
    return ret;
}

AcceptorPtr
IceInternal::UnknownEndpointI::acceptor(EndpointIPtr& endp, const string&) const
{
    endp = const_cast<UnknownEndpointI*>(this);
    return 0;
}

vector<EndpointIPtr>
IceInternal::UnknownEndpointI::expand() const
{
    vector<EndpointIPtr> endps;
    endps.push_back(const_cast<UnknownEndpointI*>(this));
    return endps;
}

bool
IceInternal::UnknownEndpointI::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::equivalent(const AcceptorPtr&) const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::operator==(const EndpointI& r) const
{
    const UnknownEndpointI* p = dynamic_cast<const UnknownEndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_type != p->_type)
    {
        return false;
    }

    if(_rawBytes != p->_rawBytes)
    {
        return false;
    }

    return true;
}

bool
IceInternal::UnknownEndpointI::operator!=(const EndpointI& r) const
{
    return !operator==(r);
}

bool
IceInternal::UnknownEndpointI::operator<(const EndpointI& r) const
{
    const UnknownEndpointI* p = dynamic_cast<const UnknownEndpointI*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(this == p)
    {
        return false;
    }

    if(_type < p->_type)
    {
        return true;
    }
    else if(p->_type < _type)
    {
        return false;
    }

    if(_rawBytes < p->_rawBytes)
    {
        return true;
    }
    else if(p->_rawBytes < _rawBytes)
    {
        return false;
    }

    return false;
}
