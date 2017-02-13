// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/Error.h>
#include <FreezeScript/Exception.h>
#include <FreezeScript/Util.h>

using namespace std;

//
// ErrorReporter
//
FreezeScript::ErrorReporter::ErrorReporter(ostream& out, bool suppress) :
    _out(out), _suppress(suppress)
{
}

ostream&
FreezeScript::ErrorReporter::stream() const
{
    return _out;
}

void
FreezeScript::ErrorReporter::warning(const string& msg)
{
    ostringstream ostr;
    if(!_descName.empty())
    {
        ostr << "warning in <" << _descName << "> descriptor, line " << _descLine << ": " << msg << endl;
    }
    else
    {
        ostr << "warning: " << msg << endl;
    }
    string warn = ostr.str();
    if(_suppress)
    {
        map<string, bool>::iterator p = _warningHistory.find(warn);
        if(p != _warningHistory.end())
        {
            return;
        }
        _warningHistory.insert(map<string, bool>::value_type(warn, true));
    }
    _out << warn;
}

void
FreezeScript::ErrorReporter::error(const string& msg)
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
    throw FailureException(__FILE__, __LINE__, ostr.str());
}

void
FreezeScript::ErrorReporter::typeMismatchError(const Slice::TypePtr& expected, const Slice::TypePtr& received,
                                               bool fatal)
{
    ostringstream ostr;
    ostr << "type mismatch: expected " << typeToString(expected) << " but received " << typeToString(received);

    if(fatal)
    {
        error(ostr.str());
    }
    else
    {
        warning(ostr.str());
    }
}

void
FreezeScript::ErrorReporter::conversionError(const string& value, const Slice::TypePtr& type, bool fatal)
{
    ostringstream ostr;
    ostr << "unable to convert `" << value << "' to " << typeToString(type);

    if(fatal)
    {
        error(ostr.str());
    }
    else
    {
        warning(ostr.str());
    }
}

void
FreezeScript::ErrorReporter::rangeError(const string& value, const Slice::TypePtr& type, bool fatal)
{
    ostringstream ostr;
    ostr << "value `" << value << "' is out of range for type " << typeToString(type);

    if(fatal)
    {
        error(ostr.str());
    }
    else
    {
        warning(ostr.str());
    }
}

void
FreezeScript::ErrorReporter::expressionSyntaxError(const string& msg)
{
    assert(!_expression.empty());
    ostringstream ostr;
    ostr << "syntax error in expression `" << _expression << "': " << msg;
    error(ostr.str());
}

void
FreezeScript::ErrorReporter::descriptorError(const string& msg, int line)
{
    ostringstream ostr;
    ostr << "XML error on line " << line << ":" << endl << msg;
    error(ostr.str());
}

void
FreezeScript::ErrorReporter::setDescriptor(const string& name, int line)
{
    _descName = name;
    _descLine = line;
}

void
FreezeScript::ErrorReporter::getDescriptor(string& name, int& line)
{
    name = _descName;
    line = _descLine;
}

void
FreezeScript::ErrorReporter::clearDescriptor()
{
    _descName.clear();
}

void
FreezeScript::ErrorReporter::setExpression(const string& expr)
{
    _expression = expr;
}

void
FreezeScript::ErrorReporter::clearExpression()
{
    _expression.clear();
}

//
// DescriptorErrorContext
//
FreezeScript::DescriptorErrorContext::DescriptorErrorContext(const ErrorReporterPtr& errorReporter, const string& name,
                                                             int line) :
    _errorReporter(errorReporter)
{
    //
    // Save the existing descriptor information before changing it.
    //
    _errorReporter->getDescriptor(_name, _line);
    _errorReporter->setDescriptor(name, line);
}

FreezeScript::DescriptorErrorContext::~DescriptorErrorContext()
{
    //
    // Restore the original descriptor information.
    //
    _errorReporter->setDescriptor(_name, _line);
}
