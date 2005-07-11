// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/IdentityUtil.h>
#include <Ice/Endpoint.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocatorInfo.h>
#include <Ice/LoggerUtil.h>
#include <Ice/BasicStream.h>
#include <Ice/Properties.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceInternal::ReferenceFactory* p) { p->__incRef(); }
void IceInternal::decRef(::IceInternal::ReferenceFactory* p) { p->__decRef(); }

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
				      Reference::Mode mode,
				      bool secure,
				      const vector<EndpointPtr>& endpoints,
				      const RouterInfoPtr& routerInfo,
				      bool collocationOptimization)
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
    return new DirectReference(_instance, ident, context, facet, mode, secure,
			       endpoints, routerInfo, collocationOptimization);
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
				      const Context& context,
				      const string& facet,
				      Reference::Mode mode,
				      bool secure,
				      const string& adapterId,
				      const RouterInfoPtr& routerInfo,
				      const LocatorInfoPtr& locatorInfo,
				      bool collocationOptimization)
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
    return new IndirectReference(_instance, ident, context, facet, mode, secure,
				 adapterId, routerInfo, locatorInfo, collocationOptimization);
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
				      const Context& context,
				      const string& facet,
				      Reference::Mode mode,
				      const vector<Ice::ConnectionIPtr>& fixedConnections)
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
    return new FixedReference(_instance, ident, context, facet, mode, fixedConnections);
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
	ProxyParseException ex(__FILE__, __LINE__);
	ex.str = str;
	throw ex;
    }
    
    //
    // Extract the identity, which may be enclosed in single
    // or double quotation marks.
    //
    string idstr;
    end = IceUtil::checkQuote(s, beg);
    if(end == string::npos)
    {
	ProxyParseException ex(__FILE__, __LINE__);
	ex.str = str;
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
	ex.str = str;
	throw ex;
    }

    //
    // Parsing the identity may raise IdentityParseException.
    //
    Identity ident = stringToIdentity(idstr);

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
            ex.str = str;
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
	    ex.str = str;
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
                end = IceUtil::checkQuote(s, beg);
                if(end == string::npos)
                {
		    ProxyParseException ex(__FILE__, __LINE__);
		    ex.str = str;
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
		    ex.str = str;
		    throw ex;
		}

		if(!IceUtil::unescapeString(argument, 0, argument.size(), facet))
		{
		    ProxyParseException ex(__FILE__, __LINE__);
		    ex.str = str;
		    throw ex;
		}

		break;
	    }

	    case 't':
	    {
		if(!argument.empty())
		{
		    ProxyParseException ex(__FILE__, __LINE__);
		    ex.str = str;
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
		    ex.str = str;
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
		    ex.str = str;
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
		    ex.str = str;
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
		    ex.str = str;
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
		    ex.str = str;
		    throw ex;
		}
		secure = true;
		break;
	    }

	    default:
	    {
		ProxyParseException ex(__FILE__, __LINE__);
		ex.str = str;
		throw ex;
	    }
	}
    }

    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
    LocatorInfoPtr locatorInfo = _instance->locatorManager()->get(getDefaultLocator());

    if(beg == string::npos)
    {
	return create(ident, Context(), facet, mode, secure, "", routerInfo, locatorInfo, true);
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
		EndpointPtr endp = _instance->endpointFactoryManager()->create(es);
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
	        EndpointParseException ex(__FILE__, __LINE__);
		ex.str = unknownEndpoints.front();
		throw ex;
	    }
	    else if(unknownEndpoints.size() != 0 && 
	    	    _instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.Endpoints", 1) > 0)
	    {
		Warning out(_instance->logger());
	        out << "Proxy contains unknown endpoints:";
	        for(unsigned int idx = 0; idx < unknownEndpoints.size(); ++idx)
		{
		    out << " `" << unknownEndpoints[idx] << "'";
		}
	    }

	    return create(ident, Context(), facet, mode, secure, endpoints, routerInfo, true);
	    break;
	}
	case '@':
	{
	    beg = s.find_first_not_of(delim, beg + 1);
	    if(beg == string::npos)
	    {
		ProxyParseException ex(__FILE__, __LINE__);
		ex.str = str;
		throw ex;
	    }

	    end = IceUtil::checkQuote(s, beg);
	    if(end == string::npos)
	    {
		ProxyParseException ex(__FILE__, __LINE__);
		ex.str = str;
		throw ex;
	    }
	    else if(end == 0)
	    {
		end = s.find_first_of(delim, beg);
		if(end == string::npos)
		{
		    end = s.size();
		}
	    }
	    else
	    {
		beg++; // Skip leading quote
	    }

	    if(!IceUtil::unescapeString(s, beg, end, adapter) || adapter.size() == 0)
	    {
		ProxyParseException ex(__FILE__, __LINE__);
		ex.str = str;
		throw ex;
	    }
	    return create(ident, Context(), facet, mode, secure, adapter, routerInfo, locatorInfo, true);
	    break;
	}
	default:
	{
	    ProxyParseException ex(__FILE__, __LINE__);
	    ex.str = str;
	    throw ex;
	}
    }
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

    vector<EndpointPtr> endpoints;
    string adapterId;

    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
    LocatorInfoPtr locatorInfo = _instance->locatorManager()->get(getDefaultLocator());

    Ice::Int sz;
    s->readSize(sz);
    
    if(sz > 0)
    {
	endpoints.reserve(sz);
	while(sz--)
	{
	    EndpointPtr endpoint = _instance->endpointFactoryManager()->read(s);
	    endpoints.push_back(endpoint);
	}
	return create(ident, Context(), facet, mode, secure, endpoints, routerInfo, true);
    }
    else
    {
	s->read(adapterId);
	return create(ident, Context(), facet, mode, secure, adapterId, routerInfo, locatorInfo, true);
    }
}

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

IceInternal::ReferenceFactory::ReferenceFactory(const InstancePtr& instance) :
    _instance(instance)
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
    _defaultRouter = 0;
    _defaultLocator = 0;
}
