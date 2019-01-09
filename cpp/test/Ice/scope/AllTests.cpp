// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

#ifndef ICE_CPP11_MAPPING

namespace Test
{

class Callback : public IceUtil::Shared
{
public:

    Callback() : _called(false)
    {
    }

    void opS(const Test::S& s2, const Test::S& s3)
    {
        test(s2 == s3);
        called();
    }

    void opSSeq(const Test::SSeq& s2, const Test::SSeq& s3)
    {
        test(s2 == s3);
        called();
    }

    void opSMap(const Test::SMap& s2, const Test::SMap& s3)
    {
        test(s2 == s3);
        called();
    }

    void opC(const Test::CPtr& c2, const Test::CPtr& c3)
    {
        test(c2 == c3);
        called();
    }

    void opCSeq(const Test::CSeq& c2, const Test::CSeq& c3)
    {
        test(c2 == c3);
        called();
    }

    void opCMap(const Test::CMap& c2, const Test::CMap& c3)
    {
        test(c2 == c3);
        called();
    }

    void error(const Ice::Exception&)
    {
        test(false);
    }

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        assert(!_called);
        _called = true;
        _m.notify();
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(!_called)
        {
            _m.wait();
        }
        _called = false;
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _m;
    bool _called;
};

namespace Inner
{

class Callback : public IceUtil::Shared
{
public:

    Callback() : _called(false)
    {
    }

    void opS(const Test::Inner::Inner2::S& s2, const Test::Inner::Inner2::S& s3)
    {
        test(s2 == s3);
        called();
    }

    void opSSeq(const Test::Inner::Inner2::SSeq& s2, const Test::Inner::Inner2::SSeq& s3)
    {
        test(s2 == s3);
        called();
    }

    void opSMap(const Test::Inner::Inner2::SMap& s2, const Test::Inner::Inner2::SMap& s3)
    {
        test(s2 == s3);
        called();
    }

    void opC(const Test::Inner::Inner2::CPtr& c2, const Test::Inner::Inner2::CPtr& c3)
    {
        test(c2 == c3);
        called();
    }

    void opCSeq(const Test::Inner::Inner2::CSeq& c2, const Test::Inner::Inner2::CSeq& c3)
    {
        test(c2 == c3);
        called();
    }

    void opCMap(const Test::Inner::Inner2::CMap& c2, const Test::Inner::Inner2::CMap& c3)
    {
        test(c2 == c3);
        called();
    }

    void error(const Ice::Exception&)
    {
        test(false);
    }

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        assert(!_called);
        _called = true;
        _m.notify();
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(!_called)
        {
            _m.wait();
        }
        _called = false;
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _m;
    bool _called;
};

namespace Inner2
{

class Callback : public IceUtil::Shared
{
public:

    Callback() : _called(false)
    {
    }

    void opS(const Test::Inner::Inner2::S& s2, const Test::Inner::Inner2::S& s3)
    {
        test(s2 == s3);
        called();
    }

    void opSSeq(const Test::Inner::Inner2::SSeq& s2, const Test::Inner::Inner2::SSeq& s3)
    {
        test(s2 == s3);
        called();
    }

    void opSMap(const Test::Inner::Inner2::SMap& s2, const Test::Inner::Inner2::SMap& s3)
    {
        test(s2 == s3);
        called();
    }

    void opC(const Test::Inner::Inner2::CPtr& s2, const Test::Inner::Inner2::CPtr& s3)
    {
        test(s2 == s3);
        called();
    }

    void opCSeq(const Test::Inner::Inner2::CSeq& s2, const Test::Inner::Inner2::CSeq& s3)
    {
        test(s2 == s3);
        called();
    }

    void opCMap(const Test::Inner::Inner2::CMap& s2, const Test::Inner::Inner2::CMap& s3)
    {
        test(s2 == s3);
        called();
    }

    void error(const Ice::Exception&)
    {
        test(false);
    }

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        assert(!_called);
        _called = true;
        _m.notify();
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(!_called)
        {
            _m.wait();
        }
        _called = false;
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _m;
    bool _called;
};

}

}

}

namespace Inner
{

namespace Test
{

namespace Inner2
{

class Callback : public IceUtil::Shared
{
public:

    Callback() : _called(false)
    {
    }

    void opS(const ::Test::S& s2, const ::Test::S& s3)
    {
        test(s2 == s3);
        called();
    }

    void opSSeq(const ::Test::SSeq& s2, const ::Test::SSeq& s3)
    {
        test(s2 == s3);
        called();
    }

