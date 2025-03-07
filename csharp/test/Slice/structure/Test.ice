// Copyright (c) ZeroC, Inc.
    
module Test
{
    sequence<string> StringSeq;
    ["cs:generic:List"]sequence<int> IntList;
    dictionary<string, string> StringDict;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class C
    {
        int i;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    ["cs:class"]
    struct S1
    {
        string name;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
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
