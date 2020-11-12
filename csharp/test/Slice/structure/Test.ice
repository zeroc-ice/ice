//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module ZeroC::Slice::Test::Structure
{
    sequence<string> StringSeq;
    sequence<int> IntList;
    dictionary<string, string> StringDict;

    class C
    {
        int i;
    }

    struct S1
    {
        string name;
    }

    struct S2
    {
        bool bo;
        byte by;
        short sh;
        int i;
        long l;
        float? f;
        double d;
        string str;
        StringSeq ss;
        IntList il;
        StringDict sd;
        S1 s;
        C cls;
        Object? prx;
    }

    [cs:custom-equals] struct S3
    {
        string s;
        IntList intList;
    }
}
