// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONFIG_H
#define ICE_SSL_CONFIG_H

#include <dom/DOM.hpp>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/SslCertificateDesc.h>
#include <Ice/SslGeneralConfig.h>
#include <Ice/SslCertificateAuthority.h>
#include <Ice/SslBaseCerts.h>
#include <Ice/SslTempCerts.h>
#include <string>

namespace IceSecurity
{

namespace Ssl
{

class Parser
{

public:
    // Constructor, based on the indicated file.
    Parser(const std::string&);
    Parser(const std::string&, const std::string&);
    ~Parser();

    // Performs a complete parsing of the file.
    void process();

    // Loads the Client/Server portions of the config.
    bool loadClientConfig(GeneralConfig&, CertificateAuthority&, BaseCertificates&);
    bool loadServerConfig(GeneralConfig&, CertificateAuthority&, BaseCertificates&, TempCertificates&);

    inline void setTrace(IceInternal::TraceLevelsPtr traceLevels) { _traceLevels = traceLevels; };
    inline bool isTraceSet() const { return _traceLevels; };

    inline void setLogger(Ice::LoggerPtr traceLevels) { _logger = traceLevels; };
    inline bool isLoggerSet() const { return _logger; };

private:

    DOM_Node _root;
    std::string _configFile;
    std::string _configPath;

    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;

    // Tree walking utility methods.
    void popRoot(std::string&, std::string&, std::string&);
    DOM_Node find(std::string&);
    DOM_Node find(DOM_Node, std::string&);

    // Loading of the base elements of the file.
    void getGeneral(DOM_Node, GeneralConfig&);
    void getCertAuth(DOM_Node, CertificateAuthority&);
    void getBaseCerts(DOM_Node, BaseCertificates&);
    void getTempCerts(DOM_Node, TempCertificates&);

    // Loading of temporary certificates/params (Ephemeral Keys).
    void loadDHParams(DOM_Node, TempCertificates&);
    void loadRSACert(DOM_Node, TempCertificates&);
    void loadDSACert(DOM_Node, TempCertificates&);

    // Populates classes with information from the indicated node in the parse tree.
    void getCert(DOM_Node, CertificateDesc&);
    void getDHParams(DOM_Node, DiffieHellmanParamsFile&);

    // Populate a certificate file object, basis of all certificates.
    void loadCertificateFile(DOM_Node, CertificateFile&);
    int  parseEncoding(std::string&);

    std::string toString(const DOMString&);
};

}

}

#endif
