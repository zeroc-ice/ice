// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Note: This pragma is used to disable spurious warning messages having
//       to do with the length of debug symbols exceeding 255 characters.
//       This is due to STL template identifiers expansion.
//       The MSDN Library recommends that you put this pragma directive
//       in place to avoid the warnings.
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <sstream>
#include <iostream>

#include <util/PlatformUtils.hpp>
#include <parsers/DOMParser.hpp>
#include <framework/LocalFileInputSource.hpp>
#include <util/Janitor.hpp>

#include <Ice/OpenSSL.h>
#include <Ice/SecurityException.h>
#include <Ice/SslConfigErrorReporter.h>
#include <Ice/SslConfig.h>

#include <algorithm>

using namespace std;
using namespace IceSSL;

//
// Public Methods
//

IceSSL::Parser::Parser(const string& configFile) :
               _configFile(configFile)
{
    assert(!configFile.empty());
    _configPath  = "./";
    _traceLevels = 0;
    _logger      = 0;
}

IceSSL::Parser::Parser(const string& configFile, const string& configPath) :
               _configFile(configFile),
               _configPath(configPath)
{
    assert(!configFile.empty());
    assert(!configPath.empty());
    _traceLevels = 0;
    _logger      = 0;
}

IceSSL::Parser::~Parser()
{
}

void
IceSSL::Parser::process()
{
    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch(const XMLException& toCatch)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream s;
        s << "While parsing " << _configFile << flush;
	s << "Xerces-c Init Exception: " << DOMString(toCatch.getMessage());

        configEx._message = s.str();

        throw configEx;
    }

    int errorCount = 0;

    ErrorReporter* errReporter = new ErrorReporter(_traceLevels, _logger);
    assert(errReporter != 0);

    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML document.
    DOMParser parser;
    parser.setValidationScheme(DOMParser::Val_Auto);
    parser.setDoNamespaces(false);
    parser.setDoSchema(false);
    parser.setCreateEntityReferenceNodes(false);
    parser.setToCreateXMLDeclTypeNode(true);
    parser.setErrorHandler(errReporter);

    try
    {
        string::iterator fileBegin = _configFile.begin();

        if (*fileBegin != '/')
        {
            string::reverse_iterator pathEnd = _configPath.rbegin();

            if (*pathEnd != '/')
            {
                _configPath += "/";
            }

            XMLCh* xmlConfigPath = XMLString::transcode(_configPath.c_str());
            XMLCh* xmlConfigFile = XMLString::transcode(_configFile.c_str());
            ArrayJanitor<XMLCh> janPath(xmlConfigPath);
            ArrayJanitor<XMLCh> janFile(xmlConfigFile);
            LocalFileInputSource configSource(xmlConfigPath, xmlConfigFile);

            parser.parse(configSource);
        }
        else
        {
            XMLCh* xmlConfigFile = XMLString::transcode(_configFile.c_str());
            ArrayJanitor<XMLCh> janFile(xmlConfigFile);
            LocalFileInputSource configSource(xmlConfigFile);

            parser.parse(configSource);
        }

        errorCount = parser.getErrorCount();

        if (errorCount == 0)
        {
            // Get the root of the parse tree.
            _root = parser.getDocument();
        }
    }
    catch (const XMLException& e)
    {
        if (errReporter != 0)
        {
            delete errReporter;
        }

        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream s;
        s << "While parsing " << _configFile << flush;
	s << "Xerces-c Parsing Error: " << DOMString(e.getMessage());

        configEx._message = s.str();

        throw configEx;
    }
    catch (const DOM_DOMException& e)
    {
        if (errReporter != 0)
        {
            delete errReporter;
        }

        ConfigParseException configEx(__FILE__, __LINE__);

	ostringstream s;
        s << "While parsing " << _configFile << flush;
	s << "Xerces-c DOM Parsing Error, DOMException code: " << e.code;
        s << ", message: " << e.msg;

        configEx._message = s.str();

        throw configEx;
    }
    catch (...)
    {
        if (errReporter != 0)
        {
            delete errReporter;
        }

        ConfigParseException configEx(__FILE__, __LINE__);

        configEx._message = "While parsing " + _configFile + "\n" + "An unknown error occured during parsing.";

        throw configEx;
    }

    if (errReporter != 0)
    {
        delete errReporter;
    }

    if (errorCount)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream errStr;

        errStr << dec << errorCount << " errors occured during parsing.";

        configEx._message = errStr.str();

        throw configEx;
    }
}

