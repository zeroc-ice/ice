// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <Test.h>

class InitialI : public Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);

    virtual MA::CAPrx caop(const Ice::Current&);
    virtual MB::CBPrx cbop(const Ice::Current&);
    virtual MA::CCPrx ccop(const Ice::Current&);
    virtual MA::CDPrx cdop(const Ice::Current&);
    virtual MA::IAPrx iaop(const Ice::Current&);
    virtual MB::IB1Prx ib1op(const Ice::Current&);
    virtual MB::IB2Prx ib2op(const Ice::Current&);
    virtual MA::ICPrx icop(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    MA::CAPrx _ca;
    MB::CBPrx _cb;
    MA::CCPrx _cc;
    MA::CDPrx _cd;
    MA::IAPrx _ia;
    MB::IB1Prx _ib1;
    MB::IB2Prx _ib2;
    MA::ICPrx _ic;
};

class CAI_ : virtual public MA::CA
{
public:

    virtual MA::CAPrx caop(const MA::CAPrx&, const Ice::Current&);
};

class CBI : virtual public MB::CB, virtual public CAI_
{
public:

    virtual MB::CBPrx cbop(const MB::CBPrx&, const Ice::Current&);
};

class CCI : virtual public MA::CC, virtual public CBI
{
public:

    virtual MA::CCPrx ccop(const MA::CCPrx&, const Ice::Current&);
};

class IAI : virtual public MA::IA
{
public:

    virtual MA::IAPrx iaop(const MA::IAPrx&, const Ice::Current&);
};

class IB1I : virtual public MB::IB1, virtual public IAI
{
public:

    virtual MB::IB1Prx ib1op(const MB::IB1Prx&, const Ice::Current&);		   
};

class IB2I : virtual public MB::IB2, virtual public IAI
{
public:

    virtual MB::IB2Prx ib2op(const MB::IB2Prx&, const Ice::Current&);
};

class ICI : virtual public MA::IC, virtual public IB1I, virtual public IB2I
{
public:

    virtual MA::ICPrx icop(const MA::ICPrx&, const Ice::Current&);
};

class CDI : virtual public MA::CD, virtual public CCI, virtual public IB1I, virtual public IB2I
{
public:

    virtual MA::CDPrx cdop(const MA::CDPrx&, const Ice::Current&);
};

#endif
