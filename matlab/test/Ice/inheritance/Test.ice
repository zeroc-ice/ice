// Copyright (c) ZeroC, Inc.

#pragma once

module Test::MA
{
    interface IA
    {
        IA* iaop(IA* p);
    }
}

module Test::MB
{
    interface IB1 extends MA::IA
    {
        IB1* ib1op(IB1* p);
    }

    interface IB2 extends MA::IA
    {
        IB2* ib2op(IB2* p);
    }
}

module Test::MA
{
    interface IC extends MB::IB1, MB::IB2
    {
        IC* icop(IC* p);
    }
}

module Test
{
    interface Initial
    {
        void shutdown();
        MA::IA* iaop();
        MB::IB1* ib1op();
        MB::IB2* ib2op();
        MA::IC* icop();
    }
}
