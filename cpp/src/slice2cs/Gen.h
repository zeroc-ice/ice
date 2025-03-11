// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "CsUtil.h"

namespace Slice
{
    class CsVisitor : public CsGenerator, public ParserVisitor
    {
    public:
        CsVisitor(IceInternal::Output&);
        ~CsVisitor() override;

    protected:
        void writeMarshalUnmarshalParams(
            const ParameterList&,
            const OperationPtr&,
            bool,
            const std::string&,
            bool = false,
            bool = false,
            const std::string& = "");
        void writeMarshalDataMember(const DataMemberPtr&, const std::string&, const std::string&, bool = false);
        void writeUnmarshalDataMember(const DataMemberPtr&, const std::string&, const std::string&, bool = false);

        void writeMarshaling(const ClassDefPtr&);

        static std::vector<std::string> getParams(const OperationPtr&, const std::string&);
        static std::vector<std::string> getInParams(const OperationPtr&, const std::string&, bool = false);
        static std::vector<std::string> getOutParams(const OperationPtr&, const std::string&, bool, bool);
        static std::vector<std::string> getArgs(const OperationPtr&);
        static std::vector<std::string> getInArgs(const OperationPtr&, bool = false);
        static std::string getDispatchParams(
            const OperationPtr&,
            std::string&,
            std::vector<std::string>&,
            std::vector<std::string>&,
            const std::string&);

        void emitAttributes(const ContainedPtr&);
        void emitNonBrowsableAttribute();

        static std::string getParamAttributes(const ParameterPtr&);

        void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

        // Generates "= null!" for non-nullable fields (Slice class and exception only).
        void writeDataMemberInitializers(const DataMemberList&);

        void writeDocComment(const ContainedPtr& p, const std::string& generatedType, const std::string& notes = "");
        void writeHelperDocComment(
            const ContainedPtr& p,
            const std::string& comment,
            const std::string& generatedType,
            const std::string& notes = "");

        void
        writeOpDocComment(const OperationPtr& operation, const std::vector<std::string>& extraParams, bool isAsync);
        void writeParameterDocComments(const DocComment&, const ParameterList&);

        void moduleStart(const ModulePtr&);
        void moduleEnd(const ModulePtr&);

        IceInternal::Output& _out;
    };

    class Gen final
    {
    public:
        Gen(const std::string&, const std::vector<std::string>&, const std::string&);
        Gen(const Gen&) = delete;
        ~Gen();

        void generate(const UnitPtr&);

    private:
        IceInternal::Output _out;
        std::vector<std::string> _includePaths;

        void printHeader();

        class TypesVisitor final : public CsVisitor
        {
        public:
            TypesVisitor(IceInternal::Output&);

            bool visitUnitStart(const UnitPtr&) final;

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            void visitClassDefEnd(const ClassDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitExceptionEnd(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitStructEnd(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;
            void visitDataMember(const DataMemberPtr&) final;

            // For proxies
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
        };

        class ResultVisitor final : public CsVisitor
        {
        public:
            ResultVisitor(IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            void visitOperation(const OperationPtr&) final;
        };

        class ServantVisitor final : public CsVisitor
        {
        public:
            ServantVisitor(IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            void writeDispatch(const InterfaceDefPtr&);
        };
    };
}

#endif
