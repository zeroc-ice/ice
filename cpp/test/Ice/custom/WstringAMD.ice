//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[cpp:type:wstring] module Test1
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

[amd] interface WstringClass
{
    (string r1, string r2) opString(string s1);

    (WstringStruct r1, WstringStruct r2) opStruct(WstringStruct s1);

    void throwExcept(string reason)
        throws WstringException;
}

}

module Test2
{

sequence<[cpp:type:wstring] string> WstringSeq;

dictionary<[cpp:type:wstring] string, [cpp:type:wstring] string> WstringWStringDict;

[cpp:type:wstring] struct WstringStruct
{
    string s;
}

[cpp:type:wstring] exception WstringException
{
    string reason;
}

[amd] [cpp:type:wstring] interface WstringClass
{
    (string r1, string r2) opString(string s1);

    (WstringStruct r1, WstringStruct r2) opStruct(WstringStruct s1);

    void throwExcept(string reason)
        throws WstringException;
}

}
