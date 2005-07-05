// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    InitialI(const IceE::ObjectAdapterPtr&);

    virtual void shutdown(const IceE::Current&);

    virtual Test::MA::CAPrx caop(const IceE::Current&);
    virtual Test::MB::CBPrx cbop(const IceE::Current&);
    virtual Test::MA::CCPrx ccop(const IceE::Current&);
    virtual Test::MA::CDPrx cdop(const IceE::Current&);
    virtual Test::MA::IAPrx iaop(const IceE::Current&);
    virtual Test::MB::IB1Prx ib1op(const IceE::Current&);
    virtual Test::MB::IB2Prx ib2op(const IceE::Current&);
    virtual Test::MA::ICPrx icop(const IceE::Current&);

private:

    IceE::ObjectAdapterPtr _adapter;
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

    virtual Test::MA::CAPrx caop(const Test::MA::CAPrx&, const IceE::Current&);
};

class CBI : virtual public Test::MB::CB, virtual public CAI_
{
public:

    virtual Test::MB::CBPrx cbop(const Test::MB::CBPrx&, const IceE::Current&);
};

class CCI : virtual public Test::MA::CC, virtual public CBI
{
public:

    virtual Test::MA::CCPrx ccop(const Test::MA::CCPrx&, const IceE::Current&);
};

class IAI : virtual public Test::MA::IA
{
public:

    virtual Test::MA::IAPrx iaop(const Test::MA::IAPrx&, const IceE::Current&);
};

class IB1I : virtual public Test::MB::IB1, virtual public IAI
{
public:

    virtual Test::MB::IB1Prx ib1op(const Test::MB::IB1Prx&, const IceE::Current&);		   
};

class IB2I : virtual public Test::MB::IB2, virtual public IAI
{
public:

    virtual Test::MB::IB2Prx ib2op(const Test::MB::IB2Prx&, const IceE::Current&);
};

class ICI : virtual public Test::MA::IC, virtual public IB1I, virtual public IB2I
{
public:

    virtual Test::MA::ICPrx icop(const Test::MA::ICPrx&, const IceE::Current&);
};

class CDI : virtual public Test::MA::CD, virtual public CCI, virtual public IB1I, virtual public IB2I
{
public:

    virtual Test::MA::CDPrx cdop(const Test::MA::CDPrx&, const IceE::Current&);
};

#endif
