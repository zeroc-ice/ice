// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/ConfigParser.h>
#include <IceSSL/ConfigParserErrorReporter.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/Exception.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/util/Janitor.hpp>

#include <algorithm>

using namespace std;
using namespace IceSSL;

//
// Public Methods
//

IceSSL::ConfigParser::ConfigParser(const string& configFile) :
    _root(0),
    _configFile(configFile)
{
    assert(!configFile.empty());
    // TODO: ML Initialize int the initializer list. Remove unnecessary 0 initialization.
    _configPath = "./";
    _traceLevels = 0;
    _logger = 0;
}

IceSSL::ConfigParser::ConfigParser(const string& configFile, const string& configPath) :
    _root(0),
    _configFile(configFile),
    _configPath(configPath)
{
    assert(!configFile.empty());
    assert(!configPath.empty());
    // TODO: ML Initialize int the initializer list. Remove unnecessary 0 initialization.
    _traceLevels = 0;
    _logger = 0;
}

IceSSL::ConfigParser::~ConfigParser()
{
    if(_root)
    {
        _root->release();
    }

    XMLPlatformUtils::Terminate();
}

void
IceSSL::ConfigParser::process()
{
    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch(const XMLException& toCatch)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream s;
	// TODO: ML: ": " can just be ":", as nothing follows at the same line.
        s << "while parsing " << _configFile << ": " << endl;
	s << "xerces-c init exception: " << toString(toCatch.getMessage());

        configEx.message = s.str();

        throw configEx;
    }

    int errorCount = 0;

    ConfigParserErrorReporter errReporter(_traceLevels, _logger);

    // Create our parser, then attach an error handler to the parser.
    // The parser will call back to methods of the ConfigParserErrorHandler
    // if it discovers errors during the course of parsing the XML document.
    XercesDOMParser parser;
    parser.setValidationScheme(AbstractDOMParser::Val_Auto);
    parser.setDoNamespaces(false);
    parser.setDoSchema(false);
    parser.setCreateEntityReferenceNodes(false);
    parser.setErrorHandler(&errReporter);

    try
    {
	// TODO: ML: _configPath.front();
	// TODO: ML: What about files that start with c:\, or just \?
        if(*(_configFile.begin()) != '/')
        {
	    // TODO: ML: _configPath.back();
            if(*(_configPath.rbegin()) != '/')
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

        if(errorCount == 0)
        {
            // Get the root of the parse tree.
            _root = parser.adoptDocument();
        }
    }
    catch(const XMLException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream s;
	// TODO: ML: ": " can just be ":", as nothing follows at the same line.
        s << "while parsing " << _configFile << ": " << endl;
	s << "xerces-c parsing error: " << toString(e.getMessage());

        configEx.message = s.str();

        throw configEx;
    }
    catch(const DOMException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

	ostringstream s;
	// TODO: ML: ": " can just be ":", as nothing follows at the same line.
        s << "while parsing " << _configFile << ": " << endl;
	s << "xerces-c DOM parsing error, DOMException code: " << e.code;
        s << ", message: " << e.msg;

        configEx.message = s.str();

        throw configEx;
    }
    catch(...)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

	// TODO: ML: ":\n" (: is missing)
	// TODO: ML: "unknown exception" instead of "unknown error
	// occured during parsing". The latter is redundant, given
	// that it already says "while parsing".
        configEx.message = "while parsing " + _configFile + "\n" + "unknown error occured during parsing";

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
    DOMNode* clientSection = find(clientSectionString);

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
    catch(const DOMException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

	ostringstream s;
	// TODO: ML: ": " can just be ":", as nothing follows at the same line.
        s << "while loading client configuration: " << endl;
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
    DOMNode* serverSection = find(serverSectionString);

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
    catch(const DOMException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

	ostringstream s;
        s << "while loading server configuration " << endl;
	s << "xerces-c DOM parsing error, DOMException code: " << e.code;
        s << ", message: " << e.msg;

        configEx.message = s.str();

        throw configEx;
    }
    
    return false;
}

// TODO: ML: Can you get rid of these functions? The logger and
// trace level should be set in the constructor, either by
// configuration, or by passing arguments.
void
IceSSL::ConfigParser::setTrace(const TraceLevelsPtr& traceLevels)
{
    _traceLevels = traceLevels;
}

bool
IceSSL::ConfigParser::isTraceSet() const 
{
    return _traceLevels;
}

void
IceSSL::ConfigParser::setLogger(const Ice::LoggerPtr& logger)
{
    _logger = logger;
}

bool
IceSSL::ConfigParser::isLoggerSet() const
{
    return _logger;
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

DOMNode*
IceSSL::ConfigParser::find(string& nodePath)
{
    return find(_root, nodePath);
}

DOMNode*
IceSSL::ConfigParser::find(DOMNode* rootNode, string& nodePath)
{
    // The target node that we're looking for.
    DOMNode* tNode = 0;

    if(rootNode == 0)
    {
        return tNode;
    }

    string rootNodeName;
    string tailNodes;

    // Pop the root off the path.
    popRoot(nodePath, rootNodeName, tailNodes);

    DOMNode* child = rootNode->getFirstChild();

    while(child != 0)
    {
        // Ignore any other node types - we're only interested in ELEMENT_NODEs.
        if(child->getNodeType() == DOMNode::ELEMENT_NODE)
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
IceSSL::ConfigParser::getGeneral(DOMNode* rootNode, GeneralConfig& generalConfig)
{
    if(rootNode == 0)
    {
        return;
    }

    string generalString("general");
    DOMNode* general = find(rootNode, generalString);

    DOMNamedNodeMap* attributes = general->getAttributes();

    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        DOMNode* attribute = attributes->item(i);
        string nodeName = toString(attribute->getNodeName());
        string nodeValue = toString(attribute->getNodeValue());

        // Set the property.
        generalConfig.set(nodeName, nodeValue);
    }
}

void
IceSSL::ConfigParser::getCertAuth(DOMNode* rootNode, CertificateAuthority& certAuth)
{
    if(rootNode == 0)
    {
        return;
    }

    string nodeName = "certauthority";
    DOMNode* certAuthNode = find(rootNode, nodeName);

    if(certAuthNode == 0)
    {
        return;
    }

    DOMNamedNodeMap* attributes = certAuthNode->getAttributes();

    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        DOMNode* attribute = attributes->item(i);
        string nodeName = toString(attribute->getNodeName());
        string nodeValue = toString(attribute->getNodeValue());

        if(nodeName.compare("file") == 0)
        {
            string filename = nodeValue;

            // Just a filename, no path component, append path.
	    // TODO: ML: Append path? Prepend path! Also, why this
	    // rule? I can understand this for absolute filenames, but
	    // for something like foo/bar? Shouldn't this be relative
	    // to _configPath?
            if((filename.find("/") == string::npos) && (filename.find("\\") == string::npos))
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
IceSSL::ConfigParser::getBaseCerts(DOMNode* rootNode, BaseCertificates& baseCerts)
{
    if(rootNode == 0)
    {
        return;
    }

    string nodeName = "basecerts";
    DOMNode* baseCertsRoot = find(rootNode, nodeName);

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
IceSSL::ConfigParser::getTempCerts(DOMNode* rootNode, TempCertificates& tempCerts)
{
    if(rootNode == 0)
    {
        return;
    }

    string nodeName = "tempcerts";
    DOMNode* tempCertsRoot = find(rootNode, nodeName);

    if(tempCertsRoot == 0)
    {
        return;
    }

    DOMNode* child = tempCertsRoot->getFirstChild();

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
IceSSL::ConfigParser::loadDHParams(DOMNode* rootNode, TempCertificates& tempCerts)
{
    DiffieHellmanParamsFile dhParams;

    getDHParams(rootNode, dhParams);

    tempCerts.addDHParams(dhParams);
}

void
IceSSL::ConfigParser::loadRSACert(DOMNode* rootNode, TempCertificates& tempCerts)
{
    CertificateDesc rsaCert;

    getCert(rootNode, rsaCert);

    tempCerts.addRSACert(rsaCert);
}

void
IceSSL::ConfigParser::getCert(DOMNode* rootNode, CertificateDesc& certDesc)
{
    if(rootNode == 0)
    {
        return;
    }

    CertificateFile publicFile;
    CertificateFile privateFile;
    int keySize = 0;

    DOMNamedNodeMap* attributes = rootNode->getAttributes();
    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        DOMNode* attribute = attributes->item(i);
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
IceSSL::ConfigParser::getDHParams(DOMNode* rootNode, DiffieHellmanParamsFile& dhParams)
{
    if(rootNode == 0)
    {
        return;
    }

    CertificateFile certFile;
    loadCertificateFile(rootNode, certFile);

    DOMNamedNodeMap* attributes = rootNode->getAttributes();
    int keySize = 0;
    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        DOMNode* attribute = attributes->item(i);
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
IceSSL::ConfigParser::loadCertificateFile(DOMNode* rootNode, CertificateFile& certFile)
{
    if(rootNode == 0)
    {
        return;
    }

    string filename;
    int encoding = 0; // Initialize, to keep the compiler from complaining.

    DOMNamedNodeMap* attributes = rootNode->getAttributes();
    int attrCount = attributes->getLength();

    for(int i = 0; i < attrCount; i++)
    {
        DOMNode* attribute = attributes->item(i);
        string nodeName = toString(attribute->getNodeName());
        string nodeValue = toString(attribute->getNodeValue());

        if(nodeName.compare("encoding") == 0)
        {
            encoding = parseEncoding(nodeValue);
        }
        else if(nodeName.compare("filename") == 0)
        {
            filename = nodeValue;

            // Just a filename, no path component, append path.
	    // TODO: ML: See comments above.
            if((filename.find("/") == string::npos) && (filename.find("\\") == string::npos))
            {
                filename = _configPath + filename;
            }
        }
    }

    certFile = CertificateFile(filename, encoding);
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
    char* t = XMLString::transcode(s);
    string r(t);
    delete[] t;
    return r;
}

