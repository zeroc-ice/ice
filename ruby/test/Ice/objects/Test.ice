// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

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

class AbstractBase extends Base
{
    void op();
};

class B;
class C;

class A
{
    B theB;
    C theC;

    bool preMarshalInvoked;
    bool postUnmarshalInvoked;
};

class B extends A
{
    A theA;
};

class C
{
    B theB;

    bool preMarshalInvoked;
    bool postUnmarshalInvoked;
};

class D
{
    A theA;
    B theB;
    C theC;

    bool preMarshalInvoked;
    bool postUnmarshalInvoked;
};

["protected"] class E
{
    int i;
    string s;

    bool checkValues();
};

class F
{
    ["protected"] E e1;
    E e2;

    bool checkValues();
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

sequence<Base> BaseSeq;

class CompactExt;

class Compact(1)
{
};

const int CompactExtId = 789;

class CompactExt(CompactExtId) extends Compact
{
};

class A1
{
    string name;
};

class B1
{
    A1 a1;
    A1 a2;
};

class D1 extends B1
{
    A1 a3;
    A1 a4;
};

exception EBase
{
    A1 a1;
    A1 a2;
};

exception EDerived extends EBase
{
    A1 a3;
    A1 a4;
};

class Initial
{
    void shutdown();
    B getB1();
    B getB2();
    C getC();
    D getD();
    E getE();
    F getF();

    ["marshaled-result"] B getMB();
    ["amd", "marshaled-result"] B getAMDMB();

    void getAll(out B b1, out B b2, out C theC, out D theD);

    I getI();
    I getJ();
    I getH();

    D1 getD1(D1 d1);
    void throwEDerived() throws EDerived;

    void setI(I theI);

    BaseSeq opBaseSeq(BaseSeq inSeq, out BaseSeq outSeq);

    Compact getCompact();
};

};
