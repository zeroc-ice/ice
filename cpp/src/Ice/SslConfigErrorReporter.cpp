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
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/OpenSSL.h>

using namespace std;

void
IceSSL::ErrorReporter::warning(const SAXParseException& toCatch)
{
    if (_traceLevels->security >= IceSSL::SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "SSL configuration file parse warning.\n" << flush;
        s << "Xerces-c Init Exception: Warning at file \"" << flush;
        s << DOMString(toCatch.getSystemId()) << flush;
        s << "\", line " << toCatch.getLineNumber() << flush;
        s << ", column " << toCatch.getColumnNumber() << flush;
        s << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, "PWN " + s.str());
    }
}

void
IceSSL::ErrorReporter::error(const SAXParseException& toCatch)
{
    _sawErrors = true;

    if (_traceLevels->security >= IceSSL::SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "SSL configuration file parse error.\n" << flush;
        s << "Xerces-c Init Exception: Error at file \"" << flush;
        s << DOMString(toCatch.getSystemId()) << flush;
        s << "\", line " << toCatch.getLineNumber() << flush;
        s << ", column " << toCatch.getColumnNumber() << flush;
        s << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, "PWN " + s.str());
    }
}

void
IceSSL::ErrorReporter::fatalError(const SAXParseException& toCatch)
{
    _sawErrors = true;

    if (_traceLevels->security >= IceSSL::SECURITY_PARSE_WARNINGS)
    {
	ostringstream s;

        s << "SSL configuration file parse error.\n" << flush;
        s << "Xerces-c Init Exception: Fatal error at file \"" << flush;
        s << DOMString(toCatch.getSystemId()) << flush;
        s << "\", line " << toCatch.getLineNumber() << flush;
        s << ", column " << toCatch.getColumnNumber() << flush;
        s << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

        _logger->trace(_traceLevels->securityCat, "PWN " + s.str());
    }
}

void
IceSSL::ErrorReporter::resetErrors()
{
    // No-op in this case
}

bool
IceSSL::ErrorReporter::getSawErrors() const
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
