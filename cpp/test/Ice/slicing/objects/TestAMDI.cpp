//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <TestAMDI.h>
#include <Ice/Ice.h>
#include <TestHelper.h>

using namespace Test;
using namespace std;

TestI::TestI()
{
}

#ifdef ICE_CPP11_MAPPING

void
TestI::SBaseAsObjectAsync(function<void(const shared_ptr<Ice::Value>&)> response,
                           function<void(exception_ptr)>,
                           const ::Ice::Current&)
{
    auto sb = make_shared<SBase>();
    sb->sb = "SBase.sb";
    response(sb);
}

void
TestI::SBaseAsSBaseAsync(function<void(const shared_ptr<Test::SBase>&)> response,
                          function<void(exception_ptr)>,
                          const ::Ice::Current&)
{
    auto sb = make_shared<SBase>();
    sb->sb = "SBase.sb";
    response(sb);
}

void
TestI::SBSKnownDerivedAsSBaseAsync(function<void(const shared_ptr<Test::SBase>&)> response,
                                    function<void(exception_ptr)>,
                                    const ::Ice::Current&)
{
    auto sbskd = make_shared<SBSKnownDerived>();
    sbskd->sb = "SBSKnownDerived.sb";
    sbskd->sbskd = "SBSKnownDerived.sbskd";
    response(sbskd);
}

void
TestI::SBSKnownDerivedAsSBSKnownDerivedAsync(function<void(const shared_ptr<Test::SBSKnownDerived>&)> response,
                                              function<void(exception_ptr)>,
                                              const ::Ice::Current&)
{
    auto sbskd = make_shared<SBSKnownDerived>();
    sbskd->sb = "SBSKnownDerived.sb";
    sbskd->sbskd = "SBSKnownDerived.sbskd";
    response(sbskd);
}

void
TestI::SBSUnknownDerivedAsSBaseAsync(function<void(const shared_ptr<Test::SBase>&)> response,
                                      function<void(exception_ptr)>,
                                      const ::Ice::Current&)
{
    auto sbsud = make_shared<SBSUnknownDerived>();
    sbsud->sb = "SBSUnknownDerived.sb";
    sbsud->sbsud = "SBSUnknownDerived.sbsud";
    response(sbsud);
}

void
TestI::SBSUnknownDerivedAsSBaseCompactAsync(function<void(const shared_ptr<Test::SBase>&)> response,
                                             function<void(exception_ptr)>,
                                             const ::Ice::Current&)
{
    auto sbsud = make_shared<SBSUnknownDerived>();
    sbsud->sb = "SBSUnknownDerived.sb";
    sbsud->sbsud = "SBSUnknownDerived.sbsud";
    response(sbsud);
}

void
TestI::SUnknownAsObjectAsync(function<void(const shared_ptr<Ice::Value>&)> response,
                              function<void(exception_ptr)>,
                              const ::Ice::Current&)
{
    auto su = make_shared<SUnknown>();
    su->su = "SUnknown.su";
    su->cycle = su;
    response(su);
}

void
TestI::checkSUnknownAsync(Ice::ValuePtr obj,
                           function<void()> response,
                           function<void(exception_ptr)>,
                           const ::Ice::Current& current)
{
    auto su = dynamic_pointer_cast<SUnknown>(obj);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!su);
    }
    else
    {
        test(su);
        test(su->su == "SUnknown.su");
    }
    response();
}

void
TestI::oneElementCycleAsync(function<void(const shared_ptr<Test::B>&)> response,
                             function<void(exception_ptr)>,
                             const ::Ice::Current&)
{
    auto b = make_shared<B>();
    b->sb = "B1.sb";
    b->pb = b;
    response(b);
}

void
TestI::twoElementCycleAsync(function<void(const shared_ptr<Test::B>&)> response,
                             function<void(exception_ptr)>,
                             const ::Ice::Current&)
{
    auto b1 = make_shared<B>();
    b1->sb = "B1.sb";
    auto b2 = make_shared<B>();
    b2->sb = "B2.sb";
    b2->pb = b1;
    b1->pb = b2;
    response(b1);
}

