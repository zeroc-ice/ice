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
#include <Ice/Stream.h>
#include <Ice/LocalException.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Reference* p) { p->__incRef(); }
void IceInternal::decRef(Reference* p) { p->__decRef(); }

IceInternal::Reference::Reference(const InstancePtr& inst, const string& ident,
				  const vector<EndpointPtr>& origEndpts, const vector<EndpointPtr>& endpts) :
    instance(inst),
    identity(ident),
    mode(ModeTwoway),
    origEndpoints(origEndpts),
    endpoints(endpts)
{
}

IceInternal::Reference::Reference(const InstancePtr& inst, const string& str) :
    instance(inst),
    mode(ModeTwoway)
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

    transform(s.begin(), s.end(), s.begin(), tolower);

    while (true)
    {
	beg = s.find_first_not_of(delim, end);
	if (beg == string::npos)
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

	switch (option[1])
	{
	    case 't':
	    {
		const_cast<Mode&>(mode) = ModeTwoway;
		break;
	    }

	    case 'o':
	    {
		const_cast<Mode&>(mode) = ModeOneway;
		break;
	    }

	    case 's':
	    {
		const_cast<Mode&>(mode) = ModeSecure;
		break;
	    }

	    case 'd':
	    {
		const_cast<Mode&>(mode) = ModeDatagram;
		break;
	    }

	    default:
	    {
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

	if(orig)
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
}

IceInternal::Reference::Reference(Stream* s) :
    instance(s->instance()),
    mode(ModeTwoway)
{
    s->read(const_cast<string&>(identity));

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
}

void
IceInternal::Reference::streamWrite(Stream* s) const
{
    s->write(identity);

    vector<EndpointPtr>::const_iterator p;

    s->write(Ice::Int(origEndpoints.size()));
    for (p = origEndpoints.begin(); p != origEndpoints.end(); ++p)
    {
	(*p)->streamWrite(s);
    }

    if(endpoints == origEndpoints)
    {
	s->write(true);
    }
    else
    {
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
    
    if(endpoints != origEndpoints)
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
	return new Reference(instance, newIdentity, origEndpoints, endpoints);
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
    
    ReferencePtr ref(new Reference(instance, identity, newOrigEndpoints, newEndpoints));
    
    if (*ref.get() == *this)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }

    return ref;
}

ReferencePtr
IceInternal::Reference::changeMode(Mode m) const
{
    if (m == mode)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	ReferencePtr ref(new Reference(instance, identity, origEndpoints, endpoints));
	const_cast<Mode&>(ref->mode) = m;
	return ref;
    }
}

ReferencePtr
IceInternal::Reference::changeEndpoints(const std::vector<EndpointPtr>& endpts) const
{
    if (endpts == endpoints)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	ReferencePtr ref(new Reference(instance, identity, origEndpoints, endpts));
	return ref;
    }
}

bool
IceInternal::Reference::operator==(const Reference& r) const
{
    return !operator!=(r);
}

bool
IceInternal::Reference::operator!=(const Reference& r) const
{
    if (this == &r)
    {
	return false;
    }
    
    if (identity != r.identity)
    {
	return true;
    }

    if (mode != r.mode)
    {
	return true;
    }

    if (origEndpoints != r.origEndpoints)
    {
	return true;
    }

    if (endpoints != r.endpoints)
    {
	return true;
    }

    return false;
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

    if (mode < r.mode)
    {
	return true;
    }
    else if (mode != r.mode)
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
