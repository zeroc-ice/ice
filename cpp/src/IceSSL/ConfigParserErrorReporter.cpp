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

#include <Ice/LoggerUtil.h>
#include <IceSSL/ConfigParserErrorReporter.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/TraceLevels.h>

#include <Ice/Xerces.h>
#include <xercesc/sax/SAXParseException.hpp>

#include <sstream>

using namespace std;
ICE_XERCES_NS_USE

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
    _logger(logger),
    _errorCount(0)
{
}

IceSSL::ConfigParserErrorReporter::~ConfigParserErrorReporter()
{
}

void
IceSSL::ConfigParserErrorReporter::warning(const SAXParseException& toCatch)
{
    if(_traceLevels->security >= IceSSL::SECURITY_PARSE_WARNINGS)
    {
        Ice::Trace out(_logger, _traceLevels->securityCat);

        out << "ssl configuration file parse error" << "\n"
            << toString(toCatch.getSystemId())
            << ", line " << toCatch.getLineNumber()
            << ", column " << toCatch.getColumnNumber() << "\n"
            << "Message " << toString(toCatch.getMessage()) << "\n";
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
