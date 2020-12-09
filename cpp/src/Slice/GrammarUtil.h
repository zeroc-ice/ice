//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SLICE_GRAMMAR_UTIL_H
#define SLICE_GRAMMAR_UTIL_H

#include "Slice/Parser.h"
#include "Slice/Util.h"

#include <memory>

namespace Slice
{

class StringTok;
class StringListTok;
class BoolTok;
class IntegerTok;
class FloatingTok;
class ExceptionListTok;
class ClassListTok;
class InterfaceListTok;
class EnumeratorListTok;
class ConstDefTok;
class TaggedDefTok;
class TaggedDefListTok;
class ClassIdTok;

typedef ::IceUtil::Handle<StringTok> StringTokPtr;
typedef ::IceUtil::Handle<StringListTok> StringListTokPtr;
typedef ::IceUtil::Handle<BoolTok> BoolTokPtr;
typedef ::IceUtil::Handle<IntegerTok> IntegerTokPtr;
typedef ::IceUtil::Handle<FloatingTok> FloatingTokPtr;
typedef ::IceUtil::Handle<ExceptionListTok> ExceptionListTokPtr;
typedef ::IceUtil::Handle<ClassListTok> ClassListTokPtr;
typedef ::IceUtil::Handle<InterfaceListTok> InterfaceListTokPtr;
typedef ::IceUtil::Handle<EnumeratorListTok> EnumeratorListTokPtr;
typedef ::IceUtil::Handle<ConstDefTok> ConstDefTokPtr;
typedef ::IceUtil::Handle<TaggedDefTok> TaggedDefTokPtr;
typedef ::IceUtil::Handle<TaggedDefListTok> TaggedDefListTokPtr;
typedef ::IceUtil::Handle<ClassIdTok> ClassIdTokPtr;

// ----------------------------------------------------------------------
// StringTok
// ----------------------------------------------------------------------

class StringTok : public GrammarBase
{
public:

    std::string v;
    std::string literal;
};

// ----------------------------------------------------------------------
// StringListTok
// ----------------------------------------------------------------------

class StringListTok : public GrammarBase
{
public:

    StringList v;
};

// ----------------------------------------------------------------------
// IntegerTok
// ----------------------------------------------------------------------

class IntegerTok : public GrammarBase
{
public:

    IntegerTok() { }

    IntegerTok(IceUtil::Int64 value) :
        v(value)
    { }

    IceUtil::Int64 v;
    std::string literal;
};

// ----------------------------------------------------------------------
// FloatingTok
// ----------------------------------------------------------------------

class FloatingTok : public GrammarBase
{
public:

    double v;
    std::string literal;
};

// ----------------------------------------------------------------------
// BoolTok
// ----------------------------------------------------------------------

class BoolTok : public GrammarBase
{
public:

    BoolTok(bool value) :
        v(value)
    { }

    bool v;
};

// ----------------------------------------------------------------------
// ExceptionListTok
// ----------------------------------------------------------------------

class ExceptionListTok : public GrammarBase
{
public:

    ExceptionList v;
};

// ----------------------------------------------------------------------
// ClassListTok
// ----------------------------------------------------------------------

class ClassListTok : public GrammarBase
{
public:

    ClassList v;
};

// ----------------------------------------------------------------------
// InterfaceListTok
// ----------------------------------------------------------------------

class InterfaceListTok : public GrammarBase
{
public:

    InterfaceList v;
};

// ----------------------------------------------------------------------
// EnumeratorListTok
// ----------------------------------------------------------------------

class EnumeratorListTok : public GrammarBase
{
public:

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

    TaggedDefTok() :
        isStream(false),
        isTagged(false),
        tag(-1)
    { }

    TaggedDefTok(int t) :
        isStream(false),
        isTagged(true),
        tag(t)
    { }

    TypePtr type;
    std::string name;
    bool isStream;
    bool isTagged;
    int tag;
    StringList metadata;
};

// ----------------------------------------------------------------------
// TaggedDefListTok
// ----------------------------------------------------------------------

class TaggedDefListTok : public GrammarBase
{
public:

    std::list<TaggedDefTokPtr> v;
};

// ----------------------------------------------------------------------
// ClassIdTok
// ----------------------------------------------------------------------

class ClassIdTok : public GrammarBase
{
public:

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
    std::shared_ptr<std::string> filename;
};

}

#endif
