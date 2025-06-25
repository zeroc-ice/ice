// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../DefaultsAndOverrides.h"
#    include "../EndpointFactoryManager.h"
#    include "../HashUtil.h"
#    include "../Network.h"
#    include "../ProtocolInstance.h"
#    include "../ProtocolPluginFacade.h"
#    include "Ice/Initialize.h"
#    include "Ice/InputStream.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/OutputStream.h"
#    include "Ice/Properties.h"
#    include "iAPConnector.h"
#    include "iAPEndpointI.h"

#    include <CoreFoundation/CoreFoundation.h>

#    include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    const char* const iapPluginName = "IceIAP";

    class iAPEndpointFactoryPlugin : public Ice::Plugin
    {
    public:
        iAPEndpointFactoryPlugin(const Ice::CommunicatorPtr& com)
        {
            ProtocolPluginFacade facade{com};

            // iAP transport
            ProtocolInstancePtr iap = make_shared<ProtocolInstance>(com, iAPEndpointType, "iap", false);
            facade.addEndpointFactory(make_shared<IceObjC::iAPEndpointFactory>(iap));

            // SSL based on iAP transport
            ProtocolInstancePtr iaps = make_shared<ProtocolInstance>(com, iAPSEndpointType, "iaps", true);
            facade.addEndpointFactory(make_shared<UnderlyingEndpointFactory>(iaps, SSLEndpointType, iAPEndpointType));
        }

        virtual void initialize() {}
        virtual void destroy() {}
    };

    Plugin* createIceIAP(const CommunicatorPtr& com, const string& name, const StringSeq&)
    {
        string pluginName{iapPluginName};

        if (name != pluginName)
        {
            throw PluginInitializationException{
                __FILE__,
                __LINE__,
                "the iAP plug-in must be named '" + pluginName + "'"};
        }

        return new iAPEndpointFactoryPlugin(com);
    }
}

PluginFactory
Ice::iapPluginFactory()
{
    return {iapPluginName, createIceIAP};
}

IceObjC::iAPEndpointI::iAPEndpointI(
    const ProtocolInstancePtr& instance,
    const string& m,
    const string& o,
    const string& n,
    const string& p,
    int32_t ti,
    const string& conId,
    bool co)
    : _instance(instance),
      _manufacturer(m),
      _modelNumber(o),
      _name(n),
      _protocol(p),
      _timeout(ti),
      _connectionId(conId),
      _compress(co)
{
}

IceObjC::iAPEndpointI::iAPEndpointI(const ProtocolInstancePtr& instance)
    : _instance(instance),
      _timeout(-1),
      _compress(false)
{
}

IceObjC::iAPEndpointI::iAPEndpointI(const ProtocolInstancePtr& instance, InputStream* s)
    : _instance(instance),
      _timeout(-1),
      _compress(false)
{
    s->read(const_cast<string&>(_manufacturer), false);
    s->read(const_cast<string&>(_modelNumber), false);
    s->read(const_cast<string&>(_name), false);
    s->read(const_cast<string&>(_protocol), false);
    s->read(const_cast<int32_t&>(_timeout));
    s->read(const_cast<bool&>(_compress));
}

void
IceObjC::iAPEndpointI::streamWriteImpl(OutputStream* s) const
{
    s->write(_manufacturer, false);
    s->write(_modelNumber, false);
    s->write(_name, false);
    s->write(_protocol, false);
    s->write(_timeout);
    s->write(_compress);
}

EndpointInfoPtr
IceObjC::iAPEndpointI::getInfo() const noexcept
{
    return make_shared<Ice::IAPEndpointInfo>(
        _compress,
        _manufacturer,
        _modelNumber,
        _name,
        protocol(),
        type(),
        secure());
}

int16_t
IceObjC::iAPEndpointI::type() const
{
    return _instance->type();
}

const string&
IceObjC::iAPEndpointI::protocol() const
{
    return _instance->protocol();
}

bool
IceObjC::iAPEndpointI::datagram() const
{
    return false;
}

bool
IceObjC::iAPEndpointI::secure() const
{
    return _instance->secure();
}

int32_t
IceObjC::iAPEndpointI::timeout() const
{
    return _timeout;
}

EndpointIPtr
IceObjC::iAPEndpointI::timeout(int32_t t) const
{
    if (t == _timeout)
    {
        return const_cast<iAPEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<
            iAPEndpointI>(_instance, _manufacturer, _modelNumber, _name, _protocol, t, _connectionId, _compress);
    }
}

const string&
IceObjC::iAPEndpointI::connectionId() const
{
    return _connectionId;
}

EndpointIPtr
IceObjC::iAPEndpointI::connectionId(const string& cId) const
{
    if (cId == _connectionId)
    {
        return const_cast<iAPEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<
            iAPEndpointI>(_instance, _manufacturer, _modelNumber, _name, _protocol, _timeout, cId, _compress);
    }
}

