// Copyright (c) ZeroC, Inc.

#pragma once

// The type of a C++ struct field must be complete.
[["cpp:include:Forward.h"]]

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

    exception BaseEx
    {
        string reason;
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

    sequence<Base> BaseSeq;

    class CompactExt;

    enum CompactIdEnum { First = 1, Second = 2 }

    class Compact(First)
    {
    }

    class CompactScoped(CompactIdEnum::Second)
    {
    }

    const int CompactExtId = 789;

    class CompactExt(CompactExtId) extends Compact
    {
    }

    module Inner
    {
        class A
        {
            ::Test::A theA;
        }

        exception Ex
        {
            string reason;
        }

        module Sub
        {
            class A
            {
                ::Test::Inner::A theA;
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

        void setCycle(Recursive r);
        bool acceptsClassCycles();

        ["marshaled-result"] B getMB();
        ["amd"] ["marshaled-result"] B getAMDMB();

        void getAll(out B b1, out B b2, out C theC, out D theD);

        K getK();

        Value opValue(Value v1, out Value v2);
        ValueSeq opValueSeq(ValueSeq v1, out ValueSeq v2);
        ValueMap opValueMap(ValueMap v1, out ValueMap v2);

        D1 getD1(D1 d1);
        void throwEDerived() throws EDerived;

        void setG(G theG);

        BaseSeq opBaseSeq(BaseSeq inSeq, out BaseSeq outSeq);

        Compact getCompact();

        Inner::A getInnerA();
        Inner::Sub::A getInnerSubA();

        void throwInnerEx() throws Inner::Ex;
        void throwInnerSubEx() throws Inner::Sub::Ex;

        M opM(M v1, out M v2);

        F1 opF1(F1 f11, out F1 f12);
        F2* opF2(F2* f21, out F2* f22);
        bool hasF3();
        F3 opF3(F3 f31, out F3 f32);
    }

    interface TestIntf
    {
        Base opDerived();
        void throwDerived() throws BaseEx;
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
