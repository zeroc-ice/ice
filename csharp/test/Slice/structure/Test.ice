// Copyright (c) ZeroC, Inc.

module Test
{
    ["cs:internal"] sequence<string> StringSeq;
    ["cs:generic:List", "cs:internal"] sequence<int> IntList;
    ["cs:internal"] dictionary<string, string> StringDict;

    ["cs:internal"]
    class C
    {
        int i;
    }

    ["cs:internal", "cs:readonly"]
    struct S1
    {
        string name;
    }

    ["cs:internal"]
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
