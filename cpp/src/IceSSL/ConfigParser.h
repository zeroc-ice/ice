// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

#include <xercesc/dom/DOM.hpp>

namespace IceSSL
{

class ConfigParser
{
public:

    // Construction based on the indicated config file, or config file and
    // certificate path.
    ConfigParser(const std::string&, const TraceLevelsPtr&, const Ice::LoggerPtr&);
    ConfigParser(const std::string&, const std::string&, const TraceLevelsPtr&, const Ice::LoggerPtr&);
    ~ConfigParser();

    // Performs a complete parsing of the file.
    void process();

    // Loads the Client/Server portions of the config file.
    bool loadClientConfig(GeneralConfig&, CertificateAuthority&, BaseCertificates&);
    bool loadServerConfig(GeneralConfig&, CertificateAuthority&, BaseCertificates&, TempCertificates&);

private:

    DOMNode* _root;
    std::string _configFile;
    std::string _configPath;

    TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;

    // Parse tree walking utility methods.
    void popRoot(std::string&, std::string&, std::string&);
    DOMNode* find(std::string&);
    DOMNode* find(DOMNode*, std::string&);

    // Loading of the base elements of the file.
    void getGeneral(DOMNode*, GeneralConfig&);
    void getCertAuth(DOMNode*, CertificateAuthority&);
    void getBaseCerts(DOMNode*, BaseCertificates&);
    void getTempCerts(DOMNode*, TempCertificates&);

    // Loading of temporary certificates/params (Ephemeral Keys).
    void loadDHParams(DOMNode*, TempCertificates&);
    void loadRSACert(DOMNode*, TempCertificates&);

    // Populate with information from the indicated node in the parse tree.
    void getCert(DOMNode*, CertificateDesc&);
    void getDHParams(DOMNode*, DiffieHellmanParamsFile&);

    // Populate a certificate file object, basis of all certificates.
    void loadCertificateFile(DOMNode*, CertificateFile&);

    // Detemines if the string represents an absolute pathname.
    bool isAbsolutePath(std::string&);

    // Parses the certificate encoding format from a string representation
    // to the proper integer value used by the underlying SSL framework.
    int parseEncoding(std::string&);

    std::string toString(const XMLCh*);
};

}

#endif
