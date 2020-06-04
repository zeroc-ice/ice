//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Scope
{
    struct S
    {
        int v;
    }

    dictionary<string, S> SMap;
    sequence<S> SSeq;

    class C
    {
        S s;
    }

    dictionary<string, C> CMap;
    sequence<C> CSeq;

    enum E1
    {
        v1,
        v2,
        v3
    }

    struct S1
    {
        string s;
    }

    class C1
    {
        string s;
    }

    struct S2
    {
        E1 E1;
        S1 S1;
        C1 C1;
    }

    class C2
    {
        E1 E1;
        S1 S1;
        C1 C1;
    }

    interface I
    {
        S opS(S s1, out S s2);
        SSeq opSSeq(SSeq s1, out SSeq s2);
        SMap opSMap(SMap s1, out SMap s2);

        C opC(C c1, out C c2);
        CSeq opCSeq(CSeq c1, out CSeq c2);
        CMap opCMap(CMap c1, out CMap c2);

        E1 opE1(E1 E1);
        S1 opS1(S1 S1);
        C1 opC1(C1 C1);

        void shutdown();
    }

    dictionary<string, I*> IMap;
    sequence<I*> ISeq;

    module Inner
    {
        struct S
        {
            int v;
        }

        module Inner2
        {
            struct S
            {
                int v;
            }

            dictionary<string, S> SMap;
            sequence<S> SSeq;

            class C
            {
                S s;
            }

            dictionary<string, C> CMap;
            sequence<C> CSeq;

            interface I
            {
                S opS(S s1, out S s2);
                SSeq opSSeq(SSeq s1, out SSeq s2);
                SMap opSMap(SMap s1, out SMap s2);

                C opC(C c1, out C c2);
                CSeq opCSeq(CSeq c1, out CSeq c2);
                CMap opCMap(CMap c1, out CMap c2);

                void shutdown();
            }

            dictionary<string, I*> IMap;
            sequence<I*> ISeq;
        }

        class C
        {
            S s;
        }

        sequence<Inner2::S> SSeq;
        dictionary<string, Inner2::S> SMap;

        dictionary<string, Inner2::C> CMap;
        sequence<Inner2::C> CSeq;

        interface I
        {
            Inner2::S opS(Inner2::S s1, out Inner2::S s2);
            Inner2::SSeq opSSeq(Inner2::SSeq s1, out Inner2::SSeq s2);
            Inner2::SMap opSMap(Inner2::SMap s1, out Inner2::SMap s2);

            Inner2::C opC(Inner2::C c1, out Inner2::C c2);
            Inner2::CSeq opCSeq(Inner2::CSeq c1, out Inner2::CSeq c2);
            Inner2::CMap opCMap(Inner2::CMap c1, out Inner2::CMap c2);

            void shutdown();
        }

        dictionary<string, I*> IMap;
        sequence<I*> ISeq;
    }
}

module ZeroC::Ice::Test::Scope::Inner::Test::Inner2
{
    interface I
    {
        ZeroC::Ice::Test::Scope::S opS(ZeroC::Ice::Test::Scope::S s1, out ZeroC::Ice::Test::Scope::S s2);
        ZeroC::Ice::Test::Scope::SSeq opSSeq(ZeroC::Ice::Test::Scope::SSeq s1, out ZeroC::Ice::Test::Scope::SSeq s2);
        ZeroC::Ice::Test::Scope::SMap opSMap(ZeroC::Ice::Test::Scope::SMap s1, out ZeroC::Ice::Test::Scope::SMap s2);

        ZeroC::Ice::Test::Scope::C opC(ZeroC::Ice::Test::Scope::C c1, out ZeroC::Ice::Test::Scope::C c2);
        ZeroC::Ice::Test::Scope::CSeq opCSeq(ZeroC::Ice::Test::Scope::CSeq c1, out ZeroC::Ice::Test::Scope::CSeq c2);
        ZeroC::Ice::Test::Scope::CMap opCMap(ZeroC::Ice::Test::Scope::CMap c1, out ZeroC::Ice::Test::Scope::CMap c2);

        void shutdown();
    }
}
