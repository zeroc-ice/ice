// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

struct S
{
    string str;
};

class Base
{
    S theS;
    string str;
};

class B;
class C;

class A
{
    B theB;
    C theC;

    bool preMarshalInvoked;
    bool postUnmarshalInvoked();
};

class B extends A
{
    A theA;
};

class C
{
    B theB;

    bool preMarshalInvoked;
    bool postUnmarshalInvoked();
};

class D
{
    A theA;
    B theB;
    C theC;    

    bool preMarshalInvoked;
    bool postUnmarshalInvoked();
};

interface I
{
};

interface J extends I
{
};

class H implements I
{
};

class Initial
{
    void shutdown();
    B getB1();
    B getB2();
    C getC();
    D getD();

    void getAll(out B b1, out B b2, out C theC, out D theD);

    I getI();
    I getJ();
    I getH();

    void setI(I theI);
};

class Empty
{
};

class AlsoEmpty
{
};

interface UnexpectedObjectExceptionTest
{
    Empty op();
};

//
// Remaining definitions are here to ensure that the generated code compiles.
//

class COneMember
{
    Empty e;
};

class CTwoMembers
{
    Empty e1;
    Empty e2;
};

exception EOneMember
{
    Empty e;
};

exception ETwoMembers
{
    Empty e1;
    Empty e2;
};

struct SOneMember
{
    Empty e;
};

struct STwoMembers
{
    Empty e1;
    Empty e2;
};

dictionary<int, COneMember> DOneMember;
dictionary<int, CTwoMembers> DTwoMembers;

};

#endif
