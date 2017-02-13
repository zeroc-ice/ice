// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <WstringAMDI.h>

void
Test1::WstringClassI::opString_async(const ::Test1::AMD_WstringClass_opStringPtr& opStringCB,
                                       const ::std::wstring& s1,
                                       const Ice::Current&)
{
    opStringCB->ice_response(s1, s1);
}

void
Test1::WstringClassI::opStruct_async(const ::Test1::AMD_WstringClass_opStructPtr& opStructCB,
                                       const ::Test1::WstringStruct& s1,
                                       const Ice::Current&)
{
    opStructCB->ice_response(s1, s1);
}

void
Test1::WstringClassI::throwExcept_async(const ::Test1::AMD_WstringClass_throwExceptPtr& throwExceptCB,
                                          const ::std::wstring& reason,
                                          const Ice::Current&)
{
    Test1::WstringException ex;
    ex.reason = reason;
    throwExceptCB->ice_exception(ex);
}

void
Test2::WstringClassI::opString_async(const ::Test2::AMD_WstringClass_opStringPtr& opStringCB,
                                       const ::std::wstring& s1,
                                       const Ice::Current&)
{
    opStringCB->ice_response(s1, s1);
}

void
Test2::WstringClassI::opStruct_async(const ::Test2::AMD_WstringClass_opStructPtr& opStructCB,
                                       const ::Test2::WstringStruct& s1,
                                       const Ice::Current&)
{
    opStructCB->ice_response(s1, s1);
}

void
Test2::WstringClassI::throwExcept_async(const ::Test2::AMD_WstringClass_throwExceptPtr& throwExceptCB,
                                          const ::std::wstring& reason,
                                          const Ice::Current&)
{
    Test2::WstringException ex;
    ex.reason = reason;
    throwExceptCB->ice_exception(ex);
}
