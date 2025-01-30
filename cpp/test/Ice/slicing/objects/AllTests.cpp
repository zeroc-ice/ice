// Copyright (c) ZeroC, Inc.

#include "ClientPrivate.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

namespace
{
    void breakCycles(const Ice::ValuePtr&);

    template<typename T> void breakCycles(const vector<shared_ptr<T>>& s)
    {
        for (const auto& e : s)
        {
            breakCycles(e);
        }
    }

    template<typename K, typename V> void breakCycles(const map<K, shared_ptr<V>>& d)
    {
        for (const auto& e : d)
        {
            breakCycles(e.second);
        }
    }

    void breakCycles(const Ice::ValuePtr& o)
    {
        if (dynamic_pointer_cast<D1>(o))
        {
            auto d1 = dynamic_pointer_cast<D1>(o);
            auto tmp = d1->pd1;
            d1->pd1 = nullptr;
            if (tmp != d1)
            {
                breakCycles(tmp);
            }
        }
        if (dynamic_pointer_cast<D3>(o))
        {
            auto d3 = dynamic_pointer_cast<D3>(o);
            auto tmp = d3->pd3;
            d3->pd3 = nullptr;
            if (tmp != d3)
            {
                breakCycles(tmp);
            }
        }
        if (dynamic_pointer_cast<B>(o))
        {
            auto b = dynamic_pointer_cast<B>(o);
            auto tmp = b->pb;
            b->pb = nullptr;
            if (tmp != b)
            {
                breakCycles(tmp);
            }
            if (b->ice_getSlicedData())
            {
                b->ice_getSlicedData()->clear();
            }
        }
        if (dynamic_pointer_cast<PDerived>(o))
        {
            auto p = dynamic_pointer_cast<PDerived>(o);
            p->pb = nullptr;
        }
        if (dynamic_pointer_cast<CompactPDerived>(o))
        {
            auto p = dynamic_pointer_cast<CompactPDerived>(o);
            p->pb = nullptr;
        }
        if (dynamic_pointer_cast<PCDerived>(o))
        {
            auto p = dynamic_pointer_cast<PCDerived>(o);
            auto seq(p->pbs);
            p->pbs.clear();
            breakCycles(seq);
        }
        if (dynamic_pointer_cast<CompactPCDerived>(o))
        {
            auto p = dynamic_pointer_cast<CompactPCDerived>(o);
            auto seq(p->pbs);
            p->pbs.clear();
            breakCycles(seq);
        }
        if (dynamic_pointer_cast<PCDerived3>(o))
        {
            auto p = dynamic_pointer_cast<PCDerived3>(o);
            p->pcd3 = nullptr;
        }
        if (dynamic_pointer_cast<PNode>(o))
        {
            auto curr = dynamic_pointer_cast<PNode>(o);
            while (curr && o != curr->next)
            {
                auto next = curr->next;
                curr->next = nullptr;
                curr = next;
            }
        }
        if (dynamic_pointer_cast<SS1>(o))
        {
            auto s = dynamic_pointer_cast<SS1>(o);
            breakCycles(s->s);
        }
        if (dynamic_pointer_cast<SS2>(o))
        {
            auto s = dynamic_pointer_cast<SS2>(o);
            breakCycles(s->s);
        }
        if (dynamic_pointer_cast<Forward>(o))
        {
            auto f = dynamic_pointer_cast<Forward>(o);
            f->h = nullptr;
        }
    }

    class CallbackBase
    {
    public:
        CallbackBase() = default;

        virtual ~CallbackBase() = default;

        void check()
        {
            unique_lock lock(_mutex);
            _condition.wait(lock, [this] { return _called; });
            _called = false;
        }

    protected:
        void called()
        {
            lock_guard lock(_mutex);
            assert(!_called);
            _called = true;
            _condition.notify_one();
        }

    private:
        bool _called{false};
        mutex _mutex;
        condition_variable _condition;
    };

    class Callback : public CallbackBase
    {
    public:
        void response_SBaseAsObject(const Ice::ValuePtr& o)
        {
            test(o);
            test(string{o->ice_id()} == "::Test::SBase");
            SBasePtr sb = dynamic_pointer_cast<SBase>(o);
            test(sb);
            test(sb->sb == "SBase.sb");
            called();
        }

        void response_SBaseAsSBase(const SBasePtr& sb)
        {
            test(sb->sb == "SBase.sb");
            breakCycles(sb);
            called();
        }

        void response_SBSKnownDerivedAsSBase(const SBasePtr& sb)
        {
            SBSKnownDerivedPtr sbskd = dynamic_pointer_cast<SBSKnownDerived>(sb);
            test(sbskd);
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
            breakCycles(sbskd);
            called();
        }

        void response_SBSKnownDerivedAsSBSKnownDerived(const SBSKnownDerivedPtr& sbskd)
        {
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
            breakCycles(sbskd);
            called();
        }

        void response_SBSUnknownDerivedAsSBase(const SBasePtr& sb)
        {
            test(sb->sb == "SBSUnknownDerived.sb");
            breakCycles(sb);
            called();
        }

        void response_SBSUnknownDerivedAsSBaseCompact(const SBasePtr&) { test(false); }

        void response_SUnknownAsObject10(const Ice::ValuePtr&) { test(false); }

        void response_SUnknownAsObject11(const Ice::ValuePtr& o)
        {
            test(dynamic_pointer_cast<Ice::UnknownSlicedValue>(o));
            test(string{dynamic_pointer_cast<Ice::UnknownSlicedValue>(o)->ice_id()} == "::Test::SUnknown");
            called();
            dynamic_pointer_cast<Ice::UnknownSlicedValue>(o)->ice_getSlicedData()->clear();
        }

        void exception_SUnknownAsObject11(const Ice::Exception&) { test(false); }

        void response_oneElementCycle(const BPtr& b)
        {
            test(b);
            test(string{b->ice_id()} == "::Test::B");
            test(b->sb == "B1.sb");
            test(b->pb == b);
            breakCycles(b);
            called();
        }

        void response_twoElementCycle(const BPtr& b1)
        {
            test(b1);
            test(string{b1->ice_id()} == "::Test::B");
            test(b1->sb == "B1.sb");

            BPtr b2 = b1->pb;
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "B2.sb");
            test(b2->pb == b1);
            breakCycles(b1);
            called();
        }

