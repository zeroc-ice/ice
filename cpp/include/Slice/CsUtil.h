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

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

class SLICE_API CsGenerator : public ::IceUtil::noncopyable
{
public:

    virtual ~CsGenerator() {};

    //
    // Validate all metadata in the unit with a "cs:" prefix.
    //
    static void validateMetaData(const UnitPtr&);

protected:
    static std::string fixId(const std::string&);
    static std::string typeToString(const TypePtr&);
    static bool isValueType(const TypePtr&);
    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(::IceUtil::Output&, const TypePtr&, const std::string&, bool, bool,
                                   bool = false, const std::string& = "");
    void writeSequenceMarshalUnmarshalCode(::IceUtil::Output&, const SequencePtr&, const std::string&, bool, bool);

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitParamDecl(const ParamDeclPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        void validate(const ContainedPtr&);

        StringSet _history;
    };
};

}

#endif
