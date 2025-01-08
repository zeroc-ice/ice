//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GEN_H
#define GEN_H

#include "CsUtil.h"

namespace Slice
{
    class CsVisitor : public CsGenerator, public ParserVisitor
    {
    public:
        CsVisitor(::IceInternal::Output&);
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

        std::string writeValue(const TypePtr&, const std::string&);

        void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

        // Generates "= null!" for non-nullable fields (Slice class and exception only).
        void writeDataMemberInitializers(const DataMemberList&, unsigned int);

        std::string toCsIdent(const std::string&);
        std::string editMarkup(const std::string&);
        StringList splitIntoLines(const std::string&);
        void splitDocComment(const ContainedPtr&, StringList&, StringList&);
        StringList getSummary(const ContainedPtr&);
        void writeDocComment(const ContainedPtr&, const std::string& = "");
        void writeDocCommentOp(const OperationPtr&);

        enum ParamDir
        {
            InParam,
            OutParam
        };
        void writeDocCommentAMI(
            const OperationPtr&,
            ParamDir,
            const std::string& = "",
            const std::string& = "",
            const std::string& = "");
        void writeDocCommentTaskAsyncAMI(
            const OperationPtr&,
            const std::string& = "",
            const std::string& = "",
            const std::string& = "");
        void writeDocCommentAMD(const OperationPtr&, const std::string&);
        void writeDocCommentParam(const OperationPtr&, ParamDir, bool);

        void moduleStart(const ModulePtr&);
        void moduleEnd(const ModulePtr&);

        ::IceInternal::Output& _out;
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

        class UnitVisitor final : public CsVisitor
        {
        public:
            UnitVisitor(::IceInternal::Output&);

            bool visitUnitStart(const UnitPtr&) final;
        };

        class TypesVisitor final : public CsVisitor
        {
        public:
            TypesVisitor(::IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            void visitClassDefEnd(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitExceptionEnd(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitStructEnd(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;
            void visitDataMember(const DataMemberPtr&) final;
        };

        class ResultVisitor final : public CsVisitor
        {
        public:
            ResultVisitor(::IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            void visitOperation(const OperationPtr&) final;
        };

        class ProxyVisitor final : public CsVisitor
        {
        public:
            ProxyVisitor(::IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
        };

        // Generates the iceD helper methods in the server-side interface.
        class DispatchAdapterVisitor final : public CsVisitor
        {
        public:
            DispatchAdapterVisitor(::IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
        };

        class HelperVisitor final : public CsVisitor
        {
        public:
            HelperVisitor(::IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
        };

        class DispatcherVisitor final : public CsVisitor
        {
        public:
            DispatcherVisitor(::IceInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;

        private:
            void writeDispatch(const InterfaceDefPtr&);
        };
    };
}

#endif
