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
#include <util/XercesDefs.hpp>
#include <dom/DOMString.hpp>
#include <sax/ErrorHandler.hpp>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

namespace IceSecurity
{

namespace Ssl
{

using IceInternal::TraceLevelsPtr;
using Ice::LoggerPtr;

class ErrorReporter : public ErrorHandler
{

public:

     ErrorReporter(TraceLevelsPtr traceLevels, LoggerPtr logger) :
            _sawErrors(false),
            _traceLevels(traceLevels),
            _logger(logger)
    {
    }

    ~ErrorReporter() { }

    //  Implementation of the error handler interface.
    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

    inline bool getSawErrors() const { return _sawErrors; };

private:
    // This is set if we get any errors, and is queryable via a getter method.
    // It's used by the main code to suppress output if there are errors.
    bool _sawErrors;

    TraceLevelsPtr _traceLevels;
    LoggerPtr _logger;
};

using std::ostream;

inline ostream&
operator << (ostream& target, const DOMString& s)
{
    char *p = s.transcode();
    target << p;
    delete [] p;
    return target;
}

}

}

#endif
