//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_TRUST_MANAGER_H
#define ICESSL_TRUST_MANAGER_H

#include "Ice/Certificate.h"
#include "Ice/InstanceF.h"
#include "Ice/SSLConnectionInfoF.h"
#include "RFC2253.h"
#include "TrustManagerF.h"
#include <list>
#include <map>

namespace IceSSL
{
    class TrustManager
    {
    public:
        TrustManager(const IceInternal::InstancePtr&);

        bool verify(const ConnectionInfoPtr&, const std::string&);

    private:
        bool match(const std::list<DistinguishedName>&, const DistinguishedName&) const;
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
