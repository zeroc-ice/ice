// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/BasicStream.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceInternal::Reference* p) { p->__incRef(); }
void IceInternal::decRef(::IceInternal::Reference* p) { p->__decRef(); }

IceInternal::Reference::Reference(const InstancePtr& inst, const string& ident, const string& fac, Mode md, bool sec,
				  const vector<EndpointPtr>& origEndpts, const vector<EndpointPtr>& endpts) :
    instance(inst),
    identity(ident),
    facet(fac),
    mode(md),
    secure(sec),
    origEndpoints(origEndpts),
    endpoints(endpts),
    hashValue(0)
{
    calcHashValue();
}

IceInternal::Reference::Reference(const InstancePtr& inst, const string& str) :
    instance(inst),
    mode(ModeTwoway),
    secure(false),
    hashValue(0)
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

    const_cast<string&>(identity) = s.substr(beg, end - beg);    

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

	switch (option[1])
	{
	    case 'f':
	    {
		if (argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		const_cast<std::string&>(facet) = argument;
		break;
	    }

	    case 't':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		const_cast<Mode&>(mode) = ModeTwoway;
		break;
	    }

	    case 'o':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		const_cast<Mode&>(mode) = ModeOneway;
		break;
	    }

	    case 'O':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		const_cast<Mode&>(mode) = ModeBatchOneway;
		break;
	    }

	    case 'd':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		const_cast<Mode&>(mode) = ModeDatagram;
		break;
	    }

	    case 'D':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		const_cast<Mode&>(mode) = ModeBatchDatagram;
		break;
	    }

	    case 's':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}

		const_cast<bool&>(secure) = true;
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
	EndpointPtr endp = Endpoint::endpointFromString(es);

	if (orig)
	{
	    const_cast<vector<EndpointPtr>&>(origEndpoints).push_back(endp);
	}
	else
	{
	    const_cast<vector<EndpointPtr>&>(endpoints).push_back(endp);
	}
    }

    if (orig)
    {
	const_cast<vector<EndpointPtr>&>(endpoints) = origEndpoints;
    }

    if (!origEndpoints.size() || !endpoints.size())
    {
	throw ReferenceParseException(__FILE__, __LINE__);
    }

    calcHashValue();
}

IceInternal::Reference::Reference(const string& ident, BasicStream* s) :
    instance(s->instance()),
    identity(ident),
    mode(ModeTwoway),
    secure(false),
    hashValue(0)
{
    //
    // Don't read the identity here. Operations calling this
    // constructor read the identity, and pass it as a parameter.
    //

    string fac;
    s->read(fac);
    const_cast<string&>(facet) = fac;

    Byte mod;
    s->read(mod);
    if (mod < 0 || mod > static_cast<Byte>(ModeBatchLast))
    {
	throw ProxyUnmarshalException(__FILE__, __LINE__);
    }
    const_cast<Mode&>(mode) = static_cast<Mode>(mod);

    bool sec;
    s->read(sec);
    const_cast<bool&>(secure) = sec;

    vector<EndpointPtr>::const_iterator p;
    Ice::Int sz;

    s->read(sz);
    const_cast<vector<EndpointPtr>&>(origEndpoints).resize(sz);
    for (p = origEndpoints.begin(); p != origEndpoints.end(); ++p)
    {
	Endpoint::streamRead(s, const_cast<EndpointPtr&>(*p));
    }

    bool same;
    s->read(same);
    if (same) // origEndpoints == endpoints
    {
	const_cast<vector<EndpointPtr>&>(endpoints) = origEndpoints;
    }
    else
    {
	s->read(sz);
	const_cast<vector<EndpointPtr>&>(endpoints).resize(sz);
	for (p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    Endpoint::streamRead(s, const_cast<EndpointPtr&>(*p));
	}
    }

    calcHashValue();
}

bool
IceInternal::Reference::operator==(const Reference& r) const
{
    if (this == &r)
    {
	return true;
    }
    
    if (identity != r.identity)
    {
	return false;
    }

    if (facet != r.facet)
    {
	return false;
    }

    if (mode != r.mode)
    {
	return false;
    }

    if (secure != r.secure)
    {
	return false;
    }

    if (origEndpoints != r.origEndpoints)
    {
	return false;
    }

    if (endpoints != r.endpoints)
    {
	return false;
    }

    return true;
}

