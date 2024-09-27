//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "ReferenceFactory.h"
#include "ConnectionI.h"
#include "DefaultsAndOverrides.h"
#include "EndpointFactoryManager.h"
#include "EndpointI.h"
#include "Ice/Communicator.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Locator.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/ProxyFunctions.h"
#include "Ice/Router.h"
#include "Ice/StringUtil.h"
#include "Instance.h"
#include "LocatorInfo.h"
#include "PropertyNames.h"
#include "RouterInfo.h"

#include <chrono>
#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceInternal;

ReferencePtr
IceInternal::ReferenceFactory::create(
    const Identity& ident,
    const string& facet,
    const ReferencePtr& tmpl,
    const vector<EndpointIPtr>& endpoints)
{
    assert(!ident.name.empty());

    return create(
        ident,
        facet,
        tmpl->getMode(),
        tmpl->getSecure(),
        tmpl->getProtocol(),
        tmpl->getEncoding(),
        endpoints,
        "",
        "");
}

ReferencePtr
IceInternal::ReferenceFactory::create(
    const Identity& ident,
    const string& facet,
    const ReferencePtr& tmpl,
    const string& adapterId)
{
    assert(!ident.name.empty());

    return create(
        ident,
        facet,
        tmpl->getMode(),
        tmpl->getSecure(),
        tmpl->getProtocol(),
        tmpl->getEncoding(),
        vector<EndpointIPtr>(),
        adapterId,
        "");
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident, const Ice::ConnectionIPtr& connection)
{
    assert(!ident.name.empty());

    //
    // Create new reference
    //
    return make_shared<FixedReference>(
        _instance,
        _communicator,
        ident,
        "", // Facet
        connection->endpoint()->datagram() ? Reference::ModeDatagram : Reference::ModeTwoway,
        connection->endpoint()->secure(),
        nullopt,
        Ice::Protocol_1_0,
        _instance->defaultsAndOverrides()->defaultEncoding,
        connection,
        -1ms,
        Ice::Context());
}

