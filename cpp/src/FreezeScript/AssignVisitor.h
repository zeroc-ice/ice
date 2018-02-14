// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_ASSIGN_VISITOR_H
#define FREEZE_SCRIPT_ASSIGN_VISITOR_H

#include <FreezeScript/Data.h>

namespace FreezeScript
{

class AssignVisitor : public DataVisitor
{
public:

    AssignVisitor(const DataPtr&, const DataFactoryPtr&, const ErrorReporterPtr&, bool,
                  const std::string& = std::string());

    virtual void visitBoolean(const BooleanDataPtr&);
    virtual void visitInteger(const IntegerDataPtr&);
    virtual void visitDouble(const DoubleDataPtr&);
    virtual void visitString(const StringDataPtr&);
    virtual void visitProxy(const ProxyDataPtr&);
    virtual void visitStruct(const StructDataPtr&);
    virtual void visitSequence(const SequenceDataPtr&);
    virtual void visitEnum(const EnumDataPtr&);
    virtual void visitDictionary(const DictionaryDataPtr&);
    virtual void visitObject(const ObjectRefPtr&);

private:

    bool isCompatible(const Slice::TypePtr&, const Slice::TypePtr&);
    bool checkClasses(const Slice::ClassDeclPtr&, const Slice::ClassDeclPtr&);
    void typeMismatchError(const Slice::TypePtr&, const Slice::TypePtr&);
    void conversionError(const Slice::TypePtr&, const Slice::TypePtr&, const std::string&);
    void rangeError(const std::string&, const Slice::TypePtr&);
    void error(const std::string&);

    DataPtr _src;
    DataFactoryPtr _factory;
    ErrorReporterPtr _errorReporter;
    bool _convert;
    std::string _context; // Provides additional detail for use in warning messages.
};

} // End of namespace FreezeScript

#endif
