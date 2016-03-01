// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/EndpointI.h>
#include <Ice/ConnectionI.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/RouterInfo.h>
#include <Ice/Router.h>
#include <Ice/LocatorInfo.h>
#include <Ice/Locator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/BasicStream.h>
#include <Ice/Properties.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/PropertyNames.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(::IceInternal::ReferenceFactory* p) { return p; }

ReferencePtr
IceInternal::ReferenceFactory::copy(const Reference* r) const
{
    const Ice::Identity& ident = r->getIdentity();
    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    return r->clone();
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
                                      const string& facet,
                                      const ReferencePtr& tmpl,
                                      const vector<EndpointIPtr>& endpoints)
{
    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    return create(ident, facet, tmpl->getMode(), tmpl->getSecure(), tmpl->getProtocol(), tmpl->getEncoding(),
                  endpoints, "", "");
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
                                      const string& facet,
                                      const ReferencePtr& tmpl,
                                      const string& adapterId)
{
    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    return create(ident, facet, tmpl->getMode(), tmpl->getSecure(), tmpl->getProtocol(), tmpl->getEncoding(),
                  vector<EndpointIPtr>(), adapterId, "");
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident, const Ice::ConnectionIPtr& connection)
{
    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    //
    // Create new reference
    //
    return new FixedReference(_instance,
                              _communicator,
                              ident,
                              "",  // Facet
                              connection->endpoint()->datagram() ? Reference::ModeDatagram : Reference::ModeTwoway,
                              connection->endpoint()->secure(),
                              _instance->defaultsAndOverrides()->defaultEncoding,
                              connection);
}