ReferencePtr
IceInternal::ReferenceFactory::create(string_view str, const string& propertyPrefix)
{
    if (str.empty())
    {
        return nullptr;
    }

    const string delim = " \t\r\n";

    string s(str);
    string::size_type beg;
    string::size_type end = 0;

    beg = s.find_first_not_of(delim, end);
    if (beg == string::npos)
    {
        throw ParseException(__FILE__, __LINE__, "no non-whitespace characters found in proxy string '" + s + "'");
    }

    //
    // Extract the identity, which may be enclosed in single
    // or double quotation marks.
    //
    string idstr;
    end = IceInternal::checkQuote(s, beg);
    if (end == string::npos)
    {
        throw ParseException(__FILE__, __LINE__, "mismatched quotes around identity in proxy string '" + s + "'");
    }
    else if (end == 0)
    {
        end = s.find_first_of(delim + ":@", beg);
        if (end == string::npos)
        {
            end = s.size();
        }
        idstr = s.substr(beg, end - beg);
    }
    else
    {
        beg++; // Skip leading quote
        idstr = s.substr(beg, end - beg);
        end++; // Skip trailing quote
    }

    if (beg == end)
    {
        throw ParseException(__FILE__, __LINE__, "no identity in proxy string '" + s + "'");
    }

    if (idstr.empty())
    {
        //
        // Treat a stringified proxy containing two double
        // quotes ("") the same as an empty string, i.e.,
        // a null proxy, but only if nothing follows the
        // quotes.
        //
        if (s.find_first_not_of(delim, end) != string::npos)
        {
            throw ParseException(__FILE__, __LINE__, "invalid characters after identity in proxy string '" + s + "'");
        }
        else
        {
            return nullptr;
        }
    }

    //
    // Parsing the identity may raise ParseException.
    //
    Identity ident = Ice::stringToIdentity(idstr);

    string facet;
    Reference::Mode mode = Reference::ModeTwoway;
    bool secure = false;
    Ice::EncodingVersion encoding = _instance->defaultsAndOverrides()->defaultEncoding;
    Ice::ProtocolVersion protocol = Protocol_1_0;
    string adapter;

    while (true)
    {
        beg = s.find_first_not_of(delim, end);
        if (beg == string::npos)
        {
            break;
        }

        if (s[beg] == ':' || s[beg] == '@')
        {
            break;
        }

        end = s.find_first_of(delim + ":@", beg);
        if (end == string::npos)
        {
            end = s.length();
        }

        if (beg == end)
        {
            break;
        }

        string option = s.substr(beg, end - beg);
        if (option.length() != 2 || option[0] != '-')
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "expected a proxy option but found '" + option + "' in proxy string '" + s + "'");
        }

        //
        // Check for the presence of an option argument. The
        // argument may be enclosed in single or double
        // quotation marks.
        //
        string argument;
        string::size_type argumentBeg = s.find_first_not_of(delim, end);
        if (argumentBeg != string::npos)
        {
            if (s[argumentBeg] != '@' && s[argumentBeg] != ':' && s[argumentBeg] != '-')
            {
                beg = argumentBeg;
                end = IceInternal::checkQuote(s, beg);
                if (end == string::npos)
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "mismatched quotes around value for " + option + " option in proxy string '" + s + "'");
                }
                else if (end == 0)
                {
                    end = s.find_first_of(delim + ":@", beg);
                    if (end == string::npos)
                    {
                        end = s.size();
                    }
                    argument = s.substr(beg, end - beg);
                }
                else
                {
                    beg++; // Skip leading quote
                    argument = s.substr(beg, end - beg);
                    end++; // Skip trailing quote
                }
            }
        }

        //
        // If any new options are added here,
        // IceInternal::Reference::toString() and its derived classes must be updated as well.
        //
        switch (option[1])
        {
            case 'f':
            {
                if (argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "no argument provided for -f option in proxy string '" + s + "'");
                }

                try
                {
                    facet = unescapeString(argument, 0, argument.size(), "");
                }
                catch (const invalid_argument& ex)
                {
                    throw ParseException(__FILE__, __LINE__, "invalid facet in proxy string '" + s + "': " + ex.what());
                }

                break;
            }

            case 't':
            {
                if (!argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "unexpected argument '" + argument + "' provided for -t option in proxy string '" + s + "'");
                }
                mode = Reference::ModeTwoway;
                break;
            }

            case 'o':
            {
                if (!argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "unexpected argument '" + argument + "' provided for -o option in proxy string '" + s + "'");
                }
                mode = Reference::ModeOneway;
                break;
            }

            case 'O':
            {
                if (!argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "unexpected argument '" + argument + "' provided for -O option in proxy string '" + s + "'");
                }
                mode = Reference::ModeBatchOneway;
                break;
            }

            case 'd':
            {
                if (!argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "unexpected argument '" + argument + "' provided for -d option in proxy string '" + s + "'");
                }
                mode = Reference::ModeDatagram;
                break;
            }

            case 'D':
            {
                if (!argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "unexpected argument '" + argument + "' provided for -D option in proxy string '" + s + "'");
                }
                mode = Reference::ModeBatchDatagram;
                break;
            }

            case 's':
            {
                if (!argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "unexpected argument '" + argument + "' provided for -s option in proxy string '" + s + "'");
                }
                secure = true;
                break;
            }

            case 'e':
            {
                if (argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "no argument provided for -e option in proxy string '" + s + "'");
                }

                try
                {
                    encoding = Ice::stringToEncodingVersion(argument);
                }
                catch (const Ice::ParseException& ex)
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "invalid encoding version '" + argument + "' in proxy string '" + s + "':\n" + ex.what());
                }
                break;
            }

            case 'p':
            {
                if (argument.empty())
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "no argument provided for -p option in proxy string '" + s + "'");
                }

                try
                {
                    protocol = Ice::stringToProtocolVersion(argument);
                }
                catch (const Ice::ParseException& ex)
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "invalid protocol version '" + argument + "' in proxy string '" + s + "':\n" + ex.what());
                }
                break;
            }

            default:
            {
                throw ParseException(__FILE__, __LINE__, "unknown option '" + option + "' in proxy string '" + s + "'");
            }
        }
    }

    if (beg == string::npos)
    {
        return create(ident, facet, mode, secure, protocol, encoding, vector<EndpointIPtr>(), "", propertyPrefix);
    }

    vector<EndpointIPtr> endpoints;
    switch (s[beg])
    {
        case ':':
        {
            vector<string> unknownEndpoints;
            end = beg;

            while (end < s.length() && s[end] == ':')
            {
                beg = end + 1;

                end = beg;
                while (true)
                {
                    end = s.find(':', end);
                    if (end == string::npos)
                    {
                        end = s.length();
                        break;
                    }
                    else
                    {
                        bool quoted = false;
                        string::size_type quote = beg;
                        while (true)
                        {
                            quote = s.find('\"', quote);
                            if (quote == string::npos || end < quote)
                            {
                                break;
                            }
                            else
                            {
                                quote = s.find('\"', ++quote);
                                if (quote == string::npos)
                                {
                                    break;
                                }
                                else if (end < quote)
                                {
                                    quoted = true;
                                    break;
                                }
                                ++quote;
                            }
                        }
                        if (!quoted)
                        {
                            break;
                        }
                        ++end;
                    }
                }

                string es = s.substr(beg, end - beg);
                EndpointIPtr endp = _instance->endpointFactoryManager()->create(es, false);
                if (endp != nullptr)
                {
                    endpoints.push_back(endp);
                }
                else
                {
                    unknownEndpoints.push_back(es);
                }
            }
            if (endpoints.size() == 0)
            {
                assert(!unknownEndpoints.empty());
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "invalid endpoint '" + unknownEndpoints.front() + "' in proxy string '" + s + "'");
            }
            else if (
                unknownEndpoints.size() != 0 &&
                _instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Endpoints") > 0)
            {
                Warning out(_instance->initializationData().logger);
                out << "Proxy contains unknown endpoints:";
                for (unsigned int idx = 0; idx < unknownEndpoints.size(); ++idx)
                {
                    out << " '" << unknownEndpoints[idx] << "'";
                }
            }

            return create(ident, facet, mode, secure, protocol, encoding, endpoints, "", propertyPrefix);
            break;
        }
        case '@':
        {
            beg = s.find_first_not_of(delim, beg + 1);
            if (beg == string::npos)
            {
                throw ParseException(__FILE__, __LINE__, "missing adapter id in proxy string '" + s + "'");
            }

            string adapterstr;
            end = IceInternal::checkQuote(s, beg);
            if (end == string::npos)
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "mismatched quotes around adapter id in proxy string '" + s + "'");
            }
            else if (end == 0)
            {
                end = s.find_first_of(delim, beg);
                if (end == string::npos)
                {
                    end = s.size();
                }
                adapterstr = s.substr(beg, end - beg);
            }
            else
            {
                beg++; // Skip leading quote
                adapterstr = s.substr(beg, end - beg);
                end++; // Skip trailing quote.
            }

            // Check for trailing whitespace.
            if (end != string::npos && s.find_first_not_of(delim, end) != string::npos)
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "invalid trailing characters after '" + s.substr(0, end + 1) + "' in proxy string '" + s + "'");
            }

            try
            {
                adapter = unescapeString(adapterstr, 0, adapterstr.size(), "");
            }
            catch (const invalid_argument& ex)
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "invalid adapter id in proxy string '" + s + "': " + ex.what());
            }
            if (adapter.size() == 0)
            {
                throw ParseException(__FILE__, __LINE__, "empty adapter id in proxy string '" + s + "'");
            }

            return create(ident, facet, mode, secure, protocol, encoding, endpoints, adapter, propertyPrefix);
            break;
        }
        default:
        {
            throw ParseException(__FILE__, __LINE__, "malformed proxy '" + s + "'");
        }
    }
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident, InputStream* s)
{
    //
    // Don't read the identity here. Operations calling this
    // constructor read the identity, and pass it as a parameter.
    //
    assert(!ident.name.empty());

    //
    // For compatibility with the old FacetPath.
    //
    vector<string> facetPath;
    s->read(facetPath);
    string facet;
    if (!facetPath.empty())
    {
        if (facetPath.size() > 1)
        {
            throw MarshalException{__FILE__, __LINE__, "received facet path with more than one element"};
        }
        facet.swap(facetPath[0]);
    }

    uint8_t modeAsByte;
    s->read(modeAsByte);
    Reference::Mode mode = static_cast<Reference::Mode>(modeAsByte);
    if (mode < 0 || mode > Reference::ModeLast)
    {
        throw MarshalException{__FILE__, __LINE__, "received proxy with invalid mode " + to_string(mode)};
    }

    bool secure;
    s->read(secure);

    Ice::ProtocolVersion protocol;
    Ice::EncodingVersion encoding;
    if (s->getEncoding() != Ice::Encoding_1_0)
    {
        s->read(protocol);
        s->read(encoding);
    }
    else
    {
        protocol = Ice::Protocol_1_0;
        encoding = Ice::Encoding_1_0;
    }

    vector<EndpointIPtr> endpoints;
    string adapterId;

    int32_t sz = s->readSize();

    if (sz > 0)
    {
        endpoints.reserve(static_cast<size_t>(sz));
        while (sz--)
        {
            EndpointIPtr endpoint = _instance->endpointFactoryManager()->read(s);
            endpoints.push_back(endpoint);
        }
    }
    else
    {
        s->read(adapterId);
    }

    return create(ident, facet, mode, secure, protocol, encoding, endpoints, adapterId, "");
}

