//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning(reserved-identifier)]]

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
        (S r1, S r2) opS(S s1);
        (SSeq r1, SSeq r2) opSSeq(SSeq s1);
        (SMap r1, SMap r2) opSMap(SMap s1);

        (C r1, C r2) opC(C c1);
        (CSeq r1, CSeq r2) opCSeq(CSeq c1);
        (CMap r1, CMap r2) opCMap(CMap c1);

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
                (S r1, S r2) opS(S s1);
                (SSeq r1, SSeq r2) opSSeq(SSeq s1);
                (SMap r1, SMap r2) opSMap(SMap s1);

                (C r1, C r2) opC(C c1);
                (CSeq r1, CSeq r2) opCSeq(CSeq c1);
                (CMap r1, CMap r2) opCMap(CMap c1);

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
            (Inner2::S r1, Inner2::S r2) opS(Inner2::S s1);
            (Inner2::SSeq r1, Inner2::SSeq r2) opSSeq(Inner2::SSeq s1);
            (Inner2::SMap r1, Inner2::SMap r2) opSMap(Inner2::SMap s1);

            (Inner2::C r1, Inner2::C r2) opC(Inner2::C c1);
            (Inner2::CSeq r1, Inner2::CSeq r2) opCSeq(Inner2::CSeq c1);
            (Inner2::CMap r1, Inner2::CMap r2) opCMap(Inner2::CMap c1);

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
        (ZeroC::Ice::Test::Scope::S r1, ZeroC::Ice::Test::Scope::S r2) opS(ZeroC::Ice::Test::Scope::S s1);
        (ZeroC::Ice::Test::Scope::SSeq r1, ZeroC::Ice::Test::Scope::SSeq r2) opSSeq(ZeroC::Ice::Test::Scope::SSeq s1);
        (ZeroC::Ice::Test::Scope::SMap r1, ZeroC::Ice::Test::Scope::SMap r2) opSMap(ZeroC::Ice::Test::Scope::SMap s1);

        (ZeroC::Ice::Test::Scope::C r1, ZeroC::Ice::Test::Scope::C r2) opC(ZeroC::Ice::Test::Scope::C c1);
        (ZeroC::Ice::Test::Scope::CSeq r1, ZeroC::Ice::Test::Scope::CSeq r2) opCSeq(ZeroC::Ice::Test::Scope::CSeq c1);
        (ZeroC::Ice::Test::Scope::CMap r1, ZeroC::Ice::Test::Scope::CMap r2) opCMap(ZeroC::Ice::Test::Scope::CMap c1);

        void shutdown();
    }
}
