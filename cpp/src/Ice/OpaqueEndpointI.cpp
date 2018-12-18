// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/OpaqueEndpointI.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/Exception.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Base64.h>
#include <Ice/HashUtil.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

static string opaqueEndpointProtocol = "opaque";
static string opaqueEndpointConnectionId;

}

IceInternal::OpaqueEndpointI::OpaqueEndpointI(vector<string>& args) :
    _type(-1), _rawEncoding(Encoding_1_0)
{
    initWithOptions(args);

    if(_type < 0)
    {
        throw EndpointParseException(__FILE__, __LINE__, "no -t option in endpoint " + toString());
    }
    if(_rawBytes.empty())
    {
        throw EndpointParseException(__FILE__, __LINE__, "no -v option in endpoint " + toString());
    }
}

IceInternal::OpaqueEndpointI::OpaqueEndpointI(Short type, InputStream* s) : _type(type)
{
    _rawEncoding = s->getEncoding();
    Int sz = s->getEncapsulationSize();
    s->readBlob(const_cast<vector<Byte>&>(_rawBytes), sz);
}

namespace
{

class OpaqueEndpointInfoI : public Ice::OpaqueEndpointInfo
{
public:

    OpaqueEndpointInfoI(Ice::Short type, const Ice::EncodingVersion& rawEncoding, const Ice::ByteSeq& rawBytes);

    virtual Ice::Short
    type() const ICE_NOEXCEPT
    {
        return _type;
    }

    virtual bool
    datagram() const ICE_NOEXCEPT
    {
        return false;
    }

    virtual bool
    secure() const ICE_NOEXCEPT
    {
        return false;
    }

private:

    Ice::Short _type;
};

}
//
// COMPILERFIX: inlining this constructor causes crashes with gcc 4.0.1.
//
OpaqueEndpointInfoI::OpaqueEndpointInfoI(Ice::Short type, const Ice::EncodingVersion& rawEncodingP,
                                         const Ice::ByteSeq& rawBytesP) :
    Ice::OpaqueEndpointInfo(ICE_NULLPTR, -1, false, rawEncodingP, rawBytesP),
    _type(type)
{
}

void
IceInternal::OpaqueEndpointI::streamWrite(OutputStream* s) const
{
    s->startEncapsulation(_rawEncoding, ICE_ENUM(FormatType, DefaultFormat));
    s->writeBlob(_rawBytes);
    s->endEncapsulation();
}

Ice::EndpointInfoPtr
IceInternal::OpaqueEndpointI::getInfo() const ICE_NOEXCEPT
{
    return ICE_MAKE_SHARED(OpaqueEndpointInfoI, _type, _rawEncoding, _rawBytes);
}

Short
IceInternal::OpaqueEndpointI::type() const
{
    return _type;
}

const string&
IceInternal::OpaqueEndpointI::protocol() const
{
    return opaqueEndpointProtocol;
}

Int
IceInternal::OpaqueEndpointI::timeout() const
{
    return -1;
}

EndpointIPtr
IceInternal::OpaqueEndpointI::timeout(Int) const
{
    return ICE_SHARED_FROM_CONST_THIS(OpaqueEndpointI);
}

const string&
IceInternal::OpaqueEndpointI::connectionId() const
{
    return opaqueEndpointConnectionId;
}

EndpointIPtr
IceInternal::OpaqueEndpointI::connectionId(const string&) const
{
    return ICE_SHARED_FROM_CONST_THIS(OpaqueEndpointI);
}

bool
IceInternal::OpaqueEndpointI::compress() const
{
    return false;
}

EndpointIPtr
IceInternal::OpaqueEndpointI::compress(bool) const
{
    return ICE_SHARED_FROM_CONST_THIS(OpaqueEndpointI);
}

bool
IceInternal::OpaqueEndpointI::datagram() const
{
    return false;
}

bool
IceInternal::OpaqueEndpointI::secure() const
{
    return false;
}

TransceiverPtr
IceInternal::OpaqueEndpointI::transceiver() const
{
    return ICE_NULLPTR;
}

void
IceInternal::OpaqueEndpointI::connectors_async(Ice::EndpointSelectionType, const EndpointI_connectorsPtr& cb) const
{
    cb->connectors(vector<ConnectorPtr>());
}

AcceptorPtr
IceInternal::OpaqueEndpointI::acceptor(const string&) const
{
    return ICE_NULLPTR;
}

