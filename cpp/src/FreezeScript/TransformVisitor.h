// **********************************************************************
//
// Copyright (c) 2004
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

#ifndef FREEZE_SCRIPT_TRANSFORM_VISITOR_H
#define FREEZE_SCRIPT_TRANSFORM_VISITOR_H

#include <FreezeScript/Data.h>

namespace FreezeScript
{

//
// ObjectDataMap associates old instances of ObjectData to their
// transformed equivalents.
//
typedef std::map<const ObjectData*, ObjectDataPtr> ObjectDataMap;

//
// TransformInfo supplies information required by TransformVisitor.
//
class TransformInfo
{
public:

    virtual ~TransformInfo() {}

    //
    // Indicates whether the default transformation should be performed.
    //
    virtual bool doDefaultTransform(const Slice::TypePtr&) = 0;

    //
    // Indicates whether a base class transformation should be performed.
    //
    virtual bool doBaseTransform(const Slice::ClassDefPtr&) = 0;

    //
    // Given an old type, return the equivalent new type if the type
    // has been renamed.
    //
    virtual Slice::TypePtr getRenamedType(const Slice::TypePtr&) = 0;

    //
    // Execute the custom transformation for the given old and new Data values.
    //
    virtual void executeCustomTransform(const DataPtr&, const DataPtr&) = 0;

    //
    // Indicates whether objects should be removed if no class definition is found.
    //
    virtual bool purgeObjects() = 0;

    //
    // Associates old object instances with their transformed equivalents.
    //
    virtual ObjectDataMap& getObjectDataMap() = 0;
};

//
// TransformVisitor is used to visit a destination Data value and
// preserve as much information as possible from the source Data value.
//
class TransformVisitor : public DataVisitor
{
public:

    TransformVisitor(const DataPtr&, const DataFactoryPtr&, const ErrorReporterPtr&, TransformInfo*,
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

    void transformObject(const ObjectDataPtr&, const ObjectDataPtr&);
    bool checkRename(const Slice::TypePtr&, const Slice::TypePtr&);
    bool isCompatible(const Slice::TypePtr&, const Slice::TypePtr&);
    bool checkClasses(const Slice::ClassDeclPtr&, const Slice::ClassDeclPtr&);
    void typeMismatchError(const Slice::TypePtr&, const Slice::TypePtr&);
    void conversionError(const Slice::TypePtr&, const Slice::TypePtr&, const std::string&);
    void rangeError(const std::string&, const Slice::TypePtr&);
    void warning(const std::string&);

    DataPtr _src;
    DataFactoryPtr _factory;
    ErrorReporterPtr _errorReporter;
    TransformInfo* _info;
    std::string _context; // Provides additional detail for use in warning messages.
};

} // End of namespace FreezeScript

#endif