void
TestI::D1AsBAsync(function<void(const shared_ptr<Test::B>&)> response,
                   function<void(exception_ptr)>,
                   const ::Ice::Current&)
{
    auto d1 = make_shared<D1>();
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    auto d2 = make_shared<D2>();
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    response(d1);
}

void
TestI::D1AsD1Async(function<void(const shared_ptr<Test::D1>&)> response,
                    function<void(exception_ptr)>,
                    const ::Ice::Current&)
{
    auto d1 = make_shared<D1>();
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    auto d2 = make_shared<D2>();
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    response(d1);
}

void
TestI::D2AsBAsync(function<void(const shared_ptr<Test::B>&)> response,
                   function<void(exception_ptr)>,
                   const ::Ice::Current&)
{
    auto d2 = make_shared<D2>();
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    auto d1 = make_shared<D1>();
    d1->pb = d2;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    d1->pd1 = d2;
    d2->pb = d1;
    d2->pd2 = d1;
    response(d2);
}

void
TestI::paramTest1Async(function<void(const shared_ptr<Test::B>&,
                                       const shared_ptr<Test::B>&)> response,
                        function<void(exception_ptr)>,
                        const ::Ice::Current&)
{
    auto d1 = make_shared<D1>();
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = make_shared<D2>();
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    response(d1, d2);
}

void
TestI::paramTest2Async(function<void(const shared_ptr<Test::B>&, const shared_ptr<Test::B>&)> response,
                        function<void(exception_ptr)>,
                        const ::Ice::Current&)
{
    auto d1 = make_shared<D1>();
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    auto d2 = make_shared<D2>();
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    response(d2, d1);
}

void
TestI::paramTest3Async(function<void(const shared_ptr<Test::B>&,
                                       const shared_ptr<Test::B>&,
                                       const shared_ptr<Test::B>&)> response,
                        function<void(exception_ptr)>,
                        const ::Ice::Current&)
{
    auto d2 = make_shared<D2>();
    d2->sb = "D2.sb (p1 1)";
    d2->pb = nullptr;
    d2->sd2 = "D2.sd2 (p1 1)";

    auto d1 = make_shared<D1>();
    d1->sb = "D1.sb (p1 2)";
    d1->pb = nullptr;
    d1->sd1 = "D1.sd2 (p1 2)";
    d1->pd1 = nullptr;
    d2->pd2 = d1;

    auto d4 = make_shared<D2>();
    d4->sb = "D2.sb (p2 1)";
    d4->pb = nullptr;
    d4->sd2 = "D2.sd2 (p2 1)";

    auto d3 = make_shared<D1>();
    d3->sb = "D1.sb (p2 2)";
    d3->pb = nullptr;
    d3->sd1 = "D1.sd2 (p2 2)";
    d3->pd1 = nullptr;
    d4->pd2 = d3;

    response(d3, d2, d4);
}

void
TestI::paramTest4Async(function<void(const shared_ptr<Test::B>&,
                                       const shared_ptr<Test::B>&)> response,
                        function<void(exception_ptr)>,
                        const ::Ice::Current&)
{
    auto d4 = make_shared<D4>();
    d4->sb = "D4.sb (1)";
    d4->pb = nullptr;
    d4->p1 = make_shared<B>();
    d4->p1->sb = "B.sb (1)";
    d4->p2 = make_shared<B>();
    d4->p2->sb = "B.sb (2)";
    response(d4->p2, d4);
}

void
TestI::returnTest1Async(function<void(const shared_ptr<Test::B>&,
                                        const shared_ptr<Test::B>&,
                                        const shared_ptr<Test::B>&)> response,
                         function<void(exception_ptr)>,
                         const ::Ice::Current&)
{
    auto d1 = make_shared<D1>();
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    auto d2 = make_shared<D2>();
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    response(d2, d2, d1);
}

