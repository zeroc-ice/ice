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

void IceInternal::incRef(Reference* p) { p->__incRef(); }
void IceInternal::decRef(Reference* p) { p->__decRef(); }

IceInternal::Reference::Reference(const InstancePtr& inst, const string& ident, Mode md, bool sec,
				  const vector<EndpointPtr>& origEndpts, const vector<EndpointPtr>& endpts) :
    instance(inst),
    identity(ident),
    mode(md),
    secure(sec),
    origEndpoints(origEndpts),
    endpoints(endpts)
{
}

IceInternal::Reference::Reference(const InstancePtr& inst, const string& str) :
    instance(inst),
    mode(ModeTwoway),
    secure(false)
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

	    case 'O':
	    {
		const_cast<Mode&>(mode) = ModeBatchOneway;
		break;
	    }

	    case 'd':
	    {
		const_cast<Mode&>(mode) = ModeDatagram;
		break;
	    }

	    case 'D':
	    {
		const_cast<Mode&>(mode) = ModeBatchDatagram;
		break;
	    }

	    case 's':
	    {
		const_cast<bool&>(secure) = true;
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

IceInternal::Reference::Reference(const string& ident, BasicStream* s) :
    instance(s->instance()),
    identity(ident),
    mode(ModeTwoway),
    secure(false)
{
    //
    // Don't read the identity here. Operations calling this
    // constructor read the identity, and pass it as a parameter.
    //

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
IceInternal::Reference::streamWrite(BasicStream* s) const
{
    //
    // Don't write the identity here. Operations calling streamWrite
    // write the identity.
    //

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
	return new Reference(instance, newIdentity, mode, secure, origEndpoints, endpoints);
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
    
    ReferencePtr ref(new Reference(instance, identity, mode, secure, newOrigEndpoints, newEndpoints));
    
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
	return new Reference(instance, identity, newMode, secure, origEndpoints, endpoints);
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
	return new Reference(instance, identity, mode, newSecure, origEndpoints, endpoints);
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
	return new Reference(instance, identity, mode, secure, origEndpoints, newEndpoints);
    }
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
