// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceInternal::ReferenceFactory* p) { p->__incRef(); }
void IceInternal::decRef(::IceInternal::ReferenceFactory* p) { p->__decRef(); }

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident,
				      const string& facet,
				      Reference::Mode mode,
				      bool secure,
				      bool compress,
				      const string& adapterId,
				      const vector<EndpointPtr>& endpoints,
				      const RouterInfoPtr& routerInfo,
				      const LocatorInfoPtr& locatorInfo,
				      const ObjectAdapterPtr& reverseAdapter)
{
    Mutex::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    //
    // Create new reference
    //
    ReferencePtr ref = new Reference(_instance, ident, facet, mode, secure, compress, adapterId,
				     endpoints, routerInfo, locatorInfo, reverseAdapter);

    //
    // If we already have an equivalent reference, use such equivalent
    // reference. Otherwise add the new reference to the reference
    // set.
    //
    set<ReferencePtr>::iterator p = _references.end();
    
    if(_referencesHint != _references.end())
    {
	if(*_referencesHint == ref)
	{
	    p = _referencesHint;
	}
    }
    
    if(p == _references.end())
    {
	p = _references.find(ref);
    }
    
    if(p == _references.end())
    {
	_referencesHint = _references.insert(_referencesHint, ref);
    }
    else
    {
	_referencesHint = p;
	ref = *_referencesHint;
    }

    //
    // At every 10th call, evict references which are not in use anymore.
    //
    if(++_evict >= 10)
    {
	_evict = 0;
	p = _references.begin();
	while(p != _references.end())
	{
	    if((*p)->__getRef() == 1)
	    {
		assert(p != _referencesHint);
		_references.erase(p++);
	    }
	    else
	    {
		++p;
	    }
	}
    }

    return ref;
}

ReferencePtr
IceInternal::ReferenceFactory::create(const string& str)
{
    const string delim = " \t\n\r";

    string s(str);
    string::size_type beg;
    string::size_type end = 0;

    beg = s.find_first_not_of(delim, end);
    if(beg == string::npos)
    {
	throw ProxyParseException(__FILE__, __LINE__);
    }
    
    end = s.find_first_of(delim + ":@", beg);
    if(end == string::npos)
    {
	end = s.length();
    }
    
    if(beg == end)
    {
	throw ProxyParseException(__FILE__, __LINE__);
    }

    Identity ident = stringToIdentity(s.substr(beg, end - beg));
    string facet;
    Reference::Mode mode = Reference::ModeTwoway;
    bool secure = false;
    bool compress = false;
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
	    throw ProxyParseException(__FILE__, __LINE__);
	}
	
	string argument;
	string::size_type argumentBeg = str.find_first_not_of(delim, end);
	if(argumentBeg != string::npos && str[argumentBeg] != '-')
	{
	    beg = argumentBeg;
	    end = str.find_first_of(delim + ":@", beg);
	    if(end == string::npos)
	    {
		end = str.length();
	    }
	    argument = str.substr(beg, end - beg);
	}

	//
	// If any new options are added here,
	// IceInternal::Reference::toString() must be updated as well.
	//
	switch(option[1])
	{
	    case 'f':
	    {
		if(argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__);
		}
		facet = argument;
		break;
	    }

	    case 't':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__);
		}
		mode = Reference::ModeTwoway;
		break;
	    }

	    case 'o':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__);
		}
		mode = Reference::ModeOneway;
		break;
	    }

	    case 'O':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__);
		}
		mode = Reference::ModeBatchOneway;
		break;
	    }

	    case 'd':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__);
		}
		mode = Reference::ModeDatagram;
		break;
	    }

	    case 'D':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__);
		}
		mode = Reference::ModeBatchDatagram;
		break;
	    }

	    case 's':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__);
		}
		secure = true;
		break;
	    }

	    case 'c':
	    {
		if(!argument.empty())
		{
		    throw ProxyParseException(__FILE__, __LINE__);
		}
		compress = true;
		break;
	    }

	    default:
	    {
		throw ProxyParseException(__FILE__, __LINE__);
	    }
	}
    }

    vector<EndpointPtr> endpoints;
    if(beg != string::npos)
    {
	if(s[beg] == ':')
	{
	    end = beg;
	    
	    while (end < s.length() && s[end] == ':')
	    {
		beg = end + 1;
		
		end = s.find(':', beg);
		if (end == string::npos)
		{
		    end = s.length();
		}
		
		string es = s.substr(beg, end - beg);
		EndpointPtr endp = _instance->endpointFactoryManager()->create(es);
		endpoints.push_back(endp);
	    }
	}
	else if(s[beg] == '@')
	{
	    beg = str.find_first_not_of(delim, beg + 1);
	    if (beg == string::npos)
	    {
		beg = end + 1;
	    }
	    
	    end = str.find_first_of(delim, beg);
	    if (end == string::npos)
	    {
		end = str.length();
	    }
	    
	    adapter = str.substr(beg, end - beg);
	    if(adapter.empty())
	    {
		throw ProxyParseException(__FILE__, __LINE__);
	    }
	}
    }

    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
    LocatorInfoPtr locatorInfo = _instance->locatorManager()->get(getDefaultLocator());
    return create(ident, facet, mode, secure, compress, adapter, endpoints, routerInfo, locatorInfo, 0);
}

ReferencePtr
IceInternal::ReferenceFactory::create(const Identity& ident, BasicStream* s)
{
    //
    // Don't read the identity here. Operations calling this
    // constructor read the identity, and pass it as a parameter.
    //

    string facet;
    s->read(facet);

    Byte modeAsByte;
    s->read(modeAsByte);
    Reference::Mode mode = static_cast<Reference::Mode>(modeAsByte);
    if(mode < 0 || mode > Reference::ModeLast)
    {
	throw ProxyUnmarshalException(__FILE__, __LINE__);
    }

    bool secure;
    s->read(secure);

    bool compress;
    s->read(compress);

    vector<EndpointPtr> endpoints;
    string adapterId;

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
    }
    else
    {
	s->read(adapterId);
    }

    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
    LocatorInfoPtr locatorInfo = _instance->locatorManager()->get(getDefaultLocator());
    return create(ident, facet, mode, secure, compress, adapterId, endpoints, routerInfo, locatorInfo, 0);
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
    _instance(instance),
    _referencesHint(_references.end()),
    _evict(0)
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
    _references.clear();
    _referencesHint = _references.end();
}
