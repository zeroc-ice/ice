//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning(reserved-identifier)]]

#include <Ice/Identity.ice>

module ZeroC::Ice::Test::Objects
{

class IdentityWrapper
{
    Ice::Identity myId;
    string s = "";
    int n;
}

class DerivedIdentityWrapper : IdentityWrapper
{
    string s2;
    int n2;
}

struct S
{
    string str;
}

class Base
{
    S theS;
    string str;
}

class B;
class C;

class A
{
    B theB;
    C theC;
}

class B : A
{
    A theA;
}

class C
{
    B theB;
}

class D
{
    A theA;
    B theB;
    C theC;
}

// Exercise empty class with non-empty base
class G : Base
{
}

sequence<Base> BaseSeq;

class CompactExt;

class Compact(1)
{
}

const int CompactExtId = 789;

class CompactExt(CompactExtId) : Compact
{
}

module Inner
{

class A
{
    ZeroC::Ice::Test::Objects::A theA;
}

exception Ex
{
    string reason;
}

module Sub
{

class A
{
    ZeroC::Ice::Test::Objects::Inner::A theA;
}

exception Ex
{
    string reason;
}

}

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

class D1 : B1
{
    A1 a3;
    A1 a4;
}

exception EBase
{
    A1 a1;
    A1 a2;
}

exception EDerived : EBase
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
    AnyClass? value;
}

class L
{
    string data;
}

sequence<AnyClass?> ClassSeq;
dictionary<string, AnyClass?> ClassMap;

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

// Forward declarations
class F1;
interface F2;

class F3
{
    F1 f1;
    F2* f2;
}

interface Initial
{
    void shutdown();
    B getB1();
    B getB2();
    C getC();
    D getD();

    void setRecursive(Recursive p);
    bool supportsClassGraphMaxDepth();

    [marshaled-result] B getMB();
    [amd] [marshaled-result] B getAMDMB();

    (B r1, B r2, C r3, D r4) getAll();

    K getK();

    (AnyClass? r1, AnyClass? r2) opClass(AnyClass? v1);
    (ClassSeq r1, ClassSeq r2) opClassSeq(ClassSeq v1);
    (ClassMap r1, ClassMap r2) opClassMap(ClassMap v1);

    D1 getD1(D1 d1);
    void throwEDerived();

    void setG(G theG);

    (BaseSeq r1, BaseSeq r2) opBaseSeq(BaseSeq inSeq);

    Compact getCompact();

    Inner::A getInnerA();
    Inner::Sub::A getInnerSubA();

    void throwInnerEx();
    void throwInnerSubEx();

    (M r1, M r2) opM(M v1);

    (F1 r1, F1 r2) opF1(F1 f11);
    (F2* r1, F2* r2) opF2(F2* f21);
    (F3 r1, F3 r2) opF3(F3 f31);
    bool hasF3();
}

class Empty
{
}

class AlsoEmpty
{
}

interface UnexpectedObjectExceptionTest
{
    Empty op();
}

class IBase
{
    string id = "";
}

class IDerived : IBase
{
    string name = "";
}

class IDerived2 : IBase
{
}

class I2
{
}

struct S1
{
    int id;
}

//
// Remaining definitions are here to ensure that the generated code compiles.
//

class COneMember
{
    Empty e;
}

class CTwoMembers
{
    Empty e1;
    Empty e2;
}

exception EOneMember
{
    Empty e;
}

exception ETwoMembers
{
    Empty e1;
    Empty e2;
}

struct SOneMember
{
    Empty e;
}

struct STwoMembers
{
    Empty e1;
    Empty e2;
}

dictionary<int, COneMember> DOneMember;
dictionary<int, CTwoMembers> DTwoMembers;

}
