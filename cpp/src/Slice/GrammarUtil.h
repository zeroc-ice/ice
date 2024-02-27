//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SLICE_GRAMMAR_UTIL_H
#define SLICE_GRAMMAR_UTIL_H

#include "Parser.h"
#include "Util.h"
#include <memory>

namespace Slice
{

    class StringTok;
    class StringListTok;
    class TypeStringTok;
    class TypeStringListTok;
    class BoolTok;
    class IntegerTok;
    class FloatingTok;
    class ExceptionListTok;
    class ClassListTok;
    class InterfaceListTok;
    class EnumeratorListTok;
    class ConstDefTok;
    class TaggedDefTok;
    class ClassIdTok;

    using StringTokPtr = std::shared_ptr<StringTok>;
    using StringListTokPtr = std::shared_ptr<StringListTok>;
    using TypeStringTokPtr = std::shared_ptr<TypeStringTok>;
    using TypeStringListTokPtr = std::shared_ptr<TypeStringListTok>;
    using BoolTokPtr = std::shared_ptr<BoolTok>;
    using IntegerTokPtr = std::shared_ptr<IntegerTok>;
    using FloatingTokPtr = std::shared_ptr<FloatingTok>;
    using ExceptionListTokPtr = std::shared_ptr<ExceptionListTok>;
    using ClassListTokPtr = std::shared_ptr<ClassListTok>;
    using InterfaceListTokPtr = std::shared_ptr<InterfaceListTok>;
    using EnumeratorListTokPtr = std::shared_ptr<EnumeratorListTok>;
    using ConstDefTokPtr = std::shared_ptr<ConstDefTok>;
    using TaggedDefTokPtr = std::shared_ptr<TaggedDefTok>;
    using ClassIdTokPtr = std::shared_ptr<ClassIdTok>;

    // ----------------------------------------------------------------------
    // StringTok
    // ----------------------------------------------------------------------

    class StringTok : public GrammarBase
    {
    public:
        StringTok() {}
        std::string v;
        std::string literal;
    };

    // ----------------------------------------------------------------------
    // StringListTok
    // ----------------------------------------------------------------------

    class StringListTok : public GrammarBase
    {
    public:
        StringListTok() {}
        StringList v;
    };

    // ----------------------------------------------------------------------
    // TypeStringTok
    // ----------------------------------------------------------------------

    class TypeStringTok : public GrammarBase
    {
    public:
        TypeStringTok() {}
        TypeString v;
    };

    // ----------------------------------------------------------------------
    // TypeStringListTok
    // ----------------------------------------------------------------------

    class TypeStringListTok : public GrammarBase
    {
    public:
        TypeStringListTok() {}
        TypeStringList v;
    };

    // ----------------------------------------------------------------------
    // IntegerTok
    // ----------------------------------------------------------------------

    class IntegerTok : public GrammarBase
    {
    public:
        IntegerTok() : v(0) {}
        IceUtil::Int64 v;
        std::string literal;
    };

    // ----------------------------------------------------------------------
    // FloatingTok
    // ----------------------------------------------------------------------

    class FloatingTok : public GrammarBase
    {
    public:
        FloatingTok() : v(0) {}
        double v;
        std::string literal;
    };

    // ----------------------------------------------------------------------
    // BoolTok
    // ----------------------------------------------------------------------

    class BoolTok : public GrammarBase
    {
    public:
        BoolTok() : v(false) {}
        bool v;
    };

    // ----------------------------------------------------------------------
    // ExceptionListTok
    // ----------------------------------------------------------------------

    class ExceptionListTok : public GrammarBase
    {
    public:
        ExceptionListTok() {}
        ExceptionList v;
    };

    // ----------------------------------------------------------------------
    // ClassListTok
    // ----------------------------------------------------------------------

    class ClassListTok : public GrammarBase
    {
    public:
        ClassListTok() {}
        ClassList v;
    };

    // ----------------------------------------------------------------------
    // InterfaceListTok
    // ----------------------------------------------------------------------

    class InterfaceListTok : public GrammarBase
    {
    public:
        InterfaceListTok() {}
        InterfaceList v;
    };

    // ----------------------------------------------------------------------
    // EnumeratorListTok
    // ----------------------------------------------------------------------

    class EnumeratorListTok : public GrammarBase
    {
    public:
        EnumeratorListTok() {}
        EnumeratorList v;
    };

    // ----------------------------------------------------------------------
    // ConstDefTok
    // ----------------------------------------------------------------------

    class ConstDefTok : public GrammarBase
    {
    public:
        ConstDefTok() {}
        ConstDefTok(SyntaxTreeBasePtr value, std::string stringValue, std::string literalValue)
            : v(value),
              valueAsString(stringValue),
              valueAsLiteral(literalValue)
        {
        }

        SyntaxTreeBasePtr v;
        std::string valueAsString;
        std::string valueAsLiteral;
    };

    // ----------------------------------------------------------------------
    // TaggedDefTok
    // ----------------------------------------------------------------------

    class TaggedDefTok : public GrammarBase
    {
    public:
        TaggedDefTok() : isTagged(false), tag(0) {}

        TaggedDefTok(int t) : isTagged(t >= 0), tag(t) {}

        TypePtr type;
        std::string name;
        bool isTagged;
        int tag;
    };

    // ----------------------------------------------------------------------
    // ClassIdTok
    // ----------------------------------------------------------------------

    class ClassIdTok : public GrammarBase
    {
    public:
        ClassIdTok() : t(0) {}
        std::string v;
        int t;
    };

    // ----------------------------------------------------------------------
    // TokenContext: stores the location of tokens.
    // ----------------------------------------------------------------------

    struct TokenContext
    {
        int firstLine;
        int lastLine;
        int firstColumn;
        int lastColumn;
        std::string filename;
    };

}

#endif
