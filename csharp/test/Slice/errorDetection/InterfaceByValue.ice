//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

[[3.7]]

module Test
{
    interface MyObject;

    interface Test
    {
        void op1(MyObject x);
        void op2(int x, out MyObject y);
        MyObject op3(int x);
    }

    struct S
    {
        MyObject m;
    }

    class C
    {
       MyObject m;
    }

    exception E
    {
        MyObject m;
    }

    sequence<MyObject> MyObjectSeq;
    dictionary<int, MyObject> MyObjectDict;
}