ReferencePtr
IceInternal::ReferenceFactory::create(const string& str, const string& propertyPrefix)
{
    if(str.empty())
    {
        return 0;
    }

    const string delim = " \t\r\n";

    string s(str);
    string::size_type beg;
    string::size_type end = 0;

    beg = s.find_first_not_of(delim, end);
    if(beg == string::npos)
    {
        ProxyParseException ex(__FILE__, __LINE__);
        ex.str = "no non-whitespace characters found in `" + s + "'";
        throw ex;
    }

    //
    // Extract the identity, which may be enclosed in single
    // or double quotation marks.
    //
    string idstr;
    end = IceUtilInternal::checkQuote(s, beg);
    if(end == string::npos)
    {
        ProxyParseException ex(__FILE__, __LINE__);
        ex.str = "mismatched quotes around identity in `" + s + "'";
        throw ex;
    }
    else if(end == 0)
    {
        end = s.find_first_of(delim + ":@", beg);
        if(end == string::npos)
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

    if(beg == end)
    {
        ProxyParseException ex(__FILE__, __LINE__);
        ex.str = "no identity in `" + s + "'";
        throw ex;
    }

    //
    // Parsing the identity may raise IdentityParseException.
    //
    Identity ident = _instance->stringToIdentity(idstr);
    if(ident.name.empty())
    {
        //
        // An identity with an empty name and a non-empty
        // category is illegal.
        //
        if(!ident.category.empty())
        {
            IllegalIdentityException e(__FILE__, __LINE__);
            e.id = ident;
            throw e;
        }
        //
        // Treat a stringified proxy containing two double
        // quotes ("") the same as an empty string, i.e.,
        // a null proxy, but only if nothing follows the
        // quotes.
        //
        else if(s.find_first_not_of(delim, end) != string::npos)
        {
            ProxyParseException ex(__FILE__, __LINE__);
            ex.str = "invalid characters after identity in `" + s + "'";
            throw ex;
        }
        else
        {
            return 0;
        }
    }

    string facet;
    Reference::Mode mode = Reference::ModeTwoway;
    bool secure = false;
    Ice::EncodingVersion encoding = _instance->defaultsAndOverrides()->defaultEncoding;
    Ice::ProtocolVersion protocol = Protocol_1_0;
    string adapter;

    while(true)
    {
        beg = s.find_first_not_of(delim, end);
        if(beg == string::npos)
        {
            break;
        }

        if(s[beg] == ':' || s[beg] == '@')
        {
            break;
        }

        end = s.find_first_of(delim + ":@", beg);
        if(end == string::npos)
        {
            end = s.length();
        }

        if(beg == end)
        {
            break;
        }

        string option = s.substr(beg, end - beg);
        if(option.length() != 2 || option[0] != '-')
        {
            ProxyParseException ex(__FILE__, __LINE__);
            ex.str = "expected a proxy option but found `" + option + "' in `" + s + "'";
            throw ex;
        }

        //
        // Check for the presence of an option argument. The
        // argument may be enclosed in single or double
        // quotation marks.
        //
        string argument;
        string::size_type argumentBeg = s.find_first_not_of(delim, end);
        if(argumentBeg != string::npos)
        {
            if(s[argumentBeg] != '@' && s[argumentBeg] != ':' && s[argumentBeg] != '-')
            {
                beg = argumentBeg;
                end = IceUtilInternal::checkQuote(s, beg);
                if(end == string::npos)
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "mismatched quotes around value for " + option + " option in `" + s + "'";
                    throw ex;
                }
                else if(end == 0)
                {
                    end = s.find_first_of(delim + ":@", beg);
                    if(end == string::npos)
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
        switch(option[1])
        {
            case 'f':
            {
                if(argument.empty())
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "no argument provided for -f option in `" + s + "'";
                    throw ex;
                }

                try
                {
                    facet = IceUtilInternal::unescapeString(argument, 0, argument.size());
                }
                catch(const IceUtil::IllegalArgumentException& e)
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "invalid facet in `" + s + "': " + e.reason();
                    throw ex;
                }

                facet = UTF8ToNative(facet, _instance->getStringConverter());
                break;
            }

            case 't':
            {
                if(!argument.empty())
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "unexpected argument `" + argument + "' provided for -t option in `" + s + "'";
                    throw ex;
                }
                mode = Reference::ModeTwoway;
                break;
            }

            case 'o':
            {
                if(!argument.empty())
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "unexpected argument `" + argument + "' provided for -o option in `" + s + "'";
                    throw ex;
                }
                mode = Reference::ModeOneway;
                break;
            }

            case 'O':
            {
                if(!argument.empty())
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "unexpected argument `" + argument + "' provided for -O option in `" + s + "'";
                    throw ex;
                }
                mode = Reference::ModeBatchOneway;
                break;
            }

            case 'd':
            {
                if(!argument.empty())
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "unexpected argument `" + argument + "' provided for -d option in `" + s + "'";
                    throw ex;
                }
                mode = Reference::ModeDatagram;
                break;
            }

            case 'D':
            {
                if(!argument.empty())
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "unexpected argument `" + argument + "' provided for -D option in `" + s + "'";
                    throw ex;
                }
                mode = Reference::ModeBatchDatagram;
                break;
            }

            case 's':
            {
                if(!argument.empty())
                {
                    ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "unexpected argument `" + argument + "' provided for -s option in `" + s + "'";
                    throw ex;
                }
                secure = true;
                break;
            }

            case 'e':
            {
                if(argument.empty())
                {
                    Ice::ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "no argument provided for -e option in `" + s + "'";
                    throw ex;
                }

                try
                {
                    encoding = Ice::stringToEncodingVersion(argument);
                }
                catch(const Ice::VersionParseException& e)
                {
                    Ice::ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "invalid encoding version `" + argument + "' in `" + s + "':\n" + e.str;
                    throw ex;
                }
                break;
            }

            case 'p':
            {
                if(argument.empty())
                {
                    Ice::ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "no argument provided for -p option in `" + s + "'";
                    throw ex;
                }

                try
                {
                    protocol = Ice::stringToProtocolVersion(argument);
                }
                catch(const Ice::VersionParseException& e)
                {
                    Ice::ProxyParseException ex(__FILE__, __LINE__);
                    ex.str = "invalid protocol version `" + argument + "' in `" + s + "':\n" + e.str;
                    throw ex;
                }
                break;
            }

            default:
            {
                ProxyParseException ex(__FILE__, __LINE__);
                ex.str = "unknown option `" + option + "' in `" + s + "'";
                throw ex;
            }
        }
    }

    if(beg == string::npos)
    {
        return create(ident, facet, mode, secure, protocol, encoding, vector<EndpointIPtr>(), "", propertyPrefix);
    }

    vector<EndpointIPtr> endpoints;
    switch(s[beg])
    {
        case ':':
        {
            vector<string> unknownEndpoints;
            end = beg;

            while(end < s.length() && s[end] == ':')
            {
                beg = end + 1;

                end = beg;
                while(true)
                {
                    end = s.find(':', end);
                    if(end == string::npos)
                    {
                        end = s.length();
                        break;
                    }
                    else
                    {
                        bool quoted = false;
                        string::size_type quote = beg;
                        while(true)
                        {
                            quote = s.find('\"', quote);
                            if(quote == string::npos || end < quote)
                            {
                                break;
                            }
                            else
                            {
                                quote = s.find('\"', ++quote);
                                if(quote == string::npos)
                                {
                                    break;
                                }
                                else if(end < quote)
                                {
                                    quoted = true;
                                    break;
                                }
                                ++quote;
                            }
                        }
                        if(!quoted)
                        {
                            break;
                        }
                        ++end;
                    }
                }

                string es = s.substr(beg, end - beg);
                EndpointIPtr endp = _instance->endpointFactoryManager()->create(es, false);
                if(endp != 0)
                {
                    endpoints.push_back(endp);
                }
                else
                {
                    unknownEndpoints.push_back(es);
                }
            }
            if(endpoints.size() == 0)
            {
                assert(!unknownEndpoints.empty());
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "invalid endpoint `" + unknownEndpoints.front() + "' in `" + s + "'";
                throw ex;
            }
            else if(unknownEndpoints.size() != 0 &&
                    _instance->initializationData().properties->
                                getPropertyAsIntWithDefault("Ice.Warn.Endpoints", 1) > 0)
            {
                Warning out(_instance->initializationData().logger);
                out << "Proxy contains unknown endpoints:";
                for(unsigned int idx = 0; idx < unknownEndpoints.size(); ++idx)
                {
                    out << " `" << unknownEndpoints[idx] << "'";
                }
            }

            return create(ident, facet, mode, secure, protocol, encoding, endpoints, "", propertyPrefix);
            break;
        }
        case '@':
        {
            beg = s.find_first_not_of(delim, beg + 1);
            if(beg == string::npos)
            {
                ProxyParseException ex(__FILE__, __LINE__);
                ex.str = "missing adapter id in `" + s + "'";
                throw ex;
            }

            string adapterstr;
            end = IceUtilInternal::checkQuote(s, beg);
            if(end == string::npos)
            {
                ProxyParseException ex(__FILE__, __LINE__);
                ex.str = "mismatched quotes around adapter id in `" + s + "'";
                throw ex;
            }
            else if(end == 0)
            {
                end = s.find_first_of(delim, beg);
                if(end == string::npos)
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
            if(end != string::npos && s.find_first_not_of(delim, end) != string::npos)
            {
                ProxyParseException ex(__FILE__, __LINE__);
                ex.str = "invalid trailing characters after `" + s.substr(0, end + 1) + "' in `" + s + "'";
                throw ex;
            }

            try
            {
                adapter = IceUtilInternal::unescapeString(adapterstr, 0, adapterstr.size());
            }
            catch(const IceUtil::IllegalArgumentException& e)
            {
                ProxyParseException ex(__FILE__, __LINE__);
                ex.str = "invalid adapter id in `" + s + "': " + e.reason();
                throw ex;
            }
            if(adapter.size() == 0)
            {
                ProxyParseException ex(__FILE__, __LINE__);
                ex.str = "empty adapter id in `" + s + "'";
                throw ex;
            }

            adapter = UTF8ToNative(adapter, _instance->getStringConverter());

            return create(ident, facet, mode, secure, protocol, encoding, endpoints, adapter, propertyPrefix);
            break;
        }
        default:
        {
            ProxyParseException ex(__FILE__, __LINE__);
            ex.str = "malformed proxy `" + s + "'";
            throw ex;
        }
    }

    return 0; // Unreachable, prevents compiler warning.
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident, BasicStream* s)
{
    //
    // Don't read the identity here. Operations calling this
    // constructor read the identity, and pass it as a parameter.
    //

    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    //
    // For compatibility with the old FacetPath.
    //
    vector<string> facetPath;
    s->read(facetPath);
    string facet;
    if(!facetPath.empty())
    {
        if(facetPath.size() > 1)
        {
            throw ProxyUnmarshalException(__FILE__, __LINE__);
        }
        facet.swap(facetPath[0]);
    }

    Byte modeAsByte;
    s->read(modeAsByte);
    Reference::Mode mode = static_cast<Reference::Mode>(modeAsByte);
    if(mode < 0 || mode > Reference::ModeLast)
    {
        throw ProxyUnmarshalException(__FILE__, __LINE__);
    }

    bool secure;
    s->read(secure);

    Ice::ProtocolVersion protocol;
    Ice::EncodingVersion encoding;
    if(s->getReadEncoding() != Ice::Encoding_1_0)
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

    Ice::Int sz = s->readSize();

    if(sz > 0)
    {
        endpoints.reserve(sz);
        while(sz--)
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
IceInternal::ReferenceFactory::setDefaultRouter(const RouterPrx& defaultRouter)
{
    if(defaultRouter == _defaultRouter)
    {
        return this;
    }

    ReferenceFactoryPtr factory = new ReferenceFactory(_instance, _communicator);
    factory->_defaultLocator = _defaultLocator;
    factory->_defaultRouter = defaultRouter;
    return factory;
}

RouterPrx
IceInternal::ReferenceFactory::getDefaultRouter() const
{
    return _defaultRouter;
}

ReferenceFactoryPtr
IceInternal::ReferenceFactory::setDefaultLocator(const LocatorPrx& defaultLocator)
{
    if(defaultLocator == _defaultLocator)
    {
        return this;
    }

    ReferenceFactoryPtr factory = new ReferenceFactory(_instance, _communicator);
    factory->_defaultRouter = _defaultRouter;
    factory->_defaultLocator = defaultLocator;
    return factory;
}

LocatorPrx
IceInternal::ReferenceFactory::getDefaultLocator() const
{
    return _defaultLocator;
}

IceInternal::ReferenceFactory::ReferenceFactory(const InstancePtr& instance, const CommunicatorPtr& communicator) :
    _instance(instance),
    _communicator(communicator)
{
}

void
IceInternal::ReferenceFactory::checkForUnknownProperties(const string& prefix)
{
    static const string suffixes[] =
    {
        "EndpointSelection",
        "ConnectionCached",
        "PreferSecure",
        "LocatorCacheTimeout",
        "InvocationTimeout",
        "Locator",
        "Router",
        "CollocationOptimized",
        "Context.*"
    };

    //
    // Do not warn about unknown properties list if Ice prefix, ie Ice, Glacier2, etc
    //
    for(const char** i = IceInternal::PropertyNames::clPropNames; *i != 0; ++i)
    {
        if(prefix.find(*i) == 0)
        {
            return;
        }
    }

    StringSeq unknownProps;
    PropertyDict props = _instance->initializationData().properties->getPropertiesForPrefix(prefix + ".");
    for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        bool valid = false;
        for(unsigned int i = 0; i < sizeof(suffixes)/sizeof(*suffixes); ++i)
        {
            string prop = prefix + "." + suffixes[i];
            if(IceUtilInternal::match(p->first, prop))
            {
                valid = true;
                break;
            }
        }

        if(!valid)
        {
            unknownProps.push_back(p->first);
        }
    }

    if(unknownProps.size())
    {
        Warning out(_instance->initializationData().logger);
        out << "found unknown properties for proxy '" << prefix << "':";
        for(unsigned int i = 0; i < unknownProps.size(); ++i)
        {
            out << "\n    " << unknownProps[i];
        }
    }
}

RoutableReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
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
    if(_defaultLocator)
    {
        if(_defaultLocator->ice_getEncodingVersion() != encoding)
        {
            locatorInfo = _instance->locatorManager()->get(_defaultLocator->ice_encodingVersion(encoding));
        }
        else
        {
            locatorInfo = _instance->locatorManager()->get(_defaultLocator);
        }
    }
    RouterInfoPtr routerInfo = _instance->routerManager()->get(_defaultRouter);
    bool collocationOptimized = defaultsAndOverrides->defaultCollocationOptimization;
    bool cacheConnection = true;
    bool preferSecure = defaultsAndOverrides->defaultPreferSecure;
    Ice::EndpointSelectionType endpointSelection = defaultsAndOverrides->defaultEndpointSelection;
    int locatorCacheTimeout = defaultsAndOverrides->defaultLocatorCacheTimeout;
    int invocationTimeout = defaultsAndOverrides->defaultInvocationTimeout;
    Ice::Context ctx;

    //
    // Override the defaults with the proxy properties if a property prefix is defined.
    //
    if(!propertyPrefix.empty())
    {
        PropertiesPtr properties = _instance->initializationData().properties;
        if(properties->getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
        {
            checkForUnknownProperties(propertyPrefix);
        }

        string property;

        property = propertyPrefix + ".Locator";
        LocatorPrx locator = LocatorPrx::uncheckedCast(_communicator->propertyToProxy(property));
        if(locator)
        {
            if(locator->ice_getEncodingVersion() != encoding)
            {
                locatorInfo = _instance->locatorManager()->get(locator->ice_encodingVersion(encoding));
            }
            else
            {
                locatorInfo = _instance->locatorManager()->get(locator);
            }
        }

        property = propertyPrefix + ".Router";
        RouterPrx router = RouterPrx::uncheckedCast(_communicator->propertyToProxy(property));
        if(router)
        {
            if(propertyPrefix.size() > 7 && propertyPrefix.substr(propertyPrefix.size() - 7, 7) == ".Router")
            {
                Warning out(_instance->initializationData().logger);
                out << "`" << property << "=" << properties->getProperty(property)
                    << "': cannot set a router on a router; setting ignored";
            }
            else
            {
                routerInfo = _instance->routerManager()->get(router);
            }
        }

        property = propertyPrefix + ".CollocationOptimized";
        collocationOptimized = properties->getPropertyAsIntWithDefault(property, collocationOptimized) > 0;

        property = propertyPrefix + ".ConnectionCached";
        cacheConnection = properties->getPropertyAsIntWithDefault(property, cacheConnection) > 0;

        property = propertyPrefix + ".PreferSecure";
        preferSecure = properties->getPropertyAsIntWithDefault(property, preferSecure) > 0;

        property = propertyPrefix + ".EndpointSelection";
        if(!properties->getProperty(property).empty())
        {
            string type = properties->getProperty(property);
            if(type == "Random")
            {
                endpointSelection = Random;
            }
            else if(type == "Ordered")
            {
                endpointSelection = Ordered;
            }
            else
            {
                EndpointSelectionTypeParseException ex(__FILE__, __LINE__);
                ex.str = "illegal value `" + type + "'; expected `Random' or `Ordered'";
                throw ex;
            }
        }

        property = propertyPrefix + ".LocatorCacheTimeout";
        string value = properties->getProperty(property);
        if(!value.empty())
        {
            locatorCacheTimeout = properties->getPropertyAsIntWithDefault(property, locatorCacheTimeout);
            if(locatorCacheTimeout < -1)
            {
                locatorCacheTimeout = -1;

                Warning out(_instance->initializationData().logger);
                out << "invalid value for " << property << "`" << properties->getProperty(property) << "'"
                    << ": defaulting to -1";
            }
        }

        property = propertyPrefix + ".InvocationTimeout";
        value = properties->getProperty(property);
        if(!value.empty())
        {
            invocationTimeout = properties->getPropertyAsIntWithDefault(property, invocationTimeout);
            if(invocationTimeout < 1 && invocationTimeout != -1)
            {
                invocationTimeout = -1;

                Warning out(_instance->initializationData().logger);
                out << "invalid value for " << property << "`" << properties->getProperty(property) << "'"
                    << ": defaulting to -1";
            }
        }

        property = propertyPrefix + ".Context.";
        PropertyDict contexts = properties->getPropertiesForPrefix(property);
        for(PropertyDict::const_iterator p = contexts.begin(); p != contexts.end(); ++p)
        {
            ctx.insert(make_pair(p->first.substr(property.length()), p->second));
        }
    }

    //
    // Create new reference
    //
    return new RoutableReference(_instance,
                                 _communicator,
                                 ident,
                                 facet,
                                 mode,
                                 secure,
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

