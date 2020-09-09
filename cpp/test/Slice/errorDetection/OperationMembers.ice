//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

interface Foo
{
    void bar(long l1, out long l2, long l3);
    void baz(out long l2, long l3);

    (int r1, int r2) op1(int i, int j);
    int op2(int i, out int j);
    (int r1, int r2) op3(out int i, int j, out int k);
    (out int r1, int r2, out int r3) op4();
}

}
