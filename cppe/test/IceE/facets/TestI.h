// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    virtual std::string callA(const IceE::Current&);
};

class BI : virtual public Test::B, virtual public AI
{
public:

    virtual std::string callB(const IceE::Current&);
};

class CI : virtual public Test::C, virtual public AI
{
public:

    virtual std::string callC(const IceE::Current&);
};

class DI : virtual public Test::D, virtual public BI, virtual public CI
{
public:

    virtual std::string callD(const IceE::Current&);
};

class EI : virtual public Test::E
{
public:

    virtual std::string callE(const IceE::Current&);
};

class FI : virtual public Test::F, virtual public EI
{
public:

    virtual std::string callF(const IceE::Current&);
};

class GI : virtual public Test::G
{
public:

    GI(const IceE::CommunicatorPtr&);
    virtual void shutdown(const IceE::Current&);
    virtual std::string callG(const IceE::Current&);

private:

    IceE::CommunicatorPtr _communicator;
};

class HI : virtual public Test::H, virtual public GI
{
public:

    HI(const IceE::CommunicatorPtr&);
    virtual std::string callH(const IceE::Current&);
};

#endif
