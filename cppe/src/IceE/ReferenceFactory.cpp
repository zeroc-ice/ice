// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ReferenceFactory.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/Endpoint.h>
#include <IceE/EndpointFactory.h>
#ifdef ICEE_HAS_ROUTER
#   include <IceE/RouterInfo.h>
#   include <IceE/Router.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#   include <IceE/LocatorInfo.h>
#   include <IceE/Locator.h>
#endif
#include <IceE/BasicStream.h>
#include <IceE/StringUtil.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Properties.h>
#include <IceE/Communicator.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(::IceInternal::ReferenceFactory* p) { return p; }

ReferencePtr
IceInternal::ReferenceFactory::copy(const Reference* r) const
{
    Mutex::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    const Ice::Identity& ident = r->getIdentity();
    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    return r->clone();
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
				      const Context& context,
				      const string& facet,
				      ReferenceMode mode,
				      bool secure,
				      const vector<EndpointPtr>& endpoints
#ifdef ICEE_HAS_ROUTER
				      , const RouterInfoPtr& routerInfo
#endif
				      )
{
    Mutex::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    //
    // Create new reference
    //
#ifdef ICEE_HAS_ROUTER
    return new DirectReference(_instance, _communicator, ident, context, facet, mode, secure, endpoints, routerInfo);
#else
    return new DirectReference(_instance, _communicator, ident, context, facet, mode, secure, endpoints);
#endif
}

#ifdef ICEE_HAS_LOCATOR

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
				      const Context& context,
				      const string& facet,
				      ReferenceMode mode,
				      bool secure,
				      const string& adapterId
#ifdef ICEE_HAS_ROUTER
				      , const RouterInfoPtr& routerInfo
#endif
				      , const LocatorInfoPtr& locatorInfo)
{
    Mutex::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    //
    // Create new reference
    //
#ifdef ICEE_HAS_ROUTER
    return new IndirectReference(_instance, _communicator, ident, context, facet, mode, secure, adapterId, routerInfo,
				locatorInfo);
#else
    return new IndirectReference(_instance, _communicator, ident, context, facet, mode, secure, adapterId, locatorInfo);
#endif
}

#endif

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
				      const Context& context,
				      const string& facet,
				      ReferenceMode mode,
				      const vector<Ice::ConnectionPtr>& fixedConnections)
{
    Mutex::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(ident.name.empty() && ident.category.empty())
    {
        return 0;
    }

    //
    // Create new reference
    //
    return new FixedReference(_instance, _communicator, ident, context, facet, mode, fixedConnections);
}

ReferencePtr
IceInternal::ReferenceFactory::create(const string& str)
{
    if(str.empty())
    {
        return 0;
    }

    const string delim = " \t\n\r";

    string s(str);
    string::size_type beg;
    string::size_type end = 0;

    beg = s.find_first_not_of(delim, end);
    if(beg == string::npos)
    {
	throw ProxyParseException(__FILE__, __LINE__, str);
    }
    
    //
    // Extract the identity, which may be enclosed in single
    // or double quotation marks.
    //
    string idstr;
    end = IceUtil::checkQuote(s, beg);
    if(end == string::npos)
    {
	throw ProxyParseException(__FILE__, __LINE__, str);
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
	throw ProxyParseException(__FILE__, __LINE__, str);
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
            throw IllegalIdentityException(__FILE__, __LINE__, ident);
        }
        //
        // Treat a stringified proxy containing two double
        // quotes ("") the same as an empty string, i.e.,
        // a null proxy, but only if nothing follows the
        // quotes.
        //
        else if(s.find_first_not_of(delim, end) != string::npos)
        {
            throw ProxyParseException(__FILE__, __LINE__, str);
        }
        else
        {
            return 0;
        }
    }

    string facet;
    ReferenceMode mode = ReferenceModeTwoway;
    bool secure = false;
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
            throw ProxyParseException(__FILE__, __LINE__, str);
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
                end = IceUtil::checkQuote(s, beg);
                if(end == string::npos)
                {
		    throw ProxyParseException(__FILE__, __LINE__, str);
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
		    throw ProxyParseException(__FILE__, __LINE__, str);
		}

		if(!IceUtil::unescapeString(argument, 0, argument.size(), facet))
		{
		    throw ProxyParseException(__FILE__, __LINE__, str);
		}
#ifdef ICEE_HAS_WSTRING
                if(_instance->initializationData().stringConverter)
                {
                    string tmpFacet;
                    _instance->initializationData().stringConverter->fromUTF8(
                                reinterpret_cast<const Byte*>(facet.data()),
                                reinterpret_cast<const Byte*>(facet.data() + facet.size()), tmpFacet);
                    facet = tmpFacet;
                }
