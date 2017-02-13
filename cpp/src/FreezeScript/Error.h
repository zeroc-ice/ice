// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_ERROR_H
#define FREEZE_SCRIPT_ERROR_H

#include <Slice/Parser.h>

namespace FreezeScript
{

class ErrorReporter : public IceUtil::SimpleShared
{
public:

    ErrorReporter(std::ostream&, bool);

    std::ostream& stream() const;

    void warning(const std::string&);
    void error(const std::string&);
    void typeMismatchError(const Slice::TypePtr&, const Slice::TypePtr&, bool);
    void conversionError(const std::string&, const Slice::TypePtr&, bool);
    void rangeError(const std::string&, const Slice::TypePtr&, bool);
    void expressionSyntaxError(const std::string&);
    void descriptorError(const std::string&, int);

    void setDescriptor(const std::string&, int);
    void getDescriptor(std::string&, int&);
    void clearDescriptor();

    void setExpression(const std::string&);
    void clearExpression();

private:

    std::ostream& _out;
    bool _suppress;
    std::map<std::string, bool> _warningHistory;
    std::string _descName;
    int _descLine;
    std::string _expression;
};
typedef IceUtil::Handle<ErrorReporter> ErrorReporterPtr;

class DescriptorErrorContext
{
public:

    DescriptorErrorContext(const ErrorReporterPtr&, const std::string&, int);
    ~DescriptorErrorContext();

private:

    ErrorReporterPtr _errorReporter;
    std::string _name;
    int _line;
};

} // End of namespace FreezeScript

#endif
