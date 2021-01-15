//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

class C
{
    int member;
    int member;
}

interface base1
{
    void x();
}

interface derived1 : base1
{
    string x(int y);
}

class base2
{
    int x;
}

class derived2 : base2
{
    string x;
}

struct S
{
    int member;
    int member;
}

}