bool
IceInternal::Reference::operator<(const Reference& r) const
{
    if (this == &r)
    {
	return false;
    }
    
    if (identity < r.identity)
    {
	return true;
    }
    else if (identity != r.identity)
    {
	return false;
    }

    if (facet < r.facet)
    {
	return true;
    }
    else if (facet != r.facet)
    {
	return false;
    }

    if (mode < r.mode)
    {
	return true;
    }
    else if (mode != r.mode)
    {
	return false;
    }
    
    if (!secure && r.secure)
    {
	return true;
    }
    else if (secure != r.secure)
    {
	return false;
    }
    
    if (origEndpoints < r.origEndpoints)
    {
	return true;
    }
    else if (origEndpoints != r.origEndpoints)
    {
	return false;
    }
    
    if (endpoints < r.endpoints)
    {
	return true;
    }
    else if (endpoints != r.endpoints)
    {
	return false;
    }
    
    return false;
}

void
IceInternal::Reference::streamWrite(BasicStream* s) const
{
    //
    // Don't write the identity here. Operations calling streamWrite
    // write the identity.
    //

    s->write(facet);
    
    s->write(static_cast<Byte>(mode));
    
    s->write(secure);
    
    vector<EndpointPtr>::const_iterator p;

    s->write(Ice::Int(origEndpoints.size()));
    for (p = origEndpoints.begin(); p != origEndpoints.end(); ++p)
    {
	(*p)->streamWrite(s);
    }

    if (endpoints == origEndpoints)
    {
	s->write(true);
    }
    else
    {
	s->write(false);
	s->write(Ice::Int(endpoints.size()));
	for (p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    (*p)->streamWrite(s);
	}
    }
}

string
IceInternal::Reference::toString() const
{
    ostringstream s;
    s << identity;

    vector<EndpointPtr>::const_iterator p;

    for (p = origEndpoints.begin(); p != origEndpoints.end(); ++p)
    {
	s << ':' << (*p)->toString();
    }
    
    if (endpoints != origEndpoints)
    {
	s << ':';
	for (p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    s << ':' << (*p)->toString();
	}
    }

    return s.str();
}

ReferencePtr
IceInternal::Reference::changeIdentity(const string& newIdentity) const
{
    if (newIdentity == identity)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return new Reference(instance, newIdentity, facet, mode, secure, origEndpoints, endpoints);
    }
}

ReferencePtr
IceInternal::Reference::changeFacet(const string& newFacet) const
{
    if (newFacet == facet)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return new Reference(instance, identity, newFacet, mode, secure, origEndpoints, endpoints);
    }
}

ReferencePtr
IceInternal::Reference::changeTimeout(int timeout) const
{
    vector<EndpointPtr>::const_iterator p;

    vector<EndpointPtr> newOrigEndpoints;
    for (p = origEndpoints.begin(); p != origEndpoints.end(); ++p)
    {
	newOrigEndpoints.push_back((*p)->timeout(timeout));
    }
    
    vector<EndpointPtr> newEndpoints;
    for (p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->timeout(timeout));
    }
    
    ReferencePtr ref(new Reference(instance, identity, facet, mode, secure, newOrigEndpoints, newEndpoints));
    
    if (*ref.get() == *this)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }

    return ref;
}

ReferencePtr
IceInternal::Reference::changeMode(Mode newMode) const
{
    if (newMode == mode)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return new Reference(instance, identity, facet, newMode, secure, origEndpoints, endpoints);
    }
}

ReferencePtr
IceInternal::Reference::changeSecure(bool newSecure) const
{
    if (newSecure == secure)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return new Reference(instance, identity, facet, mode, newSecure, origEndpoints, endpoints);
    }
}

ReferencePtr
IceInternal::Reference::changeEndpoints(const std::vector<EndpointPtr>& newEndpoints) const
{
    if (newEndpoints == endpoints)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return new Reference(instance, identity, facet, mode, secure, origEndpoints, newEndpoints);
    }
}

void
IceInternal::Reference::calcHashValue()
{
    Int h = 0;

    string::const_iterator p;

    for (p = identity.begin(); p != identity.end(); ++p)
    {
	h = 5 * h + *p;
    }

    for (p = facet.begin(); p != facet.end(); ++p)
    {
	h = 5 * h + *p;
    }

    h = 5 * h + static_cast<Int>(mode);

    h = 5 * h + static_cast<Int>(secure);

    //
    // TODO: Should we also take the endpoints into account for hash
    // calculation? Perhaps not, the code above should be good enough
    // for a good hash value.
    //

    const_cast<Int&>(hashValue) = h;
}
