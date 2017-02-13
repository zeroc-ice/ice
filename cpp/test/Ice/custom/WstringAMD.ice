// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
};

exception WstringException
{
    string reason;
};

["amd"] class WstringClass
{
    string opString(string s1, out string s2);

    WstringStruct opStruct(WstringStruct s1, out WstringStruct s2);

    void throwExcept(string reason)
        throws WstringException;

    string s;
};

};

module Test2
{

sequence<["cpp:type:wstring"] string> WstringSeq;

dictionary<["cpp:type:wstring"] string, ["cpp:type:wstring"] string> WstringWStringDict;

["cpp:type:wstring"] struct WstringStruct
{
    string s;
};

["cpp:type:wstring"] exception WstringException
{
    string reason;
};

["amd", "cpp:type:wstring"] class WstringClass
{
    string opString(string s1, out string s2);

    WstringStruct opStruct(WstringStruct s1, out WstringStruct s2);

    void throwExcept(string reason)
        throws WstringException;

    string s;
};

};

