//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    class CsGenerator
    {
    public:
        CsGenerator() = default;
        CsGenerator(const CsGenerator&) = delete;
        virtual ~CsGenerator() = default;

        CsGenerator& operator=(const CsGenerator&) = delete;

        //
        // Convert a dimension-less array declaration to one with a dimension.
        //
        static std::string toArrayAlloc(const std::string& decl, const std::string& sz);

        static void validateMetadata(const UnitPtr&);

        //
        // Returns the namespace of a Contained entity.
        //
        static std::string getNamespace(const ContainedPtr&);

        static std::string getUnqualified(const std::string&, const std::string&, bool builtin = false);
        static std::string getUnqualified(
            const ContainedPtr&,
            const std::string& package = "",
            const std::string& prefix = "",
            const std::string& suffix = "");

        static std::string fixId(const std::string&, unsigned int = 0, bool = false);

    protected:
        //
        // Returns the namespace prefix of a Contained entity.
        //
        static std::string getNamespacePrefix(const ContainedPtr&);

        static std::string resultStructName(const std::string&, const std::string&, bool = false);
        static std::string resultType(const OperationPtr&, const std::string&, bool = false);
        static std::string taskResultType(const OperationPtr&, const std::string&, bool = false);
        static std::string getOptionalFormat(const TypePtr&);
        static std::string getStaticId(const TypePtr&);
        static std::string typeToString(const TypePtr&, const std::string&, bool = false);

        // Is this Slice type mapped to a C# value type?
        static bool isValueType(const TypePtr&);

        // Is this Slice struct mapped to a C# class?
        static bool isMappedToClass(const StructPtr& p) { return !isValueType(p); }

        // Is the mapped C# type for this field a non-nullable C# reference type?
        static bool isMappedToNonNullableReference(const DataMemberPtr& p);

        // Is the mapped C# type for this field a non-nullable reference type?
        // string fields are not included since they have a "" default.
        static bool isMappedToRequiredField(const DataMemberPtr&);

        //
        // Generate code to marshal or unmarshal a type
        //
        void writeMarshalUnmarshalCode(
            ::IceInternal::Output&,
            const TypePtr&,
            const std::string&,
            const std::string&,
            bool,
            const std::string& = "");
        void writeOptionalMarshalUnmarshalCode(
            ::IceInternal::Output&,
            const TypePtr&,
            const std::string&,
            const std::string&,
            int,
            bool,
            const std::string& = "");
        void writeSequenceMarshalUnmarshalCode(
            ::IceInternal::Output&,
            const SequencePtr&,
            const std::string&,
            const std::string&,
            bool,
            bool,
            const std::string& = "");
        void writeOptionalSequenceMarshalUnmarshalCode(
            ::IceInternal::Output&,
            const SequencePtr&,
            const std::string&,
            const std::string&,
            int,
            bool,
            const std::string& = "");
    };
}

#endif
