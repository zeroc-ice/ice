//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivateAMD.h>

class TestI : public virtual ::Test::TestIntf
{
public:

    TestI();
    virtual void SBaseAsObjectAsync(std::function<void(const std::shared_ptr<Ice::Value>&)>,
                                    std::function<void(std::exception_ptr)>,
                                    const ::Ice::Current&);

    virtual void SBaseAsSBaseAsync(std::function<void(const std::shared_ptr<Test::SBase>&)>,
                                   std::function<void(std::exception_ptr)>,
                                   const ::Ice::Current&);

    virtual void SBSKnownDerivedAsSBaseAsync(std::function<void(const std::shared_ptr<Test::SBase>&)>,
                                             std::function<void(std::exception_ptr)>,
                                             const ::Ice::Current&);

    virtual void SBSKnownDerivedAsSBSKnownDerivedAsync(std::function<void(const std::shared_ptr<Test::SBSKnownDerived>&)>,
                                                       std::function<void(std::exception_ptr)>,
                                                       const ::Ice::Current&);

    virtual void SBSUnknownDerivedAsSBaseAsync(std::function<void(const std::shared_ptr<Test::SBase>&)>,
                                               std::function<void(std::exception_ptr)>,
                                               const ::Ice::Current&);

    virtual void SBSUnknownDerivedAsSBaseCompactAsync(std::function<void(const std::shared_ptr<Test::SBase>&)>,
                                                      std::function<void(std::exception_ptr)>,
                                                      const ::Ice::Current&);

    virtual void SUnknownAsObjectAsync(std::function<void(const std::shared_ptr<Ice::Value>&)>,
                                       std::function<void(std::exception_ptr)>,
                                       const ::Ice::Current&);

    virtual void checkSUnknownAsync(std::shared_ptr<Ice::Value>,
                                    std::function<void()>,
                                    std::function<void(std::exception_ptr)>,
                                    const ::Ice::Current&);

    virtual void oneElementCycleAsync(std::function<void(const std::shared_ptr<Test::B>&)>,
                                      std::function<void(std::exception_ptr)>,
                                      const ::Ice::Current&);

    virtual void twoElementCycleAsync(std::function<void(const std::shared_ptr<Test::B>&)>,
                                      std::function<void(std::exception_ptr)>,
                                      const ::Ice::Current&);

    virtual void D1AsBAsync(std::function<void(const std::shared_ptr<Test::B>&)>,
                            std::function<void(std::exception_ptr)>,
                            const ::Ice::Current&);

    virtual void D1AsD1Async(std::function<void(const std::shared_ptr<Test::D1>&)>,
                             std::function<void(std::exception_ptr)>,
                             const ::Ice::Current&);

    virtual void D2AsBAsync(std::function<void(const std::shared_ptr<Test::B>&)>,
                            std::function<void(std::exception_ptr)>,
                            const ::Ice::Current&);

