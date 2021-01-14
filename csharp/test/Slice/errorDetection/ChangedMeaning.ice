//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

sequence<long> ls;

struct s00
{
    ls ls;      // OK as of Ice 3.6 (data member has its own scope)
    ls l;
}

struct s0
{
    Test::ls ls;        // OK
}

struct s1
{
    ls mem;
    long ls;    // OK as of Ice 3.6 (data member has its own scope)
}

struct s2
{
    Test::ls mem;
    long ls;    // OK
}

module M
{

sequence<long> ls;

}

interface i1
{
    M::ls op();
    void M();   // Changed meaning
}

interface i2
{
    M::ls op();
    long M();   // Changed meaning
}

module N
{

    interface n1 : i1 {}
    interface i1 {}            // Changed meaning
    interface i2 : i2 {} // Changed meaning

}

module O
{

    interface n1 : ::Test::i1 {}
    interface i1 {}                    // OK
    interface i2 : ::Test::i2 {} // OK

}

exception e1 {}

exception e2 {}

module E
{
    exception ee1 : e1 {}
    exception e1 {}            // Changed meaning
    exception e2 : e2 {} // Changed meaning
}

interface c1 {}

class c2 {}

module C
{
    class c1 {}                // Changed meaning
    class c2 : c2 {}     // Changed meaning
}

enum color { blue }

module B
{
    const color fc = blue;
    interface blue {}          // OK as of Ice 3.7 (enumerators are in their enum's namespace)
}

enum counter { one, two }
sequence<counter> CounterSeq;

module SS
{
    sequence<CounterSeq> y;
    enum CounterSeq { a, b }
}

interface ParamTest
{
    void op(long param);
    void op2(counter param);
    void param(counter counter);        // OK as of Ice 3.6 (parameters have their own scope)
    void op3(long counter, counter x);  // OK as of Ice 3.6.1 (second "counter" is not a type)
    void op4(long param, long param);
}

sequence<int> IS;
struct x
{
    IS is;                              // OK as of Ice 3.6 (data member has its own scope)
}

struct y
{
    ::Test::IS is;                      // OK, nothing introduced
}

interface Blah
{
    void op1() throws ::Test::E::ee1;   // Nothing introduced
    void E();                           // OK
    void op2() throws E;                // Changed meaning
}

interface Blah2
{
    void op3() throws ::Test::E::ee1;   // Nothing introduced
    void E();                           // OK
    void op4() throws E::ee1;           // Changed meaning
}

interface Blah3
{
    void op5() throws E::ee1;           // Introduces E
    void E();                           // Changed meaning
}

module M1
{
    enum smnpEnum { a }

    struct smnpStruct
    {
        smnpEnum e;
    }

    exception smnpException
    {
    }

    module M2
    {
        enum C { C1, C2, C3 }
    }
}

const Test::M1::M2::C MyConstant1 = Test::M1::M2::C2; // OK
const ::Test::M1::M2::C MyConstant2 = Test::M1::M2::C2; // OK
const Test::M1::M2::C MyConstant3 = ::Test::M1::M2::C2; // OK
const ::Test::M1::M2::C MyConstant4 = ::Test::M1::M2::C2; // OK

interface smnpTest1Class
{
    M1::smnpStruct smnpTest1Op1() throws M1::smnpException; // OK
}

class Foo
{
    string x;
    string X;
}

struct Foo1
{
    string x;
    string X;
}

interface Foo2
{
    void op1(int a, int A); // Changed meaning
    void op2(int Foo2); // OK
    void op3(int op3); // Ok
}

exception Foo5
{
    string x;
    int X; // Changed meaning
}

}