void
TestI::returnTest2Async(function<void(const shared_ptr<Test::B>&,
                                        const shared_ptr<Test::B>&,
                                        const shared_ptr<Test::B>&)> response,
                         function<void(exception_ptr)>,
                         const ::Ice::Current&)
{
    auto d1 = make_shared<D1>();
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    auto d2 = make_shared<D2>();
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    response(d1, d1, d2);
}

void
TestI::returnTest3Async(shared_ptr<::Test::B> p1,
                         shared_ptr<::Test::B>,
                         function<void(const shared_ptr<Test::B>&)> response,
                         function<void(exception_ptr)>,
                         const ::Ice::Current&)
{
    response(p1);
}

void
TestI::sequenceTestAsync(shared_ptr<::Test::SS1> p1,
                          shared_ptr<::Test::SS2> p2,
                          function<void(const ::Test::SS3&)> response,
                          function<void(exception_ptr)>,
                          const ::Ice::Current&)
{
    SS3 ss;
    ss.c1 = move(p1);
    ss.c2 = move(p2);
    response(ss);
}

void
TestI::dictionaryTestAsync(Test::BDict bin,
                            function<void(const ::Test::BDict&, const ::Test::BDict&)> response,
                            function<void(exception_ptr)>,
                            const ::Ice::Current&)
{
    BDict bout;
    int i;
    for(i = 0; i < 10; ++i)
    {
        auto b = bin.find(i)->second;
        auto d2 = make_shared<D2>();
        d2->sb = b->sb;
        d2->pb = b->pb;
        d2->sd2 = "D2";
        d2->pd2 = d2;
        bout[i * 10] = d2;
    }
    BDict r;
    for(i = 0; i < 10; ++i)
    {
        std::ostringstream s;
        s << "D1." << i * 20;
        auto d1 = make_shared<D1>();
        d1->sb = s.str();
        d1->pb = (i == 0 ? BPtr(0) : r.find((i - 1) * 20)->second);
        d1->sd1 = s.str();
        d1->pd1 = d1;
        r[i * 20] = d1;
    }
    response(r, bout);
}

void
TestI::exchangePBaseAsync(shared_ptr<::Test::PBase> pb,
                           function<void(const shared_ptr<::Test::PBase>&)> response,
                           function<void(exception_ptr)>,
                           const ::Ice::Current&)
{
    response(pb);
}

void
TestI::PBSUnknownAsPreservedAsync(function<void(const shared_ptr<::Test::Preserved>&)> response,
                                   function<void(exception_ptr)>,
                                   const ::Ice::Current& current)
{
    auto r = make_shared<PSUnknown>();
    r->pi = 5;
    r->ps = "preserved";
    r->psu = "unknown";
    r->graph = nullptr;
    if(current.encoding != Ice::Encoding_1_0)
    {
        //
        // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
        // from unread slice.
        //
        r->cl = make_shared<MyClass>(15);
    }
    response(r);
}

void
TestI::checkPBSUnknownAsync(shared_ptr<::Test::Preserved> p,
                             function<void()> response,
                             function<void(exception_ptr)>,
                             const ::Ice::Current& current)
{
    auto pu = dynamic_pointer_cast<PSUnknown>(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->psu == "unknown");
        test(!pu->graph);
        test(pu->cl && pu->cl->i == 15);
    }
    response();
}

void
TestI::PBSUnknownAsPreservedWithGraphAsync(function<void(const shared_ptr<::Test::Preserved>&)> response,
                                            function<void(exception_ptr)>,
                                            const ::Ice::Current&)
{
    auto r = make_shared<PSUnknown>();
    r->pi = 5;
    r->ps = "preserved";
    r->psu = "unknown";
    r->graph = make_shared<PNode>();
    r->graph->next = make_shared<PNode>();
    r->graph->next->next = make_shared<PNode>();
    r->graph->next->next->next = r->graph;
    response(r);
    r->graph->next->next->next = nullptr; // Break the cycle.
}