ReferenceFactoryPtr
IceInternal::ReferenceFactory::setDefaultRouter(const optional<RouterPrx>& defaultRouter)
{
    if (defaultRouter == _defaultRouter)
    {
        return shared_from_this();
    }

    ReferenceFactoryPtr factory = make_shared<ReferenceFactory>(_instance, _communicator);
    factory->_defaultLocator = _defaultLocator;
    factory->_defaultRouter = defaultRouter;
    return factory;
}

std::optional<RouterPrx>
IceInternal::ReferenceFactory::getDefaultRouter() const
{
    return _defaultRouter;
}

ReferenceFactoryPtr
IceInternal::ReferenceFactory::setDefaultLocator(const std::optional<LocatorPrx>& defaultLocator)
{
    if (defaultLocator == _defaultLocator)
    {
        return shared_from_this();
    }

    ReferenceFactoryPtr factory = make_shared<ReferenceFactory>(_instance, _communicator);
    factory->_defaultRouter = _defaultRouter;
    factory->_defaultLocator = defaultLocator;
    return factory;
}

std::optional<LocatorPrx>
IceInternal::ReferenceFactory::getDefaultLocator() const
{
    return _defaultLocator;
}

IceInternal::ReferenceFactory::ReferenceFactory(const InstancePtr& instance, const CommunicatorPtr& communicator)
    : _instance(instance),
      _communicator(communicator)
{
}

