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
    class Gen
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

        class ImportVisitor : public SwiftGenerator, public ParserVisitor
        {
        public:
            ImportVisitor(IceUtilInternal::Output&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual bool visitStructStart(const StructPtr&);
            virtual bool visitExceptionStart(const ExceptionPtr&);
            virtual void visitSequence(const SequencePtr&);
            virtual void visitDictionary(const DictionaryPtr&);

            void writeImports();

        private:
            void addImport(const TypePtr&, const ContainedPtr&);
            void addImport(const ContainedPtr&, const ContainedPtr&);
            void addImport(const std::string&);

            IceUtilInternal::Output& out;
            std::vector<std::string> _imports;
        };

        class TypesVisitor : public SwiftGenerator, public ParserVisitor
        {
        public:
            TypesVisitor(IceUtilInternal::Output&);

            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual bool visitExceptionStart(const ExceptionPtr&);
            virtual bool visitStructStart(const StructPtr&);
            virtual void visitSequence(const SequencePtr&);
            virtual void visitDictionary(const DictionaryPtr&);
            virtual void visitEnum(const EnumPtr&);
            virtual void visitConst(const ConstPtr&);

        private:
            IceUtilInternal::Output& out;
        };

        class ProxyVisitor : public SwiftGenerator, public ParserVisitor
        {
        public:
            ProxyVisitor(::IceUtilInternal::Output&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
            virtual void visitOperation(const OperationPtr&);

        private:
            IceUtilInternal::Output& out;
        };

        class ValueVisitor : public SwiftGenerator, public ParserVisitor
        {
        public:
            ValueVisitor(::IceUtilInternal::Output&);

            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual void visitClassDefEnd(const ClassDefPtr&);

        private:
            IceUtilInternal::Output& out;
        };

        class ObjectVisitor : public SwiftGenerator, public ParserVisitor
        {
        public:
            ObjectVisitor(::IceUtilInternal::Output&);

            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
            virtual void visitOperation(const OperationPtr&);

        private:
            IceUtilInternal::Output& out;
        };

        class ObjectExtVisitor : public SwiftGenerator, public ParserVisitor
        {
        public:
            ObjectExtVisitor(::IceUtilInternal::Output&);

            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
            virtual void visitOperation(const OperationPtr&);

        private:
            IceUtilInternal::Output& out;
        };
    };
}

#endif
