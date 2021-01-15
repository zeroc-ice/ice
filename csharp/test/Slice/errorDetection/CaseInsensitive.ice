//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

interface i1
{
    void op();
    void op();
}

interface i2
{
    void op();
    void oP();
}

class c1
{
    long l;
    long l;
}

class c2
{
    long l;
    long L;
}

module m1
{}
module m1
{}
module M1
{}
module c1
{}
module C1
{}

interface i3
{
    void op(long aa, int aa);
    void op2(long bb, out int BB);
}

interface i4
{
    void I4();
}

interface i5
{
    void i5();
}

interface i6
{
    void op();
}

interface i7 : i6
{
    void op();
}

interface i8 : i6
{
    void OP();
}

class c3
{
    long l;
}

class c4 : c3
{
    long l;
}

class c5 : c3
{
    long L;
}

class c6
{
    long l;
    int L;
}

class c7 : c3
{
    long l;
}

class c8 : c3
{
    long L;
}

class c9
{
     int L;
     long l;
}

exception e1
{
    long l;
    string l;
}

exception e2
{
    long l;
    string L;
}

exception e3
{
    long e3;
}

exception e4
{
    long E4;
}

exception e5
{
    long l;
}

exception e6 : e5
{
    string l;
}

exception e7 : e5
{
    string L;
}

struct s1
{
    long l;
    string l;
}

struct s2
{
    long l;
    string L;
}

struct s3
{
    long s3;
    string x;
}

struct s4
{
    long S4;
    string x;
}

sequence<long> ls;
sequence<long> LS;
sequence<long> m1;
sequence<long> M1;

dictionary<long, string> d;
dictionary<long, string> D;
dictionary<long, string> m1;
dictionary<long, string> M1;

enum en1 { red }
enum eN1 { lilac }
enum m1 { green }
enum M1 { blue }
enum en2 { yellow, en1, EN1}

module xxx::xx
{

interface Base
{
    void op();
}

interface Derived : Base
{
}

interface Derived : base
{
}

exception e1
{
}

exception e2 : E1
{
}

sequence<long> s1;
struct s
{
    S1 x;
    xxx::xx::S1 y;
    xxx::XX::s1 z;
    xxx::XX::s1 w;
}

struct s2
{
    Derived* blah;
    derived* bletch;
}

}

interface Foo
{
    void op(long param, string Param);
    void op2() throws e1;
    void op3() throws E1;
    void op4() throws Test::xxx::xx::e1;
    void op5() throws Test::xxx::xx::E1;
    void op6() throws Test::xxx::XX::e1;
    void op7() throws Test::XXX::xx::e1;
    void op8() throws ::Test::xxx::xx::e1;
    void op9() throws ::Test::xxx::xx::E1;
    void op10() throws ::Test::xxx::XX::e1;
    void op11() throws ::Test::XXX::xx::e1;
    void op12(long op12);
    void op13(long OP13);
}

module CI
{
    interface base1
    {
        void op();
    }

    interface base2
    {
        void OP();
    }

    interface derived : base1, base2
    {
    }
}

module DI
{
    interface base
    {
        void aa(int AA);
        void xx(int base);
    }

    class base1
    {
        int base1;
        int BASE1;
    }

    struct Foo
    {
        int foo;
    }

    struct Foo1
    {
        int Foo1;
    }

    exception Bar
    {
        string bar;
    }

    exception Bar1
    {
        string Bar1;
    }

    class base2
    {
        int X;
    }

    class derived2 : base2
    {
        string x;
    }

}

}
