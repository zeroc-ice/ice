// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SLICE_GRAMMER_UTIL_H
#define SLICE_GRAMMER_UTIL_H

#include <Slice/Parser.h>

namespace Slice
{

class StringTok;
class TypeStringTok;
class TypeStringListTok;
class StringListTok;
class BoolTok;
class ExceptionListTok;
class ClassListTok;
class EnumeratorListTok;

typedef ::IceUtil::Handle<StringTok> StringTokPtr;
typedef ::IceUtil::Handle<TypeStringTok> TypeStringTokPtr;
typedef ::IceUtil::Handle<TypeStringListTok> TypeStringListTokPtr;
typedef ::IceUtil::Handle<StringListTok> StringListTokPtr;
typedef ::IceUtil::Handle<BoolTok> BoolTokPtr;
typedef ::IceUtil::Handle<ExceptionListTok> ExceptionListTokPtr;
typedef ::IceUtil::Handle<ClassListTok> ClassListTokPtr;
typedef ::IceUtil::Handle<EnumeratorListTok> EnumeratorListTokPtr;

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
// StringListTok
// ----------------------------------------------------------------------

class SLICE_API StringListTok : public GrammarBase
{
public:

    StringListTok() { }
    StringList v;
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

}

#endif
