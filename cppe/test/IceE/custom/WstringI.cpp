// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifdef ICEE_HAS_WSTRING

#include <WstringI.h>

::std::wstring
Test1::WstringClassI::opString(const ::std::wstring& s1,
			       ::std::wstring& s2,
			       const Ice::Current& current)
{
    s2 = s1;
    return s1;
}

::Test1::WstringStruct
Test1::WstringClassI::opStruct(const ::Test1::WstringStruct& s1,
			       ::Test1::WstringStruct& s2,
			       const Ice::Current& current)
{
    s2 = s1;
    return s1;
}

void
Test1::WstringClassI::throwExcept(const ::std::wstring& reason,
				  const Ice::Current& current)
{
    Test1::WstringException ex;
    ex.reason = reason;
    throw ex;
}

::std::wstring
Test2::WstringClassI::opString(const ::std::wstring& s1,
			       ::std::wstring& s2,
			       const Ice::Current& current)
{
    s2 = s1;
    return s1;
}

::Test2::WstringStruct
Test2::WstringClassI::opStruct(const ::Test2::WstringStruct& s1,
			       ::Test2::WstringStruct& s2,
			       const Ice::Current& current)
{
    s2 = s1;
    return s1;
}

void
Test2::WstringClassI::throwExcept(const ::std::wstring& reason,
				  const Ice::Current& current)
{
    Test2::WstringException ex;
    ex.reason = reason;
    throw ex;
}

#endif
