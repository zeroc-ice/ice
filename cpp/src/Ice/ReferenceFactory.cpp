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
#include <Ice/Instance.h>
#include <Ice/IdentityUtil.h>
#include <Ice/Endpoint.h>
#include <Ice/RouterInfo.h>

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
				      const vector<EndpointPtr>& origEndpoints,
				      const vector<EndpointPtr>& endpoints,
				      const RouterInfoPtr& routerInfo,
				      const ObjectAdapterPtr& reverseAdapter)
{
    Mutex::Lock sync(*this);

    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    //
    // Create new reference
    //
    ReferencePtr ref = new Reference(_instance, ident, facet, mode, secure,
				     origEndpoints, endpoints,
				     routerInfo, reverseAdapter);

    //
    // If we already have an equivalent reference, use such equivalent
    // reference. Otherwise add the new reference to the reference
    // set.
    //
    set<ReferencePtr>::iterator p = _references.end();
    
    if (_referencesHint != _references.end())
    {
	if (*_referencesHint == ref)
	{
	    p = _referencesHint;
	}
    }
    
    if (p == _references.end())
    {
	p = _references.find(ref);
    }
    
    if (p == _references.end())
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
    if (++_evict >= 10)
    {
	_evict = 0;
	p = _references.begin();
	while (p != _references.end())
	{
	    if ((*p)->__getRef() == 1)
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
    if (beg == string::npos)
    {
	throw ReferenceParseException(__FILE__, __LINE__);
    }
    
    end = s.find_first_of(delim + ":", beg);
    if (end == string::npos)
    {
	end = s.length();
    }
    
    if (beg == end)
    {
	throw ReferenceParseException(__FILE__, __LINE__);
    }

    Identity ident = stringToIdentity(s.substr(beg, end - beg));
    string facet;
    Reference::Mode mode = Reference::ModeTwoway;
    bool secure = false;

    while (true)
    {
	beg = s.find_first_not_of(delim, end);
	if (beg == string::npos)
	{
	    break;
	}

        if (s[beg] == ':')
        {
            break;
        }
        
	end = s.find_first_of(delim + ":", beg);
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
	    throw ReferenceParseException(__FILE__, __LINE__);
	}
	
	string argument;
	string::size_type argumentBeg = str.find_first_not_of(delim, end);
	if (argumentBeg != string::npos && str[argumentBeg] != '-')
	{
	    beg = argumentBeg;
	    end = str.find_first_of(delim + ":", beg);
	    if (end == string::npos)
	    {
		end = str.length();
	    }
	    argument = str.substr(beg, end - beg);
	}

	//
	// If any new options are added here,
	// IceInternal::Reference::toString() must be updated as well.
	//
	switch (option[1])
	{
	    case 'f':
	    {
		if (argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		facet = argument;
		break;
	    }

	    case 't':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		mode = Reference::ModeTwoway;
		break;
	    }

	    case 'o':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		mode = Reference::ModeOneway;
		break;
	    }

	    case 'O':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		mode = Reference::ModeBatchOneway;
		break;
	    }

	    case 'd':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		mode = Reference::ModeDatagram;
		break;
	    }

	    case 'D':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		mode = Reference::ModeBatchDatagram;
		break;
	    }

	    case 's':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		secure = true;
		break;
	    }

	    default:
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		throw ReferenceParseException(__FILE__, __LINE__);
	    }
	}
    }

    vector<EndpointPtr> origEndpoints;
    vector<EndpointPtr> endpoints;

    bool orig = true;
    while (end < s.length() && s[end] == ':')
    {
	beg = end + 1;
	
	end = s.find(':', beg);
	if (end == string::npos)
	{
	    end = s.length();
	}

	if (beg == end) // "::"
	{
	    if (!orig)
	    {
		throw ReferenceParseException(__FILE__, __LINE__);
	    }

	    orig = false;
	    continue;
	}
	
	string es = s.substr(beg, end - beg);
	EndpointPtr endp = Endpoint::endpointFromString(_instance, es);

	if (orig)
	{
	    origEndpoints.push_back(endp);
	}
	else
	{
	    endpoints.push_back(endp);
	}
    }

    if (orig)
    {
	endpoints = origEndpoints;
    }

    if (!origEndpoints.size() || !endpoints.size())
    {
	throw ReferenceParseException(__FILE__, __LINE__);
    }

    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
    return create(ident, facet, mode, secure, origEndpoints, endpoints, routerInfo, 0);
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
    if (mode < 0 || mode > Reference::ModeLast)
    {
	throw ProxyUnmarshalException(__FILE__, __LINE__);
    }

    bool secure;
    s->read(secure);

    vector<EndpointPtr> origEndpoints;
    vector<EndpointPtr> endpoints;

    Ice::Int sz;
    s->read(sz);
    origEndpoints.reserve(sz);
    while (sz--)
    {
	EndpointPtr endpoint;
	Endpoint::streamRead(s, endpoint);
	origEndpoints.push_back(endpoint);
    }

    bool same;
    s->read(same);
    if (same) // origEndpoints == endpoints
    {
	endpoints = origEndpoints;
    }
    else
    {
	s->read(sz);
	endpoints.reserve(sz);
	while (sz--)
	{
	    EndpointPtr endpoint;
	    Endpoint::streamRead(s, endpoint);
	    origEndpoints.push_back(endpoint);
	}
    }

    RouterInfoPtr routerInfo = _instance->routerManager()->get(getDefaultRouter());
    return create(ident, facet, mode, secure, origEndpoints, endpoints, routerInfo, 0);
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

    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _instance = 0;
    _defaultRouter = 0;
    _references.clear();
    _referencesHint = _references.end();
}
