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

#ifndef TRANSFORM_ERROR_H
#define TRANSFORM_ERROR_H

#include <Slice/Parser.h>

namespace Transform
{

class ErrorReporter : public IceUtil::SimpleShared
{
public:

    ErrorReporter(std::ostream&);

    std::ostream& stream() const;

    void warning(const std::string&);
    void error(const std::string&);
    void typeMismatchError(const Slice::TypePtr&, const Slice::TypePtr&, bool);
    void conversionError(const std::string&, const Slice::TypePtr&);
    void rangeError(const std::string&, const Slice::TypePtr&);
    void expressionSyntaxError(const std::string&);
    void descriptorError(const std::string&, int);

    void setDescriptor(const std::string&, int);
    void getDescriptor(std::string&, int&);
    void clearDescriptor();

    void setExpression(const std::string&);
    void clearExpression();

    bool raise() const;
    void raise(bool);

private:

    std::ostream& _out;
    std::string _descName;
    int _descLine;
    std::string _expression;
    bool _raise;
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

} // End of namespace Transform

#endif
