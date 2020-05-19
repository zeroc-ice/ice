//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[swift:class-resolver-prefix:IceInheritance]]

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

interface IB1 : MA::IA
{
    IB1* ib1op(IB1* p);
}

interface IB2 : MA::IA
{
    IB2* ib2op(IB2* p);
}

}

module MA
{

interface IC : MB::IB1, MB::IB2
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

module MC
{

class A
{
    int aA;
}

class B : A
{
    int bB;
}

class C : B
{
    int cC;
}

 class D : C
{
    int dD;
}

}

module MD
{

 class A
{
    int aA;
}

 class B : A
{
    int bB;
}

 class C : B
{
    int cC;
}

 class D : C
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

class B : A
{
    int bB;
}

 class C : B
{
    int cC;
}

 class D : C
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

 class B : A
{
    int bB;
}

class C : B
{
    int cC;
}

 class D : C
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

 class B : A
{
    int bB;
}

 class C : B
{
    int cC;
}

class D : C
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

class B : A
{
    int bB;
}

class C : B
{
    int cC;
}

class D : C
{
    int dD;
}

}

}
