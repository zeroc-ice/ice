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

#include <sax/SAXParseException.hpp>

#include <sstream>

using namespace std;

void ::IceInternal::incRef(::IceSSL::ConfigParserErrorReporter* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::ConfigParserErrorReporter* p) { p->__decRef(); }

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

        s << "ssl configuration file parse error" << endl;
        s << DOMString(toCatch.getSystemId());
        s << ", line " << toCatch.getLineNumber();
        s << ", column " << toCatch.getColumnNumber() << endl;
        s << "Message " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, "PWN " + s.str());
    }
}

void
IceSSL::ConfigParserErrorReporter::error(const SAXParseException& toCatch)
{
    _errorCount++;

    _errors << "ssl configuration file parse error" << endl;
    _errors << "  " << DOMString(toCatch.getSystemId());
    _errors << ", line " << toCatch.getLineNumber();
    _errors << ", column " << toCatch.getColumnNumber() << endl;
    _errors << "  " << "Message " << DOMString(toCatch.getMessage()) << endl;
}

void
IceSSL::ConfigParserErrorReporter::fatalError(const SAXParseException& toCatch)
{
    _errorCount++;

    _errors << "ssl configuration file parse error" << endl;
    _errors << "  " << DOMString(toCatch.getSystemId());
    _errors << ", line " << toCatch.getLineNumber();
    _errors << ", column " << toCatch.getColumnNumber() << endl;
    _errors << "  " << "Message " << DOMString(toCatch.getMessage()) << endl;
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

std::ostream&
IceSSL::operator << (std::ostream& target, const DOMString& s)
{
    char *p = s.transcode();
    target << p;
    delete [] p;
    return target;
}
