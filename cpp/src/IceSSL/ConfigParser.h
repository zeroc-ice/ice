// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_CONFIG_PARSER_H
#define ICE_SSL_CONFIG_PARSER_H

#include <Ice/LoggerF.h>
#include <IceSSL/CertificateDesc.h>
#include <IceSSL/GeneralConfig.h>
#include <IceSSL/CertificateAuthority.h>
#include <IceSSL/BaseCerts.h>
#include <IceSSL/TempCerts.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceXML/Parser.h>

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

    IceXML::NodePtr _root;
    std::string _configFile;
    std::string _configPath;

    TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;

    // Parse tree walking utility methods.
    void popRoot(std::string&, std::string&, std::string&);
    IceXML::NodePtr find(std::string&);
    IceXML::NodePtr find(const IceXML::NodePtr&, std::string&);

    // Loading of the base elements of the file.
    void getGeneral(const IceXML::NodePtr&, GeneralConfig&);
    void getCertAuth(const IceXML::NodePtr&, CertificateAuthority&);
    void getBaseCerts(const IceXML::NodePtr&, BaseCertificates&);
    void getTempCerts(const IceXML::NodePtr&, TempCertificates&);

    // Loading of temporary certificates/params (Ephemeral Keys).
    void loadDHParams(const IceXML::NodePtr&, TempCertificates&);
    void loadRSACert(const IceXML::NodePtr&, TempCertificates&);

    // Populate with information from the indicated node in the parse tree.
    void getCert(const IceXML::NodePtr&, CertificateDesc&);
    void getDHParams(const IceXML::NodePtr&, DiffieHellmanParamsFile&);

    // Populate a certificate file object, basis of all certificates.
    void loadCertificateFile(const IceXML::NodePtr&, CertificateFile&);

    // Determines if the string represents an absolute pathname.
    bool isAbsolutePath(std::string&);

    // Parses the certificate encoding format from a string representation
    // to the proper integer value used by the underlying SSL framework.
    int parseEncoding(std::string&);
};

}

#endif
