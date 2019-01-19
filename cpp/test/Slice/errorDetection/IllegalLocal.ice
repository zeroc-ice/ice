//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

interface i1 {}

local interface i2 extends i1 {}       // error

interface i3 extends i2 {}             // error

local interface i4 extends i2 {}       // OK

class c1 implements i1 {}              // Warning

class c2 implements i2 {}              // error

local exception le {}

interface i
{
    void op() throws le;                // error
}

exception le2 extends le {}            // error

local sequence<long> lls;

class c3
{
    LocalObject lo1;                    // error
    i2*         lo2;                    // error
    i2          lo3;                    // error
    lls         ls;                     // error
}

struct s1
{
    lls ls;                             // error
}

exception e3
{
    lls ls;                             // error
}

sequence<LocalObject> los;              // error

local sequence<byte> bs;
dictionary<bs, long> d1;                // error
dictionary<long, bs> d2;                // error
dictionary<bs, bs> d3;                  // error

local interface i4;
struct s2
{
    i4 m;
}

local enum en { a }
struct s3
{
    en e;
}

interface i5
{
    void op(en p);
    LocalObject op2();
}

exception E
{
}

local interface i6
{
    void op() throws E;                 // error
}

local class c7
{
    void op() throws E;                 // error
}

}
