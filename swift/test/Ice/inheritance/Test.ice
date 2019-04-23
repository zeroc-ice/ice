//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["swift:class-resolver-prefix:IceInheritance",
  "suppress-warning:deprecated"]] // For classes with operations

module Test
{

module MA
{

interface IA
{
    IA* iaop(IA* p);
}

class CA
{
    CA* caop(CA* p);
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

["cpp:virtual"]class CB extends MA::CA
{
    CB* cbop(CB* p);
}

}

module MA
{

interface IC extends MB::IB1, MB::IB2
{
    IC* icop(IC* p);
}

["cpp:virtual"]class CC extends MB::CB
{
    CC* ccop(CC* p);
}

["cpp:virtual"]class CD extends CC implements MB::IB1, MB::IB2
{
    CD* cdop(CD* p);
}

}

interface Initial
{
    void shutdown();
    MA::CA* caop();
    MB::CB* cbop();
    MA::CC* ccop();
    MA::CD* cdop();
    MA::IA* iaop();
    MB::IB1* ib1op();
    MB::IB2* ib2op();
    MA::IC* icop();
}

module MC
{

class A
{
    int aA;
}

class B extends A
{
    int bB;
}

class C extends B
{
    int cC;
}

["cpp:virtual"] class D extends C
{
    int dD;
}

}

module MD
{

["cpp:virtual"] class A
{
    int aA;
}

["cpp:virtual"] class B extends A
{
    int bB;
}

["cpp:virtual"] class C extends B
{
    int cC;
}

["cpp:virtual"] class D extends C
{
    int dD;
}

}

module ME
{

class A
{
    int aA;
}

class B extends A
{
    int bB;
}

["cpp:virtual"] class C extends B
{
    int cC;
}

["cpp:virtual"] class D extends C
{
    int dD;
}

}

module MF
{

class A
{
    int aA;
}

["cpp:virtual"] class B extends A
{
    int bB;
}

class C extends B
{
    int cC;
}

["cpp:virtual"] class D extends C
{
    int dD;
}

}

module MG
{

class A
{
    int aA;
}

["cpp:virtual"] class B extends A
{
    int bB;
}

["cpp:virtual"] class C extends B
{
    int cC;
}

class D extends C
{
    int dD;
}

}

module MH
{

class A
{
    int aA;
}

class B extends A
{
    int bB;
}

class C extends B
{
    int cC;
}

class D extends C
{
    int dD;
}

}

}
