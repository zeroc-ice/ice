// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SLICE_GRAMMER_UTIL_H
#define SLICE_GRAMMER_UTIL_H

#include <Slice/Parser.h>
#include <map>

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
class SyntaxTreeBaseStringTok;

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
typedef ::IceUtil::Handle<SyntaxTreeBaseStringTok> SyntaxTreeBaseStringTokPtr;

// ----------------------------------------------------------------------
// StringTok
// ----------------------------------------------------------------------

class SLICE_API StringTok : public GrammarBase
{
public:

    StringTok() { }
    std::string v;
};

// ----------------------------------------------------------------------
// StringListTok
// ----------------------------------------------------------------------

class SLICE_API StringListTok : public GrammarBase
{
public:

    StringListTok() { }
    StringList v;
};

// ----------------------------------------------------------------------
// TypeStringTok
// ----------------------------------------------------------------------

class SLICE_API TypeStringTok : public GrammarBase
{
public:

    TypeStringTok() { }
    TypeString v;
};

// ----------------------------------------------------------------------
// TypeStringListTok
// ----------------------------------------------------------------------

class SLICE_API TypeStringListTok : public GrammarBase
{
public:

    TypeStringListTok() { }
    TypeStringList v;
};

// ----------------------------------------------------------------------
// IntegerTok
// ----------------------------------------------------------------------

class SLICE_API IntegerTok : public GrammarBase
{
public:

    IntegerTok() { }
    IceUtil::Int64 v;
};

// ----------------------------------------------------------------------
// FloatingTok
// ----------------------------------------------------------------------

class SLICE_API FloatingTok : public GrammarBase
{
public:

    FloatingTok() { }
    Double v;
};

// ----------------------------------------------------------------------
// BoolTok
// ----------------------------------------------------------------------

class SLICE_API BoolTok : public GrammarBase
{
public:

    BoolTok() { }
    bool v;
};

// ----------------------------------------------------------------------
// ExceptionListTok
// ----------------------------------------------------------------------

class SLICE_API ExceptionListTok : public GrammarBase
{
public:

    ExceptionListTok() { }
    ExceptionList v;
};

// ----------------------------------------------------------------------
// ClassListTok
// ----------------------------------------------------------------------

class SLICE_API ClassListTok : public GrammarBase
{
public:

    ClassListTok() { }
    ClassList v;
};

// ----------------------------------------------------------------------
// EnumeratorListTok
// ----------------------------------------------------------------------

class SLICE_API EnumeratorListTok : public GrammarBase
{
public:

    EnumeratorListTok() { }
    EnumeratorList v;
};

// ----------------------------------------------------------------------
// SyntaxTreeBaseStringTok
// ----------------------------------------------------------------------

class SLICE_API SyntaxTreeBaseStringTok : public GrammarBase
{
public:

    SyntaxTreeBaseStringTok() { }
    SyntaxTreeBaseString v;
};

}

#endif