void
TestI::checkPBSUnknownWithGraphAsync(shared_ptr<::Test::Preserved> p,
                                      function<void()> response,
                                      function<void(exception_ptr)>,
                                      const ::Ice::Current& current)
{
    auto pu = dynamic_pointer_cast<PSUnknown>(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->psu == "unknown");
        test(pu->graph != pu->graph->next);
        test(pu->graph->next != pu->graph->next->next);
        test(pu->graph->next->next->next == pu->graph);
        pu->graph->next->next->next = nullptr;          // Break the cycle.
    }
    response();
}

void
TestI::PBSUnknown2AsPreservedWithGraphAsync(function<void(const shared_ptr<::Test::Preserved>&)> response,
                                             function<void(exception_ptr)>,
                                             const ::Ice::Current&)
{
    auto r = make_shared<PSUnknown2>();
    r->pi = 5;
    r->ps = "preserved";
    r->pb = r;
    response(r);
    r->pb = nullptr; // Break the cycle.
}

void
TestI::checkPBSUnknown2WithGraphAsync(shared_ptr<::Test::Preserved> p,
                                       function<void()> response,
                                       function<void(exception_ptr)>,
                                       const ::Ice::Current& current)
{
    auto pu = dynamic_pointer_cast<PSUnknown2>(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->pb == pu);
        pu->pb = nullptr; // Break the cycle.
    }
    response();
}

void
TestI::exchangePNodeAsync(shared_ptr<::Test::PNode> pn,
                           function<void(const shared_ptr<::Test::PNode>&)> response,
                           function<void(exception_ptr)>,
                           const ::Ice::Current&)
{
    response(pn);
}

void
TestI::throwBaseAsBaseAsync(function<void()>,
                             function<void(exception_ptr)> exception,
                             const ::Ice::Current&)
{
    try
    {
        BaseException be;
        be.sbe = "sbe";
        be.pb = make_shared<B>();
        be.pb->sb = "sb";
        be.pb->pb = be.pb;
        throw be;
    }
    catch(...)
    {
        exception(current_exception());
    }
}

void
TestI::throwDerivedAsBaseAsync(function<void()>,
                                function<void(exception_ptr)> exception,
                                const ::Ice::Current&)
{
    try
    {
        DerivedException de;
        de.sbe = "sbe";
        de.pb = make_shared<B>();
        de.pb->sb = "sb1";
        de.pb->pb = de.pb;
        de.sde = "sde1";
        de.pd1 = make_shared<D1>();
        de.pd1->sb = "sb2";
        de.pd1->pb = de.pd1;
        de.pd1->sd1 = "sd2";
        de.pd1->pd1 = de.pd1;
        throw de;
    }
    catch(...)
    {
        exception(current_exception());
    }
}

void
TestI::throwDerivedAsDerivedAsync(function<void()>,
                                   function<void(exception_ptr)> exception,
                                   const ::Ice::Current&)
{
    try
    {
        DerivedException de;
        de.sbe = "sbe";
        de.pb = make_shared<B>();
        de.pb->sb = "sb1";
        de.pb->pb = de.pb;
        de.sde = "sde1";
        de.pd1 = make_shared<D1>();
        de.pd1->sb = "sb2";
        de.pd1->pb = de.pd1;
        de.pd1->sd1 = "sd2";
        de.pd1->pd1 = de.pd1;
        throw de;
    }
    catch(...)
    {
        exception(current_exception());
    }
}

void
TestI::throwUnknownDerivedAsBaseAsync(function<void()>,
                                       function<void(exception_ptr)> exception,
                                       const ::Ice::Current&)
{
    try
    {
        auto d2 = make_shared<D2>();
        d2->sb = "sb d2";
        d2->pb = d2;
        d2->sd2 = "sd2 d2";
        d2->pd2 = d2;

        UnknownDerivedException ude;
        ude.sbe = "sbe";
        ude.pb = d2;
        ude.sude = "sude";
        ude.pd2 = d2;
        throw ude;
    }
    catch(...)
    {
        exception(current_exception());
    }
}