    void opSMap(const ::Test::SMap& s2, const ::Test::SMap& s3)
    {
        test(s2 == s3);
        called();
    }

    void opC(const ::Test::CPtr& s2, const ::Test::CPtr& s3)
    {
        test(s2 == s3);
        called();
    }

    void opCSeq(const ::Test::CSeq& s2, const ::Test::CSeq& s3)
    {
        test(s2 == s3);
        called();
    }

    void opCMap(const ::Test::CMap& s2, const ::Test::CMap& s3)
    {
        test(s2 == s3);
        called();
    }

    void error(const Ice::Exception&)
    {
        test(false);
    }

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        assert(!_called);
        _called = true;
        _m.notify();
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(!_called)
        {
            _m.wait();
        }
        _called = false;
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _m;
    bool _called;
};

}

}

}

#endif

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    //
    // Scoped types
    //
    {
        Test::IPrxPtr i = ICE_CHECKED_CAST(Test::IPrx, communicator->stringToProxy("i1:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        Test::S s2;
        Test::S s3 = i->opS(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        Test::SSeq sseq2;
        Test::SSeq sseq3 = i->opSSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::SMap smap1;
        smap1["a"] = s1;
        Test::SMap smap2;
        Test::SMap smap3 = i->opSMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::CPtr c1 = ICE_MAKE_SHARED(Test::C, s1);
        Test::CPtr c2;
        Test::CPtr c3 = i->opC(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        Test::CSeq cseq2;
        Test::CSeq cseq3 = i->opCSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::CMap cmap1;
        cmap1["a"] = c1;
        Test::CMap cmap2;
        Test::CMap cmap3 = i->opCMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

#ifdef ICE_CPP11_MAPPING
    //
    // C++ 11 Future-Based Async Function
    //
    {
        Test::IPrxPtr i =
            ICE_CHECKED_CAST(Test::IPrx, communicator->stringToProxy("i1:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        {
            auto result = i->opSAsync(s1).get();
            test(result.returnValue == s1);
            test(result.s2 == s1);
        }

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opSSeqAsync(sseq1).get();
            test(result.returnValue == sseq1);
            test(result.s2 == sseq1);
        }

        Test::SMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opSMapAsync(smap1).get();
            test(result.returnValue == smap1);
            test(result.s2 == smap1);
        }

        Test::CPtr c1 =  make_shared<Test::C>(s1);
        {
            auto result = i->opCAsync(c1).get();
            test(Ice::targetEqualTo(result.returnValue, c1));
            test(Ice::targetEqualTo(result.c2, c1));
        }

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opCSeqAsync(cseq1).get();
            test(Ice::targetEqualTo(result.returnValue[0], c1));
            test(Ice::targetEqualTo(result.s2[0], c1));
        }

        Test::CMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opCMapAsync(cmap1).get();
            test(Ice::targetEqualTo(result.returnValue["a"], c1));
            test(Ice::targetEqualTo(result.c2["a"], c1));
        }
    }

    //
    // C++11 Callback-Based Async Function
    //
    {
        Test::IPrxPtr i =
            ICE_CHECKED_CAST(Test::IPrx, communicator->stringToProxy("i1:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSAsync(s1,
                                      [&p, &s1](Test::S s2, Test::S s3)
                                      {
                                          test(s2 == s1);
                                          test(s3 == s1);
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSSeqAsync(sseq1,
                                         [&p, &sseq1](Test::SSeq s2, Test::SSeq s3)
                                         {
                                             test(s2 == sseq1);
                                             test(s3 == sseq1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::SMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSMapAsync(smap1,
                                         [&p, &smap1](Test::SMap s2, Test::SMap s3)
                                         {
                                             test(s2 == smap1);
                                             test(s3 == smap1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CPtr c1 = make_shared<Test::C>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCAsync(c1,
                                      [&p, &c1](Test::CPtr c2, Test::CPtr c3)
                                      {
                                          test(Ice::targetEqualTo(c2, c1));
                                          test(Ice::targetEqualTo(c3, c1));
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCSeqAsync(cseq1,
                                         [&p, c1](Test::CSeq c2, Test::CSeq c3)
                                         {
                                             test(Ice::targetEqualTo(c2[0], c1));
                                             test(Ice::targetEqualTo(c3[0], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCMapAsync(cmap1,
                                         [&p, c1](Test::CMap c2, Test::CMap c3)
                                         {
                                             test(Ice::targetEqualTo(c2["a"], c1));
                                             test(Ice::targetEqualTo(c3["a"], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }
#else
    //
    // C++ 98 AsyncResult API
    //
    {
        Test::IPrxPtr i = ICE_CHECKED_CAST(Test::IPrx, communicator->stringToProxy("i1:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;

        Test::S s2;
        Test::S s3 = i->end_opS(s2, i->begin_opS(s1));

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        sseq1.push_back(s2);
        sseq1.push_back(s3);

        Test::SSeq sseq2;
        Test::SSeq sseq3 = i->end_opSSeq(sseq2, i->begin_opSSeq(sseq1));

        Test::SMap smap1;
        smap1["a"] = s1;
        smap1["b"] = s2;
        smap1["c"] = s3;

        Test::SMap smap2;
        Test::SMap smap3 = i->end_opSMap(smap2, i->begin_opSMap(smap1));

        Test::CPtr c1 = new Test::C(s1);
        Test::CPtr c2;
        Test::CPtr c3 = i->end_opC(c2, i->begin_opC(c1));

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        cseq1.push_back(c2);
        cseq1.push_back(c3);

        Test::CSeq cseq2;
        Test::CSeq cseq3 = i->end_opCSeq(cseq2, i->begin_opCSeq(cseq1));

        Test::CMap cmap1;
        cmap1["a"] = c1;
        cmap1["b"] = c2;
        cmap1["c"] = c3;

        Test::CMap cmap2;
        Test::CMap cmap3 = i->end_opCMap(cmap2, i->begin_opCMap(cmap1));
    }
    //
    // C++ 98 type safe callbacks
    //
    {
        Test::IPrxPtr i =
            ICE_CHECKED_CAST(Test::IPrx, communicator->stringToProxy("i1:" + helper->getTestEndpoint()));
        IceUtil::Handle<Test::Callback> cb = new Test::Callback();

        Test::S s1;
        s1.v = 0;
        Test::Callback_I_opSPtr opSCB =
            Test::newCallback_I_opS(cb, &Test::Callback::opS, &Test::Callback::error);
        i->begin_opS(s1, opSCB);
        cb->check();

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        Test::Callback_I_opSSeqPtr opSSeqCB =
            Test::newCallback_I_opSSeq(cb, &Test::Callback::opSSeq, &Test::Callback::error);
        i->begin_opSSeq(sseq1, opSSeqCB);
        cb->check();

        Test::SMap smap1;
        smap1["a"] = s1;
        Test::Callback_I_opSMapPtr opSMapCB =
            Test::newCallback_I_opSMap(cb, &Test::Callback::opSMap, &Test::Callback::error);
        i->begin_opSMap(smap1, opSMapCB);
        cb->check();

        Test::CPtr c1 = new Test::C(s1);
        Test::Callback_I_opCPtr opCCB =
            Test::newCallback_I_opC(cb, &Test::Callback::opC, &Test::Callback::error);
        i->begin_opC(c1, opCCB);
        cb->check();

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        Test::Callback_I_opCSeqPtr opCSeqCB =
            Test::newCallback_I_opCSeq(cb, &Test::Callback::opCSeq, &Test::Callback::error);
        i->begin_opCSeq(cseq1, opCSeqCB);
        cb->check();

        Test::CMap cmap1;
        cmap1["a"] = c1;
        Test::Callback_I_opCMapPtr opCMapCB =
            Test::newCallback_I_opCMap(cb, &Test::Callback::opCMap, &Test::Callback::error);
        i->begin_opCMap(cmap1, opCMapCB);
        cb->check();

    }
#endif

    {
        Test::Inner::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::Inner2::IPrx,
                             communicator->stringToProxy("i2:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        Test::Inner::Inner2::S s2;
        Test::Inner::Inner2::S s3 = i->opS(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        Test::Inner::Inner2::SSeq sseq2;
        Test::Inner::Inner2::SSeq sseq3 = i->opSSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        Test::Inner::Inner2::SMap smap2;
        Test::Inner::Inner2::SMap smap3 = i->opSMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::Inner::Inner2::CPtr c1 = ICE_MAKE_SHARED(Test::Inner::Inner2::C, s1);
        Test::Inner::Inner2::CPtr c2;
        Test::Inner::Inner2::CPtr c3 = i->opC(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        Test::Inner::Inner2::CSeq cseq2;
        Test::Inner::Inner2::CSeq cseq3 = i->opCSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        Test::Inner::Inner2::CMap cmap2;
        Test::Inner::Inner2::CMap cmap3 = i->opCMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }
#ifdef ICE_CPP11_MAPPING
    //
    // C++ 11 Future-Based Async Function
    //
    {
        Test::Inner::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::Inner2::IPrx,
                             communicator->stringToProxy("i2:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        {
            auto result = i->opSAsync(s1).get();
            test(result.returnValue == s1);
            test(result.s2 == s1);
        }

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opSSeqAsync(sseq1).get();
            test(result.returnValue == sseq1);
            test(result.s2 == sseq1);
        }

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opSMapAsync(smap1).get();
            test(result.returnValue == smap1);
            test(result.s2 == smap1);
        }

        Test::Inner::Inner2::CPtr c1 = make_shared<Test::Inner::Inner2::C>(s1);
        {
            auto result = i->opCAsync(c1).get();
            test(Ice::targetEqualTo(result.returnValue, c1));
            test(Ice::targetEqualTo(result.c2, c1));
        }

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opCSeqAsync(cseq1).get();
            test(Ice::targetEqualTo(result.returnValue[0], c1));
            test(Ice::targetEqualTo(result.c2[0], c1));
        }

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opCMapAsync(cmap1).get();
            test(Ice::targetEqualTo(result.returnValue["a"], c1));
            test(Ice::targetEqualTo(result.c2["a"], c1));
        }
    }

    //
    // C++11 Callback-Based Async Function
    //
    {
        Test::Inner::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::Inner2::IPrx,
                             communicator->stringToProxy("i2:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSAsync(s1,
                                      [&p, &s1](Test::Inner::Inner2::S s2, Test::Inner::Inner2::S s3)
                                      {
                                          test(s2 == s1);
                                          test(s3 == s1);
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSSeqAsync(sseq1,
                                         [&p, &sseq1](Test::Inner::Inner2::SSeq s2, Test::Inner::Inner2::SSeq s3)
                                         {
                                             test(s2 == sseq1);
                                             test(s3 == sseq1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSMapAsync(smap1,
                                         [&p, &smap1](Test::Inner::Inner2::SMap s2, Test::Inner::Inner2::SMap s3)
                                         {
                                             test(s2 == smap1);
                                             test(s3 == smap1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::Inner::Inner2::C>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCAsync(c1,
                                      [&p, &c1](shared_ptr<Test::Inner::Inner2::C> c2,
                                                shared_ptr<Test::Inner::Inner2::C> c3)
                                      {
                                          test(Ice::targetEqualTo(c2, c1));
                                          test(Ice::targetEqualTo(c3, c1));
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCSeqAsync(cseq1,
                                         [&p, c1](Test::Inner::Inner2::CSeq c2, Test::Inner::Inner2::CSeq c3)
                                         {
                                             test(Ice::targetEqualTo(c2[0], c1));
                                             test(Ice::targetEqualTo(c3[0], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCMapAsync(cmap1,
                                         [&p, c1](Test::Inner::Inner2::CMap c2, Test::Inner::Inner2::CMap c3)
                                         {
                                             test(Ice::targetEqualTo(c2["a"], c1));
                                             test(Ice::targetEqualTo(c3["a"], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }
#else
    //
    // C++ 98 AsyncResult API
    //
    {
        Test::Inner::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::Inner2::IPrx,
                             communicator->stringToProxy("i2:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;

        s1.v = 0;

        Test::Inner::Inner2::S s2;
        Test::Inner::Inner2::S s3 = i->end_opS(s2, i->begin_opS(s1));

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        sseq1.push_back(s2);
        sseq1.push_back(s3);

        Test::Inner::Inner2::SSeq sseq2;
        Test::Inner::Inner2::SSeq sseq3 = i->end_opSSeq(sseq2, i->begin_opSSeq(sseq1));

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        smap1["b"] = s2;
        smap1["c"] = s3;

        Test::Inner::Inner2::SMap smap2;
        Test::Inner::Inner2::SMap smap3 = i->end_opSMap(smap2, i->begin_opSMap(smap1));

        Test::Inner::Inner2::CPtr c1 = new Test::Inner::Inner2::C(s1);
        Test::Inner::Inner2::CPtr c2;
        Test::Inner::Inner2::CPtr c3 = i->end_opC(c2, i->begin_opC(c1));

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        cseq1.push_back(c2);
        cseq1.push_back(c3);

        Test::Inner::Inner2::CSeq cseq2;
        Test::Inner::Inner2::CSeq cseq3 = i->end_opCSeq(cseq2, i->begin_opCSeq(cseq1));

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        cmap1["b"] = c2;
        cmap1["c"] = c3;

        Test::Inner::Inner2::CMap cmap2;
        Test::Inner::Inner2::CMap cmap3 = i->end_opCMap(cmap2, i->begin_opCMap(cmap1));
    }
    //
    // C++ 98 type safe callbacks
    //
    {
        Test::Inner::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::Inner2::IPrx,
                             communicator->stringToProxy("i2:" + helper->getTestEndpoint()));

        IceUtil::Handle<Test::Inner::Callback> cb = new Test::Inner::Callback();

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        Test::Inner::Inner2::Callback_I_opSPtr opSCB =
            Test::Inner::Inner2::newCallback_I_opS(cb, &Test::Inner::Callback::opS, &Test::Inner::Callback::error);
        i->begin_opS(s1, opSCB);
        cb->check();

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        Test::Inner::Inner2::Callback_I_opSSeqPtr opSSeqCB =
            Test::Inner::Inner2::newCallback_I_opSSeq(cb,
                                                      &Test::Inner::Callback::opSSeq,
                                                      &Test::Inner::Callback::error);
        i->begin_opSSeq(sseq1, opSSeqCB);
        cb->check();

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        Test::Inner::Inner2::Callback_I_opSMapPtr opSMapCB =
            Test::Inner::Inner2::newCallback_I_opSMap(cb,
                                                      &Test::Inner::Callback::opSMap,
                                                      &Test::Inner::Callback::error);
        i->begin_opSMap(smap1, opSMapCB);
        cb->check();

        Test::Inner::Inner2::CPtr c1 = new Test::Inner::Inner2::C(s1);
        Test::Inner::Inner2::Callback_I_opCPtr opCCB =
            Test::Inner::Inner2::newCallback_I_opC(cb,
                                                   &Test::Inner::Callback::opC,
                                                   &Test::Inner::Callback::error);
        i->begin_opC(c1, opCCB);
        cb->check();

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        Test::Inner::Inner2::Callback_I_opCSeqPtr opCSeqCB =
            Test::Inner::Inner2::newCallback_I_opCSeq(cb,
                                                      &Test::Inner::Callback::opCSeq,
                                                      &Test::Inner::Callback::error);
        i->begin_opCSeq(cseq1, opCSeqCB);
        cb->check();

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        Test::Inner::Inner2::Callback_I_opCMapPtr opCMapCB =
            Test::Inner::Inner2::newCallback_I_opCMap(cb,
                                                      &Test::Inner::Callback::opCMap,
                                                      &Test::Inner::Callback::error);
        i->begin_opCMap(cmap1, opCMapCB);
        cb->check();
    }
#endif

    {
        Test::Inner::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::IPrx, communicator->stringToProxy("i3:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        Test::Inner::Inner2::S s2;
        Test::Inner::Inner2::S s3 = i->opS(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        Test::Inner::Inner2::SSeq sseq2;
        Test::Inner::Inner2::SSeq sseq3 = i->opSSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        Test::Inner::Inner2::SMap smap2;
        Test::Inner::Inner2::SMap smap3 = i->opSMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::Inner::Inner2::CPtr c1 = ICE_MAKE_SHARED(Test::Inner::Inner2::C, s1);
        Test::Inner::Inner2::CPtr c2;
        Test::Inner::Inner2::CPtr c3 = i->opC(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        Test::Inner::Inner2::CSeq cseq2;
        Test::Inner::Inner2::CSeq cseq3 = i->opCSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        Test::Inner::Inner2::CMap cmap2;
        Test::Inner::Inner2::CMap cmap3 = i->opCMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

#ifdef ICE_CPP11_MAPPING
    //
    // C++ 11 Future-Based Async Function
    //
    {
        Test::Inner::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::IPrx, communicator->stringToProxy("i3:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        {
            auto result = i->opSAsync(s1).get();
            test(result.returnValue == s1);
            test(result.s2 == s1);
        }

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opSSeqAsync(sseq1).get();
            test(result.returnValue == sseq1);
            test(result.s2 == sseq1);
        }

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opSMapAsync(smap1).get();
            test(result.returnValue == smap1);
            test(result.s2 == smap1);
        }

        Test::Inner::Inner2::CPtr c1 = make_shared<Test::Inner::Inner2::C>(s1);
        {
            auto result = i->opCAsync(c1).get();
            test(Ice::targetEqualTo(result.returnValue, c1));
            test(Ice::targetEqualTo(result.c2, c1));
        }

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opCSeqAsync(cseq1).get();
            test(Ice::targetEqualTo(result.returnValue[0], c1));
            test(Ice::targetEqualTo(result.c2[0], c1));
        }

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opCMapAsync(cmap1).get();
            test(Ice::targetEqualTo(result.returnValue["a"], c1));
            test(Ice::targetEqualTo(result.c2["a"], c1));
        }
    }

    //
    // C++11 Callback-Based Async Function
    //
    {
        Test::Inner::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::IPrx, communicator->stringToProxy("i3:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSAsync(s1,
                                      [&p, &s1](Test::Inner::Inner2::S s2, Test::Inner::Inner2::S s3)
                                      {
                                          test(s2 == s1);
                                          test(s3 == s1);
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSSeqAsync(sseq1,
                                         [&p, &sseq1](Test::Inner::Inner2::SSeq s2, Test::Inner::Inner2::SSeq s3)
                                         {
                                             test(s2 == sseq1);
                                             test(s3 == sseq1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSMapAsync(smap1,
                                         [&p, &smap1](Test::Inner::Inner2::SMap s2, Test::Inner::Inner2::SMap s3)
                                         {
                                             test(s2 == smap1);
                                             test(s3 == smap1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::Inner::Inner2::C>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCAsync(c1,
                                      [&p, &c1](shared_ptr<Test::Inner::Inner2::C> c2,
                                                shared_ptr<Test::Inner::Inner2::C> c3)
                                      {
                                          test(Ice::targetEqualTo(c2, c1));
                                          test(Ice::targetEqualTo(c3, c1));
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCSeqAsync(cseq1,
                                         [&p, c1](Test::Inner::Inner2::CSeq c2, Test::Inner::Inner2::CSeq c3)
                                         {
                                             test(Ice::targetEqualTo(c2[0], c1));
                                             test(Ice::targetEqualTo(c3[0], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCMapAsync(cmap1,
                                         [&p, c1](Test::Inner::Inner2::CMap c2, Test::Inner::Inner2::CMap c3)
                                         {
                                             test(Ice::targetEqualTo(c2["a"], c1));
                                             test(Ice::targetEqualTo(c3["a"], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }
#else
    //
    // C++ 98 AsyncResult API
    //
    {
        Test::Inner::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::IPrx,
                             communicator->stringToProxy("i3:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;

        Test::Inner::Inner2::S s2;
        Test::Inner::Inner2::S s3 = i->end_opS(s2, i->begin_opS(s1));

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        sseq1.push_back(s2);
        sseq1.push_back(s3);

        Test::Inner::Inner2::SSeq sseq2;
        Test::Inner::Inner2::SSeq sseq3 = i->end_opSSeq(sseq2, i->begin_opSSeq(sseq1));

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        smap1["b"] = s2;
        smap1["c"] = s3;

        Test::Inner::Inner2::SMap smap2;
        Test::Inner::Inner2::SMap smap3 = i->end_opSMap(smap2, i->begin_opSMap(smap1));

        Test::Inner::Inner2::CPtr c1 = new Test::Inner::Inner2::C(s1);
        Test::Inner::Inner2::CPtr c2;
        Test::Inner::Inner2::CPtr c3 = i->end_opC(c2, i->begin_opC(c1));

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        cseq1.push_back(c2);
        cseq1.push_back(c3);

        Test::Inner::Inner2::CSeq cseq2;
        Test::Inner::Inner2::CSeq cseq3 = i->end_opCSeq(cseq2, i->begin_opCSeq(cseq1));

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        cmap1["b"] = c2;
        cmap1["c"] = c3;

        Test::Inner::Inner2::CMap cmap2;
        Test::Inner::Inner2::CMap cmap3 = i->end_opCMap(cmap2, i->begin_opCMap(cmap1));
    }
    //
    // C++ 98 type safe callbacks
    //
    {
        Test::Inner::IPrxPtr i =
            ICE_CHECKED_CAST(Test::Inner::IPrx,
                             communicator->stringToProxy("i3:" + helper->getTestEndpoint()));

        IceUtil::Handle<Test::Inner::Callback> cb = new Test::Inner::Callback();

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        Test::Inner::Callback_I_opSPtr opSCB =
            Test::Inner::newCallback_I_opS(cb,
                                           &Test::Inner::Callback::opS,
                                           &Test::Inner::Callback::error);
        i->begin_opS(s1, opSCB);
        cb->check();

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        Test::Inner::Callback_I_opSSeqPtr opSSeqCB =
            Test::Inner::newCallback_I_opSSeq(cb,
                                              &Test::Inner::Callback::opSSeq,
                                              &Test::Inner::Callback::error);
        i->begin_opSSeq(sseq1, opSSeqCB);
        cb->check();

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        Test::Inner::Callback_I_opSMapPtr opSMapCB =
            Test::Inner::newCallback_I_opSMap(cb,
                                              &Test::Inner::Callback::opSMap,
                                              &Test::Inner::Callback::error);
        i->begin_opSMap(smap1, opSMapCB);
        cb->check();

        Test::Inner::Inner2::CPtr c1 = new Test::Inner::Inner2::C(s1);
        Test::Inner::Callback_I_opCPtr opCCB =
            Test::Inner::newCallback_I_opC(cb,
                                           &Test::Inner::Callback::opC,
                                           &Test::Inner::Callback::error);
        i->begin_opC(c1, opCCB);
        cb->check();

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        Test::Inner::Callback_I_opCSeqPtr opCSeqCB =
            Test::Inner::newCallback_I_opCSeq(cb,
                                              &Test::Inner::Callback::opCSeq,
                                              &Test::Inner::Callback::error);
        i->begin_opCSeq(cseq1, opCSeqCB);
        cb->check();

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        Test::Inner::Callback_I_opCMapPtr opCMapCB =
            Test::Inner::newCallback_I_opCMap(cb,
                                              &Test::Inner::Callback::opCMap,
                                              &Test::Inner::Callback::error);
        i->begin_opCMap(cmap1, opCMapCB);
        cb->check();
    }
#endif

    {
        Inner::Test::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Inner::Test::Inner2::IPrx, communicator->stringToProxy("i4:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        Test::S s2;
        Test::S s3 = i->opS(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        Test::SSeq sseq2;
        Test::SSeq sseq3 = i->opSSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::SMap smap1;
        smap1["a"] = s1;
        Test::SMap smap2;
        Test::SMap smap3 = i->opSMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::CPtr c1 = ICE_MAKE_SHARED(Test::C, s1);
        Test::CPtr c2;
        Test::CPtr c3 = i->opC(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        Test::CSeq cseq2;
        Test::CSeq cseq3 = i->opCSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::CMap cmap1;
        cmap1["a"] = c1;
        Test::CMap cmap2;
        Test::CMap cmap3 = i->opCMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

#ifdef ICE_CPP11_MAPPING
    //
    // C++ 11 Future-Based Async Function
    //
    {
        Inner::Test::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Inner::Test::Inner2::IPrx, communicator->stringToProxy("i4:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        {
            auto result = i->opSAsync(s1).get();
            test(result.returnValue == s1);
            test(result.s2 == s1);
        }

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opSSeqAsync(sseq1).get();
            test(result.returnValue == sseq1);
            test(result.s2 == sseq1);
        }

        Test::SMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opSMapAsync(smap1).get();
            test(result.returnValue == smap1);
            test(result.s2 == smap1);
        }

        Test::CPtr c1 = make_shared<Test::C>(s1);
        {
            auto result = i->opCAsync(c1).get();
            test(Ice::targetEqualTo(result.returnValue, c1));
            test(Ice::targetEqualTo(result.c2, c1));
        }

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opCSeqAsync(cseq1).get();
            test(Ice::targetEqualTo(result.returnValue[0], c1));
            test(Ice::targetEqualTo(result.c2[0], c1));
        }

        Test::CMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opCMapAsync(cmap1).get();
            test(Ice::targetEqualTo(result.returnValue["a"], c1));
            test(Ice::targetEqualTo(result.c2["a"], c1));
        }
    }

    //
    // C++11 Callback-Based Async Function
    //
    {
        Inner::Test::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Inner::Test::Inner2::IPrx, communicator->stringToProxy("i4:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSAsync(s1,
                                      [&p, &s1](Test::S s2, Test::S s3)
                                      {
                                          test(s2 == s1);
                                          test(s3 == s1);
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSSeqAsync(sseq1,
                                         [&p, &sseq1](Test::SSeq s2, Test::SSeq s3)
                                         {
                                             test(s2 == sseq1);
                                             test(s3 == sseq1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::SMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSMapAsync(smap1,
                                         [&p, &smap1](Test::SMap s2, Test::SMap s3)
                                         {
                                             test(s2 == smap1);
                                             test(s3 == smap1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::C>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCAsync(c1,
                                      [&p, &c1](shared_ptr<Test::C> c2,
                                                shared_ptr<Test::C> c3)
                                      {
                                          test(Ice::targetEqualTo(c2, c1));
                                          test(Ice::targetEqualTo(c3, c1));
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCSeqAsync(cseq1,
                                         [&p, c1](Test::CSeq c2, Test::CSeq c3)
                                         {
                                             test(Ice::targetEqualTo(c2[0], c1));
                                             test(Ice::targetEqualTo(c3[0], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCMapAsync(cmap1,
                                         [&p, c1](Test::CMap c2, Test::CMap c3)
                                         {
                                             test(Ice::targetEqualTo(c2["a"], c1));
                                             test(Ice::targetEqualTo(c3["a"], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }
#else
    //
    // C++ 98 AsyncResult API
    //
    {
        Inner::Test::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Inner::Test::Inner2::IPrx,
                             communicator->stringToProxy("i4:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;

        Test::S s2;
        Test::S s3 = i->end_opS(s2, i->begin_opS(s1));

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        sseq1.push_back(s2);
        sseq1.push_back(s3);

        Test::SSeq sseq2;
        Test::SSeq sseq3 = i->end_opSSeq(sseq2, i->begin_opSSeq(sseq1));

        Test::SMap smap1;
        smap1["a"] = s1;
        smap1["b"] = s2;
        smap1["c"] = s3;

        Test::SMap smap2;
        Test::SMap smap3 = i->end_opSMap(smap2, i->begin_opSMap(smap1));

        Test::CPtr c1 = new Test::C(s1);
        Test::CPtr c2;
        Test::CPtr c3 = i->end_opC(c2, i->begin_opC(c1));

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        cseq1.push_back(c2);
        cseq1.push_back(c3);

        Test::CSeq cseq2;
        Test::CSeq cseq3 = i->end_opCSeq(cseq2, i->begin_opCSeq(cseq1));

        Test::CMap cmap1;
        cmap1["a"] = c1;
        cmap1["b"] = c2;
        cmap1["c"] = c3;

        Test::CMap cmap2;
        Test::CMap cmap3 = i->end_opCMap(cmap2, i->begin_opCMap(cmap1));
    }
    //
    // C++ 98 type safe callbacks
    //
    {
        Inner::Test::Inner2::IPrxPtr i =
            ICE_CHECKED_CAST(Inner::Test::Inner2::IPrx,
                             communicator->stringToProxy("i4:" + helper->getTestEndpoint()));

        IceUtil::Handle<Inner::Test::Inner2::Callback> cb = new Inner::Test::Inner2::Callback();

        Test::S s1;
        s1.v = 0;
        Inner::Test::Inner2::Callback_I_opSPtr opSCB =
            Inner::Test::Inner2::newCallback_I_opS(cb,
                                                   &Inner::Test::Inner2::Callback::opS,
                                                   &Inner::Test::Inner2::Callback::error);
        i->begin_opS(s1, opSCB);
        cb->check();

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        Inner::Test::Inner2::Callback_I_opSSeqPtr opSSeqCB =
            Inner::Test::Inner2::newCallback_I_opSSeq(cb,
                                                      &Inner::Test::Inner2::Callback::opSSeq,
                                                      &Inner::Test::Inner2::Callback::error);
        i->begin_opSSeq(sseq1, opSSeqCB);
        cb->check();

        Test::SMap smap1;
        smap1["a"] = s1;
        Inner::Test::Inner2::Callback_I_opSMapPtr opSMapCB =
            Inner::Test::Inner2::newCallback_I_opSMap(cb,
                                                      &Inner::Test::Inner2::Callback::opSMap,
                                                      &Inner::Test::Inner2::Callback::error);
        i->begin_opSMap(smap1, opSMapCB);
        cb->check();

        Test::CPtr c1 = new Test::C(s1);
        Inner::Test::Inner2::Callback_I_opCPtr opCCB =
            Inner::Test::Inner2::newCallback_I_opC(cb,
                                                   &Inner::Test::Inner2::Callback::opC,
                                                   &Inner::Test::Inner2::Callback::error);
        i->begin_opC(c1, opCCB);
        cb->check();

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        Inner::Test::Inner2::Callback_I_opCSeqPtr opCSeqCB =
            Inner::Test::Inner2::newCallback_I_opCSeq(cb,
                                                      &Inner::Test::Inner2::Callback::opCSeq,
                                                      &Inner::Test::Inner2::Callback::error);
        i->begin_opCSeq(cseq1, opCSeqCB);
        cb->check();

        Test::CMap cmap1;
        cmap1["a"] = c1;
        Inner::Test::Inner2::Callback_I_opCMapPtr opCMapCB =
            Inner::Test::Inner2::newCallback_I_opCMap(cb,
                                                      &Inner::Test::Inner2::Callback::opCMap,
                                                      &Inner::Test::Inner2::Callback::error);
        i->begin_opCMap(cmap1, opCMapCB);
        cb->check();
    }
#endif

    Test::IPrxPtr i = ICE_CHECKED_CAST(Test::IPrx, communicator->stringToProxy("i1:" + helper->getTestEndpoint()));
    i->shutdown();
}
