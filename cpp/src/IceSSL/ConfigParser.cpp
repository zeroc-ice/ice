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

#include <IceSSL/ConfigParser.h>
#include <IceSSL/ConfigParserErrorReporter.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/Exception.h>

#include <Ice/Xerces.h>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/util/Janitor.hpp>

#include <algorithm>

using namespace std;
using namespace IceSSL;

#ifdef WINDOWS
    #define CURRENTDIR ".\\"
#else
    #define CURRENTDIR "./"
#endif

//
// Public Methods
//

IceSSL::ConfigParser::ConfigParser(const string& configFile, const TraceLevelsPtr& traceLevels,
                                   const Ice::LoggerPtr& logger) :
    _root(0),
    _configFile(configFile),
    _configPath(CURRENTDIR),
    _traceLevels(traceLevels),
    _logger(logger)
{
    assert(!configFile.empty());
}

IceSSL::ConfigParser::ConfigParser(const string& configFile, const string& configPath,
                                   const TraceLevelsPtr& traceLevels, const Ice::LoggerPtr& logger) :
    _root(0),
    _configFile(configFile),
    _configPath(configPath),
    _traceLevels(traceLevels),
    _logger(logger)
{
    assert(!configFile.empty());
    assert(!configPath.empty());
}

IceSSL::ConfigParser::~ConfigParser()
{
    if(_root)
    {
        _root->release();
    }

    ICE_XERCES_NS XMLPlatformUtils::Terminate();
}

void
IceSSL::ConfigParser::process()
{
    try
    {
        ICE_XERCES_NS XMLPlatformUtils::Initialize();
    }
    catch(const ICE_XERCES_NS XMLException& toCatch)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream s;
        s << "while parsing " << _configFile << ":\n";
	s << "xerces-c init exception: " << toString(toCatch.getMessage());

        configEx.message = s.str();

        throw configEx;
    }

    int errorCount = 0;

    ConfigParserErrorReporter errReporter(_traceLevels, _logger);

    // Create our parser, then attach an error handler to the parser.
    // The parser will call back to methods of the ConfigParserErrorHandler
    // if it discovers errors during the course of parsing the XML document.
    ICE_XERCES_NS XercesDOMParser parser;
    parser.setValidationScheme(ICE_XERCES_NS AbstractDOMParser::Val_Auto);
    parser.setDoNamespaces(false);
    parser.setDoSchema(false);
    parser.setCreateEntityReferenceNodes(false);
    parser.setErrorHandler(&errReporter);

    try
    {
        if(!isAbsolutePath(_configFile))
        {
	    // TODO: ML: _configPath.back();
            //       ASN: There is no back() method in basic_string.
#ifdef WINDOWS
            if(*(_configPath.rbegin()) != '\\')
            {
                _configPath += "\\";
            }
#else
            if(*(_configPath.rbegin()) != '/')
            {
                _configPath += "/";
            }
#endif

            XMLCh* xmlConfigPath = ICE_XERCES_NS XMLString::transcode(_configPath.c_str());
            XMLCh* xmlConfigFile = ICE_XERCES_NS XMLString::transcode(_configFile.c_str());
            ICE_XERCES_NS ArrayJanitor<XMLCh> janPath(xmlConfigPath);
            ICE_XERCES_NS ArrayJanitor<XMLCh> janFile(xmlConfigFile);
            ICE_XERCES_NS LocalFileInputSource configSource(xmlConfigPath, xmlConfigFile);

            parser.parse(configSource);
        }
        else
        {
            XMLCh* xmlConfigFile = ICE_XERCES_NS XMLString::transcode(_configFile.c_str());
            ICE_XERCES_NS ArrayJanitor<XMLCh> janFile(xmlConfigFile);
            ICE_XERCES_NS LocalFileInputSource configSource(xmlConfigFile);

            parser.parse(configSource);
        }

        errorCount = parser.getErrorCount();

        if(errorCount == 0)
        {
            // Get the root of the parse tree.
            _root = parser.adoptDocument();
        }
    }
    catch(const ICE_XERCES_NS XMLException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream s;
        s << "while parsing " << _configFile << ":\n";
	s << "xerces-c parsing error: " << toString(e.getMessage());

        configEx.message = s.str();

        throw configEx;
    }
    catch(const ICE_XERCES_NS DOMException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

	ostringstream s;
        s << "while parsing " << _configFile << ":\n";
	s << "xerces-c DOM parsing error, DOMException code: " << e.code;
        s << ", message: " << e.msg;

        configEx.message = s.str();

        throw configEx;
    }
    catch(...)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

	// occured during parsing". The latter is redundant, given
	// that it already says "while parsing".
        configEx.message = "while parsing " + _configFile + ":\n" + "unknown exception occured during parsing";

        throw configEx;
    }

    if(errorCount)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream errStr;
        errStr << dec << errorCount << " errors during parsing";

        configEx.message = errStr.str();

        string reporterErrors = errReporter.getErrors();

        if(!reporterErrors.empty())
        {
            configEx.message += "\n";
            configEx.message += reporterErrors;
        }

        throw configEx;
    }
}

