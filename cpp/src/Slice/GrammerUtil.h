// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GRAMMER_UTIL_H
#define GRAMMER_UTIL_H

#include <Parser.h>

namespace Slice
{

class StringTok;
class TypeStringListTok;
class StringListTok;
class BoolTok;
class TypeListTok;
class ClassListTok;

}

namespace IceInternal
{

void ICE_API incRef(::Slice::StringTok*);
void ICE_API decRef(::Slice::StringTok*);
void ICE_API incRef(::Slice::TypeStringListTok*);
void ICE_API decRef(::Slice::TypeStringListTok*);
void ICE_API incRef(::Slice::StringListTok*);
void ICE_API decRef(::Slice::StringListTok*);
void ICE_API incRef(::Slice::BoolTok*);
void ICE_API decRef(::Slice::BoolTok*);
void ICE_API incRef(::Slice::TypeListTok*);
void ICE_API decRef(::Slice::TypeListTok*);
void ICE_API incRef(::Slice::ClassListTok*);
void ICE_API decRef(::Slice::ClassListTok*);

}

namespace Slice
{

typedef ::IceInternal::Handle<StringTok> StringTokPtr;
typedef ::IceInternal::Handle<TypeStringListTok> TypeStringListTokPtr;
typedef ::IceInternal::Handle<StringListTok> StringListTokPtr;
typedef ::IceInternal::Handle<BoolTok> BoolTokPtr;
typedef ::IceInternal::Handle<TypeListTok> TypeListTokPtr;
typedef ::IceInternal::Handle<ClassListTok> ClassListTokPtr;

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
