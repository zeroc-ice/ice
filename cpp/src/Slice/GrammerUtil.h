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

typedef ::Ice::Handle<StringTok> StringTokPtr;
typedef ::Ice::Handle<TypeStringListTok> TypeStringListTokPtr;
typedef ::Ice::Handle<StringListTok> StringListTokPtr;
typedef ::Ice::Handle<BoolTok> BoolTokPtr;
typedef ::Ice::Handle<TypeListTok> TypeListTokPtr;
typedef ::Ice::Handle<ClassListTok> ClassListTokPtr;

}

namespace Slice
{

// ----------------------------------------------------------------------
// StringTok
// ----------------------------------------------------------------------

class ICE_API StringTok : public GrammerBase
{
public:

    StringTok() { }
    std::string v;
};

// ----------------------------------------------------------------------
// TypeStringListTok
// ----------------------------------------------------------------------

class ICE_API TypeStringListTok : public GrammerBase
{
public:

    TypeStringListTok() { }
    TypeStringList v;
};

// ----------------------------------------------------------------------
// StringListTok
// ----------------------------------------------------------------------

class ICE_API StringListTok : public GrammerBase
{
public:

    StringListTok() { }
    StringList v;
};

// ----------------------------------------------------------------------
// BoolTok
// ----------------------------------------------------------------------

class ICE_API BoolTok : public GrammerBase
{
public:

    BoolTok() { }
    bool v;
};

// ----------------------------------------------------------------------
// TypeListTok
// ----------------------------------------------------------------------

class ICE_API TypeListTok : public GrammerBase
{
public:

    TypeListTok() { }
    TypeList v;
};

// ----------------------------------------------------------------------
// ClassListTok
// ----------------------------------------------------------------------

class ICE_API ClassListTok : public GrammerBase
{
public:

    ClassListTok() { }
    ClassList v;
};

}

#endif