bool
IceSSL::ConfigParser::loadClientConfig(GeneralConfig& general,
                                       CertificateAuthority& certAuth,
                                       BaseCertificates& baseCerts)
{
    string clientSectionString("SSLConfig:client");
    ICE_XERCES_NS DOMNode* clientSection = find(clientSectionString);

    try
    {
        // If we actually have a client section.
        if(clientSection != 0)
        {
            getGeneral(clientSection, general);
            getCertAuth(clientSection, certAuth);
            getBaseCerts(clientSection, baseCerts);
            return true;
        }
    }
    catch(const ICE_XERCES_NS DOMException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

	ostringstream s;
        s << "while loading client configuration:\n";
	s << "xerces-c DOM parsing error, DOMException code: " << e.code;
        s << ", message: " << e.msg;

        configEx.message = s.str();

        throw configEx;
    }
    
    return false;
}

bool
IceSSL::ConfigParser::loadServerConfig(GeneralConfig& general,
                                       CertificateAuthority& certAuth,
                                       BaseCertificates& baseCerts,
                                       TempCertificates& tempCerts)
{
    string serverSectionString("SSLConfig:server");
    ICE_XERCES_NS DOMNode* serverSection = find(serverSectionString);

    try
    {
        // If we actually have a client section.
        if(serverSection != 0)
        {
            getGeneral(serverSection, general);
            getCertAuth(serverSection, certAuth);
            getBaseCerts(serverSection, baseCerts);
            getTempCerts(serverSection, tempCerts);
            return true;
        }
    }
    catch(const ICE_XERCES_NS DOMException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

	ostringstream s;
        s << "while loading server configuration:\n";
	s << "xerces-c DOM parsing error, DOMException code: " << e.code;
        s << ", message: " << e.msg;

        configEx.message = s.str();

        throw configEx;
    }
    
    return false;
}

//
// Private Methods
//

// Path is of the form "sslconfig:client:general"
void
IceSSL::ConfigParser::popRoot(string& path, string& root, string& tail)
{
    string::size_type pos = path.find_first_of(':');

    if(pos != string::npos)
    {
        root = path.substr(0,pos);
        tail = path.substr(pos+1);
    }
    else
    {
        root = path;
        tail = "";
    }
}

ICE_XERCES_NS DOMNode*
IceSSL::ConfigParser::find(string& nodePath)
{
    return find(_root, nodePath);
}

ICE_XERCES_NS DOMNode*
IceSSL::ConfigParser::find(ICE_XERCES_NS DOMNode* rootNode, string& nodePath)
{
    // The target node that we're looking for.
    ICE_XERCES_NS DOMNode* tNode = 0;

    if(rootNode == 0)
    {
        return tNode;
    }

    string rootNodeName;
    string tailNodes;

    // Pop the root off the path.
    popRoot(nodePath, rootNodeName, tailNodes);

    ICE_XERCES_NS DOMNode* child = rootNode->getFirstChild();

    while(child != 0)
    {
        // Ignore any other node types - we're only interested in ELEMENT_NODEs.
        if(child->getNodeType() == ICE_XERCES_NS DOMNode::ELEMENT_NODE)
        {
            string nodeName = toString(child->getNodeName());

            if(nodeName.compare(rootNodeName) == 0)
            {
                // No further to recurse, this must be it.
                if(tailNodes.empty())
                {
                    tNode = child;
                }
                else
                {
                    // Recursive call.
                    tNode = find(child, tailNodes);
                }
            }
        }

        child = child->getNextSibling();
    }

    return tNode;
}