#endif
		break;
	    }

	    case 't':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__, str);
		}
		mode = ReferenceModeTwoway;
		break;
	    }

	    case 'o':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__, str);
		}
		mode = ReferenceModeOneway;
		break;
	    }

	    case 'O':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__, str);
		}
		mode = ReferenceModeBatchOneway;
		break;
	    }

	    case 'd':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__, str);
		}
		mode = ReferenceModeDatagram;
		break;
	    }

	    case 'D':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__, str);
		}
		mode = ReferenceModeBatchDatagram;
		break;
	    }

	    case 's':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__, str);
		}
		secure = true;
		break;
	    }

	    default:
	    {
		throw ProxyParseException(__FILE__, __LINE__, str);
	    }
	}
    }

#ifdef ICEE_HAS_ROUTER
    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
#endif
#ifdef ICEE_HAS_LOCATOR
    LocatorInfoPtr locatorInfo = _instance->locatorManager()->get(getDefaultLocator());
#endif

    if(beg == string::npos)
    {
#ifdef ICEE_HAS_LOCATOR
#   ifdef ICEE_HAS_ROUTER
	return create(ident, Ice::Context(), facet, mode, secure, "", routerInfo, locatorInfo);
#   else
	return create(ident, Ice::Context(), facet, mode, secure, "", locatorInfo);
#   endif
#else	
        throw FeatureNotSupportedException(__FILE__, __LINE__, 
					   "indirect proxy `" + str + "' (no locator support built-in)");
#endif
    }
    vector<EndpointPtr> endpoints;

    switch(s[beg])
    {
	case ':':
	{
	    vector<string> unknownEndpoints;
	    end = beg;
	    
	    while(end < s.length() && s[end] == ':')
	    {
		beg = end + 1;
		
		end = s.find(':', beg);
		if(end == string::npos)
		{
		    end = s.length();
		}
		
		string es = s.substr(beg, end - beg);
		EndpointPtr endp = _instance->endpointFactory()->create(es);
		if(endp != 0)
		{
		    vector<EndpointPtr> endps = endp->expand(false);
		    endpoints.insert(endpoints.end(), endps.begin(), endps.end());
		}
		else
		{
		    unknownEndpoints.push_back(es);
		}
	    }
	    if(endpoints.size() == 0)
	    {
	        throw EndpointParseException(__FILE__, __LINE__, unknownEndpoints.front());
	    }
	    else if(unknownEndpoints.size() != 0 &&
	            _instance->initializationData().properties->getPropertyAsIntWithDefault(
		    							"Ice.Warn.Endpoints", 1) > 0)
	    {
	        Warning out(_instance->initializationData().logger);
		out << "Proxy contains unknown endpoints:";
		for(unsigned int idx = 0; idx < unknownEndpoints.size(); ++idx)
		{
		    out << " `" << unknownEndpoints[idx] << "'";
		}
	    }

#ifdef ICEE_HAS_ROUTER
	    return create(ident, Ice::Context(), facet, mode, secure, endpoints, routerInfo);
#else
	    return create(ident, Ice::Context(), facet, mode, secure, endpoints);
#endif
	    break;
	}

	case '@':
	{
#ifdef ICEE_HAS_LOCATOR
	    beg = s.find_first_not_of(delim, beg + 1);
	    if(beg == string::npos)
	    {
		throw ProxyParseException(__FILE__, __LINE__, str);
	    }

            string adapterstr;
	    end = IceUtil::checkQuote(s, beg);
	    if(end == string::npos)
	    {
		throw ProxyParseException(__FILE__, __LINE__, str);
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
		throw ProxyParseException(__FILE__, __LINE__, str);
            }

	    if(!IceUtil::unescapeString(adapterstr, 0, adapterstr.size(), adapter) || adapter.size() == 0)
	    {
		throw ProxyParseException(__FILE__, __LINE__, str);
	    }
#ifdef ICEE_HAS_WSTRING
            if(_instance->initializationData().stringConverter)
            {
                string tmpAdapter;
                _instance->initializationData().stringConverter->fromUTF8(
                               reinterpret_cast<const Byte*>(adapter.data()),
                               reinterpret_cast<const Byte*>(adapter.data() + adapter.size()), tmpAdapter);
                adapter = tmpAdapter;
            }
#endif

#ifdef ICEE_HAS_ROUTER
	    return create(ident, Ice::Context(), facet, mode, secure, adapter, routerInfo, locatorInfo);
#else
	    return create(ident, Ice::Context(), facet, mode, secure, adapter, locatorInfo);
#endif
#else
            throw FeatureNotSupportedException(__FILE__, __LINE__, 
					       "indirect proxy `" + str + "' (no locator support built-in)");
#endif
	    break;
	}

	default:
	{
	    throw ProxyParseException(__FILE__, __LINE__, str);
	}
    }

    return 0; // Unreachable, fixes compiler warning.
}

