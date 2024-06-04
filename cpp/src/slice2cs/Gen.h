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
        CsVisitor(::IceUtilInternal::Output&);
        virtual ~CsVisitor();

    protected:
        void writeMarshalUnmarshalParams(
            const ParamDeclList&,
            const OperationPtr&,
            bool,
            const std::string&,
            bool = false,
            bool = false,
            const std::string& = "");
        void writeMarshalDataMember(const DataMemberPtr&, const std::string&, const std::string&, bool = false);
        void writeUnmarshalDataMember(const DataMemberPtr&, const std::string&, const std::string&, bool = false);

        virtual void writeInheritedOperations(const InterfaceDefPtr&);
        virtual void writeMarshaling(const ClassDefPtr&);

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
        void emitComVisibleAttribute();
        void emitGeneratedCodeAttribute();
        void emitNonBrowsableAttribute();
        void emitPartialTypeAttributes();

        static std::string getParamAttributes(const ParamDeclPtr&);

        std::string writeValue(const TypePtr&, const std::string&);

        void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

        // Generates "= null!" for non-nullable fields (Slice class and exception only).
        void writeDataMemberInitializers(const DataMemberList&, unsigned int);

        std::string toCsIdent(const std::string&);
        std::string editMarkup(const std::string&);
        StringList splitIntoLines(const std::string&);
        void splitComment(const ContainedPtr&, StringList&, StringList&);
        StringList getSummary(const ContainedPtr&);
        void writeDocComment(const ContainedPtr&, const std::string&, const std::string& = "");
        void writeDocCommentOp(const OperationPtr&);

        enum ParamDir
        {
            InParam,
            OutParam
        };
        void writeDocCommentAMI(
            const OperationPtr&,
            ParamDir,
            const std::string&,
            const std::string& = "",
            const std::string& = "",
            const std::string& = "");
        void writeDocCommentTaskAsyncAMI(
            const OperationPtr&,
            const std::string&,
            const std::string& = "",
            const std::string& = "",
            const std::string& = "");
        void writeDocCommentAMD(const OperationPtr&, const std::string&);
        void writeDocCommentParam(const OperationPtr&, ParamDir, bool);

        void moduleStart(const ModulePtr&);
        void moduleEnd(const ModulePtr&);

        ::IceUtilInternal::Output& _out;
    };

    class Gen
    {
    public:
        Gen(const std::string&, const std::vector<std::string>&, const std::string&);
        Gen(const Gen&) = delete;
        ~Gen();

        void generate(const UnitPtr&);

    private:
        IceUtilInternal::Output _out;
        std::vector<std::string> _includePaths;

        void printHeader();

        class UnitVisitor : public CsVisitor
        {
        public:
            UnitVisitor(::IceUtilInternal::Output&);

            virtual bool visitUnitStart(const UnitPtr&);
        };

        class TypesVisitor : public CsVisitor
        {
        public:
            TypesVisitor(::IceUtilInternal::Output&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual void visitModuleEnd(const ModulePtr&);
            virtual bool visitClassDefStart(const ClassDefPtr&);
            virtual void visitClassDefEnd(const ClassDefPtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
            virtual void visitOperation(const OperationPtr&);
            virtual bool visitExceptionStart(const ExceptionPtr&);
            virtual void visitExceptionEnd(const ExceptionPtr&);
            virtual bool visitStructStart(const StructPtr&);
            virtual void visitStructEnd(const StructPtr&);
            virtual void visitSequence(const SequencePtr&);
            virtual void visitEnum(const EnumPtr&);
            virtual void visitConst(const ConstPtr&);
            virtual void visitDataMember(const DataMemberPtr&);
        };

        class ResultVisitor : public CsVisitor
        {
        public:
            ResultVisitor(::IceUtilInternal::Output&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual void visitModuleEnd(const ModulePtr&);
            virtual void visitOperation(const OperationPtr&);
        };

        class ProxyVisitor : public CsVisitor
        {
        public:
            ProxyVisitor(::IceUtilInternal::Output&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual void visitModuleEnd(const ModulePtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
            virtual void visitOperation(const OperationPtr&);
        };

        // Generates the iceD helper methods in the server-side interface.
        class DispatchAdapterVisitor final : public CsVisitor
        {
        public:
            DispatchAdapterVisitor(::IceUtilInternal::Output&);

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
        };

        class HelperVisitor : public CsVisitor
        {
        public:
            HelperVisitor(::IceUtilInternal::Output&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual void visitModuleEnd(const ModulePtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
            virtual void visitSequence(const SequencePtr&);
            virtual void visitDictionary(const DictionaryPtr&);
        };

        class DispatcherVisitor : public CsVisitor
        {
        public:
            DispatcherVisitor(::IceUtilInternal::Output&);

            virtual bool visitModuleStart(const ModulePtr&);
            virtual void visitModuleEnd(const ModulePtr&);
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);

        private:
            void writeDispatch(const InterfaceDefPtr&);
        };
    };
}

#endif