void
IceSSL::ConfigParser::getGeneral(ICE_XERCES_NS DOMNode* rootNode, GeneralConfig& generalConfig)
{
    if(rootNode == 0)
    {
        return;
    }

    string generalString("general");
    ICE_XERCES_NS DOMNode* general = find(rootNode, generalString);

    ICE_XERCES_NS DOMNamedNodeMap* attributes = general->getAttributes();

    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        ICE_XERCES_NS DOMNode* attribute = attributes->item(i);
        string nodeName = toString(attribute->getNodeName());
        string nodeValue = toString(attribute->getNodeValue());

        // Set the property.
        generalConfig.set(nodeName, nodeValue);
    }
}

void
IceSSL::ConfigParser::getCertAuth(ICE_XERCES_NS DOMNode* rootNode, CertificateAuthority& certAuth)
{
    if(rootNode == 0)
    {
        return;
    }

    string nodeName = "certauthority";
    ICE_XERCES_NS DOMNode* certAuthNode = find(rootNode, nodeName);

    if(certAuthNode == 0)
    {
        return;
    }

    ICE_XERCES_NS DOMNamedNodeMap* attributes = certAuthNode->getAttributes();

    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        ICE_XERCES_NS DOMNode* attribute = attributes->item(i);
        string nodeName = toString(attribute->getNodeName());
        string nodeValue = toString(attribute->getNodeValue());

        if(nodeName.compare("file") == 0)
        {
            string filename = nodeValue;

            // Just a filename, no path component, prepend path.
            if(!isAbsolutePath(filename))
            {
                filename = _configPath + filename;
            }

            certAuth.setCAFileName(filename);
        }
        else if(nodeName.compare("path") == 0)
        {
            certAuth.setCAPath(nodeValue);
        }
    }
}

void
IceSSL::ConfigParser::getBaseCerts(ICE_XERCES_NS DOMNode* rootNode, BaseCertificates& baseCerts)
{
    if(rootNode == 0)
    {
        return;
    }

    string nodeName = "basecerts";
    ICE_XERCES_NS DOMNode* baseCertsRoot = find(rootNode, nodeName);

    if(baseCertsRoot == 0)
    {
        return;
    }

    CertificateDesc rsaCert;
    CertificateDesc dsaCert;
    DiffieHellmanParamsFile dhParams;

    string rsaCertString("rsacert");
    string dsaCertString("dsacert");
    string dhParamsString("dhparams");

    getCert(find(baseCertsRoot, rsaCertString), rsaCert);
    getCert(find(baseCertsRoot, dsaCertString), dsaCert);

    getDHParams(find(baseCertsRoot, dhParamsString), dhParams);

    baseCerts = BaseCertificates(rsaCert, dsaCert, dhParams);
}

void
IceSSL::ConfigParser::getTempCerts(ICE_XERCES_NS DOMNode* rootNode, TempCertificates& tempCerts)
{
    if(rootNode == 0)
    {
        return;
    }

    string nodeName = "tempcerts";
    ICE_XERCES_NS DOMNode* tempCertsRoot = find(rootNode, nodeName);

    if(tempCertsRoot == 0)
    {
        return;
    }

    ICE_XERCES_NS DOMNode* child = tempCertsRoot->getFirstChild();

    while(child != 0)
    {
        string name = toString(child->getNodeName());

        if(name.compare("dhparams") == 0)
        {
            loadDHParams(child, tempCerts);
        }
        else if(name.compare("rsacert") == 0)
        {
            loadRSACert(child, tempCerts);
        }

        child = child->getNextSibling();
    }
}

void
IceSSL::ConfigParser::loadDHParams(ICE_XERCES_NS DOMNode* rootNode, TempCertificates& tempCerts)
{
    DiffieHellmanParamsFile dhParams;

    getDHParams(rootNode, dhParams);

    tempCerts.addDHParams(dhParams);
}

