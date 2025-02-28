// Copyright (c) ZeroC, Inc.

#include "OpaqueEndpointI.h"
#include "Base64.h"
#include "DefaultsAndOverrides.h"
#include "HashUtil.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/OutputStream.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    static string opaqueEndpointProtocol = "opaque"; // NOLINT(cert-err58-cpp)
    static string opaqueEndpointConnectionId;
}

IceInternal::OpaqueEndpointI::OpaqueEndpointI(vector<string>& args) : _type(-1), _rawEncoding(Encoding_1_0)
{
    initWithOptions(args);

    if (_type < 0)
    {
        throw ParseException(__FILE__, __LINE__, "no -t option in endpoint '" + toString() + "'");
    }
    if (_rawBytes.empty())
    {
        throw ParseException(__FILE__, __LINE__, "no -v option in endpoint '" + toString() + "'");
    }
}

IceInternal::OpaqueEndpointI::OpaqueEndpointI(int16_t type, InputStream* s) : _type(type)
{
    _rawEncoding = s->getEncoding();
    int32_t sz = s->getEncapsulationSize();
    s->readBlob(const_cast<vector<byte>&>(_rawBytes), sz);
}

void
IceInternal::OpaqueEndpointI::streamWrite(OutputStream* s) const
{
    s->startEncapsulation(_rawEncoding, nullopt);
    s->writeBlob(_rawBytes);
    s->endEncapsulation();
}

Ice::EndpointInfoPtr
IceInternal::OpaqueEndpointI::getInfo() const noexcept
{
    return make_shared<OpaqueEndpointInfo>(_type, _rawEncoding, _rawBytes);
}

int16_t
IceInternal::OpaqueEndpointI::type() const
{
    return _type;
}

const string&
IceInternal::OpaqueEndpointI::protocol() const
{
    return opaqueEndpointProtocol;
}

int32_t
IceInternal::OpaqueEndpointI::timeout() const
{
    return -1;
}

EndpointIPtr
IceInternal::OpaqueEndpointI::timeout(int32_t) const
{
    return const_cast<OpaqueEndpointI*>(this)->shared_from_this();
}

const string&
IceInternal::OpaqueEndpointI::connectionId() const
{
    return opaqueEndpointConnectionId;
}

EndpointIPtr
IceInternal::OpaqueEndpointI::connectionId(const string&) const
{
    return const_cast<OpaqueEndpointI*>(this)->shared_from_this();
}

bool
IceInternal::OpaqueEndpointI::compress() const
{
    return false;
}

EndpointIPtr
IceInternal::OpaqueEndpointI::compress(bool) const
{
    return const_cast<OpaqueEndpointI*>(this)->shared_from_this();
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
    return nullptr;
}

void
IceInternal::OpaqueEndpointI::connectorsAsync(
    function<void(vector<IceInternal::ConnectorPtr>)> response,
    function<void(exception_ptr)>) const
{
    response(vector<ConnectorPtr>());
}

AcceptorPtr
IceInternal::OpaqueEndpointI::acceptor(const string&, const optional<Ice::SSL::ServerAuthenticationOptions>&) const
{
    return nullptr;
}

vector<EndpointIPtr>
IceInternal::OpaqueEndpointI::expandHost() const
{
    return {const_cast<OpaqueEndpointI*>(this)->shared_from_this()};
}

bool
IceInternal::OpaqueEndpointI::isLoopbackOrMulticast() const
{
    return false;
}

shared_ptr<EndpointI>
IceInternal::OpaqueEndpointI::toPublishedEndpoint(string) const
{
    return const_cast<OpaqueEndpointI*>(this)->shared_from_this();
}

bool
IceInternal::OpaqueEndpointI::equivalent(const EndpointIPtr&) const
{
    return false;
}

size_t
IceInternal::OpaqueEndpointI::hash() const noexcept
{
    size_t h = 5381;
    hashAdd(h, _type);
    hashAdd(h, _rawBytes);
    return h;
}

string
IceInternal::OpaqueEndpointI::options() const
{
    ostringstream s;
    if (_type > -1)
    {
        s << " -t " << _type;
    }
    s << " -e " << _rawEncoding;
    if (!_rawBytes.empty())
    {
        s << " -v " << Base64::encode(_rawBytes);
    }
    return s.str();
}

bool
IceInternal::OpaqueEndpointI::operator==(const Endpoint& r) const
{
    const auto* p = dynamic_cast<const OpaqueEndpointI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    if (_type != p->_type)
    {
        return false;
    }

    if (_rawEncoding != p->_rawEncoding)
    {
        return false;
    }

    if (_rawBytes != p->_rawBytes)
    {
        return false;
    }

    return true;
}

bool
IceInternal::OpaqueEndpointI::operator<(const Endpoint& r) const
{
    const auto* p = dynamic_cast<const OpaqueEndpointI*>(&r);
    if (!p)
    {
        const auto* e = dynamic_cast<const EndpointI*>(&r);
        if (!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if (this == p)
    {
        return false;
    }

    if (_type < p->_type)
    {
        return true;
    }
    else if (p->_type < _type)
    {
        return false;
    }

    if (_rawEncoding < p->_rawEncoding)
    {
        return true;
    }
    else if (p->_rawEncoding < _rawEncoding)
    {
        return false;
    }

    if (_rawBytes < p->_rawBytes)
    {
        return true;
    }
    else if (p->_rawBytes < _rawBytes)
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
    switch (option[1])
    {
        case 't':
        {
            if (_type > -1)
            {
                throw ParseException(__FILE__, __LINE__, "multiple -t options in endpoint '" + endpoint + "'");
            }
            if (argument.empty())
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "no argument provided for -t option in endpoint '" + endpoint + "'");
            }
            istringstream p(argument);
            int32_t t;
            if (!(p >> t) || !p.eof())
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "invalid type value '" + argument + "' in endpoint '" + endpoint + "'");
            }
            else if (t < 0 || t > 65535)
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "type value '" + argument + "' out of range in endpoint '" + endpoint + "'");
            }
            _type = static_cast<int16_t>(t);
            return true;
        }

        case 'v':
        {
            if (!_rawBytes.empty())
            {
                throw ParseException(__FILE__, __LINE__, "multiple -v options in endpoint '" + endpoint + "'");
            }
            if (argument.empty())
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "no argument provided for -v option in endpoint '" + endpoint + "'");
            }
            for (char i : argument)
            {
                if (!Base64::isBase64(i))
                {
                    ostringstream os;
                    os << "invalid base64 character '" << i << "' (ordinal " << static_cast<int>(i) << ") in endpoint '"
                       << endpoint << "'";
                    throw ParseException(__FILE__, __LINE__, os.str());
                }
            }
            const_cast<vector<byte>&>(_rawBytes) = Base64::decode(argument);
            return true;
        }

        case 'e':
        {
            if (argument.empty())
            {
                throw Ice::ParseException(
                    __FILE__,
                    __LINE__,
                    "no argument provided for -e option in endpoint '" + endpoint + "'");
            }

            try
            {
                _rawEncoding = Ice::stringToEncodingVersion(argument);
            }
            catch (const Ice::ParseException& ex)
            {
                throw Ice::ParseException(
                    __FILE__,
                    __LINE__,
                    "invalid encoding version '" + argument + "' in endpoint '" + endpoint + "':\n" + ex.what());
            }
            return true;
        }

        default:
        {
            return false;
        }
    }
}
