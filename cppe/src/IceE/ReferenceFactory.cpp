// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ReferenceFactory.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/IdentityUtil.h>
#include <IceE/EndpointFactory.h>
#ifndef ICEE_NO_ROUTER
#  include <IceE/RouterInfo.h>
#endif
#ifndef ICEE_NO_LOCATOR
#  include <IceE/LocatorInfo.h>
#endif
#include <IceE/BasicStream.h>
#include <IceE/StringUtil.h>

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
				      const vector<EndpointPtr>& endpoints
#ifndef ICEE_NO_ROUTER
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
    return new DirectReference(_instance, ident, context, facet, mode, endpoints
#ifndef ICEE_NO_ROUTER
    			       , routerInfo
#endif
			       );
}

#ifndef ICEE_NO_LOCATOR

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
				      const Context& context,
				      const string& facet,
				      Reference::Mode mode,
				      const string& adapterId
#ifndef ICEE_NO_ROUTER
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
    return new IndirectReference(_instance, ident, context, facet, mode,
				 adapterId
#ifndef ICEE_NO_ROUTER
				 , routerInfo
#endif
				 , locatorInfo);
}

#endif

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
				      const Context& context,
				      const string& facet,
				      Reference::Mode mode,
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
    end = Ice::checkQuote(s, beg);
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
                end = Ice::checkQuote(s, beg);
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

		if(!Ice::unescapeString(argument, 0, argument.size(), facet))
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

#ifndef ICEE_NO_BATCH
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
#endif

	    default:
	    {
		ProxyParseException ex(__FILE__, __LINE__);
		ex.str = str;
		throw ex;
	    }
	}
    }

#ifndef ICEE_NO_ROUTER
    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
#endif
#ifndef ICEE_NO_LOCATOR
    LocatorInfoPtr locatorInfo = _instance->locatorManager()->get(getDefaultLocator());
#endif

    if(beg == string::npos)
    {
#ifndef ICEE_NO_LOCATOR
	return create(ident, Context(), facet, mode, ""
#ifndef ICEE_NO_ROUTER
		     , routerInfo
#endif
		     , locatorInfo
		     );
#else	
	ProxyParseException ex(__FILE__, __LINE__);
	ex.str = str;
	throw ex;
#endif
    }

    vector<EndpointPtr> endpoints;

    switch(s[beg])
    {
	case ':':
	{
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
		endpoints.push_back(endp);
	    }
	    return create(ident, Context(), facet, mode, endpoints
#ifndef ICEE_NO_ROUTER
	    		  , routerInfo
#endif
			  );
	    break;
	}
#ifndef ICEE_NO_LOCATOR
	case '@':
	{
	    beg = s.find_first_not_of(delim, beg + 1);
	    if(beg == string::npos)
	    {
		ProxyParseException ex(__FILE__, __LINE__);
		ex.str = str;
		throw ex;
	    }

	    end = Ice::checkQuote(s, beg);
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

	    if(!Ice::unescapeString(s, beg, end, adapter) || adapter.size() == 0)
	    {
		ProxyParseException ex(__FILE__, __LINE__);
		ex.str = str;
		throw ex;
	    }
	    return create(ident, Context(), facet, mode, adapter
#ifndef ICEE_NO_ROUTER
	    		  , routerInfo
#endif
			  , locatorInfo
			  );
	    break;
	}
#endif
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

    vector<EndpointPtr> endpoints;
    string adapterId;

#ifndef ICEE_NO_ROUTER
    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
#endif
#ifndef ICEE_NO_LOCATOR
    LocatorInfoPtr locatorInfo = _instance->locatorManager()->get(getDefaultLocator());
#endif

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
	return create(ident, Context(), facet, mode, endpoints
#ifndef ICEE_NO_ROUTER
		      , routerInfo
#endif
		      );
    }
    else
    {
#ifndef ICEE_NO_LOCATOR
	s->read(adapterId);
	return create(ident, Context(), facet, mode, adapterId
#ifndef ICEE_NO_ROUTER
		      , routerInfo
#endif
		      , locatorInfo
		      );
#else
	throw ProxyUnmarshalException(__FILE__, __LINE__);
#endif
    }
}

#ifndef ICEE_NO_ROUTER

void
IceInternal::ReferenceFactory::setDefaultRouter(const RouterPrx& defaultRouter)
{
    Ice::Mutex::Lock sync(*this);
    _defaultRouter = defaultRouter;
}

RouterPrx
IceInternal::ReferenceFactory::getDefaultRouter() const
{
    Ice::Mutex::Lock sync(*this);
    return _defaultRouter;
}

#endif

#ifndef ICEE_NO_LOCATOR

void
IceInternal::ReferenceFactory::setDefaultLocator(const LocatorPrx& defaultLocator)
{
    Ice::Mutex::Lock sync(*this);
    _defaultLocator = defaultLocator;
}

LocatorPrx
IceInternal::ReferenceFactory::getDefaultLocator() const
{
    Ice::Mutex::Lock sync(*this);
    return _defaultLocator;
}

#endif

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
#ifndef ICEE_NO_ROUTER
    _defaultRouter = 0;
#endif
#ifndef ICEE_NO_LOCATOR
    _defaultLocator = 0;
#endif
}