        void response_D1AsB(const BPtr& b1)
        {
            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb);
            test(b1->pb != b1);
            D1Ptr d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1);
            test(d1->pd1 != b1);
            test(b1->pb == d1->pd1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->pb == b1);
            test(b2->sb == "D2.sb");
            test(string{b2->ice_id()} == "::Test::B");
            breakCycles(b1);
            called();
        }

        void response_D1AsD1(const D1Ptr& d1)
        {
            test(d1);
            test(string{d1->ice_id()} == "::Test::D1");
            test(d1->sb == "D1.sb");
            test(d1->pb);
            test(d1->pb != d1);

            BPtr b2 = d1->pb;
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb == d1);
            breakCycles(d1);
            called();
        }

        void response_D2AsB(const BPtr& b2)
        {
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb);
            test(b2->pb != b2);

            BPtr b1 = b2->pb;
            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            breakCycles(b2);
            breakCycles(d1);

            called();
        }

        void response_paramTest1(const BPtr& b1, const BPtr& b2)
        {
            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(string{b2->ice_id()} == "::Test::B"); // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);
            breakCycles(b1);
            breakCycles(b2);
            called();
        }

        void response_returnTest1(const BPtr& r, const BPtr& p1, const BPtr& p2)
        {
            test(r == p1);
            breakCycles(r);
            breakCycles(p2);
            called();
        }

        void response_returnTest2(const BPtr& r, const BPtr& p1, const BPtr& p2)
        {
            test(r == p1);
            breakCycles(r);
            breakCycles(p2);
            called();
        }

        void response_returnTest3(const BPtr& b)
        {
            rb = b;
            called();
        }

        void response_paramTest3(const BPtr& ret, const BPtr& p1, const BPtr& p2)
        {
            test(p1);
            test(p1->sb == "D2.sb (p1 1)");
            test(p1->pb == nullptr);
            test(string{p1->ice_id()} == "::Test::B");

            test(p2);
            test(p2->sb == "D2.sb (p2 1)");
            test(p2->pb == nullptr);
            test(string{p2->ice_id()} == "::Test::B");

            test(ret);
            test(ret->sb == "D1.sb (p2 2)");
            test(ret->pb == nullptr);
            test(string{ret->ice_id()} == "::Test::D1");
            called();

            breakCycles(ret);
            breakCycles(p1);
            breakCycles(p2);
        }

        void response_paramTest4(const BPtr& ret, const BPtr& b)
        {
            test(b);
            test(b->sb == "D4.sb (1)");
            test(b->pb == nullptr);
            test(string{b->ice_id()} == "::Test::B");

            test(ret);
            test(ret->sb == "B.sb (2)");
            test(ret->pb == nullptr);
            test(string{ret->ice_id()} == "::Test::B");
            called();

            breakCycles(ret);
            breakCycles(b);
        }

        void response_sequenceTest(const SS3& ss)
        {
            rss3 = ss;
            called();
        }

        void response_dictionaryTest(const BDict& r, const BDict& bout)
        {
            rbdict = r;
            obdict = bout;
            called();
        }

        void exception_throwBaseAsBase(const Ice::Exception& ex)
        {
            test(string{ex.ice_id()} == "::Test::BaseException");
            const auto& e = dynamic_cast<const BaseException&>(ex);
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb");
            test(e.pb->pb == e.pb);
            called();
            breakCycles(e.pb);
        }

        void exception_throwDerivedAsBase(const Ice::Exception& ex)
        {
            test(string{ex.ice_id()} == "::Test::DerivedException");
            const auto& e = dynamic_cast<const DerivedException&>(ex);
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
            called();
        }

        void exception_throwDerivedAsDerived(const Ice::Exception& ex)
        {
            test(string{ex.ice_id()} == "::Test::DerivedException");
            const auto& e = dynamic_cast<const DerivedException&>(ex);
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
            called();
        }

        void exception_throwUnknownDerivedAsBase(const Ice::Exception& ex)
        {
            test(string{ex.ice_id()} == "::Test::BaseException");
            const auto& e = dynamic_cast<const BaseException&>(ex);
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb d2");
            test(e.pb->pb == e.pb);
            called();
            breakCycles(e.pb);
        }

        void response_preserved1(const PBasePtr& r)
        {
            PDerivedPtr pd = dynamic_pointer_cast<PDerived>(r);
            test(pd);
            test(pd->pi == 3);
            test(pd->ps == "preserved");
            test(pd->pb == pd);
            called();

            breakCycles(r);
        }

        void response_preserved2(const PBasePtr& r)
        {
            PCUnknownPtr p2 = dynamic_pointer_cast<PCUnknown>(r);
            test(!p2);
            test(r->pi == 3);
            called();
        }

        void response_preserved3(const PBasePtr& r)
        {
            //
            // Encoding 1.0
            //
            PCDerivedPtr p2 = dynamic_pointer_cast<PCDerived>(r);
            test(!p2);
            test(r->pi == 3);
            called();

            breakCycles(r);
        }

        void response_preserved4(const PBasePtr& r)
        {
            //
            // Encoding > 1.0
            //
            PCDerivedPtr p2 = dynamic_pointer_cast<PCDerived>(r);
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
            called();

            breakCycles(r);
        }

        void response_preserved5(const PBasePtr& r)
        {
            PCDerived3Ptr p3 = dynamic_pointer_cast<PCDerived3>(r);
            test(p3);
            test(p3->pi == 3);
            for (int i = 0; i < 300; ++i)
            {
                PCDerived2Ptr p2 = dynamic_pointer_cast<PCDerived2>(p3->pbs[static_cast<size_t>(i)]);
                test(p2->pi == i);
                test(p2->pbs.size() == 1);
                test(!p2->pbs[0]);
                test(p2->pcd2 == i);
            }
            test(p3->pcd2 == p3->pi);
            test(p3->pcd3 == p3->pbs[10]);
            called();

            breakCycles(r);
        }

        void response_compactPreserved1(const PBasePtr& r)
        {
            //
            // Encoding 1.0
            //
            CompactPCDerivedPtr p2 = dynamic_pointer_cast<CompactPCDerived>(r);
            test(!p2);
            test(r->pi == 3);
            called();

            breakCycles(r);
        }

        void response_compactPreserved2(const PBasePtr& r)
        {
            //
            // Encoding > 1.0
            //
            CompactPCDerivedPtr p2 = dynamic_pointer_cast<CompactPCDerived>(r);
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
            called();

            breakCycles(r);
        }

        void response() { test(false); }

        void exception(const Ice::Exception& ex)
        {
            if (!dynamic_cast<const Ice::OperationNotExistException*>(&ex))
            {
                test(false);
            }
            else
            {
                called();
            }
        }

        BPtr rb;
        SS3 rss3;
        BDict rbdict;
        BDict obdict;
    };
    using CallbackPtr = std::shared_ptr<Callback>;

    void testUOO(const TestIntfPrx& testPrx)
    {
        Ice::ValuePtr o;
        try
        {
            o = testPrx->SUnknownAsObject();
            test(testPrx->ice_getEncodingVersion() != Ice::Encoding_1_0);
            test(dynamic_pointer_cast<Ice::UnknownSlicedValue>(o));
            test(string{dynamic_pointer_cast<Ice::UnknownSlicedValue>(o)->ice_id()} == "::Test::SUnknown");
            test(dynamic_pointer_cast<Ice::UnknownSlicedValue>(o)->ice_getSlicedData());
            testPrx->checkSUnknown(o);
            dynamic_pointer_cast<Ice::UnknownSlicedValue>(o)->ice_getSlicedData()->clear();
        }
        catch (const Ice::MarshalException&)
        {
            test(testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0);
        }
        catch (const std::exception& ex)
        {
            cout << ex.what() << endl;
            test(false);
        }
        catch (...)
        {
            test(false);
        }
    }
}

