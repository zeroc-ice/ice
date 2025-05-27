// Copyright (c) ZeroC, Inc.

#ifndef SLICE_GRAMMAR_UTIL_H
#define SLICE_GRAMMAR_UTIL_H

#include "Parser.h"
#include "Util.h"
#include <cassert>
#include <memory>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace Slice
{
    struct StringTok final : public GrammarBase
    {
        std::string v;
        std::string literal;
    };

    struct MetadataListTok final : public GrammarBase
    {
        MetadataList v;
    };

    struct TypeStringTok final : public GrammarBase
    {
        TypeStringTok(TypePtr type, std::string name) : type(std::move(type)), name(std::move(name)) {}

        const TypePtr type;
        const std::string name;
    };

    struct IntegerTok final : public GrammarBase
    {
        std::int64_t v{0};
        std::string literal;
    };

    struct FloatingTok final : public GrammarBase
    {
        double v{0};
        std::string literal;
    };

    struct BoolTok final : public GrammarBase
    {
        BoolTok(bool value) : v(value) {}

        bool v{false};
    };

    struct ExceptionListTok final : public GrammarBase
    {
        ExceptionList v;
    };

    struct InterfaceListTok final : public GrammarBase
    {
        InterfaceList v;
    };

    struct EnumeratorListTok final : public GrammarBase
    {
        EnumeratorList v;
    };

    struct ConstDefTok final : public GrammarBase
    {
        ConstDefTok() = default; // invalid constant

        ConstDefTok(SyntaxTreeBasePtr value, std::string stringValue)
            : v(std::move(value)),
              valueAsString(std::move(stringValue))
        {
            // Occasionally, v is nullptr but valueAsString is not empty and represents an enumerator.
            // The Parser looks up the enumerator by name later in validateConstant().
            assert(v || !valueAsString.empty());
        }

        const SyntaxTreeBasePtr v;
        const std::string valueAsString;
    };

    struct OptionalDefTok final : public GrammarBase
    {
        explicit OptionalDefTok(int t) : isOptional(t >= 0), tag(t) {}

        TypePtr type;
        std::string name;
        const bool isOptional{false};
        const std::int32_t tag{0};
    };

    struct ClassIdTok final : public GrammarBase
    {
        std::string v;
        std::int32_t t{0};
    };

    struct TokenContext
    {
        int firstLine;
        int lastLine;
        int firstColumn;
        int lastColumn;
        std::string filename;
    };

    using StringTokPtr = std::shared_ptr<StringTok>;
    using IntegerTokPtr = std::shared_ptr<IntegerTok>;
    using FloatingTokPtr = std::shared_ptr<FloatingTok>;
    using ConstDefTokPtr = std::shared_ptr<ConstDefTok>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
