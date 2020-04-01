//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivate.h>

class TestI : public virtual Test::TestIntf
{
public:

    TestI();
    ~TestI();

    virtual ::Ice::ValuePtr SBaseAsObject(const ::Ice::Current&);
    virtual ::Test::SBasePtr SBaseAsSBase(const ::Ice::Current&);
    virtual ::Test::SBasePtr SBSKnownDerivedAsSBase(const ::Ice::Current&);
    virtual ::Test::SBSKnownDerivedPtr SBSKnownDerivedAsSBSKnownDerived(const ::Ice::Current&);

    virtual ::Test::SBasePtr SBSUnknownDerivedAsSBase(const ::Ice::Current&);

    virtual ::Test::SBasePtr SBSUnknownDerivedAsSBaseCompact(const ::Ice::Current&);

    virtual ::Ice::ValuePtr SUnknownAsObject(const ::Ice::Current&);
    virtual void checkSUnknown(Ice::ValuePtr object, const ::Ice::Current&);

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
    virtual ::Test::BPtr returnTest3(::Test::BPtr, ::Test::BPtr, const ::Ice::Current&);

    virtual ::Test::SS3 sequenceTest(::Test::SS1Ptr, ::Test::SS2Ptr, const ::Ice::Current&);

    virtual ::Test::BDict dictionaryTest(::Test::BDict, ::Test::BDict&, const ::Ice::Current&);

    virtual ::Test::PBasePtr exchangePBase(::Test::PBasePtr, const ::Ice::Current&);

    virtual ::Test::PreservedPtr PBSUnknownAsPreserved(const ::Ice::Current&);
    virtual void checkPBSUnknown(::Test::PreservedPtr, const ::Ice::Current&);

    virtual void PBSUnknownAsPreservedWithGraphAsync(std::function<void(const std::shared_ptr<Test::Preserved>&)>,
                                                      std::function<void(std::exception_ptr)>,
                                                      const ::Ice::Current&);
    virtual void checkPBSUnknownWithGraph(::Test::PreservedPtr, const ::Ice::Current&);

    virtual void PBSUnknown2AsPreservedWithGraphAsync(std::function<void(const std::shared_ptr<Test::Preserved>&)>,
                                                       std::function<void(std::exception_ptr)>,
                                                       const ::Ice::Current&);
    virtual void checkPBSUnknown2WithGraph(::Test::PreservedPtr, const ::Ice::Current&);

    virtual ::Test::PNodePtr exchangePNode(::Test::PNodePtr, const ::Ice::Current&);

    virtual void throwBaseAsBase(const ::Ice::Current&);
    virtual void throwDerivedAsBase(const ::Ice::Current&);
    virtual void throwDerivedAsDerived(const ::Ice::Current&);
    virtual void throwUnknownDerivedAsBase(const ::Ice::Current&);
    virtual void throwPreservedExceptionAsync(std::function<void()>,
                                               std::function<void(std::exception_ptr)>,
                                               const ::Ice::Current&);

    virtual void useForward(::Test::ForwardPtr&, const ::Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);

private:

    std::vector<std::shared_ptr<Ice::Value>> _values;
};

#endif