bool
IceObjC::iAPEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceObjC::iAPEndpointI::compress(bool compress) const
{
    if (compress == _compress)
    {
        return const_cast<iAPEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<
            iAPEndpointI>(_instance, _manufacturer, _modelNumber, _name, _protocol, _timeout, _connectionId, compress);
    }
}

TransceiverPtr
IceObjC::iAPEndpointI::transceiver() const
{
    return 0;
}

void
IceObjC::iAPEndpointI::connectorsAsync(
    function<void(vector<IceInternal::ConnectorPtr>)> response,
    function<void(exception_ptr)> exception) const
{
    try
    {
        vector<ConnectorPtr> connectors;

        EAAccessoryManager* manager = [EAAccessoryManager sharedAccessoryManager];
        if (manager == nil)
        {
            throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
        }

        NSString* protocol =
            _protocol.empty() ? @"com.zeroc.ice" : [[NSString alloc] initWithUTF8String:_protocol.c_str()];
        NSArray* array = [manager connectedAccessories];
        NSEnumerator* enumerator = [array objectEnumerator];
        EAAccessory* accessory = nil;
        while ((accessory = [enumerator nextObject]))
        {
            if (!accessory.connected)
            {
                continue;
            }
            if (!_manufacturer.empty() && _manufacturer != [accessory.manufacturer UTF8String])
            {
                continue;
            }
            if (!_modelNumber.empty() && _modelNumber != [accessory.modelNumber UTF8String])
            {
                continue;
            }
            if (!_name.empty() && _name != [accessory.name UTF8String])
            {
                continue;
            }
            if (![accessory.protocolStrings containsObject:protocol])
            {
                continue;
            }
            connectors.emplace_back(make_shared<iAPConnector>(_instance, _timeout, _connectionId, protocol, accessory));
        }
#    if defined(__clang__) && !__has_feature(objc_arc)
        [protocol release];
#    endif
        if (connectors.empty())
        {
            throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
        }
        response(std::move(connectors));
    }
    catch (const Ice::LocalException&)
    {
        exception(current_exception());
    }
}

AcceptorPtr
IceObjC::iAPEndpointI::acceptor(const string&, const optional<SSL::ServerAuthenticationOptions>&) const
{
    assert(false);
    return nullptr;
}

vector<EndpointIPtr>
IceObjC::iAPEndpointI::expandHost() const
{
    return {const_cast<iAPEndpointI*>(this)->shared_from_this()};
}

bool
IceObjC::iAPEndpointI::isLoopbackOrMulticast() const
{
    return false;
}

shared_ptr<EndpointI>
IceObjC::iAPEndpointI::toPublishedEndpoint(string) const
{
    return const_cast<iAPEndpointI*>(this)->shared_from_this();
}

bool
IceObjC::iAPEndpointI::equivalent(const EndpointIPtr& endpoint) const
{
    auto endpointI = dynamic_pointer_cast<iAPEndpointI>(endpoint);
    if (!endpointI)
    {
        return false;
    }
    return endpointI->_manufacturer == _manufacturer && endpointI->_modelNumber == _modelNumber &&
           endpointI->_name == _name && endpointI->_protocol == _protocol;
}

bool
IceObjC::iAPEndpointI::operator==(const Ice::Endpoint& r) const
{
    const iAPEndpointI* p = dynamic_cast<const iAPEndpointI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    if (_manufacturer != p->_manufacturer)
    {
        return false;
    }

    if (_modelNumber != p->_modelNumber)
    {
        return false;
    }

    if (_name != p->_name)
    {
        return false;
    }

    if (_protocol != p->_protocol)
    {
        return false;
    }

    if (_timeout != p->_timeout)
    {
        return false;
    }

    if (_connectionId != p->_connectionId)
    {
        return false;
    }

    if (_compress != p->_compress)
    {
        return false;
    }

    return true;
}

