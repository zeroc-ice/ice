//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

class I1 : public Test::I
{
public:

    virtual Test::S opS(ICE_IN(Test::S), Test::S&, const Ice::Current&);
    virtual Test::SSeq opSSeq(ICE_IN(Test::SSeq), Test::SSeq&, const Ice::Current&);
    virtual Test::SMap opSMap(ICE_IN(Test::SMap), Test::SMap&, const Ice::Current&);

    virtual Test::CPtr opC(ICE_IN(Test::CPtr), Test::CPtr&, const Ice::Current&);
    virtual Test::CSeq opCSeq(ICE_IN(Test::CSeq), Test::CSeq&, const Ice::Current&);
    virtual Test::CMap opCMap(ICE_IN(Test::CMap), Test::CMap&, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

class I2 : public Test::Inner::Inner2::I
{
public:

    virtual Test::Inner::Inner2::S
    opS(ICE_IN(Test::Inner::Inner2::S), Test::Inner::Inner2::S&, const Ice::Current&);

    virtual Test::Inner::Inner2::SSeq
    opSSeq(ICE_IN(Test::Inner::Inner2::SSeq), Test::Inner::Inner2::SSeq&, const Ice::Current&);

    virtual Test::Inner::Inner2::SMap
    opSMap(ICE_IN(Test::Inner::Inner2::SMap), Test::Inner::Inner2::SMap&, const Ice::Current&);

    virtual Test::Inner::Inner2::CPtr
    opC(ICE_IN(Test::Inner::Inner2::CPtr), Test::Inner::Inner2::CPtr&, const Ice::Current&);

    virtual Test::Inner::Inner2::CSeq
    opCSeq(ICE_IN(Test::Inner::Inner2::CSeq), Test::Inner::Inner2::CSeq&, const Ice::Current&);

    virtual Test::Inner::Inner2::CMap
    opCMap(ICE_IN(Test::Inner::Inner2::CMap), Test::Inner::Inner2::CMap&, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

class I3 : public Test::Inner::I
{
public:

    virtual Test::Inner::Inner2::S
    opS(ICE_IN(Test::Inner::Inner2::S), Test::Inner::Inner2::S&, const Ice::Current&);

    virtual Test::Inner::Inner2::SSeq
    opSSeq(ICE_IN(Test::Inner::Inner2::SSeq), Test::Inner::Inner2::SSeq&, const Ice::Current&);

    virtual Test::Inner::Inner2::SMap
    opSMap(ICE_IN(Test::Inner::Inner2::SMap), Test::Inner::Inner2::SMap&, const Ice::Current&);

    virtual Test::Inner::Inner2::CPtr
    opC(ICE_IN(Test::Inner::Inner2::CPtr), Test::Inner::Inner2::CPtr&, const Ice::Current&);

    virtual Test::Inner::Inner2::CSeq
    opCSeq(ICE_IN(Test::Inner::Inner2::CSeq), Test::Inner::Inner2::CSeq&, const Ice::Current&);

    virtual Test::Inner::Inner2::CMap
    opCMap(ICE_IN(Test::Inner::Inner2::CMap), Test::Inner::Inner2::CMap&, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

class I4 : public Inner::Test::Inner2::I
{
public:

    virtual Test::S
    opS(ICE_IN(Test::S), Test::S&, const Ice::Current&);

    virtual Test::SSeq
    opSSeq(ICE_IN(Test::SSeq), Test::SSeq&, const Ice::Current&);

    virtual Test::SMap
    opSMap(ICE_IN(Test::SMap), Test::SMap&, const Ice::Current&);

    virtual Test::CPtr
    opC(ICE_IN(Test::CPtr), Test::CPtr&, const Ice::Current&);

    virtual Test::CSeq
    opCSeq(ICE_IN(Test::CSeq), Test::CSeq&, const Ice::Current&);

    virtual Test::CMap
    opCMap(ICE_IN(Test::CMap), Test::CMap&, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

//
// I1 implementation
//
Test::S
I1::opS(ICE_IN(Test::S) s1, Test::S& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::SSeq
I1::opSSeq(ICE_IN(Test::SSeq) s1, Test::SSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::SMap
I1::opSMap(ICE_IN(Test::SMap) s1, Test::SMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::CPtr
I1::opC(ICE_IN(Test::CPtr) c1, Test::CPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::CSeq
I1::opCSeq(ICE_IN(Test::CSeq) c1, Test::CSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::CMap
I1::opCMap(ICE_IN(Test::CMap) c1, Test::CMap& c2, const Ice::Current&)
{
    c2 = c1;
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
I2::opS(ICE_IN(Test::Inner::Inner2::S) s1, Test::Inner::Inner2::S& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::SSeq
I2::opSSeq(ICE_IN(Test::Inner::Inner2::SSeq) s1, Test::Inner::Inner2::SSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::SMap
I2::opSMap(ICE_IN(Test::Inner::Inner2::SMap) s1, Test::Inner::Inner2::SMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::CPtr
I2::opC(ICE_IN(Test::Inner::Inner2::CPtr) c1, Test::Inner::Inner2::CPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::Inner::Inner2::CSeq
I2::opCSeq(ICE_IN(Test::Inner::Inner2::CSeq) c1, Test::Inner::Inner2::CSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::Inner::Inner2::CMap
I2::opCMap(ICE_IN(Test::Inner::Inner2::CMap) c1, Test::Inner::Inner2::CMap& c2, const Ice::Current&)
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
I3::opS(ICE_IN(Test::Inner::Inner2::S) s1, Test::Inner::Inner2::S& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::SSeq
I3::opSSeq(ICE_IN(Test::Inner::Inner2::SSeq) s1, Test::Inner::Inner2::SSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::SMap
I3::opSMap(ICE_IN(Test::Inner::Inner2::SMap) s1, Test::Inner::Inner2::SMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::Inner::Inner2::CPtr
I3::opC(ICE_IN(Test::Inner::Inner2::CPtr) c1, Test::Inner::Inner2::CPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::Inner::Inner2::CSeq
I3::opCSeq(ICE_IN(Test::Inner::Inner2::CSeq) c1, Test::Inner::Inner2::CSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}
Test::Inner::Inner2::CMap
I3::opCMap(ICE_IN(Test::Inner::Inner2::CMap) c1, Test::Inner::Inner2::CMap& c2, const Ice::Current&)
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
I4::opS(ICE_IN(Test::S) s1, Test::S& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::SSeq
I4::opSSeq(ICE_IN(Test::SSeq) s1, Test::SSeq& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::SMap
I4::opSMap(ICE_IN(Test::SMap) s1, Test::SMap& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::CPtr
I4::opC(ICE_IN(Test::CPtr) c1, Test::CPtr& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::CSeq
I4::opCSeq(ICE_IN(Test::CSeq) c1, Test::CSeq& c2, const Ice::Current&)
{
    c2 = c1;
    return c1;
}

Test::CMap
I4::opCMap(ICE_IN(Test::CMap) c1, Test::CMap& c2, const Ice::Current&)
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
    adapter->add(ICE_MAKE_SHARED(I1), Ice::stringToIdentity("i1"));
    adapter->add(ICE_MAKE_SHARED(I2), Ice::stringToIdentity("i2"));
    adapter->add(ICE_MAKE_SHARED(I3), Ice::stringToIdentity("i3"));
    adapter->add(ICE_MAKE_SHARED(I4), Ice::stringToIdentity("i4"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
