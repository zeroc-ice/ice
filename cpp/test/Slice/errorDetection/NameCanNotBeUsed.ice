//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

interface IFoo
{
    void IFoo();
}

class Bar
{
    string Bar;
    long l;
}

exception EBar
{
    string EBar;
    long l;
}

struct SBar
{
    string SBar;
    long l;
}

interface IBar
{
    void op1(out int returnValue); // ok
    long op2(out int returnValue); // error. Unnamed return values are implicitely named 'returnValue'.
    byte op3(out byte RETURNvalue); // error. Unnamed return values are implicitely named 'returnValue'.
}

}
