// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"
#include "CPlusPlusUtil.h"
#include "TypeContext.h"

namespace Slice
{
    class Gen final
    {
    public:
        Gen(std::string,
            std::string,
            std::string,
            const std::vector<std::string>&,
            std::string,
            const std::vector<std::string>&,
            std::string,
            std::string);
        ~Gen();

        Gen(const Gen&) = delete;
        Gen& operator=(const Gen&) = delete;

        void generate(const UnitPtr&);

        static TypeContext setUseWstring(const ContainedPtr&, std::list<TypeContext>&, TypeContext);
        static TypeContext resetUseWstring(std::list<TypeContext>&);

    private:
        void writeExtraHeaders(IceInternal::Output&);

        /// Returns the header extension defined in the file metadata for a given file,
        /// or an empty string if no file metadata was found.
        std::string getHeaderExt(std::string_view file, const UnitPtr& unit);

        /// Returns the source extension defined in the file metadata for a given file,
        /// or an empty string if no file metadata was found.
        std::string getSourceExt(std::string_view file, const UnitPtr& unit);

        IceInternal::Output H;
        IceInternal::Output C;

        std::string _base;
        std::string _headerExtension;
        std::string _sourceExtension;
        std::vector<std::string> _extraHeaders;
        std::string _include;
        std::vector<std::string> _includePaths;
        std::string _dllExport;
        std::string _dir;

        // Visitors, in code-generation order.

        /// Generates forward declarations for classes, proxies and structs. Also generates using aliases for sequences
        /// and dictionaries, enum definitions and constants, and printTypeName helper functions for sequences and
        /// dictionaries.
        class ForwardDeclVisitor final : public ParserVisitor
        {
        public:
            ForwardDeclVisitor(IceInternal::Output&, IceInternal::Output&, std::string);
            ForwardDeclVisitor(const ForwardDeclVisitor&) = delete;

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            void visitClassDecl(const ClassDeclPtr&) final;
            void visitInterfaceDecl(const InterfaceDeclPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;

        private:
            IceInternal::Output& H;
            IceInternal::Output& C;

            std::string _dllExport;
            TypeContext _useWstring{TypeContext::None};
            std::list<TypeContext> _useWstringHist;
            bool _firstElement{true};
        };

        /// Generates the code that registers classes and exceptions with the DefaultSliceLoader instance.
        class SliceLoaderVisitor final : public ParserVisitor
        {
        public:
            SliceLoaderVisitor(IceInternal::Output&);
            SliceLoaderVisitor(const SliceLoaderVisitor&) = delete;

            bool visitUnitStart(const UnitPtr&) final;
            void visitUnitEnd(const UnitPtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;

        private:
            IceInternal::Output& C;
        };

        /// Generates code for proxies. It needs to be generated before the code for structs, classes, and exceptions
        /// because a data member with a proxy type (e.g., std::optional<GreeterPrx>) needs to see a complete type.
        class ProxyVisitor final : public ParserVisitor
        {
        public:
            ProxyVisitor(IceInternal::Output&, IceInternal::Output&, std::string);
            ProxyVisitor(const ProxyVisitor&) = delete;

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            void emitOperationImpl(
                const OperationPtr& p,
                const std::string& prefix,
                const std::vector<std::string>& outgoingAsyncParams);

            IceInternal::Output& H;
            IceInternal::Output& C;

            std::string _dllExport;
            TypeContext _useWstring{TypeContext::None};
            std::list<TypeContext> _useWstringHist;
            bool _firstElement{true};
        };

        /// Generates code for definitions with data members - structs, classes, and exceptions.
        class DataDefVisitor final : public ParserVisitor
        {
        public:
            DataDefVisitor(IceInternal::Output&, IceInternal::Output&, std::string);
            DataDefVisitor(const DataDefVisitor&) = delete;

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitStructEnd(const StructPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitExceptionEnd(const ExceptionPtr&) final;
            bool visitClassDefStart(const ClassDefPtr&) final;
            void visitClassDefEnd(const ClassDefPtr&) final;
            void visitDataMember(const DataMemberPtr&) final;

        private:
            bool emitBaseInitializers(const ClassDefPtr&);
            void emitOneShotConstructor(const ClassDefPtr&);
            void emitDataMember(const DataMemberPtr&);

            void printFields(const DataMemberList& fields, bool firstField);

            IceInternal::Output& H;
            IceInternal::Output& C;

            std::string _dllExport;
            TypeContext _useWstring{TypeContext::None};
            std::list<TypeContext> _useWstringHist;
            bool _firstElement{true};
        };

        /// Generates the server-side classes that applications use to implement Ice objects.
        class InterfaceVisitor final : public ParserVisitor
        {
        public:
            InterfaceVisitor(IceInternal::Output& h, IceInternal::Output& c, std::string dllExport, bool async);
            InterfaceVisitor(const InterfaceVisitor&) = delete;

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;

        private:
            [[nodiscard]] std::string skeletonPrefix() const;
            [[nodiscard]] std::string prependSkeletonPrefix(const std::string& name) const;

            IceInternal::Output& H;
            IceInternal::Output& C;
            std::string _dllExport;
            bool _async;

            TypeContext _useWstring{TypeContext::None};
            std::list<TypeContext> _useWstringHist;
            bool _firstElement{true};
        };

        /// Generates internal StreamHelper template specializations for enums and structs.
        class StreamVisitor final : public ParserVisitor
        {
        public:
            StreamVisitor(IceInternal::Output&);
            StreamVisitor(const StreamVisitor&) = delete;

            bool visitModuleStart(const ModulePtr&) final;
            void visitModuleEnd(const ModulePtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitEnum(const EnumPtr&) final;

        private:
            IceInternal::Output& H;
            bool _firstElement{true};
        };
    };
}

#endif
