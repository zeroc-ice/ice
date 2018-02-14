// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
class TransformInfo : public IceUtil::SimpleShared
{
public:

    virtual ~TransformInfo() {}

    //
    // Get data factory.
    //
    virtual DataFactoryPtr getDataFactory() = 0;

    //
    // Get error reporter.
    //
    virtual ErrorReporterPtr getErrorReporter() = 0;

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
    // The facet name of the database (evictor only).
    //
    virtual std::string facetName() = 0;

    //
    // Indicates whether objects should be removed if no class definition is found.
    //
    virtual bool purgeObjects() = 0;

    //
    // Associates old object instances with their transformed equivalents.
    //
    virtual ObjectDataMap& getObjectDataMap() = 0;
};
typedef IceUtil::Handle<TransformInfo> TransformInfoPtr;

//
// TransformVisitor is used to visit a destination Data value and
// preserve as much information as possible from the source Data value.
//
class TransformVisitor : public DataVisitor
{
public:

    TransformVisitor(const DataPtr&, const TransformInfoPtr&, const std::string& = std::string());

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
    TransformInfoPtr _info;
    std::string _context; // Provides additional detail for use in warning messages.
};

} // End of namespace FreezeScript

#endif
