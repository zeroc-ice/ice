// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <iostream>
#include <sstream>

#include <sax/SAXParseException.hpp>
#include <Ice/ConfigParserErrorReporter.h>
#include <stdlib.h>
#include <string.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/OpenSSL.h>

using namespace std;

void ::IceInternal::incRef(::IceSSL::ConfigParserErrorReporter* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::ConfigParserErrorReporter* p) { p->__decRef(); }

IceSSL::ConfigParserErrorReporter::ConfigParserErrorReporter(const IceInternal::TraceLevelsPtr& traceLevels,
                                                             const Ice::LoggerPtr& logger) :
                                  _sawErrors(false),
                                  _traceLevels(traceLevels),
                                  _logger(logger)
{
}

IceSSL::ConfigParserErrorReporter::~ConfigParserErrorReporter()
{
}

void
IceSSL::ConfigParserErrorReporter::warning(const SAXParseException& toCatch)
{
    if (_traceLevels->security >= IceSSL::SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "SSL configuration file parse warning." << endl;
        s << "Xerces-c Init Exception: Warning at file \"" << flush;
        s << DOMString(toCatch.getSystemId()) << flush;
        s << "\", line " << toCatch.getLineNumber() << flush;
        s << ", column " << toCatch.getColumnNumber() << flush;
        s << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, "PWN " + s.str());
    }
}

void
IceSSL::ConfigParserErrorReporter::error(const SAXParseException& toCatch)
{
    _sawErrors = true;

    if (_traceLevels->security >= IceSSL::SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "SSL configuration file parse error." << endl;
        s << "Xerces-c Init Exception: Error at file \"" << flush;
        s << DOMString(toCatch.getSystemId()) << flush;
        s << "\", line " << toCatch.getLineNumber() << flush;
        s << ", column " << toCatch.getColumnNumber() << flush;
        s << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, "PWN " + s.str());
    }
}

void
IceSSL::ConfigParserErrorReporter::fatalError(const SAXParseException& toCatch)
{
    _sawErrors = true;

    if (_traceLevels->security >= IceSSL::SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "SSL configuration file parse error." << endl;
        s << "Xerces-c Init Exception: Fatal error at file \"" << flush;
        s << DOMString(toCatch.getSystemId()) << flush;
        s << "\", line " << toCatch.getLineNumber() << flush;
        s << ", column " << toCatch.getColumnNumber() << flush;
        s << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, "PWN " + s.str());
    }
}

void
IceSSL::ConfigParserErrorReporter::resetErrors()
{
    // No-op in this case
}

bool
IceSSL::ConfigParserErrorReporter::getSawErrors() const
{
    return _sawErrors;
}

std::ostream&
IceSSL::operator << (std::ostream& target, const DOMString& s)
{
    char *p = s.transcode();
    target << p;
    delete [] p;
    return target;
}
