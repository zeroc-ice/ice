// Copyright (c) ZeroC, Inc.

#pragma once

["cpp:type:wstring"] module Test1
{
    sequence<string> WstringSeq;
<<<<<<< Updated upstream

    dictionary<string, string> WstringWStringDict;

=======

    dictionary<string, string> WstringWStringDict;

>>>>>>> Stashed changes
    struct WstringStruct
    {
        string s;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception WstringException
    {
        string reason;
    }
<<<<<<< Updated upstream

    ["amd"] interface WstringClass
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

=======

    ["amd"] interface WstringClass
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

>>>>>>> Stashed changes
    ["cpp:type:wstring"] struct WstringStruct
    {
        string s;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    ["cpp:type:wstring"] exception WstringException
    {
        string reason;
    }
<<<<<<< Updated upstream

    ["amd"] ["cpp:type:wstring"] interface WstringClass
    {
        string opString(string s1, out string s2);

        WstringStruct opStruct(WstringStruct s1, out WstringStruct s2);

        void throwExcept(string reason)
        throws WstringException;
=======

    ["amd"] ["cpp:type:wstring"] interface WstringClass
    {
        string opString(string s1, out string s2);

        WstringStruct opStruct(WstringStruct s1, out WstringStruct s2);

        void throwExcept(string reason)
            throws WstringException;
>>>>>>> Stashed changes
    }
}
