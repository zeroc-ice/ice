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
    ["cs:internal"] sequence<C> CSeq;

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

    ["cs:readonly", "cs:internal"]
    struct S3
    {
        int x;
        C cls; // Mapped to a read-write field.
    }

    ["cs:readonly", "cs:property", "cs:internal"]
    struct S4
    {
        int x;
        C cls; // Mapped to a get-set property.
    }

    ["cs:readonly", "cs:internal"]
    struct S5
    {
        int x;
        S3 s3; // Mapped to a read-only field.
        CSeq seq; // Mapped to a read-only field.
    }

    ["cs:readonly", "cs:property", "cs:internal"]
    struct S6
    {
        int x;
        S4 s4; // Mapped to a get-only property.
        CSeq seq; // Mapped to a get-only property.
    }
}
