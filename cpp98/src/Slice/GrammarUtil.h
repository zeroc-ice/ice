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
class OptionalDefTok;
class OptionalTypeDefTok;
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
typedef ::IceUtil::Handle<OptionalDefTok> OptionalDefTokPtr;
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
    ConstDef v;
};

// ----------------------------------------------------------------------
// OptionalDefTok
// ----------------------------------------------------------------------

class OptionalDefTok : public GrammarBase
{
public:

    OptionalDefTok() { }
    OptionalDef v;
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

}

//
// Stuff for flex and bison
//

#define YYSTYPE Slice::GrammarBasePtr
#define YY_DECL int slice_lex(YYSTYPE* yylvalp)
YY_DECL;

//
// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
//
#define YYMAXDEPTH  10000
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

//
// Newer bison versions allow to disable stack resizing by defining
// yyoverflow.
//
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

#endif
