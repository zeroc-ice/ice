// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ERROR_REPORTER_H
#define ERROR_REPORTER_H

#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include <string>

namespace XMLTransform
{

class DOMTreeErrorReporter : public ErrorHandler
{
public:

    DOMTreeErrorReporter();

    void warning(const SAXParseException&);

    void error(const SAXParseException&);

    void fatalError(const SAXParseException&);

    void resetErrors();

    bool getSawErrors() const;

    ::std::string getErrors() const;

private:

    ::std::string _errors;

    ::std::string toString(const XMLCh*);
};

}

#endif
