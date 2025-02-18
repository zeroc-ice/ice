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

class MyInterface1 : public Test::MyInterface
{
public:
    Test::MyStruct opMyStruct(Test::MyStruct, Test::MyStruct&, const Ice::Current&) override;
    Test::MyStructSeq opMyStructSeq(Test::MyStructSeq, Test::MyStructSeq&, const Ice::Current&) override;
    Test::MyStructMap opMyStructMap(Test::MyStructMap, Test::MyStructMap&, const Ice::Current&) override;

    Test::MyClassPtr opMyClass(Test::MyClassPtr, Test::MyClassPtr&, const Ice::Current&) override;
    Test::MyClassSeq opMyClassSeq(Test::MyClassSeq, Test::MyClassSeq&, const Ice::Current&) override;
    Test::MyClassMap opMyClassMap(Test::MyClassMap, Test::MyClassMap&, const Ice::Current&) override;

    Test::MyEnum opMyEnum(Test::MyEnum, const Ice::Current&) override;
    Test::MyOtherStruct opMyOtherStruct(Test::MyOtherStruct, const Ice::Current&) override;
    Test::MyOtherClassPtr opMyOtherClass(Test::MyOtherClassPtr, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class MyInterface2 : public Test::Inner::Inner2::MyInterface
{
public:
    Test::Inner::Inner2::MyStruct
    opMyStruct(Test::Inner::Inner2::MyStruct, Test::Inner::Inner2::MyStruct&, const Ice::Current&) override;

    Test::Inner::Inner2::MyStructSeq
    opMyStructSeq(Test::Inner::Inner2::MyStructSeq, Test::Inner::Inner2::MyStructSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::MyStructMap
    opMyStructMap(Test::Inner::Inner2::MyStructMap, Test::Inner::Inner2::MyStructMap&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassPtr
    opMyClass(Test::Inner::Inner2::MyClassPtr, Test::Inner::Inner2::MyClassPtr&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassSeq
    opMyClassSeq(Test::Inner::Inner2::MyClassSeq, Test::Inner::Inner2::MyClassSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassMap
    opMyClassMap(Test::Inner::Inner2::MyClassMap, Test::Inner::Inner2::MyClassMap&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class MyInterface3 : public Test::Inner::MyInterface
{
public:
    Test::Inner::Inner2::MyStruct
    opMyStruct(Test::Inner::Inner2::MyStruct, Test::Inner::Inner2::MyStruct&, const Ice::Current&) override;

    Test::Inner::Inner2::MyStructSeq
    opMyStructSeq(Test::Inner::Inner2::MyStructSeq, Test::Inner::Inner2::MyStructSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::MyStructMap
    opMyStructMap(Test::Inner::Inner2::MyStructMap, Test::Inner::Inner2::MyStructMap&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassPtr
    opMyClass(Test::Inner::Inner2::MyClassPtr, Test::Inner::Inner2::MyClassPtr&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassSeq
    opMyClassSeq(Test::Inner::Inner2::MyClassSeq, Test::Inner::Inner2::MyClassSeq&, const Ice::Current&) override;

    Test::Inner::Inner2::MyClassMap
    opMyClassMap(Test::Inner::Inner2::MyClassMap, Test::Inner::Inner2::MyClassMap&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class MyInterface4 : public Inner::Test::Inner2::MyInterface
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
// MyInterface1 implementation
//
Test::MyStruct
MyInterface1::opMyStruct(Test::MyStruct s1, Test::MyStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyStructSeq
MyInterface1::opMyStructSeq(Test::MyStructSeq s1, Test::MyStructSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyStructMap
MyInterface1::opMyStructMap(Test::MyStructMap s1, Test::MyStructMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyClassPtr
MyInterface1::opMyClass(Test::MyClassPtr c1, Test::MyClassPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::MyClassSeq
MyInterface1::opMyClassSeq(Test::MyClassSeq c1, Test::MyClassSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::MyClassMap
MyInterface1::opMyClassMap(Test::MyClassMap c1, Test::MyClassMap& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::MyEnum
MyInterface1::opMyEnum(Test::MyEnum e1, const Ice::Current&)
{
    return e1;
}

Test::MyOtherStruct
MyInterface1::opMyOtherStruct(Test::MyOtherStruct s1, const Ice::Current&)
{
    return s1;
}

Test::MyOtherClassPtr
MyInterface1::opMyOtherClass(Test::MyOtherClassPtr c1, const Ice::Current&)
{
    return c1;
}

void
MyInterface1::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

//
// MyInterface2 implementation
//
Test::Inner::Inner2::MyStruct
MyInterface2::opMyStruct(Test::Inner::Inner2::MyStruct s1, Test::Inner::Inner2::MyStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyStructSeq
MyInterface2::opMyStructSeq(
    Test::Inner::Inner2::MyStructSeq s1,
    Test::Inner::Inner2::MyStructSeq& s2,
    const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyStructMap
MyInterface2::opMyStructMap(
    Test::Inner::Inner2::MyStructMap s1,
    Test::Inner::Inner2::MyStructMap& s2,
    const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyClassPtr
MyInterface2::opMyClass(Test::Inner::Inner2::MyClassPtr c1, Test::Inner::Inner2::MyClassPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::Inner::Inner2::MyClassSeq
MyInterface2::opMyClassSeq(Test::Inner::Inner2::MyClassSeq c1, Test::Inner::Inner2::MyClassSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::Inner::Inner2::MyClassMap
MyInterface2::opMyClassMap(Test::Inner::Inner2::MyClassMap c1, Test::Inner::Inner2::MyClassMap& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

void
MyInterface2::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

//
// MyInterface3 implementation
//
Test::Inner::Inner2::MyStruct
MyInterface3::opMyStruct(Test::Inner::Inner2::MyStruct s1, Test::Inner::Inner2::MyStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyStructSeq
MyInterface3::opMyStructSeq(
    Test::Inner::Inner2::MyStructSeq s1,
    Test::Inner::Inner2::MyStructSeq& s2,
    const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyStructMap
MyInterface3::opMyStructMap(
    Test::Inner::Inner2::MyStructMap s1,
    Test::Inner::Inner2::MyStructMap& s2,
    const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::MyClassPtr
MyInterface3::opMyClass(Test::Inner::Inner2::MyClassPtr c1, Test::Inner::Inner2::MyClassPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::Inner::Inner2::MyClassSeq
MyInterface3::opMyClassSeq(Test::Inner::Inner2::MyClassSeq c1, Test::Inner::Inner2::MyClassSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::Inner::Inner2::MyClassMap
MyInterface3::opMyClassMap(Test::Inner::Inner2::MyClassMap c1, Test::Inner::Inner2::MyClassMap& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

void
MyInterface3::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

//
// MyInterface4 implementation
//
Test::MyStruct
MyInterface4::opMyStruct(Test::MyStruct s1, Test::MyStruct& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyStructSeq
MyInterface4::opMyStructSeq(Test::MyStructSeq s1, Test::MyStructSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyStructMap
MyInterface4::opMyStructMap(Test::MyStructMap s1, Test::MyStructMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::MyClassPtr
MyInterface4::opMyClass(Test::MyClassPtr c1, Test::MyClassPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::MyClassSeq
MyInterface4::opMyClassSeq(Test::MyClassSeq c1, Test::MyClassSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::MyClassMap
MyInterface4::opMyClassMap(Test::MyClassMap c1, Test::MyClassMap& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

void
MyInterface4::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<MyInterface1>(), Ice::stringToIdentity("i1"));
    adapter->add(std::make_shared<MyInterface2>(), Ice::stringToIdentity("i2"));
    adapter->add(std::make_shared<MyInterface3>(), Ice::stringToIdentity("i3"));
    adapter->add(std::make_shared<MyInterface4>(), Ice::stringToIdentity("i4"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