void
IceInternal::ReferenceFactory::checkForUnknownProperties(const string& prefix)
{
    static const string suffixes[] = {
        "EndpointSelection",
        "ConnectionCached",
        "PreferSecure",
        "LocatorCacheTimeout",
        "InvocationTimeout",
        "Locator",
        "Router",
        "CollocationOptimized",
        "Context.*"};

    //
    // Do not warn about unknown properties list if Ice prefix, ie Ice, Glacier2, etc
    //
    for (const char** i = IceInternal::PropertyNames::clPropNames; *i != 0; ++i)
    {
        if (prefix.find(*i) == 0)
        {
            return;
        }
    }

    StringSeq unknownProps;
    PropertyDict props = _instance->initializationData().properties->getPropertiesForPrefix(prefix + ".");
    for (PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        bool valid = false;
        for (unsigned int i = 0; i < sizeof(suffixes) / sizeof(*suffixes); ++i)
        {
            string prop = prefix + "." + suffixes[i];
            if (IceInternal::match(p->first, prop))
            {
                valid = true;
                break;
            }
        }

        if (!valid)
        {
            unknownProps.push_back(p->first);
        }
    }

    if (unknownProps.size())
    {
        Warning out(_instance->initializationData().logger);
        out << "found unknown properties for proxy '" << prefix << "':";
        for (unsigned int i = 0; i < unknownProps.size(); ++i)
        {
            out << "\n    " << unknownProps[i];
        }
    }
}

