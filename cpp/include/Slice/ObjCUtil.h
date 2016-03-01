// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef OBJC_UTIL_H
#define OBJC_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

enum BaseType
{
    BaseTypeNone,
    BaseTypeObject,
    BaseTypeException
};

class SLICE_API ObjCGenerator : private ::IceUtil::noncopyable
{
public:

    virtual ~ObjCGenerator() {};

    //
    // Validate all metadata in the unit with an "objc:" prefix.
    //
    static void validateMetaData(const UnitPtr&);

protected:
    struct ModulePrefix
    {
        ModulePtr m;
        std::string name;
    };

    static bool addModule(const ModulePtr&, const std::string&);
    static ModulePrefix modulePrefix(const ModulePtr&);
    static std::string moduleName(const ModulePtr&);
    static ModulePtr findModule(const ContainedPtr&, int = 0, bool = false);
    static void modulePrefixError(const ModulePtr&, const std::string&);
    static std::string fixId(const std::string&, int = 0, bool = false);
    static std::string fixId(const ContainedPtr&, int = 0, bool = false);
    static std::string fixName(const ContainedPtr&, int = 0, bool = false);
    static std::string fixScoped(const ContainedPtr&, int = 0, bool = false);
    static std::string getParamId(const ContainedPtr&);
    static std::string getFactoryMethod(const ContainedPtr&, bool);
    static std::string typeToString(const TypePtr&);
    static std::string inTypeToString(const TypePtr&, bool, bool = false, bool = false);
    static std::string outTypeToString(const TypePtr&, bool, bool = false, bool = false);
    static std::string typeToObjCTypeString(const TypePtr&);
    static bool isValueType(const TypePtr&);
    static bool isString(const TypePtr&);
    static bool isClass(const TypePtr&);
    static bool mapsToPointerType(const TypePtr&);
    static std::string getBuiltinName(const BuiltinPtr&);
    static std::string getBuiltinSelector(const BuiltinPtr&, bool);
    static std::string getOptionalHelperGetter(const TypePtr&);
    static std::string getOptionalStreamHelper(const TypePtr&);
    static StringList splitScopedName(const std::string&);
    static std::string getOptionalFormat(const TypePtr&);

    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, bool, bool) const;
    void writeOptMemberMarshalUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, bool) const;
    void writeOptParamMarshalUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, int,
                                           bool) const;

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:
        MetaDataVisitor();

        virtual bool visitUnitStart(const UnitPtr&);
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

        static Slice::StringList getMetaData(const ContainedPtr&);
        static void modulePrefixError(const ModulePtr&, const std::string&);

        static const std::string _objcPrefix; // "objc:"
        static const std::string _msg; // "ignoring invalid metadata"

        StringSet _history;
    };


    //
    // Map of module scoped name to ModulePtr. Used to verify that objc:prefix metadata directives are consistent.
    //

    typedef std::map<std::string, ModulePrefix> ModuleMap;
    static ModuleMap _modules;
};

}

#endif
