// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONFIG_ERROR_REPORTER_H
#define ICE_SSL_CONFIG_ERROR_REPORTER_H

#include <iostream>
#include <IceUtil/Shared.h>
#include <util/XercesDefs.hpp>
#include <dom/DOMString.hpp>
#include <sax/ErrorHandler.hpp>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>

namespace IceSSL
{

class ConfigParserErrorReporter : public ErrorHandler, public IceUtil::Shared
{

public:

     ConfigParserErrorReporter(const IceInternal::TraceLevelsPtr&, const Ice::LoggerPtr&);

    ~ConfigParserErrorReporter();

    //  Implementation of the error handler interface.
    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

    bool getSawErrors() const;

private:
    // This is set if we get any errors, and is queryable via a getter method.
    // It's used by the main code to suppress output if there are errors.
    bool _sawErrors;

    IceInternal::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
};

typedef IceInternal::Handle<ConfigParserErrorReporter> ConfigParserErrorReporterPtr;

std::ostream& operator << (std::ostream& target, const DOMString& s);

}

namespace IceInternal
{

void incRef(::IceSSL::ConfigParserErrorReporter*);
void decRef(::IceSSL::ConfigParserErrorReporter*);

}

#endif