bool
IceSSL::Parser::loadClientConfig(GeneralConfig& general, CertificateAuthority& certAuth, BaseCertificates& baseCerts)
{
    bool retCode = false;
    string clientSectionString("SSLConfig:client");
    DOM_Node clientSection = find(clientSectionString);

    // If we actually have a client section.
    if (clientSection != 0)
    {
        getGeneral(clientSection, general);
        getCertAuth(clientSection, certAuth);
        getBaseCerts(clientSection, baseCerts);
        retCode = true;
    }
    
    return retCode;
}

bool
IceSSL::Parser::loadServerConfig(GeneralConfig& general,
                                 CertificateAuthority& certAuth,
                                 BaseCertificates& baseCerts,
                                 TempCertificates& tempCerts)
{
    bool retCode = false;
    string serverSectionString("SSLConfig:server");
    DOM_Node serverSection = find(serverSectionString);

    // If we actually have a client section.
    if (serverSection != 0)
    {
        getGeneral(serverSection, general);
        getCertAuth(serverSection, certAuth);
        getBaseCerts(serverSection, baseCerts);
        getTempCerts(serverSection, tempCerts);
        retCode = true;
    }
    
    return retCode;
}

void
IceSSL::Parser::setTrace(const IceInternal::TraceLevelsPtr& traceLevels)
{
    _traceLevels = traceLevels;
}

bool
IceSSL::Parser::isTraceSet() const 
{
    return _traceLevels;
}

void
IceSSL::Parser::setLogger(const Ice::LoggerPtr& logger)
{
    _logger = logger;
}

bool
IceSSL::Parser::isLoggerSet() const
{
    return _logger;
}

//
// Private Methods
//

