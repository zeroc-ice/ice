// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <JsUtil.h>

namespace Slice
{

class JsVisitor : public JsGenerator, public ParserVisitor
{
public:

    JsVisitor(::IceUtilInternal::Output&);
    virtual ~JsVisitor();

protected:

    void writeMarshalUnmarshalParams(const ParamDeclList&, const OperationPtr&, bool);
    void writePostUnmarshalParams(const ParamDeclList&, const OperationPtr&);
    void writeMarshalDataMembers(const DataMemberList&, const DataMemberList&);
    void writeUnmarshalDataMembers(const DataMemberList&, const DataMemberList&);
    void writeInitDataMembers(const DataMemberList&, const std::string&);

    virtual std::vector<std::string> getParams(const OperationPtr&);
    virtual std::vector<std::string> getParamsAsync(const OperationPtr&, bool, bool = false);
    virtual std::vector<std::string> getParamsAsyncCB(const OperationPtr&, bool = false, bool = true);
    virtual std::vector<std::string> getArgs(const OperationPtr&);
    virtual std::vector<std::string> getArgsAsync(const OperationPtr&, bool = false);
    virtual std::vector<std::string> getArgsAsyncCB(const OperationPtr&, bool = false, bool = false);

    std::string getValue(const std::string&, const TypePtr&);

    void writeConstantValue(const std::string&, const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

    static StringList splitComment(const ContainedPtr&);
    void writeDocComment(const ContainedPtr&, const std::string&, const std::string& = "");

    ::IceUtilInternal::Output& _out;
};

class Gen : public JsGenerator
{
public:

    Gen(const std::string&,
        const std::vector<std::string>&,
        const std::string&);

    Gen(const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        std::ostream&);
    ~Gen();

    void generate(const UnitPtr&);
    void closeOutput();

private:

    std::ofstream _stdout;
    IceUtilInternal::Output _out;

    std::vector<std::string> _includePaths;
    std::string _fileBase;
    bool _useStdout;

    void printHeader();

    class RequireVisitor : public JsVisitor
    {
    public:

        RequireVisitor(::IceUtilInternal::Output&, std::vector<std::string>, bool);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);

        std::vector< std::string> writeRequires(const UnitPtr&);

    private:

        bool _icejs;
        bool _seenClass;
        bool _seenCompactId;
        bool _seenOperation;
        bool _seenStruct;
        bool _seenUserException;
        bool _seenLocalException;
        bool _seenEnum;
        bool _seenObjectSeq;
        bool _seenObjectDict;
        std::vector<std::string> _includePaths;
    };

    class TypesVisitor : public JsVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&, std::vector< std::string>, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        std::string encodeTypeForOperation(const TypePtr&);

        std::vector<std::string> _seenModules;
        bool _icejs;
    };

    class ExportVisitor : public JsVisitor
    {
    public:

        ExportVisitor(::IceUtilInternal::Output&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
    private:

        bool _icejs;
    };
};

}

#endif
