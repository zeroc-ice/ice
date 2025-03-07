// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    struct S
    {
        string str;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class Base
    {
        S theS;
        string str;
    }
<<<<<<< Updated upstream

    class B;
    class C;

=======
        
    class B;
    class C;
        
>>>>>>> Stashed changes
    class A
    {
        B theB;
        C theC;
<<<<<<< Updated upstream

        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }

=======
            
        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }
        
>>>>>>> Stashed changes
    class B extends A
    {
        A theA;
    }
<<<<<<< Updated upstream

    class C
    {
        B theB;

        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }

=======
        
    class C
    {
        B theB;
            
        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }
        
>>>>>>> Stashed changes
    class D
    {
        A theA;
        B theB;
        C theC;
<<<<<<< Updated upstream

        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }

=======
            
        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }
        
>>>>>>> Stashed changes
    // Exercise empty class with non-empty base
    class G extends Base
    {
    }
<<<<<<< Updated upstream

    interface I
    {
    }

    interface J extends I
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

=======
        
    interface I
    {
    }
        
    interface J extends I
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
        
>>>>>>> Stashed changes
    class A1
    {
        string name;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class B1
    {
        A1 a1;
        A1 a2;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class D1 extends B1
    {
        A1 a3;
        A1 a4;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception EBase
    {
        A1 a1;
        A1 a2;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception EDerived extends EBase
    {
        A1 a3;
        A1 a4;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class Recursive
    {
        Recursive v;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class K
    {
        Value value;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class L
    {
        string data;
    }
<<<<<<< Updated upstream

    sequence<Value> ValueSeq;
    dictionary<string, Value> ValueMap;

    // Forward declarations
    class F1;
    interface F2;

=======
        
    sequence<Value> ValueSeq;
    dictionary<string, Value> ValueMap;
        
    // Forward declarations
    class F1;
    interface F2;
        
>>>>>>> Stashed changes
    class F3
    {
        F1 f1;
        F2* f2;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Initial
    {
        void shutdown();
        B getB1();
        B getB2();
        C getC();
        D getD();
<<<<<<< Updated upstream

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

=======
            
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
            
>>>>>>> Stashed changes
        F1 opF1(F1 f11, out F1 f12);
        F2* opF2(F2* f21, out F2* f22);
        F3 opF3(F3 f31, out F3 f32);
        bool hasF3();
    }
<<<<<<< Updated upstream

    class Empty
    {
    }

    class AlsoEmpty
    {
    }

=======
        
    class Empty
    {
    }
        
    class AlsoEmpty
    {
    }
        
>>>>>>> Stashed changes
    interface UnexpectedObjectExceptionTest
    {
        Empty op();
    }
<<<<<<< Updated upstream

    //
    // Remaining definitions are here to ensure that the generated code compiles.
    //

=======
        
    //
    // Remaining definitions are here to ensure that the generated code compiles.
    //
        
>>>>>>> Stashed changes
    class COneMember
    {
        Empty e;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class CTwoMembers
    {
        Empty e1;
        Empty e2;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception EOneMember
    {
        Empty e;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception ETwoMembers
    {
        Empty e1;
        Empty e2;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct SOneMember
    {
        Empty e;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct STwoMembers
    {
        Empty e1;
        Empty e2;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    dictionary<int, COneMember> DOneMember;
    dictionary<int, CTwoMembers> DTwoMembers;
}
