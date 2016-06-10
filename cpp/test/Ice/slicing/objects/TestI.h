// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivate.h>
#include <TestCommon.h>

class TestI : public virtual Test::TestIntf
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
    virtual void checkSUnknown(ICE_IN(Ice::ValuePtr) object, const ::Ice::Current&);

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
    virtual ::Test::BPtr returnTest3(ICE_IN(::Test::BPtr), ICE_IN(::Test::BPtr), const ::Ice::Current&);

    virtual ::Test::SS3 sequenceTest(ICE_IN(::Test::SS1Ptr), ICE_IN(::Test::SS2Ptr), const ::Ice::Current&);

    virtual ::Test::BDict dictionaryTest(ICE_IN(::Test::BDict), ::Test::BDict&, const ::Ice::Current&);

    virtual ::Test::PBasePtr exchangePBase(ICE_IN(::Test::PBasePtr), const ::Ice::Current&);

    virtual ::Test::PreservedPtr PBSUnknownAsPreserved(const ::Ice::Current&);
    virtual void checkPBSUnknown(ICE_IN(::Test::PreservedPtr), const ::Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual void PBSUnknownAsPreservedWithGraphAsync(std::function<void (const std::shared_ptr<Test::Preserved>&)>,
                                                      std::function<void (std::exception_ptr)>,
                                                      const ::Ice::Current&);
#else
    virtual void PBSUnknownAsPreservedWithGraph_async(const ::Test::AMD_TestIntf_PBSUnknownAsPreservedWithGraphPtr&,
                                                      const ::Ice::Current&);
#endif
    virtual void checkPBSUnknownWithGraph(ICE_IN(::Test::PreservedPtr), const ::Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual void PBSUnknown2AsPreservedWithGraphAsync(std::function<void (const std::shared_ptr<Test::Preserved>&)>,
                                                       std::function<void (std::exception_ptr)>,
                                                       const ::Ice::Current&);
#else
    virtual void PBSUnknown2AsPreservedWithGraph_async(const ::Test::AMD_TestIntf_PBSUnknown2AsPreservedWithGraphPtr&,
                                                       const ::Ice::Current&);
#endif
    virtual void checkPBSUnknown2WithGraph(ICE_IN(::Test::PreservedPtr), const ::Ice::Current&);

    virtual ::Test::PNodePtr exchangePNode(ICE_IN(::Test::PNodePtr), const ::Ice::Current&);

    virtual void throwBaseAsBase(const ::Ice::Current&);
    virtual void throwDerivedAsBase(const ::Ice::Current&);
    virtual void throwDerivedAsDerived(const ::Ice::Current&);
    virtual void throwUnknownDerivedAsBase(const ::Ice::Current&);
#ifdef ICE_CPP11_MAPPING
    virtual void throwPreservedExceptionAsync(std::function<void ()>,
                                               std::function<void (std::exception_ptr)>,
                                               const ::Ice::Current&);
#else
    virtual void throwPreservedException_async(const ::Test::AMD_TestIntf_throwPreservedExceptionPtr&,
                                               const ::Ice::Current&);
#endif
    
    virtual void useForward(::Test::ForwardPtr&, const ::Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);
};

#endif
