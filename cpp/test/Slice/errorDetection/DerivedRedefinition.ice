// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

module Test
{

interface Base1
{
    void op();
    void op2();
}

interface Derived1 extends Base1
{
    void op();                          // error
    long op();                          // error
    void foo();
}

interface Base2
{
    void op();
}

interface D1 extends Base1
{
    void foo();                         // OK
}

interface D2 extends D1
{
    void op();                          // error, op() in Base1
}

interface D3 extends D1, Base2
{
    void bar();                         // error, op() in Base1 and Base2
}

class c1 { long l; }
class c2 extends c1 { double l; }      // error
class c3 extends c1 { double d; }      // OK
class c4 extends c3 { short l; }       // error, l in c1

}
