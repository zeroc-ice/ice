// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Logger.h>
#include <IceSSL/ConfigParserErrorReporter.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/TraceLevels.h>

#include <xercesc/sax/SAXParseException.hpp>

#include <sstream>

using namespace std;

//
// Utility to make the usage of xerces easier.
//
static string
toString(const XMLCh* s)
{
    char* t = XMLString::transcode(s);
    string r(t);
    delete[] t;
    return r;
}

IceSSL::ConfigParserErrorReporter::ConfigParserErrorReporter(const IceSSL::TraceLevelsPtr& traceLevels,
                                                             const Ice::LoggerPtr& logger) :
                                  _traceLevels(traceLevels),
                                  _logger(logger)
{
    _errorCount = 0;
}

IceSSL::ConfigParserErrorReporter::~ConfigParserErrorReporter()
{
}

void
IceSSL::ConfigParserErrorReporter::warning(const SAXParseException& toCatch)
{
    if(_traceLevels->security >= IceSSL::SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "ssl configuration file parse error" << endl
          << toString(toCatch.getSystemId())
          << ", line " << toCatch.getLineNumber()
          << ", column " << toCatch.getColumnNumber() << endl
          << "Message " << toString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, "PWN " + s.str());
    }
}

void
IceSSL::ConfigParserErrorReporter::error(const SAXParseException& toCatch)
{
    _errorCount++;

    _errors << "ssl configuration file parse error" << endl
            << "  " << toString(toCatch.getSystemId())
            << ", line " << toCatch.getLineNumber()
            << ", column " << toCatch.getColumnNumber() << endl
            << "  " << "Message " << toString(toCatch.getMessage()) << endl;
}

void
IceSSL::ConfigParserErrorReporter::fatalError(const SAXParseException& toCatch)
{
    _errorCount++;

    _errors << "ssl configuration file parse error" << endl
            << "  " << toString(toCatch.getSystemId())
            << ", line " << toCatch.getLineNumber()
            << ", column " << toCatch.getColumnNumber() << endl
            << "  " << "Message " << toString(toCatch.getMessage()) << endl;
}

void
IceSSL::ConfigParserErrorReporter::resetErrors()
{
    // No-op in this case
}

bool
IceSSL::ConfigParserErrorReporter::getSawErrors() const
{
    return (_errorCount == 0 ? false : true);
}

string
IceSSL::ConfigParserErrorReporter::getErrors() const
{
    return _errors.str();
}
