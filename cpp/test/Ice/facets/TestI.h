// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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

    virtual std::string callA();
};

class BI : virtual public B, virtual public AI
{
public:

    virtual std::string callB();
};

class CI : virtual public C, virtual public AI
{
public:

    virtual std::string callC();
};

class DI : virtual public D, virtual public BI, virtual public CI
{
public:

    virtual std::string callD();
};

class EI : virtual public E
{
public:

    virtual std::string callE();
};

class FI : virtual public F, virtual public EI
{
public:

    virtual std::string callF();
};

class GI : virtual public G
{
public:

    GI(const Ice::CommunicatorPtr&);
    virtual void shutdown();
    virtual std::string callG();

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
