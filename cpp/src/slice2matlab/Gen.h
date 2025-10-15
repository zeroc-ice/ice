// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    //
    // CodeVisitor generates the Matlab mapping for a translation unit.
    //
    class CodeVisitor final : public ParserVisitor
    {
    public:
        CodeVisitor(std::string dir);

        bool visitClassDefStart(const ClassDefPtr&) final;
        void visitClassDefEnd(const ClassDefPtr&) final;

        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        void visitOperation(const OperationPtr&) final;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) final;

        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitExceptionEnd(const ExceptionPtr&) final;

        bool visitStructStart(const StructPtr&) final;
        void visitStructEnd(const StructPtr&) final;

        void visitDataMember(const DataMemberPtr&) final;

        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

    private:
        void openClass(const std::string& abs, const std::string& dir);
        void closeClass();

        //
        // Convert an operation mode into a string.
        //
        std::string getOperationMode(Operation::Mode);

        std::string getOptionalFormat(const TypePtr&);
        std::string getFormatType(FormatType);

        void marshal(IceInternal::Output&, const std::string&, const std::string&, const TypePtr&, bool, std::int32_t);
        void
        unmarshal(IceInternal::Output&, const std::string&, const std::string&, const TypePtr&, bool, std::int32_t);

        void unmarshalStruct(IceInternal::Output&, const StructPtr&, const std::string&);
        void convertStruct(IceInternal::Output&, const StructPtr&, const std::string&);

        const std::string _dir;

        // The current class file being written.
        std::unique_ptr<IceInternal::Output> _out;
    };

    /// Converts a Slice link to a MATLAB link.
    /// @returns a pair containing a fully-qualified MATLAB link, and a locally-scoped MATLAB link, in that order.
    std::pair<std::string, std::string>
    matlabLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);
}

#endif