void
TestI::throwPreservedExceptionAsync(function<void()>,
                                     function<void(exception_ptr)> exception,
                                     const ::Ice::Current&)
{
    try
    {
        PSUnknownException ue;
        ue.p = make_shared<PSUnknown2>();
        ue.p->pi = 5;
        ue.p->ps = "preserved";
        ue.p->pb = ue.p;
        ue.p->pb = nullptr; // Break the cycle.
        throw ue;
    }
    catch(...)
    {
        exception(current_exception());
    }
}

void
TestI::useForwardAsync(function<void(const shared_ptr<::Test::Forward>&)> response,
                        function<void(exception_ptr)>,
                        const ::Ice::Current&)
{
    auto f = make_shared<Forward>();
    f->h = make_shared<Hidden>();
    f->h->f = f;
    response(f);
}

void
TestI::shutdownAsync(function<void()> response,
                      function<void(exception_ptr)>,
                      const ::Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}
#else

void
TestI::SBaseAsObject_async(const AMD_TestIntf_SBaseAsObjectPtr& cb, const ::Ice::Current&)
{
    SBasePtr sb = new SBase;
    sb->sb = "SBase.sb";
    cb->ice_response(sb);
}

void
TestI::SBaseAsSBase_async(const AMD_TestIntf_SBaseAsSBasePtr& cb, const ::Ice::Current&)
{
    SBasePtr sb = new SBase;
    sb->sb = "SBase.sb";
    cb->ice_response(sb);
}

void
TestI::SBSKnownDerivedAsSBase_async(const AMD_TestIntf_SBSKnownDerivedAsSBasePtr& cb, const ::Ice::Current&)
{
    SBSKnownDerivedPtr sbskd = new SBSKnownDerived;
    sbskd->sb = "SBSKnownDerived.sb";
    sbskd->sbskd = "SBSKnownDerived.sbskd";
    cb->ice_response(sbskd);
}

void
TestI::SBSKnownDerivedAsSBSKnownDerived_async(const AMD_TestIntf_SBSKnownDerivedAsSBSKnownDerivedPtr& cb,
                                              const ::Ice::Current&)
{
    SBSKnownDerivedPtr sbskd = new SBSKnownDerived;
    sbskd->sb = "SBSKnownDerived.sb";
    sbskd->sbskd = "SBSKnownDerived.sbskd";
    cb->ice_response(sbskd);
}

void
TestI::SBSUnknownDerivedAsSBase_async(const AMD_TestIntf_SBSUnknownDerivedAsSBasePtr& cb, const ::Ice::Current&)
{
    SBSUnknownDerivedPtr sbsud = new SBSUnknownDerived;
    sbsud->sb = "SBSUnknownDerived.sb";
    sbsud->sbsud = "SBSUnknownDerived.sbsud";
    cb->ice_response(sbsud);
}

void
TestI::SBSUnknownDerivedAsSBaseCompact_async(const AMD_TestIntf_SBSUnknownDerivedAsSBaseCompactPtr& cb,
                                             const ::Ice::Current&)
{
    SBSUnknownDerivedPtr sbsud = new SBSUnknownDerived;
    sbsud->sb = "SBSUnknownDerived.sb";
    sbsud->sbsud = "SBSUnknownDerived.sbsud";
    cb->ice_response(sbsud);
}

void
TestI::SUnknownAsObject_async(const AMD_TestIntf_SUnknownAsObjectPtr& cb, const ::Ice::Current&)
{
    SUnknownPtr su = new SUnknown;
    su->su = "SUnknown.su";
    su->cycle = su;
#ifndef ICE_CPP11_MAPPING
    su->ice_collectable(true);
#endif
    cb->ice_response(su);
}

void
TestI::checkSUnknown_async(const AMD_TestIntf_checkSUnknownPtr& cb,
                           const Ice::ObjectPtr& obj,
                           const ::Ice::Current& current)
{
    SUnknownPtr su = SUnknownPtr::dynamicCast(obj);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!su);
    }
    else
    {
        test(su);
        test(su->su == "SUnknown.su");
    }
    cb->ice_response();
}

