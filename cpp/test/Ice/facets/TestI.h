// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class AI : virtual public A
{
public:

    virtual std::string callA(const Ice::Current&);
};

class BI : virtual public B, virtual public AI
{
public:

    virtual std::string callB(const Ice::Current&);
};

class CI : virtual public C, virtual public AI
{
public:

    virtual std::string callC(const Ice::Current&);
};

class DI : virtual public D, virtual public BI, virtual public CI
{
public:

    virtual std::string callD(const Ice::Current&);
};

class EI : virtual public E
{
public:

    virtual std::string callE(const Ice::Current&);
};

class FI : virtual public F, virtual public EI
{
public:

    virtual std::string callF(const Ice::Current&);
};

class GI : virtual public G
{
public:

    GI(const Ice::CommunicatorPtr&);
    virtual void shutdown(const Ice::Current&);
    virtual std::string callG(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

class HI : virtual public H, virtual public GI
{
public:

    HI(const Ice::CommunicatorPtr&);
    virtual std::string callH(const Ice::Current&);
};

#endif
