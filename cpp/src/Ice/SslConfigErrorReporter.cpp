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
#include <Ice/SslConfigErrorReporter.h>
#include <stdlib.h>
#include <string.h>
#include <Ice/Security.h>

using namespace std;

void
IceSecurity::Ssl::ErrorReporter::warning(const SAXParseException& toCatch)
{
    if (_traceLevels->security >= SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "Xerces-c Init Exception: "<< "Warning at file \""
          << DOMString(toCatch.getSystemId())
          << "\", line " << toCatch.getLineNumber()
          << ", column " << toCatch.getColumnNumber()
          << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, s.str());
    }
}

void
IceSecurity::Ssl::ErrorReporter::error(const SAXParseException& toCatch)
{
    _sawErrors = true;

    if (_traceLevels->security >= SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "Xerces-c Init Exception: "<< "Error at file \""
          << DOMString(toCatch.getSystemId())
          << "\", line " << toCatch.getLineNumber()
          << ", column " << toCatch.getColumnNumber()
          << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, s.str());
    }
}

void
IceSecurity::Ssl::ErrorReporter::fatalError(const SAXParseException& toCatch)
{
    _sawErrors = true;

    if (_traceLevels->security >= SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "Xerces-c Init Exception: "<< "Fatal error at file \""
          << DOMString(toCatch.getSystemId())
          << "\", line " << toCatch.getLineNumber()
          << ", column " << toCatch.getColumnNumber()
          << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, s.str());
    }
}

void
IceSecurity::Ssl::ErrorReporter::resetErrors()
{
    // No-op in this case
}