void
TestI::oneElementCycle_async(const AMD_TestIntf_oneElementCyclePtr& cb, const ::Ice::Current&)
{
    BPtr b = new B;
    b->sb = "B1.sb";
    b->pb = b;
    b->ice_collectable(true);
    cb->ice_response(b);
}

void
TestI::twoElementCycle_async(const AMD_TestIntf_twoElementCyclePtr& cb, const ::Ice::Current&)
{
    BPtr b1 = new B;
    b1->sb = "B1.sb";
    BPtr b2 = new B;
    b2->sb = "B2.sb";
    b2->pb = b1;
    b1->pb = b2;
    b1->ice_collectable(true);
    cb->ice_response(b1);
}

void
TestI::D1AsB_async(const AMD_TestIntf_D1AsBPtr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    d1->ice_collectable(true);
    d2->ice_collectable(true);
    cb->ice_response(d1);
}

void
TestI::D1AsD1_async(const AMD_TestIntf_D1AsD1Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    d1->ice_collectable(true);
    cb->ice_response(d1);
}

void
TestI::D2AsB_async(const AMD_TestIntf_D2AsBPtr& cb, const ::Ice::Current&)
{
    D2Ptr d2 = new D2;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    D1Ptr d1 = new D1;
    d1->pb = d2;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    d1->pd1 = d2;
    d2->pb = d1;
    d2->pd2 = d1;
    d2->ice_collectable(true);
    cb->ice_response(d2);
}

void
TestI::paramTest1_async(const AMD_TestIntf_paramTest1Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    d1->ice_collectable(true);
    d2->ice_collectable(true);
    cb->ice_response(d1, d2);
}

void
TestI::paramTest2_async(const AMD_TestIntf_paramTest2Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    d1->ice_collectable(true);
    cb->ice_response(d2, d1);
}

void
TestI::paramTest3_async(const AMD_TestIntf_paramTest3Ptr& cb, const ::Ice::Current&)
{
    D2Ptr d2 = new D2;
    d2->sb = "D2.sb (p1 1)";
    d2->pb = 0;
    d2->sd2 = "D2.sd2 (p1 1)";

    D1Ptr d1 = new D1;
    d1->sb = "D1.sb (p1 2)";
    d1->pb = 0;
    d1->sd1 = "D1.sd2 (p1 2)";
    d1->pd1 = 0;
    d2->pd2 = d1;

    D2Ptr d4 = new D2;
    d4->sb = "D2.sb (p2 1)";
    d4->pb = 0;
    d4->sd2 = "D2.sd2 (p2 1)";

    D1Ptr d3 = new D1;
    d3->sb = "D1.sb (p2 2)";
    d3->pb = 0;
    d3->sd1 = "D1.sd2 (p2 2)";
    d3->pd1 = 0;
    d4->pd2 = d3;

    cb->ice_response(d3, d2, d4);
}

void
TestI::paramTest4_async(const AMD_TestIntf_paramTest4Ptr& cb, const ::Ice::Current&)
{
    D4Ptr d4 = new D4;
    d4->sb = "D4.sb (1)";
    d4->pb = 0;
    d4->p1 = new B;
    d4->p1->sb = "B.sb (1)";
    d4->p2 = new B;
    d4->p2->sb = "B.sb (2)";
    d4->ice_collectable(true);
    cb->ice_response(d4->p2, d4);
}

void
TestI::returnTest1_async(const AMD_TestIntf_returnTest1Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    d1->ice_collectable(true);
    cb->ice_response(d2, d2, d1);
}

void
TestI::returnTest2_async(const AMD_TestIntf_returnTest2Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    d1->ice_collectable(true);
    d2->ice_collectable(true);
    cb->ice_response(d1, d1, d2);
}

void
TestI::returnTest3_async(const AMD_TestIntf_returnTest3Ptr& cb, const BPtr& p1, const BPtr&, const ::Ice::Current&)
{
    cb->ice_response(p1);
}

