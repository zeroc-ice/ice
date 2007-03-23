// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Ice/Network.h>

using namespace std;
using namespace IceSSL;

IceUtil::Shared* IceInternal::upCast(IceSSL::TrustManager* p) { return p; }

TrustManager::TrustManager(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    _traceLevel = properties->getPropertyAsInt("IceSSL.Trace.Security");
    string key;
    try
    {
        key = "IceSSL.TrustOnly";
        _all = parse(properties->getProperty(key));
        key = "IceSSL.TrustOnly.Client";
        _client = parse(properties->getProperty(key));
        key = "IceSSL.TrustOnly.Server";
        _allServer = parse(properties->getProperty(key));
        Ice::PropertyDict dict = properties->getPropertiesForPrefix("IceSSL.TrustOnly.Server.");
        for(Ice::PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
        {
            string name = p->first.substr(string("IceSSL.TrustOnly.Server.").size());
            key = p->first;
            _server[name] = parse(p->second);
        }
    }
    catch(const ParseException& e)
    {
        Ice::PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceSSL: invalid property " + key  + ":\n" + e.reason;
        throw ex;
    }
}

bool
TrustManager::verify(const ConnectionInfo& info)
{
    list<list<DistinguishedName> > trustset;
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
            map<string, list<DistinguishedName> >::const_iterator p = _server.find(info.adapterName);
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
        DistinguishedName subject = info.certs[0]->getSubjectDN();
        if(_traceLevel > 0)
        {
            Ice::Trace trace(_communicator->getLogger(), "Security");
            if(info.incoming)
            {
                trace << "trust manager evaluating client:\n"
                      << "subject = " << string(subject) << '\n'
                      << "adapter = " << info.adapterName << '\n'
                      << "local addr = " << IceInternal::addrToString(info.localAddr) << '\n'
                      << "remote addr = " << IceInternal::addrToString(info.remoteAddr);
            }
            else
            {
                trace << "trust manager evaluating server:\n"
                      << "subject = " << string(subject) << '\n'
                      << "local addr = " << IceInternal::addrToString(info.localAddr) << '\n'
                      << "remote addr = " << IceInternal::addrToString(info.remoteAddr);
            }
        }
        
        //
        // Try matching against everything in the trust set.
        //
        for(list<list<DistinguishedName> >::const_iterator p = trustset.begin(); p != trustset.end(); ++p)
        {
            if(_traceLevel > 1)
            {
                Ice::Trace trace(_communicator->getLogger(), "Security");
                trace << "trust manager matching PDNs:\n";
                for(list<DistinguishedName>::const_iterator r = p->begin(); r != p->end(); ++r)
                {
                    if(r != p->begin())
                    {
                        trace << ';';
                    }
                    trace << string(*r);
                }
            }
            if(match(*p, subject))
            {
                return true;
            }
        }
    }

    return false;
}

bool
TrustManager::match(const list< DistinguishedName>& matchSet, const DistinguishedName& subject) const
{
    for(list<DistinguishedName>::const_iterator r = matchSet.begin(); r != matchSet.end(); ++r)
    {
        if(subject.match(*r))
        {
            return true;
        }
    }
    return false;
}

list<DistinguishedName>
TrustManager::parse(const string& value) const
{
    list<DistinguishedName> result;
    if(!value.empty())
    {
        RFC2253::RDNSeqSeq dns = RFC2253::parse(value);
        for(RFC2253::RDNSeqSeq::const_iterator p = dns.begin(); p != dns.end(); ++p)
        {
            result.push_back(DistinguishedName(*p));
        }
    }
    return result;
}
