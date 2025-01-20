// Copyright (c) ZeroC, Inc.

#include "TrustManager.h"
#include "../Instance.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/SSL/ConnectionInfo.h"
#include "RFC2253.h"
#include "SSLUtil.h"

using namespace std;
using namespace Ice::SSL;

TrustManager::TrustManager(IceInternal::InstancePtr instance) : _instance(std::move(instance))
{
    Ice::PropertiesPtr properties = _instance->initializationData().properties;
    _traceLevel = properties->getIcePropertyAsInt("IceSSL.Trace.Security");
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
        for (const auto& p : dict)
        {
            string name = p.first.substr(string("IceSSL.TrustOnly.Server.").size());
            key = p.first;
            list<DistinguishedName> reject, accept;
            parse(p.second, reject, accept);
            if (!reject.empty())
            {
                _rejectServer[name] = reject;
            }
            if (!accept.empty())
            {
                _acceptServer[name] = accept;
            }
        }
    }
    catch (const ParseException& ex)
    {
        throw Ice::InitializationException(
            __FILE__,
            __LINE__,
            "SSL transport: invalid property '" + key + "':\n" + ex.what());
    }
}

bool
TrustManager::verify(const ConnectionInfoPtr& info) const
{
    list<list<DistinguishedName>> reject, accept;

    if (_rejectAll.size() > 0)
    {
        reject.push_back(_rejectAll);
    }
    if (info->incoming)
    {
        if (_rejectAllServer.size() > 0)
        {
            reject.push_back(_rejectAllServer);
        }
        if (info->adapterName.size() > 0)
        {
            auto p = _rejectServer.find(info->adapterName);
            if (p != _rejectServer.end())
            {
                reject.push_back(p->second);
            }
        }
    }
    else
    {
        if (_rejectClient.size() > 0)
        {
            reject.push_back(_rejectClient);
        }
    }

    if (_acceptAll.size() > 0)
    {
        accept.push_back(_acceptAll);
    }
    if (info->incoming)
    {
        if (_acceptAllServer.size() > 0)
        {
            accept.push_back(_acceptAllServer);
        }
        if (info->adapterName.size() > 0)
        {
            auto p = _acceptServer.find(info->adapterName);
            if (p != _acceptServer.end())
            {
                accept.push_back(p->second);
            }
        }
    }
    else
    {
        if (_acceptClient.size() > 0)
        {
            accept.push_back(_acceptClient);
        }
    }

    // If there is nothing to match against, then we accept the cert.
    if (reject.empty() && accept.empty())
    {
        return true;
    }

    // If there is no certificate then we match false.
    if (info->peerCertificate)
    {
        auto subject = DistinguishedName(getSubjectName(info->peerCertificate));
        if (_traceLevel > 0)
        {
            Ice::Trace trace(_instance->initializationData().logger, "Security");
            if (info->incoming)
            {
                trace << "trust manager evaluating client:\n"
                      << "subject = " << string(subject) << '\n'
                      << "adapter = " << info->adapterName << '\n';
            }
            else
            {
                trace << "trust manager evaluating server:\n"
                      << "subject = " << string(subject) << '\n';
            }
        }

        // Fail if we match anything in the reject set.
        for (const auto& p : reject)
        {
            if (_traceLevel > 1)
            {
                Ice::Trace trace(_instance->initializationData().logger, "Security");
                trace << "trust manager rejecting PDNs:\n";
                for (auto r = p.begin(); r != p.end(); ++r)
                {
                    if (r != p.begin())
                    {
                        trace << ';';
                    }
                    trace << string(*r);
                }
            }
            if (match(p, subject))
            {
                return false;
            }
        }

        // Succeed if we match anything in the accept set.
        for (const auto& p : accept)
        {
            if (_traceLevel > 1)
            {
                Ice::Trace trace(_instance->initializationData().logger, "Security");
                trace << "trust manager accepting PDNs:\n";
                for (auto r = p.begin(); r != p.end(); ++r)
                {
                    if (r != p.begin())
                    {
                        trace << ';';
                    }
                    trace << string(*r);
                }
            }
            if (match(p, subject))
            {
                return true;
            }
        }

        // At this point we accept the connection if there are no explicit accept rules.
        return accept.empty();
    }

    return false;
}

bool
TrustManager::match(const list<DistinguishedName>& matchSet, const DistinguishedName& subject) const
{
    for (const auto& r : matchSet)
    {
        if (subject.match(r))
        {
            return true;
        }
    }
    return false;
}

void
TrustManager::parse(const string& value, list<DistinguishedName>& reject, list<DistinguishedName>& accept) const
{
    if (!value.empty())
    {
        RFC2253::RDNEntrySeq dns = RFC2253::parse(value);

        for (const auto& dn : dns)
        {
            if (dn.negate)
            {
                reject.emplace_back(dn.rdn);
            }
            else
            {
                accept.emplace_back(dn.rdn);
            }
        }
    }
}
