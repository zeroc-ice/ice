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
        CodeVisitor(std::string);

        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

    private:
        //
        // Convert an operation mode into a string.
        //
        std::string getOperationMode(Operation::Mode);

        std::string getOptionalFormat(const TypePtr&);
        std::string getFormatType(FormatType);

        void marshal(IceInternal::Output&, const std::string&, const  std::string&, const TypePtr&, bool,  std::int32_t);
        void unmarshal(IceInternal::Output&, const  std::string&, const std::string&, const TypePtr&, bool, std::int32_t);

        void unmarshalStruct(IceInternal::Output&, const StructPtr&, const std::string&);
        void convertStruct(IceInternal::Output&, const StructPtr&, const std::string&);

        void writeBaseClassArrayParams(IceInternal::Output& out, const DataMemberList& baseMembers);

        const std::string _dir;
    };
}

#endif