RoutableReferencePtr
IceInternal::ReferenceFactory::create(
    const Identity& ident,
    const string& facet,
    Reference::Mode mode,
    bool secure,
    const Ice::ProtocolVersion& protocol,
    const Ice::EncodingVersion& encoding,
    const vector<EndpointIPtr>& endpoints,
    const string& adapterId,
    const string& propertyPrefix)
{
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();

    //
    // Default local proxy options.
    //
    LocatorInfoPtr locatorInfo;
    if (_defaultLocator)
    {
        if (_defaultLocator->ice_getEncodingVersion() != encoding)
        {
            locatorInfo = _instance->locatorManager()->get(_defaultLocator->ice_encodingVersion(encoding));
        }
        else
        {
            locatorInfo = _instance->locatorManager()->get(_defaultLocator.value());
        }
    }
    RouterInfoPtr routerInfo = _defaultRouter ? _instance->routerManager()->get(_defaultRouter.value()) : nullptr;
    bool collocationOptimized = defaultsAndOverrides->defaultCollocationOptimization;
    bool cacheConnection = true;
    bool preferSecure = defaultsAndOverrides->defaultPreferSecure;
    Ice::EndpointSelectionType endpointSelection = defaultsAndOverrides->defaultEndpointSelection;
    chrono::seconds locatorCacheTimeout = defaultsAndOverrides->defaultLocatorCacheTimeout;
    chrono::milliseconds invocationTimeout = defaultsAndOverrides->defaultInvocationTimeout;
    Ice::Context ctx;

    //
    // Override the defaults with the proxy properties if a property prefix is defined.
    //
    if (!propertyPrefix.empty())
    {
        PropertiesPtr properties = _instance->initializationData().properties;
        if (properties->getIcePropertyAsInt("Ice.Warn.UnknownProperties") > 0)
        {
            checkForUnknownProperties(propertyPrefix);
        }

        string property;

        property = propertyPrefix + ".Locator";
        auto locator = _communicator->propertyToProxy<LocatorPrx>(property);
        if (locator)
        {
            if (locator->ice_getEncodingVersion() != encoding)
            {
                locatorInfo = _instance->locatorManager()->get(locator->ice_encodingVersion(encoding));
            }
            else
            {
                locatorInfo = _instance->locatorManager()->get(locator.value());
            }
        }

        property = propertyPrefix + ".Router";
        auto router = _communicator->propertyToProxy<RouterPrx>(property);
        if (router)
        {
            if (propertyPrefix.size() > 7 && propertyPrefix.substr(propertyPrefix.size() - 7, 7) == ".Router")
            {
                Warning out(_instance->initializationData().logger);
                out << "'" << property << "=" << properties->getProperty(property)
                    << "': cannot set a router on a router; setting ignored";
            }
            else
            {
                routerInfo = _instance->routerManager()->get(router.value());
            }
        }

        property = propertyPrefix + ".CollocationOptimized";
        collocationOptimized = properties->getPropertyAsIntWithDefault(property, collocationOptimized) > 0;

        property = propertyPrefix + ".ConnectionCached";
        cacheConnection = properties->getPropertyAsIntWithDefault(property, cacheConnection) > 0;

        property = propertyPrefix + ".PreferSecure";
        preferSecure = properties->getPropertyAsIntWithDefault(property, preferSecure) > 0;

        property = propertyPrefix + ".EndpointSelection";
        if (!properties->getProperty(property).empty())
        {
            string type = properties->getProperty(property);
            if (type == "Random")
            {
                endpointSelection = EndpointSelectionType::Random;
            }
            else if (type == "Ordered")
            {
                endpointSelection = EndpointSelectionType::Ordered;
            }
            else
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "illegal value '" + type + "' for property " + property + "; expected 'Random' or 'Ordered'");
            }
        }

        property = propertyPrefix + ".LocatorCacheTimeout";
        if (!properties->getProperty(property).empty())
        {
            int locatorCacheTimeoutValue =
                properties->getPropertyAsIntWithDefault(property, static_cast<int32_t>(locatorCacheTimeout.count()));
            if (locatorCacheTimeoutValue < -1)
            {
                locatorCacheTimeoutValue = -1;

                Warning out(_instance->initializationData().logger);
                out << "invalid value for " << property << "'" << properties->getProperty(property) << "'"
                    << ": defaulting to -1";
            }
            locatorCacheTimeout = chrono::seconds(locatorCacheTimeoutValue);
        }

        property = propertyPrefix + ".InvocationTimeout";
        if (!properties->getProperty(property).empty())
        {
            int invocationTimeoutValue =
                properties->getPropertyAsIntWithDefault(property, static_cast<int32_t>(invocationTimeout.count()));
            if (invocationTimeoutValue < 1 && invocationTimeoutValue != -1)
            {
                invocationTimeoutValue = -1;

                Warning out(_instance->initializationData().logger);
                out << "invalid value for " << property << "'" << properties->getProperty(property) << "'"
                    << ": defaulting to -1";
            }
            invocationTimeout = chrono::milliseconds(invocationTimeoutValue);
        }

        property = propertyPrefix + ".Context.";
        PropertyDict contexts = properties->getPropertiesForPrefix(property);
        for (PropertyDict::const_iterator p = contexts.begin(); p != contexts.end(); ++p)
        {
            ctx.insert(make_pair(p->first.substr(property.length()), p->second));
        }
    }

    //
    // Create new reference
    //
    return make_shared<RoutableReference>(
        _instance,
        _communicator,
        ident,
        facet,
        mode,
        secure,
        nullopt,
        protocol,
        encoding,
        endpoints,
        adapterId,
        locatorInfo,
        routerInfo,
        collocationOptimized,
        cacheConnection,
        preferSecure,
        endpointSelection,
        locatorCacheTimeout,
        invocationTimeout,
        ctx);
}
