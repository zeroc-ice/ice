// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <Test.h>

class InitialI : public Test::Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);

    virtual Test::MA::CAPrx caop(const Ice::Current&);
    virtual Test::MB::CBPrx cbop(const Ice::Current&);
    virtual Test::MA::CCPrx ccop(const Ice::Current&);
    virtual Test::MA::CDPrx cdop(const Ice::Current&);
    virtual Test::MA::IAPrx iaop(const Ice::Current&);
    virtual Test::MB::IB1Prx ib1op(const Ice::Current&);
    virtual Test::MB::IB2Prx ib2op(const Ice::Current&);
    virtual Test::MA::ICPrx icop(const Ice::Current&);

private:

    Test::MA::CAPrx _ca;
    Test::MB::CBPrx _cb;
    Test::MA::CCPrx _cc;
    Test::MA::CDPrx _cd;
    Test::MA::IAPrx _ia;
    Test::MB::IB1Prx _ib1;
    Test::MB::IB2Prx _ib2;
    Test::MA::ICPrx _ic;
};

class CAI_ : virtual public Test::MA::CA
{
public:

    virtual Test::MA::CAPrx caop(const Test::MA::CAPrx&, const Ice::Current&);
};

class CBI : virtual public Test::MB::CB, virtual public CAI_
{
public:

    virtual Test::MB::CBPrx cbop(const Test::MB::CBPrx&, const Ice::Current&);
};

class CCI : virtual public Test::MA::CC, virtual public CBI
{
public:

    virtual Test::MA::CCPrx ccop(const Test::MA::CCPrx&, const Ice::Current&);
};

class IAI : virtual public Test::MA::IA
{
public:

    virtual Test::MA::IAPrx iaop(const Test::MA::IAPrx&, const Ice::Current&);
};

class IB1I : virtual public Test::MB::IB1, virtual public IAI
{
public:

    virtual Test::MB::IB1Prx ib1op(const Test::MB::IB1Prx&, const Ice::Current&);                  
};

class IB2I : virtual public Test::MB::IB2, virtual public IAI
{
public:

    virtual Test::MB::IB2Prx ib2op(const Test::MB::IB2Prx&, const Ice::Current&);
};

class ICI : virtual public Test::MA::IC, virtual public IB1I, virtual public IB2I
{
public:

    virtual Test::MA::ICPrx icop(const Test::MA::ICPrx&, const Ice::Current&);
};

class CDI : virtual public Test::MA::CD, virtual public CCI, virtual public IB1I, virtual public IB2I
{
public:

    virtual Test::MA::CDPrx cdop(const Test::MA::CDPrx&, const Ice::Current&);
};

#endif