void
IceSSL::ConfigParser::loadRSACert(ICE_XERCES_NS DOMNode* rootNode, TempCertificates& tempCerts)
{
    CertificateDesc rsaCert;

    getCert(rootNode, rsaCert);

    tempCerts.addRSACert(rsaCert);
}

void
IceSSL::ConfigParser::getCert(ICE_XERCES_NS DOMNode* rootNode, CertificateDesc& certDesc)
{
    if(rootNode == 0)
    {
        return;
    }

    CertificateFile publicFile;
    CertificateFile privateFile;
    int keySize = 0;

    ICE_XERCES_NS DOMNamedNodeMap* attributes = rootNode->getAttributes();
    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        ICE_XERCES_NS DOMNode* attribute = attributes->item(i);
        string nodeName = toString(attribute->getNodeName());
        string nodeValue = toString(attribute->getNodeValue());

        if(nodeName.compare("keysize") == 0)
        {
            keySize = atoi(nodeValue.c_str());
        }
    }

    string publicString("public");
    string privateString("private");

    loadCertificateFile(find(rootNode, publicString),  publicFile);
    loadCertificateFile(find(rootNode, privateString), privateFile);

    // Initialize the certificate description.
    certDesc = CertificateDesc(keySize, publicFile, privateFile);
}

void
IceSSL::ConfigParser::getDHParams(ICE_XERCES_NS DOMNode* rootNode, DiffieHellmanParamsFile& dhParams)
{
    if(rootNode == 0)
    {
        return;
    }

    CertificateFile certFile;
    loadCertificateFile(rootNode, certFile);

    ICE_XERCES_NS DOMNamedNodeMap* attributes = rootNode->getAttributes();
    int keySize = 0;
    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        ICE_XERCES_NS DOMNode* attribute = attributes->item(i);
        string nodeName = toString(attribute->getNodeName());
        string nodeValue = toString(attribute->getNodeValue());

        if(nodeName.compare("keysize") == 0)
        {
            keySize = atoi(nodeValue.c_str());
        }
    }

    dhParams = DiffieHellmanParamsFile(keySize, certFile.getFileName(), certFile.getEncoding());
}

void
IceSSL::ConfigParser::loadCertificateFile(ICE_XERCES_NS DOMNode* rootNode, CertificateFile& certFile)
{
    if(rootNode == 0)
    {
        return;
    }

    string filename;
    int encoding = 0; // Initialize, to keep the compiler from complaining.

    ICE_XERCES_NS DOMNamedNodeMap* attributes = rootNode->getAttributes();
    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        ICE_XERCES_NS DOMNode* attribute = attributes->item(i);
        string nodeName = toString(attribute->getNodeName());
        string nodeValue = toString(attribute->getNodeValue());

        if(nodeName.compare("encoding") == 0)
        {
            encoding = parseEncoding(nodeValue);
        }
        else if(nodeName.compare("filename") == 0)
        {
            filename = nodeValue;

            // Just a filename, no path component, prepend path.
            if(!isAbsolutePath(filename))
            {
                filename = _configPath + filename;
            }
        }
    }

    certFile = CertificateFile(filename, encoding);
}

bool
IceSSL::ConfigParser::isAbsolutePath(string& pathString)
{
#ifdef WINDOWS
    // Is true if the pathString begins with a \ or if its second and third characters are ":\"

    string rootDir = ":\\";
    string pathStringInternal = pathString.substr(1);
    return ((!pathStringInternal.substr(0,rootDir.length()).compare(rootDir)) ||
        (*pathStringInternal.begin()) == '\\');
#else
    // Is true if the pathString begins with a /

    string rootDir = "/";
    return !pathString.substr(0,rootDir.length()).compare(rootDir);
#endif
}

int
IceSSL::ConfigParser::parseEncoding(string& encodingString)
{
    int encoding = 0;

    if(encodingString.compare("PEM") == 0)
    {
        encoding = SSL_FILETYPE_PEM;
    }
    else if(encodingString.compare("ASN1") == 0)
    {
        encoding = SSL_FILETYPE_ASN1;
    }

    return encoding;
}

string
IceSSL::ConfigParser::toString(const XMLCh* s)
{
    char* t = ICE_XERCES_NS XMLString::transcode(s);
    string r(t);
    delete[] t;
    return r;
}