void
TestI::sequenceTest_async(const AMD_TestIntf_sequenceTestPtr& cb,
                          const SS1Ptr& p1, const SS2Ptr& p2, const ::Ice::Current&)
{
    SS3 ss;
    ss.c1 = p1;
    ss.c2 = p2;
    cb->ice_response(ss);
}

void
TestI::dictionaryTest_async(const AMD_TestIntf_dictionaryTestPtr& cb, const BDict& bin, const ::Ice::Current&)
{
    BDict bout;
    int i;
    for(i = 0; i < 10; ++i)
    {
        BPtr b = bin.find(i)->second;
        D2Ptr d2 = new D2;
        d2->sb = b->sb;
        d2->pb = b->pb;
        d2->sd2 = "D2";
        d2->pd2 = d2;
        d2->ice_collectable(true);
        bout[i * 10] = d2;
    }
    BDict r;
    for(i = 0; i < 10; ++i)
    {
        std::ostringstream s;
        s << "D1." << i * 20;
        D1Ptr d1 = new D1;
        d1->sb = s.str();
        d1->pb = (i == 0 ? BPtr(0) : r.find((i - 1) * 20)->second);
        d1->sd1 = s.str();
        d1->pd1 = d1;
        d1->ice_collectable(true);
        r[i * 20] = d1;
    }
    cb->ice_response(r, bout);
}

void
TestI::exchangePBase_async(const AMD_TestIntf_exchangePBasePtr& cb, const PBasePtr& pb, const ::Ice::Current&)
{
    cb->ice_response(pb);
}

void
TestI::PBSUnknownAsPreserved_async(const Test::AMD_TestIntf_PBSUnknownAsPreservedPtr& cb, const Ice::Current& current)
{
    PSUnknownPtr r = new PSUnknown;
    r->pi = 5;
    r->ps = "preserved";
    r->psu = "unknown";
    r->graph = 0;
    if(current.encoding != Ice::Encoding_1_0)
    {
        //
        // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
        // from unread slice.
        //
        r->cl = new MyClass(15);
    }
    cb->ice_response(r);
}

void
TestI::checkPBSUnknown_async(const Test::AMD_TestIntf_checkPBSUnknownPtr& cb, const Test::PreservedPtr& p,
                                   const Ice::Current& current)
{
    PSUnknownPtr pu = PSUnknownPtr::dynamicCast(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->psu == "unknown");
        test(!pu->graph);
        test(pu->cl && pu->cl->i == 15);
    }
    cb->ice_response();
}

void
TestI::PBSUnknownAsPreservedWithGraph_async(const Test::AMD_TestIntf_PBSUnknownAsPreservedWithGraphPtr& cb,
                                            const Ice::Current&)
{
    PSUnknownPtr r = new PSUnknown;
    r->pi = 5;
    r->ps = "preserved";
    r->psu = "unknown";
    r->graph = new PNode;
    r->graph->next = new PNode;
    r->graph->next->next = new PNode;
    r->graph->next->next->next = r->graph;
    cb->ice_response(r);
    r->graph->next->next->next = 0; // Break the cycle.
}

void
TestI::checkPBSUnknownWithGraph_async(const Test::AMD_TestIntf_checkPBSUnknownWithGraphPtr& cb,
                                      const Test::PreservedPtr& p, const Ice::Current& current)
{
    PSUnknownPtr pu = PSUnknownPtr::dynamicCast(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->psu == "unknown");
        test(pu->graph != pu->graph->next);
        test(pu->graph->next != pu->graph->next->next);
        test(pu->graph->next->next->next == pu->graph);
        pu->graph->next->next->next = 0;          // Break the cycle.
    }
    cb->ice_response();
}

void
TestI::PBSUnknown2AsPreservedWithGraph_async(const Test::AMD_TestIntf_PBSUnknown2AsPreservedWithGraphPtr& cb,
                                             const Ice::Current&)
{
    PSUnknown2Ptr r = new PSUnknown2;
    r->pi = 5;
    r->ps = "preserved";
    r->pb = r;
    cb->ice_response(r);
    r->pb = 0; // Break the cycle.
}

