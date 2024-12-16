//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

class I1 : public Test::I
{
public:
    Test::S opS(Test::S, Test::S&, const Ice::Current&) override;
    Test::SSeq opSSeq(Test::SSeq, Test::SSeq&, const Ice::Current&) override;
    Test::SMap opSMap(Test::SMap, Test::SMap&, const Ice::Current&) override;

    Test::CPtr opC(Test::CPtr, Test::CPtr&, const Ice::Current&) override;
    Test::CSeq opCSeq(Test::CSeq, Test::CSeq&, const Ice::Current&) override;
    Test::CMap opCMap(Test::CMap, Test::CMap&, const Ice::Current&) override;

    Test::E1 opE1(Test::E1, const Ice::Current&) override;
    Test::S1 opS1(Test::S1, const Ice::Current&) override;
    Test::C1Ptr opC1(Test::C1Ptr, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class I2 : public Test::Inner::Inner2::I
{
public:
    Test::Inner::Inner2::S opS(Test::Inner::Inner2::S, Test::Inner::Inner2::S&, const Ice::Current&) override;

    Test::Inner::Inner2::SSeq
    opSSeq(Test::Inner::Inner2::SSeq, Test::Inner::Inner2::SSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::SMap
    opSMap(Test::Inner::Inner2::SMap, Test::Inner::Inner2::SMap&, const Ice::Current&) override;

    Test::Inner::Inner2::CPtr opC(Test::Inner::Inner2::CPtr, Test::Inner::Inner2::CPtr&, const Ice::Current&) override;

    Test::Inner::Inner2::CSeq
    opCSeq(Test::Inner::Inner2::CSeq, Test::Inner::Inner2::CSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::CMap
    opCMap(Test::Inner::Inner2::CMap, Test::Inner::Inner2::CMap&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class I3 : public Test::Inner::I
{
public:
    Test::Inner::Inner2::S opS(Test::Inner::Inner2::S, Test::Inner::Inner2::S&, const Ice::Current&) override;

    Test::Inner::Inner2::SSeq
    opSSeq(Test::Inner::Inner2::SSeq, Test::Inner::Inner2::SSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::SMap
    opSMap(Test::Inner::Inner2::SMap, Test::Inner::Inner2::SMap&, const Ice::Current&) override;

    Test::Inner::Inner2::CPtr opC(Test::Inner::Inner2::CPtr, Test::Inner::Inner2::CPtr&, const Ice::Current&) override;

    Test::Inner::Inner2::CSeq
    opCSeq(Test::Inner::Inner2::CSeq, Test::Inner::Inner2::CSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::CMap
    opCMap(Test::Inner::Inner2::CMap, Test::Inner::Inner2::CMap&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class I4 : public Inner::Test::Inner2::I
{
public:
    Test::S opS(Test::S, Test::S&, const Ice::Current&) override;

    Test::SSeq opSSeq(Test::SSeq, Test::SSeq&, const Ice::Current&) override;

    Test::SMap opSMap(Test::SMap, Test::SMap&, const Ice::Current&) override;

    Test::CPtr opC(Test::CPtr, Test::CPtr&, const Ice::Current&) override;

    Test::CSeq opCSeq(Test::CSeq, Test::CSeq&, const Ice::Current&) override;

    Test::CMap opCMap(Test::CMap, Test::CMap&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

//
// I1 implementation
//
Test::S
I1::opS(Test::S s1, Test::S& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::SSeq
I1::opSSeq(Test::SSeq s1, Test::SSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::SMap
I1::opSMap(Test::SMap s1, Test::SMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::CPtr
I1::opC(Test::CPtr c1, Test::CPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::CSeq
I1::opCSeq(Test::CSeq c1, Test::CSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::CMap
I1::opCMap(Test::CMap c1, Test::CMap& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::E1
I1::opE1(Test::E1 e1, const Ice::Current&)
{
    return e1;
}

Test::S1
I1::opS1(Test::S1 s1, const Ice::Current&)
{
    return s1;
}

Test::C1Ptr
I1::opC1(Test::C1Ptr c1, const Ice::Current&)
{
    return c1;
}

void
I1::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

//
// I2 implementation
//
Test::Inner::Inner2::S
I2::opS(Test::Inner::Inner2::S s1, Test::Inner::Inner2::S& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::SSeq
I2::opSSeq(Test::Inner::Inner2::SSeq s1, Test::Inner::Inner2::SSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::SMap
I2::opSMap(Test::Inner::Inner2::SMap s1, Test::Inner::Inner2::SMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::CPtr
I2::opC(Test::Inner::Inner2::CPtr c1, Test::Inner::Inner2::CPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::Inner::Inner2::CSeq
I2::opCSeq(Test::Inner::Inner2::CSeq c1, Test::Inner::Inner2::CSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::Inner::Inner2::CMap
I2::opCMap(Test::Inner::Inner2::CMap c1, Test::Inner::Inner2::CMap& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

void
I2::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

//
// I3 implementation
//
Test::Inner::Inner2::S
I3::opS(Test::Inner::Inner2::S s1, Test::Inner::Inner2::S& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::SSeq
I3::opSSeq(Test::Inner::Inner2::SSeq s1, Test::Inner::Inner2::SSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::SMap
I3::opSMap(Test::Inner::Inner2::SMap s1, Test::Inner::Inner2::SMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::CPtr
I3::opC(Test::Inner::Inner2::CPtr c1, Test::Inner::Inner2::CPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::Inner::Inner2::CSeq
I3::opCSeq(Test::Inner::Inner2::CSeq c1, Test::Inner::Inner2::CSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::Inner::Inner2::CMap
I3::opCMap(Test::Inner::Inner2::CMap c1, Test::Inner::Inner2::CMap& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

void
I3::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

//
// I4 implementation
//
Test::S
I4::opS(Test::S s1, Test::S& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::SSeq
I4::opSSeq(Test::SSeq s1, Test::SSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::SMap
I4::opSMap(Test::SMap s1, Test::SMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::CPtr
I4::opC(Test::CPtr c1, Test::CPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::CSeq
I4::opCSeq(Test::CSeq c1, Test::CSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::CMap
I4::opCMap(Test::CMap c1, Test::CMap& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

void
I4::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<I1>(), Ice::stringToIdentity("i1"));
    adapter->add(std::make_shared<I2>(), Ice::stringToIdentity("i2"));
    adapter->add(std::make_shared<I3>(), Ice::stringToIdentity("i3"));
    adapter->add(std::make_shared<I4>(), Ice::stringToIdentity("i4"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
