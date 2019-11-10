//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["cs:namespace:Ice.inheritance"]
module Test
{

module MA
{

interface IA
{
    IA* iaop(IA* p);
}

}

module MB
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

module MA
{

interface IC extends MB::IB1, MB::IB2
{
    IC* icop(IC* p);
}

}

interface Initial
{
    void shutdown();
    MA::IA* iaop();
    MB::IB1* ib1op();
    MB::IB2* ib2op();
    MA::IC* icop();
}

}
