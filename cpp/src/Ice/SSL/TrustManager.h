// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_TRUST_MANAGER_H
#define ICE_SSL_TRUST_MANAGER_H

#include "DistinguishedName.h"
#include "Ice/InstanceF.h"
#include "Ice/SSL/ConnectionInfoF.h"
#include "RFC2253.h"
#include "TrustManagerF.h"

#include <list>
#include <map>

namespace Ice::SSL
{
    class TrustManager
    {
    public:
        TrustManager(IceInternal::InstancePtr);

        [[nodiscard]] bool verify(const ConnectionInfoPtr&) const;

    private:
        [[nodiscard]] bool match(const std::list<DistinguishedName>&, const DistinguishedName&) const;
        void parse(const std::string&, std::list<DistinguishedName>&, std::list<DistinguishedName>&) const;

        int _traceLevel;
        IceInternal::InstancePtr _instance;
        std::list<DistinguishedName> _rejectAll;
        std::list<DistinguishedName> _rejectClient;
        std::list<DistinguishedName> _rejectAllServer;
        std::map<std::string, std::list<DistinguishedName>> _rejectServer;

        std::list<DistinguishedName> _acceptAll;
        std::list<DistinguishedName> _acceptClient;
        std::list<DistinguishedName> _acceptAllServer;
        std::map<std::string, std::list<DistinguishedName>> _acceptServer;
    };
}

#endif
