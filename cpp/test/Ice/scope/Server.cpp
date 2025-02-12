// Copyright (c) ZeroC, Inc.

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
    Test::MyStruct opMyStruct(Test::MyStruct, Test::MyStruct&, const Ice::Current&) override;
    Test::MyStructSeq opMyStructSeq(Test::MyStructSeq, Test::MyStructSeq&, const Ice::Current&) override;
    Test::MyStructMap opMyStructMap(Test::MyStructMap, Test::MyStructMap&, const Ice::Current&) override;

    Test::MyClassPtr opMyClass(Test::MyClassPtr, Test::MyClassPtr&, const Ice::Current&) override;
    Test::MyClassSeq opMyClassSeq(Test::MyClassSeq, Test::MyClassSeq&, const Ice::Current&) override;
    Test::MyClassMap opMyClassMap(Test::MyClassMap, Test::MyClassMap&, const Ice::Current&) override;

    Test::E1 opE1(Test::E1, const Ice::Current&) override;
    Test::S1 opS1(Test::S1, const Ice::Current&) override;
    Test::C1Ptr opC1(Test::C1Ptr, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class I2 : public Test::Inner::Inner2::I
{
public:
    Test::Inner::Inner2::MyStruct opMyStruct(Test::Inner::Inner2::MyStruct, Test::Inner::Inner2::MyStruct&, const Ice::Current&) override;

    Test::Inner::Inner2::MyStructSeq
    opMyStructSeq(Test::Inner::Inner2::MyStructSeq, Test::Inner::Inner2::MyStructSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::MyStructMap
    opMyStructMap(Test::Inner::Inner2::MyStructMap, Test::Inner::Inner2::MyStructMap&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassPtr opMyClass(Test::Inner::Inner2::MyClassPtr, Test::Inner::Inner2::MyClassPtr&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassSeq
    opMyClassSeq(Test::Inner::Inner2::MyClassSeq, Test::Inner::Inner2::MyClassSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassMap
    opMyClassMap(Test::Inner::Inner2::MyClassMap, Test::Inner::Inner2::MyClassMap&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class I3 : public Test::Inner::I
{
public:
    Test::Inner::Inner2::MyStruct opMyStruct(Test::Inner::Inner2::MyStruct, Test::Inner::Inner2::MyStruct&, const Ice::Current&) override;

    Test::Inner::Inner2::MyStructSeq
    opMyStructSeq(Test::Inner::Inner2::MyStructSeq, Test::Inner::Inner2::MyStructSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::MyStructMap
    opMyStructMap(Test::Inner::Inner2::MyStructMap, Test::Inner::Inner2::MyStructMap&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassPtr opMyClass(Test::Inner::Inner2::MyClassPtr, Test::Inner::Inner2::MyClassPtr&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassSeq
    opMyClassSeq(Test::Inner::Inner2::MyClassSeq, Test::Inner::Inner2::MyClassSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassMap
    opMyClassMap(Test::Inner::Inner2::MyClassMap, Test::Inner::Inner2::MyClassMap&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class I4 : public Inner::Test::Inner2::I
{
public:
    Test::MyStruct opMyStruct(Test::MyStruct, Test::MyStruct&, const Ice::Current&) override;

    Test::MyStructSeq opMyStructSeq(Test::MyStructSeq, Test::MyStructSeq&, const Ice::Current&) override;

    Test::MyStructMap opMyStructMap(Test::MyStructMap, Test::MyStructMap&, const Ice::Current&) override;

    Test::MyClassPtr opMyClass(Test::MyClassPtr, Test::MyClassPtr&, const Ice::Current&) override;

    Test::MyClassSeq opMyClassSeq(Test::MyClassSeq, Test::MyClassSeq&, const Ice::Current&) override;

    Test::MyClassMap opMyClassMap(Test::MyClassMap, Test::MyClassMap&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

//
// I1 implementation
//
Test::MyStruct
I1::opMyStruct(Test::MyStruct s1, Test::MyStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyStructSeq
I1::opMyStructSeq(Test::MyStructSeq s1, Test::MyStructSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyStructMap
I1::opMyStructMap(Test::MyStructMap s1, Test::MyStructMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyClassPtr
I1::opMyClass(Test::MyClassPtr c1, Test::MyClassPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::MyClassSeq
I1::opMyClassSeq(Test::MyClassSeq c1, Test::MyClassSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::MyClassMap
I1::opMyClassMap(Test::MyClassMap c1, Test::MyClassMap& c2, const Ice::Current&)
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
Test::Inner::Inner2::MyStruct
I2::opMyStruct(Test::Inner::Inner2::MyStruct s1, Test::Inner::Inner2::MyStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyStructSeq
I2::opMyStructSeq(Test::Inner::Inner2::MyStructSeq s1, Test::Inner::Inner2::MyStructSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyStructMap
I2::opMyStructMap(Test::Inner::Inner2::MyStructMap s1, Test::Inner::Inner2::MyStructMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyClassPtr
I2::opMyClass(Test::Inner::Inner2::MyClassPtr c1, Test::Inner::Inner2::MyClassPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::Inner::Inner2::MyClassSeq
I2::opMyClassSeq(Test::Inner::Inner2::MyClassSeq c1, Test::Inner::Inner2::MyClassSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::Inner::Inner2::MyClassMap
I2::opMyClassMap(Test::Inner::Inner2::MyClassMap c1, Test::Inner::Inner2::MyClassMap& c2, const Ice::Current&)
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
Test::Inner::Inner2::MyStruct
I3::opMyStruct(Test::Inner::Inner2::MyStruct s1, Test::Inner::Inner2::MyStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyStructSeq
I3::opMyStructSeq(Test::Inner::Inner2::MyStructSeq s1, Test::Inner::Inner2::MyStructSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyStructMap
I3::opMyStructMap(Test::Inner::Inner2::MyStructMap s1, Test::Inner::Inner2::MyStructMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyClassPtr
I3::opMyClass(Test::Inner::Inner2::MyClassPtr c1, Test::Inner::Inner2::MyClassPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::Inner::Inner2::MyClassSeq
I3::opMyClassSeq(Test::Inner::Inner2::MyClassSeq c1, Test::Inner::Inner2::MyClassSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::Inner::Inner2::MyClassMap
I3::opMyClassMap(Test::Inner::Inner2::MyClassMap c1, Test::Inner::Inner2::MyClassMap& c2, const Ice::Current&)
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
Test::MyStruct
I4::opMyStruct(Test::MyStruct s1, Test::MyStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyStructSeq
I4::opMyStructSeq(Test::MyStructSeq s1, Test::MyStructSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyStructMap
I4::opMyStructMap(Test::MyStructMap s1, Test::MyStructMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyClassPtr
I4::opMyClass(Test::MyClassPtr c1, Test::MyClassPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::MyClassSeq
I4::opMyClassSeq(Test::MyClassSeq c1, Test::MyClassSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::MyClassMap
I4::opMyClassMap(Test::MyClassMap c1, Test::MyClassMap& c2, const Ice::Current&)
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
