// Copyright (c) ZeroC, Inc.

[["cpp:include:list"]]

module Test
{
    sequence<string> StringSeq;
    ["cpp:type:std::list<std::int32_t>"] sequence<int> IntList;
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
        float f;
        double d;
        string str;
        StringSeq ss;
        IntList il;
        StringDict sd;
        S1 s;
        C cls;
        Object* prx;
    }
}
