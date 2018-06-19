// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
