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

#include <Ice/LoggerF.h>
#include <IceSSL/CertificateDesc.h>
#include <IceSSL/GeneralConfig.h>
#include <IceSSL/CertificateAuthority.h>
#include <IceSSL/BaseCerts.h>
#include <IceSSL/TempCerts.h>
#include <IceSSL/TraceLevelsF.h>
#include <dom/DOM.hpp>

namespace IceSSL
{

class ConfigParser
{
public:

    // Construction based on the indicated config file, or config file and
    // certificate path.
    ConfigParser(const std::string&);
    ConfigParser(const std::string&, const std::string&);
    ~ConfigParser();

    // Performs a complete parsing of the file.
    void process();

    // Loads the Client/Server portions of the config file.
    bool loadClientConfig(GeneralConfig&, CertificateAuthority&, BaseCertificates&);
    bool loadServerConfig(GeneralConfig&, CertificateAuthority&, BaseCertificates&, TempCertificates&);

    void setTrace(const TraceLevelsPtr&);
    bool isTraceSet() const;

    void setLogger(const Ice::LoggerPtr&);
    bool isLoggerSet() const;

private:

    DOM_Node _root;
    std::string _configFile;
    std::string _configPath;

    TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;

    // Parse tree walking utility methods.
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

    // Populate with information from the indicated node in the parse tree.
    void getCert(DOM_Node, CertificateDesc&);
    void getDHParams(DOM_Node, DiffieHellmanParamsFile&);

    // Populate a certificate file object, basis of all certificates.
    void loadCertificateFile(DOM_Node, CertificateFile&);

    // Parses the certificate encoding format from a string representation
    // to the proper integer value used by the underlying SSL framework.
    int parseEncoding(std::string&);

    std::string toString(const DOMString&);
};

}

#endif
