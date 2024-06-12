//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        IceUtilInternal::Output _out;

        std::vector<std::string> _includePaths;
        std::string _fileBase;

        class ImportVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            ImportVisitor(IceUtilInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;

            void writeImports();

        private:
            void addImport(const TypePtr&, const ContainedPtr&);
            void addImport(const ContainedPtr&, const ContainedPtr&);
            void addImport(const std::string&);

            IceUtilInternal::Output& out;
            std::vector<std::string> _imports;
        };

        class TypesVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            TypesVisitor(IceUtilInternal::Output&);

            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;

        private:
            IceUtilInternal::Output& out;
        };

        class ProxyVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            ProxyVisitor(::IceUtilInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            IceUtilInternal::Output& out;
        };

        class ValueVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            ValueVisitor(::IceUtilInternal::Output&);

            bool visitClassDefStart(const ClassDefPtr&) final;
            void visitClassDefEnd(const ClassDefPtr&) final;

        private:
            IceUtilInternal::Output& out;
        };

        class ObjectVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            ObjectVisitor(::IceUtilInternal::Output&);

            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            IceUtilInternal::Output& out;
        };

        class ObjectExtVisitor final : public SwiftGenerator, public ParserVisitor
        {
        public:
            ObjectExtVisitor(::IceUtilInternal::Output&);

            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            IceUtilInternal::Output& out;
        };
    };
}

#endif
