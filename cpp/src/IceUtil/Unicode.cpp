// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Unicode.h>

using namespace std;

string
IceUtil::wstringToString(const wstring& str)
{
    char* s = new char[str.size() + 1];
    wcstombs(s, str.c_str(), str.size() + 1);
    string result(s);
    delete [] s;
    return result;
}

wstring
IceUtil::stringToWstring(const string& str)
{
    wchar_t* s = new wchar_t[str.size() + 1];
    mbstowcs(s, str.c_str(), str.size() + 1);
    wstring result(s);
    delete [] s;
    return result;
}