bool
IceObjC::iAPEndpointI::operator<(const Ice::Endpoint& r) const
{
    const iAPEndpointI* p = dynamic_cast<const iAPEndpointI*>(&r);
    if (!p)
    {
        const IceInternal::EndpointI* e = dynamic_cast<const IceInternal::EndpointI*>(&r);
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

    if (_manufacturer < p->_manufacturer)
    {
        return true;
    }
    else if (p->_manufacturer < _manufacturer)
    {
        return false;
    }

    if (_modelNumber < p->_modelNumber)
    {
        return true;
    }
    else if (p->_modelNumber < _modelNumber)
    {
        return false;
    }

    if (_name < p->_name)
    {
        return true;
    }
    else if (p->_name < _name)
    {
        return false;
    }

    if (_protocol < p->_protocol)
    {
        return true;
    }
    else if (p->_protocol < _protocol)
    {
        return false;
    }

    if (_timeout < p->_timeout)
    {
        return true;
    }
    else if (p->_timeout < _timeout)
    {
        return false;
    }

    if (_connectionId < p->_connectionId)
    {
        return true;
    }
    else if (p->_connectionId < _connectionId)
    {
        return false;
    }

    if (!_compress && p->_compress)
    {
        return true;
    }
    else if (p->_compress < _compress)
    {
        return false;
    }

    return false;
}

string
IceObjC::iAPEndpointI::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;
    if (!_manufacturer.empty())
    {
        s << " -m ";
        bool addQuote = _manufacturer.find(':') != string::npos;
        if (addQuote)
        {
            s << "\"";
        }
        s << _manufacturer;
        if (addQuote)
        {
            s << "\"";
        }
    }

    if (!_modelNumber.empty())
    {
        s << " -o ";
        bool addQuote = _modelNumber.find(':') != string::npos;
        if (addQuote)
        {
            s << "\"";
        }
        s << _modelNumber;
        if (addQuote)
        {
            s << "\"";
        }
    }

    if (!_name.empty())
    {
        s << " -n ";
        bool addQuote = _name.find(':') != string::npos;
        if (addQuote)
        {
            s << "\"";
        }
        s << _name;
        if (addQuote)
        {
            s << "\"";
        }
    }

    if (!_protocol.empty())
    {
        s << " -p ";
        bool addQuote = _protocol.find(':') != string::npos;
        if (addQuote)
        {
            s << "\"";
        }
        s << _protocol;
        if (addQuote)
        {
            s << "\"";
        }
    }

    if (_timeout != -1)
    {
        // Use to_string for locale independent formatting.
        s << " -t " << to_string(_timeout);
    }

    if (_compress)
    {
        s << " -z";
    }
    return s.str();
}

size_t
IceObjC::iAPEndpointI::hash() const noexcept
{
    size_t h = 5381;
    hashAdd(h, _manufacturer);
    hashAdd(h, _modelNumber);
    hashAdd(h, _name);
    hashAdd(h, _protocol);
    hashAdd(h, _timeout);
    hashAdd(h, _connectionId);
    return h;
}

bool
IceObjC::iAPEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    switch (option[1])
    {
        case 'm':
        {
            if (argument.empty())
            {
                throw ParseException{
                    __FILE__,
                    __LINE__,
                    "no argument provided for -m option in endpoint '" + endpoint + "'"};
            }
            const_cast<string&>(_manufacturer) = argument;
            break;
        }

        case 'o':
        {
            if (argument.empty())
            {
                throw ParseException{
                    __FILE__,
                    __LINE__,
                    "no argument provided for -o option in endpoint '" + endpoint + "'"};
            }
            const_cast<string&>(_modelNumber) = argument;
            break;
        }

        case 'n':
        {
            if (argument.empty())
            {
                throw ParseException{
                    __FILE__,
                    __LINE__,
                    "no argument provided for -n option in endpoint '" + endpoint + "'"};
            }
            const_cast<string&>(_name) = argument;
            break;
        }

        case 'p':
        {
            if (argument.empty())
            {
                throw ParseException{
                    __FILE__,
                    __LINE__,
                    "no argument provided for -p option in endpoint '" + endpoint + "'"};
            }
            const_cast<string&>(_protocol) = argument;
            break;
        }

        case 't':
        {
            if (argument == "infinite")
            {
                const_cast<int32_t&>(_timeout) = -1;
            }
            else
            {
                istringstream t(argument);
                if (!(t >> const_cast<int32_t&>(_timeout)) || !t.eof() || _timeout < 1)
                {
                    throw ParseException{__FILE__, __LINE__, "invalid timeout value in endpoint '" + endpoint + "'"};
                }
            }
            break;
        }

        case 'z':
        {
            if (!argument.empty())
            {
                throw ParseException{
                    __FILE__,
                    __LINE__,
                    "argument provided for -z option in endpoint '" + endpoint + "'"};
            }
            const_cast<bool&>(_compress) = true;
            break;
        }

        default:
        {
            return false;
        }
    }
    return true;
}

IceObjC::iAPEndpointFactory::iAPEndpointFactory(const ProtocolInstancePtr& instance) : _instance(instance) {}

IceObjC::iAPEndpointFactory::~iAPEndpointFactory() {}

int16_t
IceObjC::iAPEndpointFactory::type() const
{
    return _instance->type();
}

string
IceObjC::iAPEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceObjC::iAPEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    if (oaEndpoint)
    {
        return 0;
    }

    auto endpt = make_shared<iAPEndpointI>(_instance);
    endpt->initWithOptions(args);
    return endpt;
}

EndpointIPtr
IceObjC::iAPEndpointFactory::read(InputStream* s) const
{
    return make_shared<iAPEndpointI>(_instance, s);
}

EndpointFactoryPtr
IceObjC::iAPEndpointFactory::clone(const ProtocolInstancePtr& instance) const
{
    return make_shared<iAPEndpointFactory>(instance);
}

#endif
