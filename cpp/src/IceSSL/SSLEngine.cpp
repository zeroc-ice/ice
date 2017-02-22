// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/SSLEngine.h>
#include <IceSSL/TrustManager.h>

#include <IceUtil/StringUtil.h>

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>

#include <string>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceUtil::Shared* IceSSL::upCast(IceSSL::SSLEngine* p) { return p; }

IceSSL::SSLEngine::SSLEngine(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator),
    _logger(communicator->getLogger()),
    _trustManager(new TrustManager(communicator))
{
}

IceSSL::CertificateVerifierPtr
IceSSL::SSLEngine::getCertificateVerifier() const
{
    return _verifier;
}

void
IceSSL::SSLEngine::setCertificateVerifier(const IceSSL::CertificateVerifierPtr& verifier)
{
    _verifier = verifier;
}

IceSSL::PasswordPromptPtr
IceSSL::SSLEngine::getPasswordPrompt() const
{
    return _prompt;
}

void
IceSSL::SSLEngine::setPasswordPrompt(const IceSSL::PasswordPromptPtr& prompt)
{
    _prompt = prompt;
}

string
IceSSL::SSLEngine::password(bool /*encrypting*/)
{
    if(_prompt)
    {
        try
        {
            return _prompt->getPassword();
        }
        catch(...)
        {
            //
            // Don't allow exceptions to cross an OpenSSL boundary.
            //
            return string();
        }
    }
    else
    {
        return _password;
    }
}

string
IceSSL::SSLEngine::getPassword() const
{
    return _password;
}

void
IceSSL::SSLEngine::setPassword(const std::string& password)
{
    _password = password;
}

void
IceSSL::SSLEngine::initialize()
{
    const string propPrefix = "IceSSL.";
    const PropertiesPtr properties = communicator()->getProperties();

    //
    // CheckCertName determines whether we compare the name in a peer's
    // certificate against its hostname.
    //
    _checkCertName = properties->getPropertyAsIntWithDefault(propPrefix + "CheckCertName", 0) > 0;

    //
    // VerifyDepthMax establishes the maximum length of a peer's certificate
    // chain, including the peer's certificate. A value of 0 means there is
    // no maximum.
    //
    _verifyDepthMax = properties->getPropertyAsIntWithDefault(propPrefix + "VerifyDepthMax", 3);

    //
    // VerifyPeer determines whether certificate validation failures abort a connection.
    //
    _verifyPeer = properties->getPropertyAsIntWithDefault(propPrefix + "VerifyPeer", 2);

    if(_verifyPeer < 0 || _verifyPeer > 2)
    {
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceSSL: invalid value for " + propPrefix + "VerifyPeer";
        throw ex;
    }

    _securityTraceLevel = properties->getPropertyAsInt("IceSSL.Trace.Security");
    _securityTraceCategory = "Security";
}

void
IceSSL::SSLEngine::verifyPeer(const string& address, const NativeConnectionInfoPtr& info, const string& desc)
{
    const CertificateVerifierPtr verifier = getCertificateVerifier();

#if defined(ICE_USE_SCHANNEL) || \
    (defined(ICE_USE_OPENSSL) && defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x10002000L)

    //
    // For an outgoing connection, we compare the proxy address (if any) against
    // fields in the server's certificate (if any).
    //
    if(_checkCertName && !info->nativeCerts.empty() && !address.empty())
    {
        const CertificatePtr cert = info->nativeCerts[0];

        //
        // Extract the IP addresses and the DNS names from the subject
        // alternative names.
        //
        vector<pair<int, string> > subjectAltNames = cert->getSubjectAlternativeNames();
        vector<string> ipAddresses;
        vector<string> dnsNames;
        for(vector<pair<int, string> >::const_iterator p = subjectAltNames.begin(); p != subjectAltNames.end(); ++p)
        {
            if(p->first == AltNAmeIP)
            {
                ipAddresses.push_back(IceUtilInternal::toLower(p->second));
            }
            else if(p->first == AltNameDNS)
            {
                dnsNames.push_back(IceUtilInternal::toLower(p->second));
            }
        }

        bool certNameOK = false;
        string addrLower = IceUtilInternal::toLower(address);

        //
        // If address is an IP address, compare it to the subject alternative names IP adddress
        //
        if(IceInternal::isIpAddress(address))
        {
            certNameOK = find(ipAddresses.begin(), ipAddresses.end(), addrLower) != ipAddresses.end();
        }
        else
        {
            //
            // If subjectAlt is empty compare it ot the subject CN, othewise
            // compare it to the to the subject alt name dnsNames
            //
            if(dnsNames.empty())
            {
                DistinguishedName d = cert->getSubjectDN();
                string dn = IceUtilInternal::toLower(string(d));
                string cn = "cn=" + addrLower;
                string::size_type pos = dn.find(cn);
                if(pos != string::npos)
                {
                    //
                    // Ensure we match the entire common name.
                    //
                    certNameOK = (pos + cn.size() == dn.size()) || (dn[pos + cn.size()] == ',');
                }
            }
            else
            {
                certNameOK = find(dnsNames.begin(), dnsNames.end(), addrLower) != dnsNames.end();
            }
        }

        if(!certNameOK)
        {
            ostringstream ostr;
            ostr << "IceSSL: certificate validation failure: "
                 << (isIpAddress ? "IP address mismatch" : "Hostname mismatch");
            string msg = ostr.str();
            if(_securityTraceLevel >= 1)
            {
                Trace out(_logger, _securityTraceCategory);
                out << msg;
            }
            SecurityException ex(__FILE__, __LINE__);
            ex.reason = msg;
            throw ex;
        }
    }
#endif

    if(_verifyDepthMax > 0 && static_cast<int>(info->certs.size()) > _verifyDepthMax)
    {
        ostringstream ostr;
        ostr << (info->incoming ? "incoming" : "outgoing") << " connection rejected:\n"
                << "length of peer's certificate chain (" << info->certs.size() << ") exceeds maximum of "
                << _verifyDepthMax;
        string msg = ostr.str();
        if(_securityTraceLevel >= 1)
        {
            _logger->trace(_securityTraceCategory, msg + "\n" + desc);
        }
        SecurityException ex(__FILE__, __LINE__);
        ex.reason = msg;
        throw ex;
    }

    if(!_trustManager->verify(info, desc))
    {
        string msg = string(info->incoming ? "incoming" : "outgoing") + " connection rejected by trust manager";
        if(_securityTraceLevel >= 1)
        {
            _logger->trace(_securityTraceCategory, msg + "\n" + desc);
        }
        SecurityException ex(__FILE__, __LINE__);
        ex.reason = msg;
        throw ex;
    }

    if(verifier && !verifier->verify(info))
    {
        string msg = string(info->incoming ? "incoming" : "outgoing") + " connection rejected by certificate verifier";
        if(_securityTraceLevel >= 1)
        {
            _logger->trace(_securityTraceCategory, msg + "\n" + desc);
        }
        SecurityException ex(__FILE__, __LINE__);
        ex.reason = msg;
        throw ex;
    }
}

bool
IceSSL::SSLEngine::getCheckCertName() const
{
    return _checkCertName;
}

int
IceSSL::SSLEngine::getVerifyPeer() const
{
    return _verifyPeer;
}

int
IceSSL::SSLEngine::securityTraceLevel() const
{
    return _securityTraceLevel;
}

std::string
IceSSL::SSLEngine::securityTraceCategory() const
{
    return _securityTraceCategory;
}