    virtual void paramTest1Async(std::function<void(const std::shared_ptr<Test::B>&,
                                                     const std::shared_ptr<Test::B>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const ::Ice::Current&);

    virtual void paramTest2Async(std::function<void(const std::shared_ptr<Test::B>&,
                                                     const std::shared_ptr<Test::B>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const ::Ice::Current&);

    virtual void paramTest3Async(std::function<void(const std::shared_ptr<Test::B>&,
                                                     const std::shared_ptr<Test::B>&,
                                                     const std::shared_ptr<Test::B>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const ::Ice::Current&);

    virtual void paramTest4Async(std::function<void(const std::shared_ptr<Test::B>&,
                                                     const std::shared_ptr<Test::B>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const ::Ice::Current&);

    virtual void returnTest1Async(std::function<void(const std::shared_ptr<Test::B>&,
                                                      const std::shared_ptr<Test::B>&,
                                                      const std::shared_ptr<Test::B>&)>,
                                  std::function<void(std::exception_ptr)>,
                                  const ::Ice::Current&);

    virtual void returnTest2Async(std::function<void(const std::shared_ptr<Test::B>&,
                                                      const std::shared_ptr<Test::B>&,
                                                      const std::shared_ptr<Test::B>&)>,
                                  std::function<void(std::exception_ptr)>,
                                  const ::Ice::Current&);

    virtual void returnTest3Async(std::shared_ptr<::Test::B>,
                                  std::shared_ptr<::Test::B>,
                                  std::function<void(const std::shared_ptr<Test::B>&)>,
                                  std::function<void(std::exception_ptr)>,
                                  const ::Ice::Current&);

    virtual void sequenceTestAsync(std::shared_ptr<::Test::SS1>,
                                   std::shared_ptr<::Test::SS2>,
                                   std::function<void(const ::Test::SS3&)>,
                                   std::function<void(std::exception_ptr)>,
                                   const ::Ice::Current&);

    virtual void dictionaryTestAsync(Test::BDict,
                                     std::function<void(const ::Test::BDict&, const ::Test::BDict&)>,
                                     std::function<void(std::exception_ptr)>,
                                     const ::Ice::Current&);

    virtual void exchangePBaseAsync(std::shared_ptr<::Test::PBase>,
                                    std::function<void(const std::shared_ptr<::Test::PBase>&)>,
                                    std::function<void(std::exception_ptr)>,
                                    const ::Ice::Current&);

    virtual void PBSUnknownAsPreservedAsync(std::function<void(const std::shared_ptr<::Test::Preserved>&)>,
                                            std::function<void(std::exception_ptr)>,
                                            const ::Ice::Current&);

    virtual void checkPBSUnknownAsync(std::shared_ptr<::Test::Preserved>,
                                      std::function<void()>,
                                      std::function<void(std::exception_ptr)>,
                                      const ::Ice::Current&);

    virtual void PBSUnknownAsPreservedWithGraphAsync(std::function<void(const std::shared_ptr<::Test::Preserved>&)>,
                                                     std::function<void(std::exception_ptr)>,
                                                     const ::Ice::Current&);

    virtual void checkPBSUnknownWithGraphAsync(std::shared_ptr<::Test::Preserved>,
                                               std::function<void()>,
                                               std::function<void(std::exception_ptr)>,
                                               const ::Ice::Current&);

    virtual void PBSUnknown2AsPreservedWithGraphAsync(std::function<void(const std::shared_ptr<::Test::Preserved>&)>,
                                                      std::function<void(std::exception_ptr)>,
                                                      const ::Ice::Current&);

    virtual void checkPBSUnknown2WithGraphAsync(std::shared_ptr<::Test::Preserved>,
                                                std::function<void()>,
                                                std::function<void(std::exception_ptr)>,
                                                const ::Ice::Current&);

    virtual void exchangePNodeAsync(std::shared_ptr<::Test::PNode>,
                                    std::function<void(const std::shared_ptr<::Test::PNode>&)>,
                                    std::function<void(std::exception_ptr)>,
                                    const ::Ice::Current&);

    virtual void throwBaseAsBaseAsync(std::function<void()>,
                                      std::function<void(std::exception_ptr)>,
                                      const ::Ice::Current&);

    virtual void throwDerivedAsBaseAsync(std::function<void()>,
                                         std::function<void(std::exception_ptr)>,
                                         const ::Ice::Current&);

    virtual void throwDerivedAsDerivedAsync(std::function<void()>,
                                            std::function<void(std::exception_ptr)>,
                                            const ::Ice::Current&);

    virtual void throwUnknownDerivedAsBaseAsync(std::function<void()>,
                                                std::function<void(std::exception_ptr)>,
                                                const ::Ice::Current&);

    virtual void throwPreservedExceptionAsync(std::function<void()>,
                                              std::function<void(std::exception_ptr)>,
                                              const ::Ice::Current&);

    virtual void useForwardAsync(std::function<void(const std::shared_ptr<::Test::Forward>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const ::Ice::Current&);

    virtual void shutdownAsync(std::function<void()>,
                               std::function<void(std::exception_ptr)>,
                               const ::Ice::Current&);
};

#endif