void
TestI::checkPBSUnknown2WithGraph_async(const Test::AMD_TestIntf_checkPBSUnknown2WithGraphPtr& cb,
                                       const Test::PreservedPtr& p, const Ice::Current& current)
{
    PSUnknown2Ptr pu = PSUnknown2Ptr::dynamicCast(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->pb == pu);
        pu->pb = 0; // Break the cycle.
    }
    cb->ice_response();
}

void
TestI::exchangePNode_async(const AMD_TestIntf_exchangePNodePtr& cb, const PNodePtr& pn, const ::Ice::Current&)
{
    cb->ice_response(pn);
}

void
TestI::throwBaseAsBase_async(const AMD_TestIntf_throwBaseAsBasePtr& cb, const ::Ice::Current&)
{
    BaseException be;
    be.sbe = "sbe";
    be.pb = new B;
    be.pb->sb = "sb";
    be.pb->pb = be.pb;
    be.pb->ice_collectable(true);
    cb->ice_exception(be);
}

void
TestI::throwDerivedAsBase_async(const AMD_TestIntf_throwDerivedAsBasePtr& cb, const ::Ice::Current&)
{
    DerivedException de;
    de.sbe = "sbe";
    de.pb = new B;
    de.pb->sb = "sb1";
    de.pb->pb = de.pb;
    de.sde = "sde1";
    de.pd1 = new D1;
    de.pd1->sb = "sb2";
    de.pd1->pb = de.pd1;
    de.pd1->sd1 = "sd2";
    de.pd1->pd1 = de.pd1;
    de.pb->ice_collectable(true);
    de.pd1->ice_collectable(true);
    cb->ice_exception(de);
}

void
TestI::throwDerivedAsDerived_async(const AMD_TestIntf_throwDerivedAsDerivedPtr& cb, const ::Ice::Current&)
{
    DerivedException de;
    de.sbe = "sbe";
    de.pb = new B;
    de.pb->sb = "sb1";
    de.pb->pb = de.pb;
    de.sde = "sde1";
    de.pd1 = new D1;
    de.pd1->sb = "sb2";
    de.pd1->pb = de.pd1;
    de.pd1->sd1 = "sd2";
    de.pd1->pd1 = de.pd1;
    de.pb->ice_collectable(true);
    de.pd1->ice_collectable(true);
    cb->ice_exception(de);
}

void
TestI::throwUnknownDerivedAsBase_async(const AMD_TestIntf_throwUnknownDerivedAsBasePtr& cb, const ::Ice::Current&)
{
    D2Ptr d2 = new D2;
    d2->sb = "sb d2";
    d2->pb = d2;
    d2->sd2 = "sd2 d2";
    d2->pd2 = d2;
    d2->ice_collectable(true);

    UnknownDerivedException ude;
    ude.sbe = "sbe";
    ude.pb = d2;
    ude.sude = "sude";
    ude.pd2 = d2;
    cb->ice_exception(ude);
}

void
TestI::throwPreservedException_async(const AMD_TestIntf_throwPreservedExceptionPtr& cb, const ::Ice::Current&)
{
    PSUnknownException ue;
    ue.p = new PSUnknown2;
    ue.p->pi = 5;
    ue.p->ps = "preserved";
    ue.p->pb = ue.p;
    cb->ice_exception(ue);
    ue.p->pb = 0; // Break the cycle.
}

void
TestI::useForward_async(const AMD_TestIntf_useForwardPtr& cb, const ::Ice::Current&)
{
    ForwardPtr f = new Forward;
    f->h = new Hidden;
    f->h->f = f;
    f->ice_collectable(true);
    cb->ice_response(f);
}

void
TestI::shutdown_async(const AMD_TestIntf_shutdownPtr& cb, const ::Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    cb->ice_response();
}
#endif