vector<EndpointIPtr>
IceInternal::OpaqueEndpointI::expandIfWildcard() const
{
    vector<EndpointIPtr> endps;
    endps.push_back(ICE_SHARED_FROM_CONST_THIS(OpaqueEndpointI));
    return endps;
}

vector<EndpointIPtr>
IceInternal::OpaqueEndpointI::expandHost(EndpointIPtr&) const
{
    vector<EndpointIPtr> endps;
    endps.push_back(ICE_SHARED_FROM_CONST_THIS(OpaqueEndpointI));
    return endps;
}

bool
IceInternal::OpaqueEndpointI::equivalent(const EndpointIPtr&) const
{
    return false;
}

Int
IceInternal::OpaqueEndpointI::hash() const
{
    Int h = 5381;
    hashAdd(h, type());
    hashAdd(h, _rawEncoding.major);
    hashAdd(h, _rawEncoding.minor);
    hashAdd(h, _rawBytes);
    return h;
}

string
IceInternal::OpaqueEndpointI::options() const
{
    ostringstream s;
    if(_type > -1)
    {
        s << " -t " << _type;
    }
    s << " -e " << _rawEncoding;
    if(!_rawBytes.empty())
    {
        s << " -v " << Base64::encode(_rawBytes);
    }
    return s.str();
}

bool
#ifdef ICE_CPP11_MAPPING
IceInternal::OpaqueEndpointI::operator==(const Endpoint& r) const
#else
IceInternal::OpaqueEndpointI::operator==(const LocalObject& r) const
#endif
{
    const OpaqueEndpointI* p = dynamic_cast<const OpaqueEndpointI*>(&r);
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

    if(_rawEncoding != p->_rawEncoding)
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
#ifdef ICE_CPP11_MAPPING
IceInternal::OpaqueEndpointI::operator<(const Endpoint& r) const
#else
IceInternal::OpaqueEndpointI::operator<(const LocalObject& r) const
#endif
{
    const OpaqueEndpointI* p = dynamic_cast<const OpaqueEndpointI*>(&r);
    if(!p)
    {
        const EndpointI* e = dynamic_cast<const EndpointI*>(&r);
        if(!e)
        {
            return false;
        }
        return type() < e->type();
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

    if(_rawEncoding < p->_rawEncoding)
    {
        return true;
    }
    else if(p->_rawEncoding < _rawEncoding)
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

void
IceInternal::OpaqueEndpointI::streamWriteImpl(Ice::OutputStream*) const
{
    assert(false);
}

bool
IceInternal::OpaqueEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    switch(option[1])
    {
    case 't':
    {
        if(_type > -1)
        {
            throw EndpointParseException(__FILE__, __LINE__, "multiple -t options in endpoint " + endpoint);
        }
        if(argument.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "no argument provided for -t option in endpoint " +
                                         endpoint);
        }
        istringstream p(argument);
        Ice::Int t;
        if(!(p >> t) || !p.eof())
        {
            throw EndpointParseException(__FILE__, __LINE__, "invalid type value `" + argument + "' in endpoint " +
                                         endpoint);
        }
        else if(t < 0 || t > 65535)
        {
            throw EndpointParseException(__FILE__, __LINE__, "type value `" + argument + "' out of range in endpoint " +
                                         endpoint);
        }
        _type = static_cast<Ice::Short>(t);
        return true;
    }

    case 'v':
    {
        if(!_rawBytes.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "multiple -v options in endpoint " + endpoint);
        }
        if(argument.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "no argument provided for -v option in endpoint " + endpoint);
        }
        for(string::size_type i = 0; i < argument.size(); ++i)
        {
            if(!Base64::isBase64(argument[i]))
            {
                ostringstream os;
                os << "invalid base64 character `" << argument[i] << "' (ordinal " << static_cast<int>(argument[i])
                   << ") in endpoint " << endpoint;
                throw EndpointParseException(__FILE__, __LINE__, os.str());
            }
        }
        const_cast<vector<Byte>&>(_rawBytes) = Base64::decode(argument);
        return true;
    }

    case 'e':
    {
        if(argument.empty())
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__, "no argument provided for -e option in endpoint " +
                                              endpoint);
        }

        try
        {
            _rawEncoding = Ice::stringToEncodingVersion(argument);
        }
        catch(const Ice::VersionParseException& ex)
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__, "invalid encoding version `" + argument +
                                              "' in endpoint " + endpoint + ":\n" + ex.str);
        }
        return true;
    }

    default:
    {
        return false;
    }
    }
}