ReferencePtr
IceInternal::ReferenceFactory::createFromProperties(const string& propertyPrefix)
{
    PropertiesPtr properties = _instance->initializationData().properties;

    ReferencePtr ref = create(properties->getProperty(propertyPrefix));
    if(!ref)
    {
        return 0;
    }

#ifdef ICEE_HAS_LOCATOR
    string property = propertyPrefix + ".Locator";
    if(!properties->getProperty(property).empty())
    {
        ref = ref->changeLocator(
            LocatorPrx::uncheckedCast(_communicator->propertyToProxy(property)));
        if(ref->getType() == Reference::TypeDirect)
        {
            Warning out(_instance->initializationData().logger);
            out << "`" << property << "=" << properties->getProperty(property)
                << "': cannot set a locator on a direct reference; setting ignored";
        }
    }
#endif

#ifdef ICEE_HAS_ROUTER
    property = propertyPrefix + ".Router";
    if(!properties->getProperty(property).empty())
    {
        if(propertyPrefix.size() > 7 && propertyPrefix.substr(propertyPrefix.size() - 7, 7) == ".Router")
        {
            Warning out(_instance->initializationData().logger);
            out << "`" << property << "=" << properties->getProperty(property)
                << "': cannot set a router on a router; setting ignored";
        }
        else
        {
            ref = ref->changeRouter(
                RouterPrx::uncheckedCast(_communicator->propertyToProxy(property)));
        }
    }
#endif

    return ref;
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
	    throwProxyUnmarshalException(__FILE__, __LINE__);
	}
	facet.swap(facetPath[0]);
    }

    Byte modeAsByte;
    s->read(modeAsByte);
    ReferenceMode mode = static_cast<ReferenceMode>(modeAsByte);
    if(mode < 0 || mode > ReferenceModeLast)
    {
	throwProxyUnmarshalException(__FILE__, __LINE__);
    }

    vector<EndpointPtr> endpoints;
    string adapterId;

#ifdef ICEE_HAS_ROUTER
    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
#endif

    bool secure;
    s->read(secure);

    Ice::Int sz;
    s->readSize(sz);
    
    if(sz > 0)
    {
	endpoints.reserve(sz);
	while(sz--)
	{
	    EndpointPtr endpoint = _instance->endpointFactory()->read(s);
	    endpoints.push_back(endpoint);
	}
#ifdef ICEE_HAS_ROUTER
	return create(ident, Ice::Context(), facet, mode, secure, endpoints, routerInfo);
#else
	return create(ident, Ice::Context(), facet, mode, secure, endpoints);
#endif
    }
    else
    {
#ifdef ICEE_HAS_LOCATOR
	LocatorInfoPtr locatorInfo = _instance->locatorManager()->get(getDefaultLocator());
	s->read(adapterId);
#   ifdef ICEE_HAS_ROUTER
	return create(ident, Ice::Context(), facet, mode, secure, adapterId, routerInfo, locatorInfo);
#   else
	return create(ident, Ice::Context(), facet, mode, secure, adapterId, locatorInfo);
#   endif
#else
	throwProxyUnmarshalException(__FILE__, __LINE__);
        return 0; // Unreachable, fixes compiler warning.
#endif
    }
}

#ifdef ICEE_HAS_ROUTER

void
IceInternal::ReferenceFactory::setDefaultRouter(const RouterPrx& defaultRouter)
{
    IceUtil::Mutex::Lock sync(*this);
    _defaultRouter = defaultRouter;
}

RouterPrx
IceInternal::ReferenceFactory::getDefaultRouter() const
{
    IceUtil::Mutex::Lock sync(*this);
    return _defaultRouter;
}

#endif

#ifdef ICEE_HAS_LOCATOR

void
IceInternal::ReferenceFactory::setDefaultLocator(const LocatorPrx& defaultLocator)
{
    IceUtil::Mutex::Lock sync(*this);
    _defaultLocator = defaultLocator;
}

LocatorPrx
IceInternal::ReferenceFactory::getDefaultLocator() const
{
    IceUtil::Mutex::Lock sync(*this);
    return _defaultLocator;
}

#endif

IceInternal::ReferenceFactory::ReferenceFactory(const InstancePtr& instance,
						const CommunicatorPtr& communicator) :
    _instance(instance),
    _communicator(communicator)
{
}

void
IceInternal::ReferenceFactory::destroy()
{
    Mutex::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _instance = 0;
    _communicator = 0;
#ifdef ICEE_HAS_ROUTER
    _defaultRouter = 0;
#endif
#ifdef ICEE_HAS_LOCATOR
    _defaultLocator = 0;
#endif
}
