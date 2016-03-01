// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

IceUtil::Shared* IceSSL::upCast(IceSSL::TrustManager* p) { return p; }

TrustManager::TrustManager(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    _traceLevel = properties->getPropertyAsInt("IceSSL.Trace.Security");
    string key;
    try
    {
        key = "IceSSL.TrustOnly";
        parse(properties->getProperty(key), _rejectAll, _acceptAll);
        key = "IceSSL.TrustOnly.Client";
        parse(properties->getProperty(key), _rejectClient, _acceptClient);
        key = "IceSSL.TrustOnly.Server";
        parse(properties->getProperty(key), _rejectAllServer, _acceptAllServer);
        Ice::PropertyDict dict = properties->getPropertiesForPrefix("IceSSL.TrustOnly.Server.");
        for(Ice::PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
        {
            string name = p->first.substr(string("IceSSL.TrustOnly.Server.").size());
            key = p->first;
            list<DistinguishedName> reject, accept;
            parse(p->second, reject, accept);
            if(!reject.empty())
            {
                _rejectServer[name] = reject;
            }
            if(!accept.empty())
            {
                _acceptServer[name] = accept;
            }
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
TrustManager::verify(const NativeConnectionInfoPtr& info)
{
    list<list<DistinguishedName> > reject, accept;

    if(_rejectAll.size() > 0)
    {
        reject.push_back(_rejectAll);
    }
    if(info->incoming)
    {
        if(_rejectAllServer.size() > 0)
        {
            reject.push_back(_rejectAllServer);
        }
        if(info->adapterName.size() > 0)
        {
            map<string, list<DistinguishedName> >::const_iterator p = _rejectServer.find(info->adapterName);
            if(p != _rejectServer.end())
            {
                reject.push_back(p->second);
            }
        }
    }
    else
    {
        if(_rejectClient.size() > 0)
        {
            reject.push_back(_rejectClient);
        }
    }

    if(_acceptAll.size() > 0)
    {
        accept.push_back(_acceptAll);
    }
    if(info->incoming)
    {
        if(_acceptAllServer.size() > 0)
        {
            accept.push_back(_acceptAllServer);
        }
        if(info->adapterName.size() > 0)
        {
            map<string, list<DistinguishedName> >::const_iterator p = _acceptServer.find(info->adapterName);
            if(p != _acceptServer.end())
            {
                accept.push_back(p->second);
            }
        }
    }
    else
    {
        if(_acceptClient.size() > 0)
        {
            accept.push_back(_acceptClient);
        }
    }

    //
    // If there is nothing to match against, then we accept the cert.
    //
    if(reject.empty() && accept.empty())
    {
        return true;
    }

    //
    // If there is no certificate then we match false.
    //
    if(info->nativeCerts.size() != 0)
    {
        DistinguishedName subject = info->nativeCerts[0]->getSubjectDN();
        if(_traceLevel > 0)
        {
            Ice::Trace trace(_communicator->getLogger(), "Security");
            if(info->incoming)
            {
                trace << "trust manager evaluating client:\n"
                      << "subject = " << string(subject) << '\n'
                      << "adapter = " << info->adapterName << '\n'
                      << "local addr = " << info->localAddress << ":" << info->localPort << '\n'
                      << "remote addr = " << info->remoteAddress << ":" << info->remotePort;
            }
            else
            {
                trace << "trust manager evaluating server:\n"
                      << "subject = " << string(subject) << '\n'
                      << "local addr = " << info->localAddress << ":" << info->localPort << '\n'
                      << "remote addr = " << info->remoteAddress << ":" << info->remotePort;
            }
        }

        //
        // Fail if we match anything in the reject set.
        //
        for(list<list<DistinguishedName> >::const_iterator p = reject.begin(); p != reject.end(); ++p)
        {
            if(_traceLevel > 1)
            {
                Ice::Trace trace(_communicator->getLogger(), "Security");
                trace << "trust manager rejecting PDNs:\n";
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
                return false;
            }
        }

        //
        // Succeed if we match anything in the accept set.
        //
        for(list<list<DistinguishedName> >::const_iterator p = accept.begin(); p != accept.end(); ++p)
        {
            if(_traceLevel > 1)
            {
                Ice::Trace trace(_communicator->getLogger(), "Security");
                trace << "trust manager accepting PDNs:\n";
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

        //
        // At this point we accept the connection if there are no explicit accept rules.
        //
        return accept.empty();
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

void
TrustManager::parse(const string& value, list<DistinguishedName>& reject, list<DistinguishedName>& accept) const
{
    if(!value.empty())
    {
        RFC2253::RDNEntrySeq dns = RFC2253::parse(value);

        for(RFC2253::RDNEntrySeq::const_iterator p = dns.begin(); p != dns.end(); ++p)
        {
            if(p->negate)
            {
                reject.push_back(DistinguishedName(p->rdn));
            }
            else
            {
                accept.push_back(DistinguishedName(p->rdn));
            }
        }
    }
}
