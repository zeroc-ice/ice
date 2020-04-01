//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <TestAMD.h>

bool
endsWith(const std::string&, const std::string&);

class ThrowerI : public Test::Thrower
{
public:

    ThrowerI();

    virtual void shutdownAsync(std::function<void()>,
                               std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void supportsUndeclaredExceptionsAsync(std::function<void(bool)>,
                                                   std::function<void(std::exception_ptr)>,
                                                   const Ice::Current&);

    virtual void supportsAssertExceptionAsync(std::function<void(bool)>,
                                              std::function<void(std::exception_ptr)>,
                                              const Ice::Current&);

    virtual void throwAasAAsync(int,
                                std::function<void()>,
                                std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void throwAorDasAorDAsync(int,
                                      std::function<void()>,
                                      std::function<void(std::exception_ptr)>,
                                      const Ice::Current&);

    virtual void throwBasAAsync(int, int,
                                std::function<void()>,
                                std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void throwCasAAsync(int, int, int,
                                std::function<void()>,
                                std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void throwBasBAsync(int, int,
                                std::function<void()>,
                                std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void throwCasBAsync(int, int, int,
                                std::function<void()>,
                                std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void throwCasCAsync(int, int, int,
                                std::function<void()>,
                                std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void throwModAAsync(int, int,
                                std::function<void()>,
                                std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void throwUndeclaredAAsync(int,
                                       std::function<void()>,
                                       std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void throwUndeclaredBAsync(int, int,
                                       std::function<void()>,
                                       std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void throwUndeclaredCAsync(int, int, int,
                                       std::function<void()>,
                                       std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void throwLocalExceptionAsync(std::function<void()>,
                                          std::function<void(std::exception_ptr)>,
                                          const Ice::Current&);

    virtual void throwNonIceExceptionAsync(std::function<void()>,
                                           std::function<void(std::exception_ptr)>,
                                           const Ice::Current&);

    virtual void throwAssertExceptionAsync(std::function<void()>,
                                           std::function<void(std::exception_ptr)>,
                                           const Ice::Current&);

    virtual void throwMemoryLimitExceptionAsync(ICE_IN(Ice::ByteSeq),
                                                std::function<void(const Ice::ByteSeq&)>,
                                                std::function<void(std::exception_ptr)>,
                                                const Ice::Current&);

    virtual void throwLocalExceptionIdempotentAsync(std::function<void()>,
                                                    std::function<void(std::exception_ptr)>,
                                                    const Ice::Current&);

    virtual void throwAfterResponseAsync(std::function<void()>,
                                         std::function<void(std::exception_ptr)>,
                                         const Ice::Current&);

    virtual void throwAfterExceptionAsync(std::function<void()>,
                                          std::function<void(std::exception_ptr)>,
                                          const Ice::Current&);

    virtual void throwEAsync(std::function<void()>,
                             std::function<void(std::exception_ptr)>,
                             const Ice::Current&);

    virtual void throwFAsync(std::function<void()>,
                             std::function<void(std::exception_ptr)>,
                             const Ice::Current&);

    virtual void throwGAsync(std::function<void()>,
                             std::function<void(std::exception_ptr)>,
                             const Ice::Current&);

    virtual void throwHAsync(std::function<void()>,
                             std::function<void(std::exception_ptr)>,
                             const Ice::Current&);
};

#endif
