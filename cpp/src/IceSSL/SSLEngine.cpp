// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
IceSSL::SSLEngine::verifyPeer(SOCKET fd, const string& address, const NativeConnectionInfoPtr& info)
{
    const CertificateVerifierPtr verifier = getCertificateVerifier();
    
    //
    // For an outgoing connection, we compare the proxy address (if any) against
    // fields in the server's certificate (if any).
    //
    if(!info->nativeCerts.empty() && !address.empty())
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

        //
        // Compare the peer's address against the common name.
        //
        bool certNameOK = false;
        string dn;
        string addrLower = IceUtilInternal::toLower(address);
        {
            DistinguishedName d = cert->getSubjectDN();
            dn = IceUtilInternal::toLower(string(d));
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

        //
        // Compare the peer's address against the dnsName and ipAddress
        // values in the subject alternative name.
        //
        if(!certNameOK)
        {
            certNameOK = find(ipAddresses.begin(), ipAddresses.end(), addrLower) != ipAddresses.end();
        }
        if(!certNameOK)
        {
            certNameOK = find(dnsNames.begin(), dnsNames.end(), addrLower) != dnsNames.end();
        }

        //
        // Log a message if the name comparison fails. If CheckCertName is defined,
        // we also raise an exception to abort the connection. Don't log a message if
        // CheckCertName is not defined and a verifier is present.
        //
        if(!certNameOK && (_checkCertName || (_securityTraceLevel >= 1 && !verifier)))
        {
            ostringstream ostr;
            ostr << "IceSSL: ";
            if(!_checkCertName)
            {
                ostr << "ignoring ";
            }
            ostr << "certificate validation failure:\npeer certificate does not have `" << address
                    << "' as its commonName or in its subjectAltName extension";
            if(!dn.empty())
            {
                ostr << "\nSubject DN: " << dn;
            }
            if(!dnsNames.empty())
            {
                ostr << "\nDNS names found in certificate: ";
                for(vector<string>::const_iterator p = dnsNames.begin(); p != dnsNames.end(); ++p)
                {
                    if(p != dnsNames.begin())
                    {
                        ostr << ", ";
                    }
                    ostr << *p;
                }
            }
            if(!ipAddresses.empty())
            {
                ostr << "\nIP addresses found in certificate: ";
                for(vector<string>::const_iterator p = ipAddresses.begin(); p != ipAddresses.end(); ++p)
                {
                    if(p != ipAddresses.begin())
                    {
                        ostr << ", ";
                    }
                    ostr << *p;
                }
            }
            string msg = ostr.str();
            if(_securityTraceLevel >= 1)
            {
                Trace out(_logger, _securityTraceCategory);
                out << msg;
            }
            if(_checkCertName)
            {
                SecurityException ex(__FILE__, __LINE__);
                ex.reason = msg;
                throw ex;
            }
        }
    }

    if(_verifyDepthMax > 0 && static_cast<int>(info->certs.size()) > _verifyDepthMax)
    {
        ostringstream ostr;
        ostr << (info->incoming ? "incoming" : "outgoing") << " connection rejected:\n"
                << "length of peer's certificate chain (" << info->certs.size() << ") exceeds maximum of "
                << _verifyDepthMax;
        string msg = ostr.str();
        if(_securityTraceLevel >= 1)
        {
            _logger->trace(_securityTraceCategory, msg + "\n" + IceInternal::fdToString(fd));
        }
        SecurityException ex(__FILE__, __LINE__);
        ex.reason = msg;
        throw ex;
    }

    if(!_trustManager->verify(info))
    {
        string msg = string(info->incoming ? "incoming" : "outgoing") + " connection rejected by trust manager";
        if(_securityTraceLevel >= 1)
        {
            _logger->trace(_securityTraceCategory, msg + "\n" + IceInternal::fdToString(fd));
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
            _logger->trace(_securityTraceCategory, msg + "\n" + IceInternal::fdToString(fd));
        }
        SecurityException ex(__FILE__, __LINE__);
        ex.reason = msg;
        throw ex;
    }
}