// path is of the form "sslconfig:client:general"
void
IceSSL::Parser::popRoot(string& path, string& root, string& tail)
{
    string::size_type pos = path.find_first_of(':');

    if (pos != string::npos)
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

DOM_Node
IceSSL::Parser::find(string& nodePath)
{
    return find(_root, nodePath);
}

DOM_Node
IceSSL::Parser::find(DOM_Node rootNode, string& nodePath)
{
    // The target node that we're looking for.
    DOM_Node tNode;

    if (rootNode != 0)
    {
        string rootNodeName;
        string tailNodes;

        // Pop the root off the path.
        popRoot(nodePath, rootNodeName, tailNodes);

        DOM_Node child = rootNode.getFirstChild();

        while (child != 0)
        {
            // Ignore any other node types - we're only interested in ELEMENT_NODEs.
            if (child.getNodeType() == DOM_Node::ELEMENT_NODE)
            {
                string nodeName = toString(child.getNodeName());

                if (nodeName.compare(rootNodeName) == 0)
                {
                    // No further to recurse, this must be it.
                    if (tailNodes.empty())
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

            child = child.getNextSibling();
        }
    }

    return tNode;
}

void
IceSSL::Parser::getGeneral(DOM_Node rootNode, GeneralConfig& generalConfig)
{
    if (rootNode != 0)
    {
        string generalString("general");
        DOM_Node general = find(rootNode, generalString);

        DOM_NamedNodeMap attributes = general.getAttributes();

        int attrCount = attributes.getLength();

        for (int i = 0; i < attrCount; i++)
        {
            DOM_Node attribute = attributes.item(i);
            string nodeName = toString(attribute.getNodeName());
            string nodeValue = toString(attribute.getNodeValue());

            // Set the property.
            generalConfig.set(nodeName, nodeValue);
        }
    }
}

void
IceSSL::Parser::getCertAuth(DOM_Node rootNode, CertificateAuthority& certAuth)
{
    if (rootNode != 0)
    {
        string certAuthorityString("certauthority");
        DOM_Node certAuthNode = find(rootNode, certAuthorityString);

        if (certAuthNode != 0)
        {
            DOM_NamedNodeMap attributes = certAuthNode.getAttributes();

            int attrCount = attributes.getLength();

            for (int i = 0; i < attrCount; i++)
            {
                DOM_Node attribute = attributes.item(i);
                string nodeName = toString(attribute.getNodeName());
                string nodeValue = toString(attribute.getNodeValue());

                if (nodeName.compare("file") == 0)
                {
                    string filename = nodeValue;

                    // Just a filename, no path component, append path.
                    if ((filename.find("/") == string::npos) && (filename.find("\\") == string::npos))
                    {
                        filename = _configPath + filename;
                    }

                    certAuth.setCAFileName(filename);
                }
                else if (nodeName.compare("path") == 0)
                {
                    certAuth.setCAPath(nodeValue);
                }
            }
        }
    }
}

void
IceSSL::Parser::getBaseCerts(DOM_Node rootNode, BaseCertificates& baseCerts)
{
    if (rootNode != 0)
    {
        string baseCertsString("basecerts");
        DOM_Node baseCertsRoot = find(rootNode, baseCertsString);

        if (baseCertsRoot != 0)
        {
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
    }
}

void
IceSSL::Parser::getTempCerts(DOM_Node rootNode, TempCertificates& tempCerts)
{
    if (rootNode != 0)
    {
        string tempCertsString("tempcerts");
        DOM_Node tempCertsRoot = find(rootNode, tempCertsString);

        if (tempCertsRoot != 0)
        {
            DOM_Node child = tempCertsRoot.getFirstChild();

            while (child != 0)
            {
                DOMString nodeName = child.getNodeName();
                string name = toString(nodeName);

                if (name.compare("dhparams") == 0)
                {
                    loadDHParams(child, tempCerts);
                }
                else if (name.compare("rsacert") == 0)
                {
                    loadRSACert(child, tempCerts);
                }

                child = child.getNextSibling();
            }
        }
    }
}

void
IceSSL::Parser::loadDHParams(DOM_Node rootNode, TempCertificates& tempCerts)
{
    DiffieHellmanParamsFile dhParams;

    getDHParams(rootNode, dhParams);

    tempCerts.addDHParams(dhParams);
}

void
IceSSL::Parser::loadRSACert(DOM_Node rootNode, TempCertificates& tempCerts)
{
    CertificateDesc rsaCert;

    getCert(rootNode, rsaCert);

    tempCerts.addRSACert(rsaCert);
}

void
IceSSL::Parser::getCert(DOM_Node rootNode, CertificateDesc& certDesc)
{
    if (rootNode != 0)
    {
        CertificateFile publicFile;
        CertificateFile privateFile;
        int keySize = 0;

        DOM_NamedNodeMap attributes = rootNode.getAttributes();
        int attrCount = attributes.getLength();

        for (int i = 0; i < attrCount; i++)
        {
            DOM_Node attribute = attributes.item(i);
            string nodeName = toString(attribute.getNodeName());
            string nodeValue = toString(attribute.getNodeValue());

            if (nodeName.compare("keysize") == 0)
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
}

void
IceSSL::Parser::getDHParams(DOM_Node rootNode, DiffieHellmanParamsFile& dhParams)
{
    if (rootNode != 0)
    {
        CertificateFile certFile;
        loadCertificateFile(rootNode, certFile);

        DOM_NamedNodeMap attributes = rootNode.getAttributes();
        int keySize = 0;
        int attrCount = attributes.getLength();

        for (int i = 0; i < attrCount; i++)
        {
            DOM_Node attribute = attributes.item(i);
            string nodeName = toString(attribute.getNodeName());
            string nodeValue = toString(attribute.getNodeValue());

            if (nodeName.compare("keysize") == 0)
            {
                keySize = atoi(nodeValue.c_str());
            }
        }

        dhParams = DiffieHellmanParamsFile(keySize, certFile.getFileName(), certFile.getEncoding());
    }
}

void
IceSSL::Parser::loadCertificateFile(DOM_Node rootNode, CertificateFile& certFile)
{
    if (rootNode != 0)
    {
        string filename;
        int encoding = 0; // Initialize, to keep the compiler from complaining.

        DOM_NamedNodeMap attributes = rootNode.getAttributes();
        int attrCount = attributes.getLength();

        for (int i = 0; i < attrCount; i++)
        {
            DOM_Node attribute = attributes.item(i);
            string nodeName = toString(attribute.getNodeName());
            string nodeValue = toString(attribute.getNodeValue());

            if (nodeName.compare("encoding") == 0)
            {
                encoding = parseEncoding(nodeValue);
            }
            else if (nodeName.compare("filename") == 0)
            {
                filename = nodeValue;

                // Just a filename, no path component, append path.
                if ((filename.find("/") == string::npos) && (filename.find("\\") == string::npos))
                {
                    filename = _configPath + filename;
                }
            }
        }

        certFile = CertificateFile(filename, encoding);
    }
}

int
IceSSL::Parser::parseEncoding(string& encodingString)
{
    int encoding = 0;

    if (encodingString.compare("PEM") == 0)
    {
        encoding = SSL_FILETYPE_PEM;
    }
    else if (encodingString.compare("ASN1") == 0)
    {
        encoding = SSL_FILETYPE_ASN1;
    }

    return encoding;
}

string
IceSSL::Parser::toString(const DOMString& domString)
{
    char* cString = domString.transcode();

    string stlString(cString);

    delete []cString;

    return stlString;
}

