// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

    JsVisitor(::IceUtilInternal::Output&,
              const std::vector<std::pair<std::string, std::string> >& imports =
                    std::vector<std::pair<std::string, std::string> >());
    virtual ~JsVisitor();

    std::vector<std::pair<std::string, std::string> > imports() const;

protected:

    void writeMarshalDataMembers(const DataMemberList&, const DataMemberList&);
    void writeUnmarshalDataMembers(const DataMemberList&, const DataMemberList&);
    void writeInitDataMembers(const DataMemberList&);

    std::string getValue(const std::string&, const TypePtr&);

    std::string writeConstantValue(const std::string&, const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

    static StringList splitComment(const ContainedPtr&);
    void writeDocComment(const ContainedPtr&, const std::string&, const std::string& = "");

    ::IceUtilInternal::Output& _out;

    std::vector<std::pair<std::string, std::string> > _imports;
};

class Gen : public JsGenerator
{
public:

    Gen(const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        bool);

    Gen(const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        bool,
        std::ostream&);

    ~Gen();

    void generate(const UnitPtr&);
    void closeOutput();

private:

    IceUtilInternal::Output _jsout;
    IceUtilInternal::Output _tsout;

    std::vector<std::string> _includePaths;
    std::string _fileBase;
    bool _useStdout;
    bool _typeScript;

    class RequireVisitor : public JsVisitor
    {
    public:

        RequireVisitor(::IceUtilInternal::Output&, std::vector<std::string>, bool, bool);

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
        bool _es6modules;
        bool _seenClass;
        bool _seenCompactId;
        bool _seenOperation;
        bool _seenStruct;
        bool _seenUserException;
        bool _seenLocalException;
        bool _seenEnum;
        bool _seenObjectSeq;
        bool _seenObjectProxySeq;
        bool _seenObjectDict;
        bool _seenObjectProxyDict;
        std::vector<std::string> _includePaths;
    };

    class TypesVisitor : public JsVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&, std::vector<std::string>, bool);

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

        ExportVisitor(::IceUtilInternal::Output&, bool, bool);

        virtual bool visitModuleStart(const ModulePtr&);

    private:

        bool _icejs;
        bool _es6modules;
        std::vector<std::string> _exported;
    };

    class TypeScriptRequireVisitor : public JsVisitor
    {
    public:

        TypeScriptRequireVisitor(::IceUtilInternal::Output&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);

    private:

        void addImport(const TypePtr&, const ContainedPtr&);
        void addImport(const ContainedPtr&, const ContainedPtr&);
        void addImport(const std::string&, const std::string&, const std::string&, const std::string&);

        std::string nextImportPrefix();

        bool _icejs;
        int _nextImport;
        std::map<std::string, std::string> _modulePrefix;
    };

    class TypeScriptAliasVisitor : public JsVisitor
    {
    public:

        TypeScriptAliasVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);

        void writeAlias(const UnitPtr&);

    private:

        void addAlias(const ExceptionPtr&, const ContainedPtr&);
        void addAlias(const TypePtr&, const ContainedPtr&);
        void addAlias(const std::string&, const std::string&, const ContainedPtr&);
        std::vector<std::pair<std::string, std::string> > _aliases;
    };

    class TypeScriptVisitor : public JsVisitor
    {
    public:

        TypeScriptVisitor(::IceUtilInternal::Output&,
                          const std::vector<std::pair<std::string, std::string> >&);

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

        void writeImports();
        bool _wroteImports;
    };
};

}

#endif
