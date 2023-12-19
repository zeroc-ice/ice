//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SLICE_GRAMMAR_UTIL_H
#define SLICE_GRAMMAR_UTIL_H

#include <Slice/Parser.h>

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
class EnumeratorListTok;
class ConstDefTok;
class TaggedDefTok;
class ClassIdTok;

typedef ::IceUtil::Handle<StringTok> StringTokPtr;
typedef ::IceUtil::Handle<StringListTok> StringListTokPtr;
typedef ::IceUtil::Handle<TypeStringTok> TypeStringTokPtr;
typedef ::IceUtil::Handle<TypeStringListTok> TypeStringListTokPtr;
typedef ::IceUtil::Handle<BoolTok> BoolTokPtr;
typedef ::IceUtil::Handle<IntegerTok> IntegerTokPtr;
typedef ::IceUtil::Handle<FloatingTok> FloatingTokPtr;
typedef ::IceUtil::Handle<ExceptionListTok> ExceptionListTokPtr;
typedef ::IceUtil::Handle<ClassListTok> ClassListTokPtr;
typedef ::IceUtil::Handle<EnumeratorListTok> EnumeratorListTokPtr;
typedef ::IceUtil::Handle<ConstDefTok> ConstDefTokPtr;
typedef ::IceUtil::Handle<TaggedDefTok> TaggedDefTokPtr;
typedef ::IceUtil::Handle<ClassIdTok> ClassIdTokPtr;

// ----------------------------------------------------------------------
// StringTok
// ----------------------------------------------------------------------

class StringTok : public GrammarBase
{
public:

    StringTok() { }
    std::string v;
    std::string literal;
};

// ----------------------------------------------------------------------
// StringListTok
// ----------------------------------------------------------------------

class StringListTok : public GrammarBase
{
public:

    StringListTok() { }
    StringList v;
};

// ----------------------------------------------------------------------
// TypeStringTok
// ----------------------------------------------------------------------

class TypeStringTok : public GrammarBase
{
public:

    TypeStringTok() { }
    TypeString v;
};

// ----------------------------------------------------------------------
// TypeStringListTok
// ----------------------------------------------------------------------

class TypeStringListTok : public GrammarBase
{
public:

    TypeStringListTok() { }
    TypeStringList v;
};

// ----------------------------------------------------------------------
// IntegerTok
// ----------------------------------------------------------------------

class IntegerTok : public GrammarBase
{
public:

    IntegerTok() { }
    IceUtil::Int64 v;
    std::string literal;
};

// ----------------------------------------------------------------------
// FloatingTok
// ----------------------------------------------------------------------

class FloatingTok : public GrammarBase
{
public:

    FloatingTok() { }
    double v;
    std::string literal;
};

// ----------------------------------------------------------------------
// BoolTok
// ----------------------------------------------------------------------

class BoolTok : public GrammarBase
{
public:

    BoolTok() { }
    bool v;
};

// ----------------------------------------------------------------------
// ExceptionListTok
// ----------------------------------------------------------------------

class ExceptionListTok : public GrammarBase
{
public:

    ExceptionListTok() { }
    ExceptionList v;
};

// ----------------------------------------------------------------------
// ClassListTok
// ----------------------------------------------------------------------

class ClassListTok : public GrammarBase
{
public:

    ClassListTok() { }
    ClassList v;
};

// ----------------------------------------------------------------------
// EnumeratorListTok
// ----------------------------------------------------------------------

class EnumeratorListTok : public GrammarBase
{
public:

    EnumeratorListTok() { }
    EnumeratorList v;
};

// ----------------------------------------------------------------------
// ConstDefTok
// ----------------------------------------------------------------------

class ConstDefTok : public GrammarBase
{
public:

    ConstDefTok() { }
    ConstDefTok(SyntaxTreeBasePtr value, std::string stringValue, std::string literalValue) :
        v(value),
        valueAsString(stringValue),
        valueAsLiteral(literalValue)
    { }

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

    TaggedDefTok() { }
    TaggedDefTok(int t) :
        isTagged(t >= 0),
        tag(t)
    { }

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

    ClassIdTok() { }
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
