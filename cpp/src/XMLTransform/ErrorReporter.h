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
