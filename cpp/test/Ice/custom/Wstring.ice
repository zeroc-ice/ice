// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

["cpp:type:wstring"] module Test1
{

sequence<string> WstringSeq;

dictionary<string, string> WstringWStringDict;

struct WstringStruct
{
    string s;
}

exception WstringException
{
    string reason;
}

interface WstringClass
{
    string opString(string s1, out string s2);

    WstringStruct opStruct(WstringStruct s1, out WstringStruct s2);

    void throwExcept(string reason)
        throws WstringException;
}

}

module Test2
{

sequence<["cpp:type:wstring"] string> WstringSeq;

dictionary<["cpp:type:wstring"] string, ["cpp:type:wstring"] string> WstringWStringDict;

["cpp:type:wstring"] struct WstringStruct
{
    string s;
}

["cpp:type:wstring"] exception WstringException
{
    string reason;
}

["cpp:type:wstring"] interface WstringClass
{
    string opString(string s1, out string s2);

    WstringStruct opStruct(WstringStruct s1, out WstringStruct s2);

    void throwExcept(string reason)
        throws WstringException;
}

}
