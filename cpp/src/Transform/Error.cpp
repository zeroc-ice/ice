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

#include <Transform/Error.h>
#include <Transform/Exception.h>
#include <Transform/TransformUtil.h>

using namespace std;

//
// ErrorReporter
//
Transform::ErrorReporter::ErrorReporter(ostream& out) :
    _out(out), _raise(true)
{
}

ostream&
Transform::ErrorReporter::stream() const
{
    return _out;
}

void
Transform::ErrorReporter::warning(const string& msg)
{
    if(!_descName.empty())
    {
        _out << "warning in <" << _descName << "> descriptor, line " << _descLine << ": " << msg << endl;
    }
    else
    {
        _out << "warning: " << msg << endl;
    }
}

void
Transform::ErrorReporter::error(const string& msg)
{
    ostringstream ostr;
    if(!_descName.empty())
    {
        ostr << "error in <" << _descName << "> descriptor, line " << _descLine << ": " << msg << endl;
    }
    else
    {
        ostr << "error: " << msg << endl;
    }
    throw TransformException(__FILE__, __LINE__, ostr.str());
}

void
Transform::ErrorReporter::typeMismatchError(const Slice::TypePtr& expected, const Slice::TypePtr& received, bool fatal)
{
    ostringstream ostr;
    ostr << "type mismatch: expected " << typeName(expected) << " but received " << typeName(received);

    if(_raise || fatal)
    {
        error(ostr.str());
    }
    else
    {
        warning(ostr.str());
    }
}

void
Transform::ErrorReporter::conversionError(const string& value, const Slice::TypePtr& type)
{
    ostringstream ostr;
    ostr << "unable to convert `" << value << "' to " << typeName(type);

    if(_raise)
    {
        error(ostr.str());
    }
    else
    {
        warning(ostr.str());
    }
}

void
Transform::ErrorReporter::rangeError(const string& value, const Slice::TypePtr& type)
{
    ostringstream ostr;
    ostr << "value `" << value << "' is out of range for type " << typeName(type);

    if(_raise)
    {
        error(ostr.str());
    }
    else
    {
        warning(ostr.str());
    }
}

void
Transform::ErrorReporter::expressionSyntaxError(const string& msg)
{
    assert(!_expression.empty());
    ostringstream ostr;
    ostr << "syntax error in expression `" << _expression << "': " << msg;
    error(ostr.str());
}

void
Transform::ErrorReporter::descriptorError(const string& msg, int line)
{
    ostringstream ostr;
    ostr << "XML error on line " << line << ":" << endl << msg;
    error(ostr.str());
}

void
Transform::ErrorReporter::setDescriptor(const string& name, int line)
{
    _descName = name;
    _descLine = line;
}

void
Transform::ErrorReporter::getDescriptor(string& name, int& line)
{
    name = _descName;
    line = _descLine;
}

void
Transform::ErrorReporter::clearDescriptor()
{
    _descName.clear();
}

void
Transform::ErrorReporter::setExpression(const string& expr)
{
    _expression = expr;
}

void
Transform::ErrorReporter::clearExpression()
{
    _expression.clear();
}

bool
Transform::ErrorReporter::raise() const
{
    return _raise;
}

void
Transform::ErrorReporter::raise(bool b)
{
    _raise = b;
}

//
// DescriptorErrorContext
//
Transform::DescriptorErrorContext::DescriptorErrorContext(const ErrorReporterPtr& errorReporter, const string& name,
                                                          int line) :
    _errorReporter(errorReporter)
{
    //
    // Save the existing descriptor information before changing it.
    //
    _errorReporter->getDescriptor(_name, _line);
    _errorReporter->setDescriptor(name, line);
}

Transform::DescriptorErrorContext::~DescriptorErrorContext()
{
    //
    // Restore the original descriptor information.
    //
    _errorReporter->setDescriptor(_name, _line);
}
