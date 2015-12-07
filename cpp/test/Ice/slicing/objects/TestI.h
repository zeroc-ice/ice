// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivate.h>

#ifndef ICE_CPP11_MAPPING
#   include <Forward.h>
#endif

class TestI : virtual public Test::TestIntf
{
public:

    TestI();

    virtual ::Ice::ValuePtr SBaseAsObject(const ::Ice::Current&);
    virtual ::Test::SBasePtr SBaseAsSBase(const ::Ice::Current&);
    virtual ::Test::SBasePtr SBSKnownDerivedAsSBase(const ::Ice::Current&);
    virtual ::Test::SBSKnownDerivedPtr SBSKnownDerivedAsSBSKnownDerived(const ::Ice::Current&);

    virtual ::Test::SBasePtr SBSUnknownDerivedAsSBase(const ::Ice::Current&);

    virtual ::Test::SBasePtr SBSUnknownDerivedAsSBaseCompact(const ::Ice::Current&);

    virtual ::Ice::ValuePtr SUnknownAsObject(const ::Ice::Current&);
    virtual void checkSUnknown(const Ice::ValuePtr& object, const ::Ice::Current&);

    virtual ::Test::BPtr oneElementCycle(const ::Ice::Current&);
    virtual ::Test::BPtr twoElementCycle(const ::Ice::Current&);

    virtual ::Test::BPtr D1AsB(const ::Ice::Current&);
    virtual ::Test::D1Ptr D1AsD1(const ::Ice::Current&);
    virtual ::Test::BPtr D2AsB(const ::Ice::Current&);

    virtual void paramTest1(::Test::BPtr&, ::Test::BPtr&, const ::Ice::Current&);
    virtual void paramTest2(::Test::BPtr&, ::Test::BPtr&, const ::Ice::Current&);
    virtual ::Test::BPtr paramTest3(::Test::BPtr&, ::Test::BPtr&, const ::Ice::Current&);
    virtual ::Test::BPtr paramTest4(::Test::BPtr&, const ::Ice::Current&);

    virtual ::Test::BPtr returnTest1(::Test::BPtr&, ::Test::BPtr&, const ::Ice::Current&);
    virtual ::Test::BPtr returnTest2(::Test::BPtr&, ::Test::BPtr&, const ::Ice::Current&);
    virtual ::Test::BPtr returnTest3(const ::Test::BPtr&, const ::Test::BPtr&, const ::Ice::Current&);

    virtual ::Test::SS3 sequenceTest(const ::Test::SS1Ptr&, const ::Test::SS2Ptr&, const ::Ice::Current&);

    virtual ::Test::BDict dictionaryTest(const ::Test::BDict&, ::Test::BDict&, const ::Ice::Current&);

    virtual ::Test::PBasePtr exchangePBase(const ::Test::PBasePtr&, const ::Ice::Current&);

    virtual ::Test::PreservedPtr PBSUnknownAsPreserved(const ::Ice::Current&);
    virtual void checkPBSUnknown(const ::Test::PreservedPtr&, const ::Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual void PBSUnknownAsPreservedWithGraph_async(std::function<void (const std::shared_ptr<Test::Preserved>&)>,
                                                      std::function<void (const std::exception_ptr&)>,
                                                      const ::Ice::Current&);
#else
    virtual void PBSUnknownAsPreservedWithGraph_async(const ::Test::AMD_TestIntf_PBSUnknownAsPreservedWithGraphPtr&,
                                                      const ::Ice::Current&);
#endif
    virtual void checkPBSUnknownWithGraph(const ::Test::PreservedPtr&, const ::Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual void PBSUnknown2AsPreservedWithGraph_async(std::function<void (const std::shared_ptr<Test::Preserved>&)>,
                                                       std::function<void (const std::exception_ptr&)>,
                                                       const ::Ice::Current&);
#else
    virtual void PBSUnknown2AsPreservedWithGraph_async(const ::Test::AMD_TestIntf_PBSUnknown2AsPreservedWithGraphPtr&,
                                                       const ::Ice::Current&);
#endif
    virtual void checkPBSUnknown2WithGraph(const ::Test::PreservedPtr&, const ::Ice::Current&);

    virtual ::Test::PNodePtr exchangePNode(const ::Test::PNodePtr&, const ::Ice::Current&);

    virtual void throwBaseAsBase(const ::Ice::Current&);
    virtual void throwDerivedAsBase(const ::Ice::Current&);
    virtual void throwDerivedAsDerived(const ::Ice::Current&);
    virtual void throwUnknownDerivedAsBase(const ::Ice::Current&);
#ifdef ICE_CPP11_MAPPING
    virtual void throwPreservedException_async(std::function<void ()>,
                                               std::function<void (const std::exception_ptr&)>,
                                               const ::Ice::Current&);
#else
    virtual void throwPreservedException_async(const ::Test::AMD_TestIntf_throwPreservedExceptionPtr&,
                                               const ::Ice::Current&);
#endif

    virtual void shutdown(const ::Ice::Current&);
};

#endif
