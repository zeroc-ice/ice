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
#include <map>

namespace Slice
{

// TODO: ML: Consider moving to Scanner.l, as it is only used there.
//
// Function object to do case-insensitive string comparison.
//
struct CICompare : public std::binary_function<std::string, std::string, bool>
{
    // TODO: ML: Make non-inline. (Not necessary if moved to Scanner.l)
    bool operator()(const std::string& s1, const std::string& s2) const
    {
	std::string::const_iterator p1 = s1.begin();
	std::string::const_iterator p2 = s2.begin();
	while(p1 != s1.end() && p2 != s2.end() && tolower(*p1) == tolower(*p2))
	{
	    ++p1;
	    ++p2;
	}
	if(p1 == s1.end() && p2 == s2.end())
	{
	    return false;
	}
	else if(p1 == s1.end())
	{
	    return true;
	}
	else if(p2 == s2.end())
	{
	    return false;
	}
	else
	{
	    return tolower(*p1) < tolower(*p2);
	}
    }
};

//
// Definitions for the case-insensitive keyword-token map.
//
// TODO: ML: Naming conventions for types. (Should be StringTokenMap.)
// TODO: ML: Consider moving to Scanner.l, as it is only used there. Then keywordMap can also be static.
typedef std::map<std::string, int, CICompare> stringTokenMap;
extern stringTokenMap keywordMap;

//
// initialize() fills the keyword map with all keyword-token pairs.
//
// TODO: ML: Perhaps initializeKeywordMap() would be a better name?
void initialize();

class StringTok;
class StringListTok;
class TypeStringTok;
class TypeStringListTok;
class BoolTok;
class ExceptionListTok;
class ClassListTok;
class EnumeratorListTok;

typedef ::IceUtil::Handle<StringTok> StringTokPtr;
typedef ::IceUtil::Handle<StringListTok> StringListTokPtr;
typedef ::IceUtil::Handle<TypeStringTok> TypeStringTokPtr;
typedef ::IceUtil::Handle<TypeStringListTok> TypeStringListTokPtr;
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
