// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "SwiftUtil.h"

using namespace std;
using namespace Slice;

namespace Slice
{
    class Gen final
    {
    public:
        Gen(const std::string&, const std::vector<std::string>&, const std::string&);
        Gen(const Gen&) = delete;
        ~Gen();

        Gen& operator=(const Gen&) = delete;

        void generate(const UnitPtr&);
        void printHeader();

    private:
        IceInternal::Output _out;

        std::vector<std::string> _includePaths;
        std::string _fileBase;

        class ImportVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            ImportVisitor(IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;

            void writeImports();

        private:
            void addImport(const SyntaxTreeBasePtr& p, const ContainedPtr& usedBy);
            void addImport(const std::string& module);

            IceInternal::Output& out;
            std::vector<std::string> _imports;
        };

        class TypesVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            TypesVisitor(IceInternal::Output&);

            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            void visitClassDefEnd(const ClassDefPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;

            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            IceInternal::Output& out;
        };

        // Generates the Disp structs and servant protocols.
        class ServantVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            ServantVisitor(IceInternal::Output&);

            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            IceInternal::Output& out;
        };

        // Generate extensions for the servant protocols.
        class ServantExtVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            ServantExtVisitor(IceInternal::Output&);

            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            IceInternal::Output& out;
        };
    };
}

#endif
