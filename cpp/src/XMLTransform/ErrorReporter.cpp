// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <XMLTransform/ErrorReporter.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace XMLTransform;

DOMTreeErrorReporter::DOMTreeErrorReporter()
{
}

void
DOMTreeErrorReporter::warning(const SAXParseException& ex)
{
    ostringstream out;
    out << "Warning at file \"" << toString(ex.getSystemId())
        << "\", line " << ex.getLineNumber()
        << ", column " << ex.getColumnNumber()
        << "\n   Message: " << toString(ex.getMessage()) << "\n";
    _errors.append(out.str());
}

void
DOMTreeErrorReporter::error(const SAXParseException& ex)
{
    ostringstream out;
    out << "Error at file \"" << toString(ex.getSystemId())
        << "\", line " << ex.getLineNumber()
        << ", column " << ex.getColumnNumber()
        << "\n   Message: " << toString(ex.getMessage()) << "\n";
    _errors.append(out.str());
}

void
DOMTreeErrorReporter::fatalError(const SAXParseException& ex)
{
    ostringstream out;
    out << "Fatal at file \"" << toString(ex.getSystemId())
        << "\", line " << ex.getLineNumber()
        << ", column " << ex.getColumnNumber()
        << "\n   Message: " << toString(ex.getMessage()) << "\n";
    _errors.append(out.str());
}

void
DOMTreeErrorReporter::resetErrors()
{
    _errors.clear();
}

bool
DOMTreeErrorReporter::getSawErrors() const
{
    return !_errors.empty();
}

string
DOMTreeErrorReporter::getErrors() const
{
    return _errors;
}

string
DOMTreeErrorReporter::toString(const XMLCh* s)
{
    char* t = XMLString::transcode(s);
    string r(t);
    delete[] t;
    return r;
}
