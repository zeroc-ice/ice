// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************




[["suppress-warning:deprecated"]] // for classes with operations

module Test
{

class C
{
    int member;
    int member;
}

class base1
{
    void x();
}

class derived1 extends base1
{
    int x;
}

class base2
{
    int x;
}

class derived2 extends base2
{
    void x();
}

class base3
{
    string x;
}

class derived3 extends base3
{
    string x;
}

struct S
{
    int member;
    int member;
}


}
