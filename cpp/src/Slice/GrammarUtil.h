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
class TypeStringListTok;
class StringListTok;
class BoolTok;
class TypeListTok;
class ClassListTok;
class EnumeratorListTok;

typedef ::IceUtil::Handle<StringTok> StringTokPtr;
typedef ::IceUtil::Handle<TypeStringListTok> TypeStringListTokPtr;
typedef ::IceUtil::Handle<StringListTok> StringListTokPtr;
typedef ::IceUtil::Handle<BoolTok> BoolTokPtr;
typedef ::IceUtil::Handle<TypeListTok> TypeListTokPtr;
typedef ::IceUtil::Handle<ClassListTok> ClassListTokPtr;
typedef ::IceUtil::Handle<EnumeratorListTok> EnumeratorListTokPtr;

}

namespace Slice
{

// ----------------------------------------------------------------------
// StringTok
// ----------------------------------------------------------------------

class SLICE_API StringTok : public GrammerBase
{
public:

    StringTok() { }
    std::string v;
};

// ----------------------------------------------------------------------
// TypeStringListTok
// ----------------------------------------------------------------------

class SLICE_API TypeStringListTok : public GrammerBase
{
public:

    TypeStringListTok() { }
    TypeStringList v;
};

// ----------------------------------------------------------------------
// StringListTok
// ----------------------------------------------------------------------

class SLICE_API StringListTok : public GrammerBase
{
public:

    StringListTok() { }
    StringList v;
};

// ----------------------------------------------------------------------
// BoolTok
// ----------------------------------------------------------------------

class SLICE_API BoolTok : public GrammerBase
{
public:

    BoolTok() { }
    bool v;
};

// ----------------------------------------------------------------------
// TypeListTok
// ----------------------------------------------------------------------

class SLICE_API TypeListTok : public GrammerBase
{
public:

    TypeListTok() { }
    TypeList v;
};

// ----------------------------------------------------------------------
// ClassListTok
// ----------------------------------------------------------------------

class SLICE_API ClassListTok : public GrammerBase
{
public:

    ClassListTok() { }
    ClassList v;
};

// ----------------------------------------------------------------------
// EnumeratorListTok
// ----------------------------------------------------------------------

class SLICE_API EnumeratorListTok : public GrammerBase
{
public:

    EnumeratorListTok() { }
    EnumeratorList v;
};

}

#endif
