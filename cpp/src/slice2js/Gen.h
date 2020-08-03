//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    void writeMarshalDataMembers(const MemberList& dataMembers);
    void writeUnmarshalDataMembers(const MemberList& dataMembers);
    void writeInitDataMembers(const MemberList&);

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

        bool visitClassDefStart(const ClassDefPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        void visitOperation(const OperationPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;
        void visitEnum(const EnumPtr&) override;

        std::vector< std::string> writeRequires(const UnitPtr&);

    private:

        bool _icejs;
        bool _es6modules;
        bool _seenClass;
        bool _seenCompactId;
        bool _seenInterface;
        bool _seenOperation;
        bool _seenStruct;
        bool _seenUserException;
        bool _seenEnum;
        bool _seenObjectSeq;
        bool _seenValueSeq;
        bool _seenObjectDict;
        bool _seenValueDict;
        std::vector<std::string> _includePaths;
    };

    class TypesVisitor : public JsVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&, std::vector<std::string>, bool);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;
        void visitEnum(const EnumPtr&) override;
        void visitConst(const ConstPtr&) override;

    private:

        std::string encodeTypeForOperation(const TypePtr&);

        std::vector<std::string> _seenModules;
        bool _icejs;
    };

    class ExportVisitor : public JsVisitor
    {
    public:

        ExportVisitor(::IceUtilInternal::Output&, bool, bool);

        bool visitModuleStart(const ModulePtr&) override;

    private:

        bool _icejs;
        bool _es6modules;
        std::vector<std::string> _exported;
    };

    class TypeScriptRequireVisitor : public JsVisitor
    {
    public:

        TypeScriptRequireVisitor(::IceUtilInternal::Output&, bool);

        bool visitModuleStart(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;

    private:

        void addImport(const TypePtr&, const ContainedPtr&);
        void addImport(const ContainedPtr&, const ContainedPtr&);
        void addImport(const std::string&, const std::string&, const std::string&, const std::string&);

        std::string nextImportPrefix();

        bool _icejs;
        int _nextImport;
    };

    class TypeScriptAliasVisitor : public JsVisitor
    {
    public:

        TypeScriptAliasVisitor(::IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;

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

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;
        void visitEnum(const EnumPtr&) override;
        void visitConst(const ConstPtr&) override;

    private:

        void writeImports();
        bool _wroteImports;
    };
};

}

#endif
