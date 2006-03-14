// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/ConfigParser.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/Exception.h>

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
}

void
IceSSL::ConfigParser::process()
{
    try
    {
        string configFile;
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

            configFile = _configPath + _configFile;
        }
        else
        {
            configFile = _configFile;
        }

        _root = IceXML::Parser::parse(configFile);
    }
    catch(const IceXML::ParserException& e)
    {
        ConfigParseException configEx(__FILE__, __LINE__);

        ostringstream s;
        s << "error while parsing " << _configFile << ":\n";
        e.ice_print(s);

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
}

bool
IceSSL::ConfigParser::loadClientConfig(GeneralConfig& general,
                                       CertificateAuthority& certAuth,
                                       BaseCertificates& baseCerts)
{
    string clientSectionString("SSLConfig:client");
    IceXML::NodePtr clientSection = find(clientSectionString);

    // If we actually have a client section.
    if(clientSection)
    {
        getGeneral(clientSection, general);
        getCertAuth(clientSection, certAuth);
        getBaseCerts(clientSection, baseCerts);
        return true;
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
    IceXML::NodePtr serverSection = find(serverSectionString);

    // If we actually have a client section.
    if(serverSection)
    {
        getGeneral(serverSection, general);
        getCertAuth(serverSection, certAuth);
        getBaseCerts(serverSection, baseCerts);
        getTempCerts(serverSection, tempCerts);
        return true;
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

IceXML::NodePtr
IceSSL::ConfigParser::find(string& nodePath)
{
    return find(_root, nodePath);
}

IceXML::NodePtr
IceSSL::ConfigParser::find(const IceXML::NodePtr& rootNode, string& nodePath)
{
    // The target node that we're looking for.
    IceXML::NodePtr tNode;

    if(!rootNode)
    {
        return tNode;
    }

    string rootNodeName;
    string tailNodes;

    // Pop the root off the path.
    popRoot(nodePath, rootNodeName, tailNodes);

    IceXML::NodeList children = rootNode->getChildren();
    for(IceXML::NodeList::iterator p = children.begin(); p != children.end(); ++p)
    {
        // Ignore any other node types - we're only interested in elements.
        IceXML::ElementPtr elem = IceXML::ElementPtr::dynamicCast(*p);
        if(elem)
        {
            string nodeName = elem->getName();

            if(nodeName.compare(rootNodeName) == 0)
            {
                // No further to recurse, this must be it.
                if(tailNodes.empty())
                {
                    tNode = elem;
                }
                else
                {
                    // Recursive call.
                    tNode = find(elem, tailNodes);
                }
            }
        }
    }

    return tNode;
}

void
IceSSL::ConfigParser::getGeneral(const IceXML::NodePtr& rootNode, GeneralConfig& generalConfig)
{
    if(!rootNode)
    {
        return;
    }

    string generalString("general");
    IceXML::NodePtr general = find(rootNode, generalString);

    IceXML::Attributes attributes = general->getAttributes();
    for(IceXML::Attributes::iterator p = attributes.begin(); p != attributes.end(); ++p)
    {
        // Set the property.
        generalConfig.set(p->first, p->second);
    }
}

void
IceSSL::ConfigParser::getCertAuth(const IceXML::NodePtr& rootNode, CertificateAuthority& certAuth)
{
    if(!rootNode)
    {
        return;
    }

    string nodeName = "certauthority";
    IceXML::NodePtr certAuthNode = find(rootNode, nodeName);

    if(!certAuthNode)
    {
        return;
    }

    string file = certAuthNode->getAttribute("file");
    if(!file.empty())
    {
        // Just a filename, no path component, prepend path.
        if(!isAbsolutePath(file))
        {
            file = _configPath + file;
        }

        certAuth.setCAFileName(file);
    }

    string path = certAuthNode->getAttribute("path");
    if(!path.empty())
    {
        certAuth.setCAPath(path);
    }
}

void
IceSSL::ConfigParser::getBaseCerts(const IceXML::NodePtr& rootNode, BaseCertificates& baseCerts)
{
    if(!rootNode)
    {
        return;
    }

    string nodeName = "basecerts";
    IceXML::NodePtr baseCertsRoot = find(rootNode, nodeName);

    if(!baseCertsRoot)
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

    const BaseCertificates result(rsaCert, dsaCert, dhParams);
    baseCerts = result;
}

void
IceSSL::ConfigParser::getTempCerts(const IceXML::NodePtr& rootNode, TempCertificates& tempCerts)
{
    if(!rootNode)
    {
        return;
    }

    string nodeName = "tempcerts";
    IceXML::NodePtr tempCertsRoot = find(rootNode, nodeName);

    if(!tempCertsRoot)
    {
        return;
    }

    IceXML::NodeList children = tempCertsRoot->getChildren();
    for(IceXML::NodeList::iterator p = children.begin(); p != children.end(); ++p)
    {
        string name = (*p)->getName();

        if(name.compare("dhparams") == 0)
        {
            loadDHParams(*p, tempCerts);
        }
        else if(name.compare("rsacert") == 0)
        {
            loadRSACert(*p, tempCerts);
        }
    }
}

void
IceSSL::ConfigParser::loadDHParams(const IceXML::NodePtr& rootNode, TempCertificates& tempCerts)
{
    DiffieHellmanParamsFile dhParams;

    getDHParams(rootNode, dhParams);

    tempCerts.addDHParams(dhParams);
}

void
IceSSL::ConfigParser::loadRSACert(const IceXML::NodePtr& rootNode, TempCertificates& tempCerts)
{
    CertificateDesc rsaCert;

    getCert(rootNode, rsaCert);

    tempCerts.addRSACert(rsaCert);
}

void
IceSSL::ConfigParser::getCert(const IceXML::NodePtr& rootNode, CertificateDesc& certDesc)
{
    if(!rootNode)
    {
        return;
    }

    CertificateFile publicFile;
    CertificateFile privateFile;
    int keySize = 0;

    string keySizeValue = rootNode->getAttribute("keysize");
    if(!keySizeValue.empty())
    {
        keySize = atoi(keySizeValue.c_str());
    }

    string publicString("public");
    string privateString("private");

    loadCertificateFile(find(rootNode, publicString),  publicFile);
    loadCertificateFile(find(rootNode, privateString), privateFile);

    // Initialize the certificate description.
    certDesc = CertificateDesc(keySize, publicFile, privateFile);
}

void
IceSSL::ConfigParser::getDHParams(const IceXML::NodePtr& rootNode, DiffieHellmanParamsFile& dhParams)
{
    if(!rootNode)
    {
        return;
    }

    CertificateFile certFile;
    loadCertificateFile(rootNode, certFile);

    int keySize = 0;

    string keySizeValue = rootNode->getAttribute("keysize");
    if(!keySizeValue.empty())
    {
        keySize = atoi(keySizeValue.c_str());
    }

    dhParams = DiffieHellmanParamsFile(keySize, certFile.getFileName(), certFile.getEncoding());
}

void
IceSSL::ConfigParser::loadCertificateFile(const IceXML::NodePtr& rootNode, CertificateFile& certFile)
{
    if(!rootNode)
    {
        return;
    }

    string filename;
    int encoding = SSL_FILETYPE_PEM; // PEM is the default type.

    filename = rootNode->getAttribute("filename");
    if(!filename.empty())
    {
        // Just a filename, no path component, prepend path.
        if(!isAbsolutePath(filename))
        {
            filename = _configPath + filename;
        }
    }

    string encodingValue = rootNode->getAttribute("encoding");
    if(!encodingValue.empty())
    {
        encoding = parseEncoding(encodingValue);
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
    int encoding = SSL_FILETYPE_PEM;

    if(encodingString == "PEM")
    {
        encoding = SSL_FILETYPE_PEM;
    }
    else if(encodingString == "ASN1")
    {
        encoding = SSL_FILETYPE_ASN1;
    }

    return encoding;
}
