// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_SSL_CONFIG_ERROR_REPORTER_H
#define ICE_SSL_CONFIG_ERROR_REPORTER_H

#include <Ice/LoggerF.h>
#include <IceSSL/TraceLevelsF.h>

//#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

namespace IceSSL
{

class ConfigParserErrorReporter : public ErrorHandler
{
public:

     ConfigParserErrorReporter(const IceSSL::TraceLevelsPtr&, const Ice::LoggerPtr&);
    ~ConfigParserErrorReporter();

    //  Implementation of the error handler interface.
    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

    bool getSawErrors() const;
    std::string getErrors() const;

private:

    IceSSL::TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;

    // Any errors that are encountered will be output to this stream.
    std::ostringstream _errors;
    int _errorCount;
};

}

#endif
