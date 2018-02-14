// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class AI : virtual public Test::A
{
public:

    virtual std::string callA(const Ice::Current&);
};

class BI : virtual public Test::B, virtual public AI
{
public:

    virtual std::string callB(const Ice::Current&);
};

class CI : virtual public Test::C, virtual public AI
{
public:

    virtual std::string callC(const Ice::Current&);
};

class DI : virtual public Test::D, virtual public BI, virtual public CI
{
public:

    virtual std::string callD(const Ice::Current&);
};

class EI : virtual public Test::E
{
public:

    virtual std::string callE(const Ice::Current&);
};

class FI : virtual public Test::F, virtual public EI
{
public:

    virtual std::string callF(const Ice::Current&);
};

class GI : virtual public Test::G
{
public:

    GI(const Ice::CommunicatorPtr&);
    virtual void shutdown(const Ice::Current&);
    virtual std::string callG(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

class HI : virtual public Test::H, virtual public GI
{
public:

    HI(const Ice::CommunicatorPtr&);
    virtual std::string callH(const Ice::Current&);
};

#endif