TestIntfPrx
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    TestIntfPrx testPrx(communicator, "Test:" + helper->getTestEndpoint());

    cout << "base as Object... " << flush;
    {
        Ice::ValuePtr o;
        try
        {
            o = testPrx->SBaseAsObject();
            test(o);
            test(string{o->ice_id()} == "::Test::SBase");
        }
        catch (const std::exception& ex)
        {
            cerr << ex.what() << endl;
            test(false);
        }
        catch (...)
        {
            test(false);
        }
        SBasePtr sb = dynamic_pointer_cast<SBase>(o);
        test(sb);
        test(sb->sb == "SBase.sb");
    }
    cout << "ok" << endl;

    cout << "base as Object (AMI)... " << flush;
    {
        auto f = testPrx->SBaseAsObjectAsync();
        try
        {
            auto o = f.get();
            test(o);
            test(string{o->ice_id()} == "::Test::SBase");
            auto sb = dynamic_pointer_cast<SBase>(o);
            test(sb);
            test(sb->sb == "SBase.sb");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base as base... " << flush;
    {
        SBasePtr sb;
        try
        {
            sb = testPrx->SBaseAsSBase();
            test(sb->sb == "SBase.sb");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base as base (AMI)... " << flush;
    {
        try
        {
            auto sb = testPrx->SBaseAsSBaseAsync().get();
            test(sb->sb == "SBase.sb");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base with known derived as base... " << flush;
    {
        SBasePtr sb;
        try
        {
            sb = testPrx->SBSKnownDerivedAsSBase();
            test(sb->sb == "SBSKnownDerived.sb");
        }
        catch (...)
        {
            test(false);
        }
        SBSKnownDerivedPtr sbskd = dynamic_pointer_cast<SBSKnownDerived>(sb);
        test(sbskd);
        test(sbskd->sbskd == "SBSKnownDerived.sbskd");
    }
    cout << "ok" << endl;

    cout << "base with known derived as base (AMI)... " << flush;
    {
        auto f = testPrx->SBSKnownDerivedAsSBaseAsync();
        try
        {
            shared_ptr<SBase> sb = f.get();
            sb = testPrx->SBSKnownDerivedAsSBase();
            test(sb->sb == "SBSKnownDerived.sb");
            SBSKnownDerivedPtr sbskd = dynamic_pointer_cast<SBSKnownDerived>(sb);
            test(sbskd);
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base with known derived as known derived... " << flush;
    {
        SBSKnownDerivedPtr sbskd;
        try
        {
            sbskd = testPrx->SBSKnownDerivedAsSBSKnownDerived();
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base with known derived as known derived (AMI)... " << flush;
    {
        auto f = testPrx->SBSKnownDerivedAsSBSKnownDerivedAsync();
        try
        {
            SBSKnownDerivedPtr sbskd = testPrx->SBSKnownDerivedAsSBSKnownDerived();
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base with unknown derived as base... " << flush;
    {
        SBasePtr sb;
        try
        {
            sb = testPrx->SBSUnknownDerivedAsSBase();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch (...)
        {
            test(false);
        }
    }
    if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
    {
        try
        {
            //
            // This test succeeds for the 1.0 encoding.
            //
            SBasePtr sb = testPrx->SBSUnknownDerivedAsSBaseCompact();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
        catch (...)
        {
            test(false);
        }
    }
    else
    {
        try
        {
            //
            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            //
            SBasePtr sb = testPrx->SBSUnknownDerivedAsSBaseCompact();
            test(false);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
        catch (const Ice::MarshalException&)
        {
            // Expected.
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base with unknown derived as base (AMI)... " << flush;
    {
        auto f = testPrx->SBSUnknownDerivedAsSBaseAsync();
        try
        {
            SBasePtr sb = f.get();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch (...)
        {
            test(false);
        }
    }
    if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
    {
        //
        // This test succeeds for the 1.0 encoding.
        //
        auto f = testPrx->SBSUnknownDerivedAsSBaseCompactAsync();
        try
        {
            SBasePtr sb = f.get();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch (...)
        {
            test(false);
        }
    }
    else
    {
        //
        // This test fails when using the compact format because the instance cannot
        // be sliced to a known type.
        //
        auto f = testPrx->SBSUnknownDerivedAsSBaseCompactAsync();
        try
        {
            f.get();
            test(false);
        }
        catch (const Ice::MarshalException&)
        {
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "unknown with Object as Object... " << flush;
    {
        testUOO(testPrx);
    }
    cout << "ok" << endl;

    cout << "unknown with Object as Object (AMI)... " << flush;
    {
        try
        {
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                auto f = testPrx->SUnknownAsObjectAsync();
                try
                {
                    f.get();
                    test(false);
                }
                catch (const Ice::MarshalException&)
                {
                }
                catch (...)
                {
                    test(false);
                }
            }
            else
            {
                auto f = testPrx->SUnknownAsObjectAsync();
                try
                {
                    Ice::ValuePtr v = f.get();
                    test(dynamic_pointer_cast<Ice::UnknownSlicedValue>(v));
                    test(string{dynamic_pointer_cast<Ice::UnknownSlicedValue>(v)->ice_id()} == "::Test::SUnknown");
                    dynamic_pointer_cast<Ice::UnknownSlicedValue>(v)->ice_getSlicedData()->clear();
                }
                catch (...)
                {
                    test(false);
                }
            }
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "one-element cycle... " << flush;
    {
        try
        {
            BPtr b = testPrx->oneElementCycle();
            test(b);
            test(string{b->ice_id()} == "::Test::B");
            test(b->sb == "B1.sb");
            test(b->pb == b);

            breakCycles(b);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "one-element cycle (AMI)... " << flush;
    {
        auto f = testPrx->oneElementCycleAsync();
        try
        {
            auto b = f.get();
            test(b);
            test(string{b->ice_id()} == "::Test::B");
            test(b->sb == "B1.sb");
            test(b->pb == b);

            breakCycles(b);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "two-element cycle... " << flush;
    {
        try
        {
            BPtr b1 = testPrx->twoElementCycle();
            test(b1);
            test(string{b1->ice_id()} == "::Test::B");
            test(b1->sb == "B1.sb");

            BPtr b2 = b1->pb;
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "B2.sb");
            test(b2->pb == b1);

            breakCycles(b1);
            breakCycles(b2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "two-element cycle (AMI)... " << flush;
    {
        auto f = testPrx->twoElementCycleAsync();
        try
        {
            auto b1 = f.get();
            test(b1);
            test(string{b1->ice_id()} == "::Test::B");
            test(b1->sb == "B1.sb");

            auto b2 = b1->pb;
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "B2.sb");
            test(b2->pb == b1);

            breakCycles(b1);
            breakCycles(b2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "known derived pointer slicing as base... " << flush;
    {
        try
        {
            BPtr b1;
            b1 = testPrx->D1AsB();
            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb);
            test(b1->pb != b1);
            D1Ptr d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1);
            test(d1->pd1 != b1);
            test(b1->pb == d1->pd1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->pb == b1);
            test(b2->sb == "D2.sb");
            test(string{b2->ice_id()} == "::Test::B");

            breakCycles(b1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "known derived pointer slicing as base (AMI)... " << flush;
    {
        auto f = testPrx->D1AsBAsync();
        try
        {
            auto b1 = f.get();
            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb);
            test(b1->pb != b1);
            auto d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1);
            test(d1->pd1 != b1);
            test(b1->pb == d1->pd1);

            auto b2 = b1->pb;
            test(b2);
            test(b2->pb == b1);
            test(b2->sb == "D2.sb");
            test(string{b2->ice_id()} == "::Test::B");

            breakCycles(b1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "known derived pointer slicing as derived... " << flush;
    {
        try
        {
            D1Ptr d1;
            d1 = testPrx->D1AsD1();
            test(d1);
            test(string{d1->ice_id()} == "::Test::D1");
            test(d1->sb == "D1.sb");
            test(d1->pb);
            test(d1->pb != d1);

            BPtr b2 = d1->pb;
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb == d1);

            breakCycles(d1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "known derived pointer slicing as derived (AMI)... " << flush;
    {
        auto f = testPrx->D1AsD1Async();
        try
        {
            auto d1 = f.get();
            test(d1);
            test(string{d1->ice_id()} == "::Test::D1");
            test(d1->sb == "D1.sb");
            test(d1->pb);
            test(d1->pb != d1);

            auto b2 = d1->pb;
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb == d1);

            breakCycles(d1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "unknown derived pointer slicing as base... " << flush;
    {
        try
        {
            BPtr b2;
            b2 = testPrx->D2AsB();
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb);
            test(b2->pb != b2);

            BPtr b1 = b2->pb;
            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            breakCycles(b2);
            breakCycles(d1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "unknown derived pointer slicing as base (AMI)... " << flush;
    {
        auto f = testPrx->D2AsBAsync();
        try
        {
            auto b2 = f.get();
            test(b2);
            test(string{b2->ice_id()} == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb);
            test(b2->pb != b2);

            auto b1 = b2->pb;
            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            auto d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            breakCycles(b2);
            breakCycles(d1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing with known first... " << flush;
    {
        try
        {
            BPtr b1;
            BPtr b2;
            testPrx->paramTest1(b1, b2);

            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(string{b2->ice_id()} == "::Test::B"); // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);

            breakCycles(b1);
            breakCycles(b2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing with known first (AMI)... " << flush;
    {
        auto f = testPrx->paramTest1Async();
        try
        {
            auto result = f.get();
            auto b1 = std::move(std::get<0>(result));
            auto b2 = std::move(std::get<1>(result));

            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(string{b2->ice_id()} == "::Test::B"); // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);

            breakCycles(b1);
            breakCycles(b2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing with unknown first... " << flush;
    {
        try
        {
            BPtr b2;
            BPtr b1;
            testPrx->paramTest2(b2, b1);

            test(b1);
            test(string{b1->ice_id()} == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(string{b2->ice_id()} == "::Test::B"); // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);

            breakCycles(b1);
            breakCycles(b2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity with known first... " << flush;
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr r = testPrx->returnTest1(p1, p2);
            test(r == p1);

            breakCycles(r);
            breakCycles(p2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity with known first (AMI)... " << flush;
    {
        auto f = testPrx->returnTest1Async();
        try
        {
            auto result = f.get();
            test(std::get<0>(result) == std::get<1>(result));

            breakCycles(std::get<0>(result));
            breakCycles(std::get<1>(result));
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity with unknown first... " << flush;
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr r = testPrx->returnTest2(p1, p2);
            test(r == p1);

            breakCycles(r);
            breakCycles(p2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity with unknown first (AMI)... " << flush;
    {
        auto f = testPrx->returnTest2Async();
        try
        {
            auto result = f.get();
            test(std::get<0>(result) == std::get<1>(result));

            breakCycles(std::get<0>(result));
            breakCycles(std::get<1>(result));
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity for input params known first... " << flush;
    {
        try
        {
            D1Ptr d1 = std::make_shared<D1>();
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = std::make_shared<D3>();
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;

            BPtr b1 = testPrx->returnTest3(d1, d3);

            test(b1);
            test(b1->sb == "D1.sb");
            test(string{b1->ice_id()} == "::Test::D1");
            D1Ptr p1 = dynamic_pointer_cast<D1>(b1);
            test(p1);
            test(p1->sd1 == "D1.sd1");
            test(p1->pd1 == b1->pb);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D3.sb");
            test(b2->pb == b1);
            D3Ptr p3 = dynamic_pointer_cast<D3>(b2);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p3);
            }
            else
            {
                test(p3);
                test(p3->pd3 == p1);
                test(p3->sd3 == "D3.sd3");
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);

            breakCycles(b1);
            breakCycles(d1);
            breakCycles(d3);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity for input params known first (AMI)... " << flush;
    {
        try
        {
            D1Ptr d1 = std::make_shared<D1>();
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = std::make_shared<D3>();
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;

            auto f = testPrx->returnTest3Async(d1, d3);
            auto b1 = f.get();

            test(b1);
            test(b1->sb == "D1.sb");
            test(string{b1->ice_id()} == "::Test::D1");
            D1Ptr p1 = dynamic_pointer_cast<D1>(b1);
            test(p1);
            test(p1->sd1 == "D1.sd1");
            test(p1->pd1 == b1->pb);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D3.sb");
            test(b2->pb == b1);
            D3Ptr p3 = dynamic_pointer_cast<D3>(b2);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p3);
            }
            else
            {
                test(p3);
                test(p3->pd3 == p1);
                test(p3->sd3 == "D3.sd3");
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);

            breakCycles(b1);
            breakCycles(d1);
            breakCycles(d3);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity for input params unknown first... " << flush;
    {
        try
        {
            D1Ptr d1 = std::make_shared<D1>();
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = std::make_shared<D3>();
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;

            BPtr b1 = testPrx->returnTest3(d3, d1);

            test(b1);
            test(b1->sb == "D3.sb");

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D1.sb");
            test(string{b2->ice_id()} == "::Test::D1");
            test(b2->pb == b1);

            D1Ptr p3 = dynamic_pointer_cast<D1>(b2);
            test(p3);
            test(p3->sd1 == "D1.sd1");
            test(p3->pd1 == b1);

            D3Ptr p1 = dynamic_pointer_cast<D3>(b1);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p1);
            }
            else
            {
                test(p1);
                test(p1->sd3 == "D3.sd3");
                test(p1->pd3 == b2);
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);

            breakCycles(b1);
            breakCycles(d1);
            breakCycles(d3);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity for input params unknown first (AMI)... " << flush;
    {
        try
        {
            D1Ptr d1 = std::make_shared<D1>();
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = std::make_shared<D3>();
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;

            auto f = testPrx->returnTest3Async(d3, d1);
            auto b1 = f.get();

            test(b1);
            test(b1->sb == "D3.sb");
            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D1.sb");
            test(string{b2->ice_id()} == "::Test::D1");
            test(b2->pb == b1);
            D1Ptr p3 = dynamic_pointer_cast<D1>(b2);
            test(p3);
            test(p3->sd1 == "D1.sd1");
            test(p3->pd1 == b1);

            D3Ptr p1 = dynamic_pointer_cast<D3>(b1);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p1);
            }
            else
            {
                test(p1);
                test(p1->sd3 == "D3.sd3");
                test(p1->pd3 == b2);
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);

            breakCycles(b1);
            breakCycles(d1);
            breakCycles(d3);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "remainder unmarshaling (3 instances)... " << flush;
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr ret = testPrx->paramTest3(p1, p2);

            test(p1);
            test(p1->sb == "D2.sb (p1 1)");
            test(p1->pb == nullptr);
            test(string{p1->ice_id()} == "::Test::B");

            test(p2);
            test(p2->sb == "D2.sb (p2 1)");
            test(p2->pb == nullptr);
            test(string{p2->ice_id()} == "::Test::B");

            test(ret);
            test(ret->sb == "D1.sb (p2 2)");
            test(ret->pb == nullptr);
            test(string{ret->ice_id()} == "::Test::D1");

            breakCycles(ret);
            breakCycles(p1);
            breakCycles(p2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "remainder unmarshaling (3 instances) (AMI)... " << flush;
    {
        auto f = testPrx->paramTest3Async();
        try
        {
            auto result = f.get();
            auto ret = std::get<0>(result);
            auto p1 = std::get<1>(result);
            auto p2 = std::get<2>(result);

            test(p1);
            test(p1->sb == "D2.sb (p1 1)");
            test(p1->pb == nullptr);
            test(string{p1->ice_id()} == "::Test::B");

            test(p2);
            test(p2->sb == "D2.sb (p2 1)");
            test(p2->pb == nullptr);
            test(string{p2->ice_id()} == "::Test::B");

            test(ret);
            test(ret->sb == "D1.sb (p2 2)");
            test(ret->pb == nullptr);
            test(string{ret->ice_id()} == "::Test::D1");

            breakCycles(ret);
            breakCycles(p1);
            breakCycles(p2);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "remainder unmarshaling (4 instances)... " << flush;
    {
        try
        {
            BPtr b;
            BPtr ret = testPrx->paramTest4(b);

            test(b);
            test(b->sb == "D4.sb (1)");
            test(b->pb == nullptr);
            test(string{b->ice_id()} == "::Test::B");

            test(ret);
            test(ret->sb == "B.sb (2)");
            test(ret->pb == nullptr);
            test(string{ret->ice_id()} == "::Test::B");

            breakCycles(ret);
            breakCycles(b);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "remainder unmarshaling (4 instances) (AMI)... " << flush;
    {
        auto f = testPrx->paramTest4Async();
        try
        {
            auto result = f.get();
            auto ret = std::move(std::get<0>(result));
            auto b = std::move(std::get<1>(result));

            test(b);
            test(b->sb == "D4.sb (1)");
            test(b->pb == nullptr);
            test(string{b->ice_id()} == "::Test::B");

            test(ret);
            test(ret->sb == "B.sb (2)");
            test(ret->pb == nullptr);
            test(string{ret->ice_id()} == "::Test::B");

            breakCycles(ret);
            breakCycles(b);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as base... " << flush;
    {
        try
        {
            BPtr b1 = std::make_shared<B>();
            b1->sb = "B.sb(1)";
            b1->pb = b1;

            D3Ptr d3 = std::make_shared<D3>();
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = b1;

            BPtr b2 = std::make_shared<B>();
            b2->sb = "B.sb(2)";
            b2->pb = b1;

            BPtr r = testPrx->returnTest3(d3, b2);

            test(r);

            D3Ptr p3 = dynamic_pointer_cast<D3>(r);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p3);
            }
            else
            {
                test(p3);
                test(p3->sb == "D3.sb");
                test(p3->pb == r);
                test(p3->sd3 == "D3.sd3");

                test(string{p3->pd3->ice_id()} == "::Test::B");
                test(p3->pd3->sb == "B.sb(1)");
                test(p3->pd3->pb == p3->pd3);
            }

            breakCycles(b1);
            breakCycles(d3);
            breakCycles(r);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as base (AMI)... " << flush;
    {
        try
        {
            BPtr b1 = std::make_shared<B>();
            b1->sb = "B.sb(1)";
            b1->pb = b1;

            D3Ptr d3 = std::make_shared<D3>();
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = b1;

            BPtr b2 = std::make_shared<B>();
            b2->sb = "B.sb(2)";
            b2->pb = b1;

            auto r = testPrx->returnTest3Async(d3, b2).get();
            test(r);
            D3Ptr p3 = dynamic_pointer_cast<D3>(r);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p3);
            }
            else
            {
                test(p3);
                test(p3->sb == "D3.sb");
                test(p3->pb == r);
                test(p3->sd3 == "D3.sd3");

                test(string{p3->pd3->ice_id()} == "::Test::B");
                test(p3->pd3->sb == "B.sb(1)");
                test(p3->pd3->pb == p3->pd3);
            }

            breakCycles(b1);
            breakCycles(d3);
            breakCycles(r);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as derived... " << flush;
    {
        try
        {
            D1Ptr d11 = std::make_shared<D1>();
            d11->sb = "D1.sb(1)";
            d11->pb = d11;
            d11->sd1 = "D1.sd1(1)";

            D3Ptr d3 = std::make_shared<D3>();
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = d11;

            D1Ptr d12 = std::make_shared<D1>();
            d12->sb = "D1.sb(2)";
            d12->pb = d12;
            d12->sd1 = "D1.sd1(2)";
            d12->pd1 = d11;

            BPtr r = testPrx->returnTest3(d3, d12);
            test(r);
            test(r->sb == "D3.sb");
            test(r->pb == r);
            D3Ptr p3 = dynamic_pointer_cast<D3>(r);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p3);
            }
            else
            {
                test(p3);
                test(p3->sd3 == "D3.sd3");
                test(string{p3->pd3->ice_id()} == "::Test::D1");
            }

            breakCycles(d3);
            breakCycles(d11);
            breakCycles(d12);
            breakCycles(r);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as derived (AMI)... " << flush;
    {
        try
        {
            D1Ptr d11 = std::make_shared<D1>();
            d11->sb = "D1.sb(1)";
            d11->pb = d11;
            d11->sd1 = "D1.sd1(1)";

            D3Ptr d3 = std::make_shared<D3>();
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = d11;

            D1Ptr d12 = std::make_shared<D1>();
            d12->sb = "D1.sb(2)";
            d12->pb = d12;
            d12->sd1 = "D1.sd1(2)";
            d12->pd1 = d11;

            auto r = testPrx->returnTest3Async(d3, d12).get();
            test(r);
            test(r->sb == "D3.sb");
            test(r->pb == r);
            D3Ptr p3 = dynamic_pointer_cast<D3>(r);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p3);
            }
            else
            {
                test(p3);
                test(p3->sd3 == "D3.sd3");
                test(string{p3->pd3->ice_id()} == "::Test::D1");
            }

            breakCycles(d3);
            breakCycles(d11);
            breakCycles(d12);
            breakCycles(r);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "sequence slicing... " << flush;
    {
        try
        {
            SS3 ss;
            {
                BPtr ss1b = std::make_shared<B>();
                ss1b->sb = "B.sb";
                ss1b->pb = ss1b;

                D1Ptr ss1d1 = std::make_shared<D1>();
                ss1d1->sb = "D1.sb";
                ss1d1->sd1 = "D1.sd1";
                ss1d1->pb = ss1b;

                D3Ptr ss1d3 = std::make_shared<D3>();
                ss1d3->sb = "D3.sb";
                ss1d3->sd3 = "D3.sd3";
                ss1d3->pb = ss1b;

                BPtr ss2b = std::make_shared<B>();
                ss2b->sb = "B.sb";
                ss2b->pb = ss1b;

                D1Ptr ss2d1 = std::make_shared<D1>();
                ss2d1->sb = "D1.sb";
                ss2d1->sd1 = "D1.sd1";
                ss2d1->pb = ss2b;

                D3Ptr ss2d3 = std::make_shared<D3>();
                ss2d3->sb = "D3.sb";
                ss2d3->sd3 = "D3.sd3";
                ss2d3->pb = ss2b;

                ss1d1->pd1 = ss2b;
                ss1d3->pd3 = ss2d1;

                ss2d1->pd1 = ss1d3;
                ss2d3->pd3 = ss1d1;

                SS1Ptr ss1 = std::make_shared<SS1>();
                ss1->s.push_back(ss1b);
                ss1->s.push_back(ss1d1);
                ss1->s.push_back(ss1d3);

                SS2Ptr ss2 = std::make_shared<SS2>();
                ss2->s.push_back(ss2b);
                ss2->s.push_back(ss2d1);
                ss2->s.push_back(ss2d3);

                ss = testPrx->sequenceTest(ss1, ss2);

                breakCycles(ss1);
                breakCycles(ss2);
            }

            test(ss.c1);
            BPtr ss1b = ss.c1->s[0];
            BPtr ss1d1 = ss.c1->s[1];
            test(ss.c2);
            BPtr ss1d3 = ss.c1->s[2];

            test(ss.c2);
            BPtr ss2b = ss.c2->s[0];
            BPtr ss2d1 = ss.c2->s[1];
            BPtr ss2d3 = ss.c2->s[2];

            test(ss1b->pb == ss1b);
            test(ss1d1->pb == ss1b);
            test(ss1d3->pb == ss1b);

            test(ss2b->pb == ss1b);
            test(ss2d1->pb == ss2b);
            test(ss2d3->pb == ss2b);

            test(string{ss1b->ice_id()} == "::Test::B");
            test(string{ss1d1->ice_id()} == "::Test::D1");

            test(string{ss2b->ice_id()} == "::Test::B");
            test(string{ss2d1->ice_id()} == "::Test::D1");

            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(string{ss1d3->ice_id()} == "::Test::B");
                test(string{ss2d3->ice_id()} == "::Test::B");
            }
            else
            {
                test(string{ss1d3->ice_id()} == "::Test::D3");
                test(string{ss2d3->ice_id()} == "::Test::D3");
            }

            breakCycles(ss.c1);
            breakCycles(ss.c2);
        }
        catch (const Ice::Exception&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "sequence slicing (AMI)... " << flush;
    {
        try
        {
            SS3 ss;
            {
                BPtr ss1b = std::make_shared<B>();
                ss1b->sb = "B.sb";
                ss1b->pb = ss1b;

                D1Ptr ss1d1 = std::make_shared<D1>();
                ss1d1->sb = "D1.sb";
                ss1d1->sd1 = "D1.sd1";
                ss1d1->pb = ss1b;

                D3Ptr ss1d3 = std::make_shared<D3>();
                ss1d3->sb = "D3.sb";
                ss1d3->sd3 = "D3.sd3";
                ss1d3->pb = ss1b;

                BPtr ss2b = std::make_shared<B>();
                ss2b->sb = "B.sb";
                ss2b->pb = ss1b;

                D1Ptr ss2d1 = std::make_shared<D1>();
                ss2d1->sb = "D1.sb";
                ss2d1->sd1 = "D1.sd1";
                ss2d1->pb = ss2b;

                D3Ptr ss2d3 = std::make_shared<D3>();
                ss2d3->sb = "D3.sb";
                ss2d3->sd3 = "D3.sd3";
                ss2d3->pb = ss2b;

                ss1d1->pd1 = ss2b;
                ss1d3->pd3 = ss2d1;

                ss2d1->pd1 = ss1d3;
                ss2d3->pd3 = ss1d1;

                SS1Ptr ss1 = std::make_shared<SS1>();
                ss1->s.push_back(ss1b);
                ss1->s.push_back(ss1d1);
                ss1->s.push_back(ss1d3);

                SS2Ptr ss2 = std::make_shared<SS2>();
                ss2->s.push_back(ss2b);
                ss2->s.push_back(ss2d1);
                ss2->s.push_back(ss2d3);

                ss = testPrx->sequenceTestAsync(ss1, ss2).get();
                breakCycles(ss1);
                breakCycles(ss2);
            }

            test(ss.c1);
            BPtr ss1b = ss.c1->s[0];
            BPtr ss1d1 = ss.c1->s[1];
            test(ss.c2);
            BPtr ss1d3 = ss.c1->s[2];

            test(ss.c2);
            BPtr ss2b = ss.c2->s[0];
            BPtr ss2d1 = ss.c2->s[1];
            BPtr ss2d3 = ss.c2->s[2];

            test(ss1b->pb == ss1b);
            test(ss1d1->pb == ss1b);
            test(ss1d3->pb == ss1b);

            test(ss2b->pb == ss1b);
            test(ss2d1->pb == ss2b);
            test(ss2d3->pb == ss2b);

            test(string{ss1b->ice_id()} == "::Test::B");
            test(string{ss1d1->ice_id()} == "::Test::D1");

            test(string{ss2b->ice_id()} == "::Test::B");
            test(string{ss2d1->ice_id()} == "::Test::D1");

            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(string{ss1d3->ice_id()} == "::Test::B");
                test(string{ss2d3->ice_id()} == "::Test::B");
            }
            else
            {
                test(string{ss1d3->ice_id()} == "::Test::D3");
                test(string{ss2d3->ice_id()} == "::Test::D3");
            }

            breakCycles(ss.c1);
            breakCycles(ss.c2);
        }
        catch (const Ice::Exception&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "dictionary slicing... " << flush;
    {
        try
        {
            BDict bin;
            BDict bout;
            BDict r;
            int i;
            for (i = 0; i < 10; ++i)
            {
                ostringstream s;
                s << "D1." << i;
                D1Ptr d1 = std::make_shared<D1>();
                d1->sb = s.str();
                d1->pb = d1;
                d1->sd1 = s.str();
                bin[i] = d1;
            }

            r = testPrx->dictionaryTest(bin, bout);

            test(bout.size() == 10);
            for (i = 0; i < 10; ++i)
            {
                BPtr b = bout.find(i * 10)->second;
                test(b);
                std::ostringstream s;
                s << "D1." << i;
                test(b->sb == s.str());
                test(b->pb);
                test(b->pb != b);
                test(b->pb->sb == s.str());
                test(b->pb->pb == b->pb);
            }

            test(r.size() == 10);
            for (i = 0; i < 10; ++i)
            {
                BPtr b = r.find(i * 20)->second;
                test(b);
                std::ostringstream s;
                s << "D1." << i * 20;
                test(b->sb == s.str());
                test(b->pb == (i == 0 ? BPtr(nullptr) : r.find((i - 1) * 20)->second));
                D1Ptr d1 = dynamic_pointer_cast<D1>(b);
                test(d1);
                test(d1->sd1 == s.str());
                test(d1->pd1 == d1);
            }

            breakCycles(bin);
            breakCycles(bout);
            breakCycles(r);
        }
        catch (const Ice::Exception&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "dictionary slicing (AMI)... " << flush;
    {
        try
        {
            BDict bin;
            BDict bout;
            BDict r;
            int i;
            for (i = 0; i < 10; ++i)
            {
                ostringstream s;
                s << "D1." << i;
                D1Ptr d1 = std::make_shared<D1>();
                d1->sb = s.str();
                d1->pb = d1;
                d1->sd1 = s.str();
                bin[i] = d1;
            }

            auto result = testPrx->dictionaryTestAsync(bin).get();
            r = std::get<0>(result);
            bout = std::get<1>(result);

            test(bout.size() == 10);
            for (i = 0; i < 10; ++i)
            {
                BPtr b = bout.find(i * 10)->second;
                test(b);
                std::ostringstream s;
                s << "D1." << i;
                test(b->sb == s.str());
                test(b->pb);
                test(b->pb != b);
                test(b->pb->sb == s.str());
                test(b->pb->pb == b->pb);
            }

            test(r.size() == 10);
            for (i = 0; i < 10; ++i)
            {
                BPtr b = r.find(i * 20)->second;
                test(b);
                std::ostringstream s;
                s << "D1." << i * 20;
                test(b->sb == s.str());
                test(b->pb == (i == 0 ? BPtr(nullptr) : r.find((i - 1) * 20)->second));
                D1Ptr d1 = dynamic_pointer_cast<D1>(b);
                test(d1);
                test(d1->sd1 == s.str());
                test(d1->pd1 == d1);
            }

            breakCycles(bin);
            breakCycles(bout);
            breakCycles(r);
        }
        catch (const Ice::Exception&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base exception thrown as base exception... " << flush;
    {
        try
        {
            testPrx->throwBaseAsBase();
            test(false);
        }
        catch (const BaseException& e)
        {
            test(string{e.ice_id()} == "::Test::BaseException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb");
            test(e.pb->pb == e.pb);
            breakCycles(e.pb);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base exception thrown as base exception (AMI)... " << flush;
    {
        try
        {
            testPrx->throwBaseAsBaseAsync().get();
            test(false);
        }
        catch (const BaseException& ex)
        {
            test(string{ex.ice_id()} == "::Test::BaseException");
            test(ex.sbe == "sbe");
            test(ex.pb);
            test(ex.pb->sb == "sb");
            test(ex.pb->pb == ex.pb);
            breakCycles(ex.pb);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "derived exception thrown as base exception... " << flush;
    {
        try
        {
            testPrx->throwDerivedAsBase();
            test(false);
        }
        catch (const DerivedException& e)
        {
            test(string{e.ice_id()} == "::Test::DerivedException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
            breakCycles(e.pb);
            breakCycles(e.pd1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "derived exception thrown as base exception (AMI)... " << flush;
    {
        try
        {
            testPrx->throwDerivedAsBaseAsync().get();
            test(false);
        }
        catch (const DerivedException& ex)
        {
            test(string{ex.ice_id()} == "::Test::DerivedException");
            test(ex.sbe == "sbe");
            test(ex.pb);
            test(ex.pb->sb == "sb1");
            test(ex.pb->pb == ex.pb);
            test(ex.sde == "sde1");
            test(ex.pd1);
            test(ex.pd1->sb == "sb2");
            test(ex.pd1->pb == ex.pd1);
            test(ex.pd1->sd1 == "sd2");
            test(ex.pd1->pd1 == ex.pd1);
            breakCycles(ex.pb);
            breakCycles(ex.pd1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "derived exception thrown as derived exception... " << flush;
    {
        try
        {
            testPrx->throwDerivedAsDerived();
            test(false);
        }
        catch (const DerivedException& e)
        {
            test(string{e.ice_id()} == "::Test::DerivedException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
            breakCycles(e.pb);
            breakCycles(e.pd1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "derived exception thrown as derived exception (AMI)... " << flush;
    {
        try
        {
            testPrx->throwDerivedAsDerivedAsync().get();
            test(false);
        }
        catch (const DerivedException& e)
        {
            test(string{e.ice_id()} == "::Test::DerivedException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
            breakCycles(e.pb);
            breakCycles(e.pd1);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "unknown derived exception thrown as base exception... " << flush;
    {
        try
        {
            testPrx->throwUnknownDerivedAsBase();
            test(false);
        }
        catch (const BaseException& e)
        {
            test(string{e.ice_id()} == "::Test::BaseException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb d2");
            test(e.pb->pb == e.pb);
            breakCycles(e.pb);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "unknown derived exception thrown as base exception (AMI)... " << flush;
    {
        try
        {
            testPrx->throwUnknownDerivedAsBaseAsync().get();
            test(false);
        }
        catch (const BaseException& e)
        {
            test(string{e.ice_id()} == "::Test::BaseException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb d2");
            test(e.pb->pb == e.pb);
            breakCycles(e.pb);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "preserved classes... " << flush;
    try
    {
        // Server knows the most-derived class PDerived.
        //
        PDerivedPtr pd = std::make_shared<PDerived>();
        pd->pi = 3;
        pd->ps = "preserved";
        pd->pb = pd;
        PBasePtr r = testPrx->exchangePBase(pd);
        PDerivedPtr p2 = dynamic_pointer_cast<PDerived>(r);
        test(p2);
        test(p2->pi == 3);
        test(p2->ps == "preserved");
        test(p2->pb == p2);

        breakCycles(r);
        breakCycles(pd);
    }
    catch (const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Server only knows the base type.
        //
        PCUnknownPtr pu = std::make_shared<PCUnknown>();
        pu->pi = 3;
        pu->pu = "preserved";

        PBasePtr r = testPrx->exchangePBase(pu);
        test(r->pi == 3);
        PCUnknownPtr p2 = dynamic_pointer_cast<PCUnknown>(r);
        if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!p2);
        }
        else
        {
            test(p2);
            test(p2->pu == "preserved");
        }

        breakCycles(r);
    }
    catch (const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Server only knows the intermediate type Preserved. The object will be sliced to
        // Preserved for the 1.0 encoding; otherwise it should be returned intact.
        //
        PCDerivedPtr pcd = std::make_shared<PCDerived>();
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
        PBasePtr r = testPrx->exchangePBase(pcd);
        PCDerivedPtr p2 = dynamic_pointer_cast<PCDerived>(r);
        if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!p2);
            test(r->pi == 3);
        }
        else
        {
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
        }

        breakCycles(r);
        breakCycles(pcd);
        breakCycles(p2);
    }
    catch (const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Server only knows the intermediate type CompactPDerived. The object will be sliced to
        // CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
        //
        CompactPCDerivedPtr pcd = std::make_shared<CompactPCDerived>();
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
        PBasePtr r = testPrx->exchangePBase(pcd);
        CompactPCDerivedPtr p2 = dynamic_pointer_cast<CompactPCDerived>(r);
        if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!p2);
            test(r->pi == 3);
        }
        else
        {
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
        }

        breakCycles(r);
        breakCycles(pcd);
        breakCycles(p2);
    }
    catch (const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Send an object that will have multiple preserved slices in the server.
        // The object will be sliced to Preserved for the 1.0 encoding.
        //
        PCDerived3Ptr pcd = std::make_shared<PCDerived3>();
        pcd->pi = 3;
        //
        // Sending more than 254 objects exercises the encoding for object ids.
        //
        int i;
        for (i = 0; i < 300; ++i)
        {
            PCDerived2Ptr p2 = std::make_shared<PCDerived2>();
            p2->pi = i;
            p2->pbs.push_back(nullptr); // Nil reference. This slice should not have an indirection table.
            p2->pcd2 = i;
            pcd->pbs.push_back(p2);
        }
        pcd->pcd2 = pcd->pi;
        pcd->pcd3 = pcd->pbs[10];

        PBasePtr r = testPrx->exchangePBase(pcd);
        PCDerived3Ptr p3 = dynamic_pointer_cast<PCDerived3>(r);
        if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!p3);
            test(dynamic_pointer_cast<Preserved>(r));
            test(r->pi == 3);
        }
        else
        {
            test(p3);
            test(p3->pi == 3);
            for (i = 0; i < 300; ++i)
            {
                PCDerived2Ptr p2 = dynamic_pointer_cast<PCDerived2>(p3->pbs[static_cast<size_t>(i)]);
                test(p2->pi == i);
                test(p2->pbs.size() == 1);
                test(!p2->pbs[0]);
                test(p2->pcd2 == i);
            }
            test(p3->pcd2 == p3->pi);
            test(p3->pcd3 == p3->pbs[10]);
        }

        breakCycles(r);
        breakCycles(pcd);
        breakCycles(p3);
    }
    catch (const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Obtain an object with preserved slices and send it back to the server.
        // The preserved slices should be excluded for the 1.0 encoding, otherwise
        // they should be included.
        //
        PreservedPtr p = testPrx->PBSUnknownAsPreserved();
        testPrx->checkPBSUnknown(p);
        if (testPrx->ice_getEncodingVersion() != Ice::Encoding_1_0)
        {
            Ice::SlicedDataPtr slicedData = p->ice_getSlicedData();
            test(slicedData);
            test(slicedData->slices.size() == 1);
            test(slicedData->slices[0]->typeId == "::Test::PSUnknown");
            testPrx->ice_encodingVersion(Ice::Encoding_1_0)->checkPBSUnknown(p);
        }
        else
        {
            test(!p->ice_getSlicedData());
        }

        breakCycles(p);
    }
    catch (const Ice::OperationNotExistException&)
    {
    }

    cout << "ok" << endl;

    cout << "preserved classes (AMI)... " << flush;
    {
        //
        // Server knows the most-derived class PDerived.
        //
        PDerivedPtr pd = std::make_shared<PDerived>();
        pd->pi = 3;
        pd->ps = "preserved";
        pd->ps = "preserved";
        pd->pb = pd;

        try
        {
            auto r = dynamic_pointer_cast<PDerived>(testPrx->exchangePBaseAsync(pd).get());
            test(r);
            test(r->pi == 3);
            test(r->ps == "preserved");
            test(r->pb == r);
            breakCycles(r);
        }
        catch (...)
        {
            test(false);
        }
        breakCycles(pd);
    }

    {
        //
        // Server only knows the base type.
        //
        PCUnknownPtr pu = std::make_shared<PCUnknown>();
        pu->pi = 3;
        pu->pu = "preserved";
        try
        {
            auto r = testPrx->exchangePBaseAsync(pu).get();
            test(r->pi == 3);

            auto p2 = dynamic_pointer_cast<PCUnknown>(r);
            if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(!p2);
            }
            else
            {
                test(p2);
                test(p2->pu == "preserved");
            }

            breakCycles(r);
        }
        catch (...)
        {
            test(false);
        }
    }

    {
        //
        // Server only knows the intermediate type Preserved. The object will be sliced to
        // Preserved for the 1.0 encoding; otherwise it should be returned intact.
        //
        PCDerivedPtr pcd = std::make_shared<PCDerived>();
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
        if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            auto r = testPrx->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<PCDerived>(r);
            test(!p2);
            test(r->pi == 3);
            breakCycles(r);
            breakCycles(p2);
        }
        else
        {
            auto r = testPrx->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<PCDerived>(r);
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
            breakCycles(r);
            breakCycles(p2);
        }
        breakCycles(pcd);
    }

    {
        //
        // Server only knows the intermediate type CompactPDerived. The object will be sliced to
        // CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
        //
        CompactPCDerivedPtr pcd = std::make_shared<CompactPCDerived>();
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);

        if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            auto r = testPrx->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<CompactPCDerived>(r);
            test(!p2);
            test(r->pi == 3);
            breakCycles(r);
            breakCycles(p2);
        }
        else
        {
            auto r = testPrx->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<CompactPCDerived>(r);
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
            breakCycles(r);
            breakCycles(p2);
        }
        breakCycles(pcd);
    }

    {
        //
        // Send an object that will have multiple preserved slices in the server.
        // The object will be sliced to Preserved for the 1.0 encoding.
        //
        PCDerived3Ptr pcd = std::make_shared<PCDerived3>();
        pcd->pi = 3;
        //
        // Sending more than 254 objects exercises the encoding for object ids.
        //
        for (int i = 0; i < 300; ++i)
        {
            PCDerived2Ptr p2 = std::make_shared<PCDerived2>();
            p2->pi = i;
            p2->pbs.push_back(nullptr); // Nil reference. This slice should not have an indirection table.
            p2->pcd2 = i;
            pcd->pbs.push_back(p2);
        }
        pcd->pcd2 = pcd->pi;
        pcd->pcd3 = pcd->pbs[10];

        if (testPrx->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            auto r = testPrx->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<PCDerived>(r);
            test(!p2);
            test(r->pi == 3);
            breakCycles(r);
            breakCycles(pcd);
        }
        else
        {
            auto r = testPrx->exchangePBaseAsync(pcd).get();
            auto p3 = dynamic_pointer_cast<PCDerived3>(r);
            test(p3);
            test(p3->pi == 3);
            for (int i = 0; i < 300; ++i)
            {
                auto p2 = dynamic_pointer_cast<PCDerived2>(p3->pbs[static_cast<size_t>(i)]);
                test(p2->pi == i);
                test(p2->pbs.size() == 1);
                test(!p2->pbs[0]);
                test(p2->pcd2 == i);
            }
            test(p3->pcd2 == p3->pi);
            test(p3->pcd3 == p3->pbs[10]);
            breakCycles(r);
            breakCycles(pcd);
        }
    }
    cout << "ok" << endl;

    return testPrx;
}
