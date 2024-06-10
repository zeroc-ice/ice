//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GEN_H
#define GEN_H

#include "JsUtil.h"

namespace Slice
{
    class JsVisitor : public JsGenerator, public ParserVisitor
    {
    public:
        JsVisitor(
            ::IceUtilInternal::Output&,
            const std::vector<std::pair<std::string, std::string>>& imports =
                std::vector<std::pair<std::string, std::string>>());
        virtual ~JsVisitor();

        std::vector<std::pair<std::string, std::string>> imports() const;

    protected:
        void writeMarshalDataMembers(const DataMemberList&, const DataMemberList&);
        void writeUnmarshalDataMembers(const DataMemberList&, const DataMemberList&);
        void writeInitDataMembers(const DataMemberList&);

        std::string getValue(const std::string&, const TypePtr&);

        std::string
        writeConstantValue(const std::string&, const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

        static StringList splitComment(const ContainedPtr&);
        void writeDocCommentFor(const ContainedPtr&);

        ::IceUtilInternal::Output& _out;

        std::vector<std::pair<std::string, std::string>> _imports;
    };

    class Gen : public JsGenerator
    {
    public:
        Gen(const std::string&, const std::vector<std::string>&, const std::string&, bool);

        Gen(const std::string&, const std::vector<std::string>&, const std::string&, bool, std::ostream&);

        ~Gen();

        void generate(const UnitPtr&);

    private:
        IceUtilInternal::Output _jsout;
        IceUtilInternal::Output _tsout;

        std::vector<std::string> _includePaths;
        std::string _fileBase;
        bool _useStdout;
        bool _typeScript;

        class ImportVisitor : public JsVisitor
        {
        public:
            ImportVisitor(::IceUtilInternal::Output&, std::vector<std::string>, bool);

            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual bool visitStructStart(const StructPtr&);
            virtual void visitOperation(const OperationPtr&);
            virtual bool visitExceptionStart(const ExceptionPtr&);
            virtual void visitSequence(const SequencePtr&);
            virtual void visitDictionary(const DictionaryPtr&);
            virtual void visitEnum(const EnumPtr&);

            // Emit the import statements for the given unit and return a list of the imported modules.
            std::set<std::string> writeImports(const UnitPtr&);

        private:
            bool _icejs;
            bool _seenClass;
            bool _seenInterface;
            bool _seenCompactId;
            bool _seenOperation;
            bool _seenStruct;
            bool _seenUserException;
            bool _seenEnum;
            bool _seenSeq;
            bool _seenDict;
            bool _seenObjectSeq;
            bool _seenObjectProxySeq;
            bool _seenObjectDict;
            bool _seenObjectProxyDict;
            std::vector<std::string> _includePaths;
        };

        class ExportsVisitor : public JsVisitor
        {
        public:
            ExportsVisitor(::IceUtilInternal::Output&, std::set<std::string>);

            bool visitModuleStart(const ModulePtr&) final;

            std::set<std::string> exportedModules() const;

        private:
            std::string encodeTypeForOperation(const TypePtr&);

            std::set<std::string> _importedModules;
            std::set<std::string> _exportedModules;
        };

        class TypesVisitor : public JsVisitor
        {
        public:
            TypesVisitor(::IceUtilInternal::Output&);

            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual bool visitExceptionStart(const ExceptionPtr&);
            virtual bool visitStructStart(const StructPtr&);
            virtual void visitSequence(const SequencePtr&);
            virtual void visitDictionary(const DictionaryPtr&);
            virtual void visitEnum(const EnumPtr&);
            virtual void visitConst(const ConstPtr&);

        private:
            std::string encodeTypeForOperation(const TypePtr&);
        };

        class TypeScriptImportVisitor : public JsVisitor
        {
        public:
            TypeScriptImportVisitor(::IceUtilInternal::Output&, bool);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
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
        };

        class TypeScriptAliasVisitor : public JsVisitor
        {
        public:
            TypeScriptAliasVisitor(::IceUtilInternal::Output&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual bool visitStructStart(const StructPtr&);
            virtual bool visitExceptionStart(const ExceptionPtr&);
            virtual void visitSequence(const SequencePtr&);
            virtual void visitDictionary(const DictionaryPtr&);

            void writeAlias(const UnitPtr&);

        private:
            void addAlias(const ExceptionPtr&, const ContainedPtr&);
            void addAlias(const TypePtr&, const ContainedPtr&);
            void addAlias(const std::string&, const std::string&, const ContainedPtr&);
            std::vector<std::pair<std::string, std::string>> _aliases;
        };

        class TypeScriptVisitor : public JsVisitor
        {
        public:
            TypeScriptVisitor(::IceUtilInternal::Output&, const std::vector<std::pair<std::string, std::string>>&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual void visitModuleEnd(const ModulePtr&);
            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
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
