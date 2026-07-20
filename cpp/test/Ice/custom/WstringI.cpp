// Copyright (c) ZeroC, Inc.

#include "WstringI.h"

std::wstring
Test1::WstringInterfaceI::opString(std::wstring s1, std::wstring& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

::Test1::WstringStruct
Test1::WstringInterfaceI::opStruct(::Test1::WstringStruct s1, ::Test1::WstringStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

void
Test1::WstringInterfaceI::throwExcept(std::wstring reason, const Ice::Current&)
{
    throw Test1::WstringException{reason};
}

std::wstring
Test2::WstringInterfaceI::opString(std::wstring s1, std::wstring& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

::Test2::WstringStruct
Test2::WstringInterfaceI::opStruct(::Test2::WstringStruct s1, ::Test2::WstringStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

void
Test2::WstringInterfaceI::throwExcept(std::wstring reason, const Ice::Current&)
{
    throw Test2::WstringException{reason};
}
