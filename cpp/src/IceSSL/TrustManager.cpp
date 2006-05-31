// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/TrustManager.h>
#include <IceSSL/RFC2253.h>

#include <Ice/Properties.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace IceSSL;

void IceInternal::incRef(IceSSL::TrustManager* p) { p->__incRef(); }
void IceInternal::decRef(IceSSL::TrustManager* p) { p->__decRef(); }

TrustManager::TrustManager(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    _traceLevel = properties->getPropertyAsIntWithDefault("IceSSL.TrustOnly.Trace", 0);
    string key;
    try
    {
	key = "IceSSL.TrustOnly";
	_all = RFC2253::parse(properties->getProperty(key));
	key = "IceSSL.TrustOnly.Client";
	_client = RFC2253::parse(properties->getProperty(key));
	key = "IceSSL.TrustOnly.Server";
	_allServer = RFC2253::parse(properties->getProperty(key));
	Ice::PropertyDict dict = properties->getPropertiesForPrefix("IceSSL.TrustOnly.Server.");
	for(Ice::PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
	{
	    string name = p->first.substr(string("IceSSL.TrustOnly.Server.").size());
	    key = p->first;
	    _server[name] = RFC2253::parse(p->second);
	}
    }
    catch(const RFC2253::ParseException& e)
    {
	Ice::PluginInitializationException ex(__FILE__, __LINE__);
	ex.reason = "IceSSL: unable to initialize. property: " + key  + " reason: " + e.reason;
	throw ex;
    }
}

bool
TrustManager::verify(const ConnectionInfo& info)
{
    list< list< list< pair<string, string> > > > trustset;
    if(_all.size() > 0)
    {
	trustset.push_back(_all);
    }

    if(info.incoming)
    {
	if(_allServer.size() > 0)
	{
	    trustset.push_back(_allServer);
	}
	if(info.adapterName.size() > 0)
	{
	    map<string, list< list< pair<string, string> > > >::
		const_iterator p = _server.find(info.adapterName);
	    if(p != _server.end())
	    {
		trustset.push_back(p->second);
	    }
	}
    }
    else
    {
	if(_client.size() > 0)
	{
	    trustset.push_back(_client);
	}
    }


    //
    // If there is nothing to match against, then we accept the cert.
    //
    if(trustset.size() == 0)
    {
	return true;
    }

    //
    // If there is no certificate then we match false.
    //
    if(info.certs.size() != 0)
    {
	try
	{
	    //
	    // Decompose the subject DN into the RDNs.
	    //
	    if(_traceLevel > 0)
	    {
		Ice::Trace trace(_communicator->getLogger(), "TrustManager");
		trace << "peer DN: " << info.certs[0]->getSubjectDN();
	    }
	    list< list<pair<string, string> > > alldn = RFC2253::parse(info.certs[0]->getSubjectDN());
	    if(alldn.size() != 1)
	    {
		Ice::Warning warn(_communicator->getLogger());
		warn << "certificate DN parsed more than one DN. dn: " + info.certs[0]->getSubjectDN();
		return false;
	    }
	    list<pair<string, string> > dn = alldn.front();

	    //
	    // Try matching against everything in the trust set.
	    //
	    for(list< list< list<pair<string, string> > > >::const_iterator p = trustset.begin();
		p != trustset.end(); ++p)
	    {
		if(match(*p, dn))
		{
		    return true;
		}
	    }
	}
	catch(const RFC2253::ParseException& e)
	{
	    Ice::Warning warn(_communicator->getLogger());
	    warn << "certificate DN failed to parse. DN: " + info.certs[0]->getSubjectDN() + " reason: " + e.reason;
	}
    }
    return false;
}

bool
TrustManager::match(const list< list< pair<string, string> > >& matchRDNset,
		    const list< pair<string, string> >& subjectRDNs) const
{
    for(list< list< pair<string, string> > >::const_iterator r = matchRDNset.begin(); r != matchRDNset.end(); ++r)
    {
	for(list< pair<string, string> >::const_iterator p = r->begin(); p != r->end(); ++p)
	{
	    bool found = false;
	    for(list< pair<string, string> >::const_iterator q = subjectRDNs.begin(); q != subjectRDNs.end(); ++q)
	    {
		if(p->first == q->first)
		{
		    found = true;
		    if(p->second != q->second)
		    {
			return false;
		    }
		}
	    }
	    if(!found)
	    {
		return false;
	    }
	}
    }
    return true;
}
