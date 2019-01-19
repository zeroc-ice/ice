//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["suppress-warning:deprecated"]] // For classes with operations

module Test
{

struct S
{
    string str;
}

class Base
{
    S theS;
    string str;
}

class AbstractBase extends Base
{
    void op();
}

class B;
class C;

class A
{
    B theB;
    C theC;

    bool preMarshalInvoked;
    bool postUnmarshalInvoked;
}

class B extends A
{
    A theA;
}

class C
{
    B theB;

    bool preMarshalInvoked;
    bool postUnmarshalInvoked;
}

class D
{
    A theA;
    B theB;
    C theC;

    bool preMarshalInvoked;
    bool postUnmarshalInvoked;
}

["protected"] class E
{
    int i;
    string s;
}

class F
{
    ["protected"] E e1;
    E e2;
}

// Exercise empty class with non-empty base
class G extends Base
{
}

interface I
{
}

interface J extends I
{
}

class H implements I
{
}

sequence<Base> BaseSeq;

class CompactExt;

class Compact(1)
{
}

const int CompactExtId = 789;

class CompactExt(CompactExtId) extends Compact
{
}

class A1
{
    string name;
}

class B1
{
    A1 a1;
    A1 a2;
}

class D1 extends B1
{
    A1 a3;
    A1 a4;
}

exception EBase
{
    A1 a1;
    A1 a2;
}

exception EDerived extends EBase
{
    A1 a3;
    A1 a4;
}

class Recursive
{
    Recursive v;
}

class K
{
    Value value;
}

class L
{
    string data;
}

sequence<Value> ValueSeq;
dictionary<string, Value> ValueMap;

struct StructKey
{
    int i;
    string s;
}

dictionary<StructKey, L> LMap;

class M
{
    LMap v;
}

class Initial
{
    void shutdown();
    B getB1();
    B getB2();
    C getC();
    D getD();
    E getE();
    F getF();

    void setRecursive(Recursive p);
    bool supportsClassGraphDepthMax();

    ["marshaled-result"] B getMB();
    ["amd", "marshaled-result"] B getAMDMB();

    void getAll(out B b1, out B b2, out C theC, out D theD);

    I getH();
    I getI();
    I getJ();

    K getK();

    Value opValue(Value v1, out Value v2);
    ValueSeq opValueSeq(ValueSeq v1, out ValueSeq v2);
    ValueMap opValueMap(ValueMap v1, out ValueMap v2);

    D1 getD1(D1 d1);
    void throwEDerived() throws EDerived;

    void setG(G theG);
    void setI(I theI);

    BaseSeq opBaseSeq(BaseSeq inSeq, out BaseSeq outSeq);

    Compact getCompact();

    M opM(M v1, out M v2);
}

}
