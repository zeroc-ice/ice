// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

class SLICE_API VbGenerator : private ::IceUtil::noncopyable
{
public:

    virtual ~VbGenerator() {};

    //
    // Convert a dimension-less array declaration to one with a dimension.
    //
    static std::string toArrayAlloc(const std::string& decl, const std::string& sz);

    //
    // Validate all metadata in the unit with a "vb:" prefix.
    //
    static void validateMetaData(const UnitPtr&);

protected:
    static std::string fixId(const std::string&, int = 0, bool = false);
    static std::string typeToString(const TypePtr&);
    static bool isValueType(const TypePtr&);
    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(::IceUtil::Output&, const TypePtr&, const std::string&, bool, bool,
                                   bool, const std::string& = "");
    void writeSequenceMarshalUnmarshalCode(::IceUtil::Output&, const SequencePtr&, const std::string&, bool, bool);

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:
        MetaDataVisitor();

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
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
        bool _globalMetaDataDone;
    };
};

}

#endif
