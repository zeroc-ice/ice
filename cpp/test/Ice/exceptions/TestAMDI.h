// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <TestAMD.h>

bool
endsWith(const std::string&, const std::string&);

class ThrowerI : public Test::Thrower
{
public:

    ThrowerI();

#ifdef ICE_CPP11_MAPPING
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
#else
    virtual void shutdown_async(const Test::AMD_Thrower_shutdownPtr&,
                                const Ice::Current&);
    virtual void supportsUndeclaredExceptions_async(const Test::AMD_Thrower_supportsUndeclaredExceptionsPtr&,
                                                    const Ice::Current&);
    virtual void supportsAssertException_async(const Test::AMD_Thrower_supportsAssertExceptionPtr&,
                                               const Ice::Current&);

    virtual void throwAasA_async(const Test::AMD_Thrower_throwAasAPtr&,
                                 Ice::Int, const Ice::Current&);
    virtual void throwAorDasAorD_async(const Test::AMD_Thrower_throwAorDasAorDPtr&,
                                       Ice::Int, const Ice::Current&);
    virtual void throwBasA_async(const Test::AMD_Thrower_throwBasAPtr&,
                                 Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasA_async(const Test::AMD_Thrower_throwCasAPtr&,
                                 Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwBasB_async(const Test::AMD_Thrower_throwBasBPtr&,
                                 Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasB_async(const Test::AMD_Thrower_throwCasBPtr&,
                                 Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasC_async(const Test::AMD_Thrower_throwCasCPtr&,
                                 Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);

    virtual void throwModA_async(const Test::AMD_Thrower_throwModAPtr&,
                                 Ice::Int, Ice::Int, const Ice::Current&);

    virtual void throwUndeclaredA_async(const Test::AMD_Thrower_throwUndeclaredAPtr&,
                                        Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredB_async(const Test::AMD_Thrower_throwUndeclaredBPtr&,
                                        Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredC_async(const Test::AMD_Thrower_throwUndeclaredCPtr&,
                                        Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwLocalException_async(const Test::AMD_Thrower_throwLocalExceptionPtr&,
                                           const Ice::Current&);
    virtual void throwNonIceException_async(const Test::AMD_Thrower_throwNonIceExceptionPtr&,
                                            const Ice::Current&);
    virtual void throwAssertException_async(const Test::AMD_Thrower_throwAssertExceptionPtr&,
                                            const Ice::Current&);
    virtual void throwMemoryLimitException_async(const Test::AMD_Thrower_throwMemoryLimitExceptionPtr&,
                                                 const Ice::ByteSeq&, const Ice::Current&);

    virtual void throwLocalExceptionIdempotent_async(const Test::AMD_Thrower_throwLocalExceptionIdempotentPtr&,
                                                     const Ice::Current&);

    virtual void throwAfterResponse_async(const Test::AMD_Thrower_throwAfterResponsePtr&, const Ice::Current&);
    virtual void throwAfterException_async(const Test::AMD_Thrower_throwAfterExceptionPtr&, const Ice::Current&);

    virtual void throwE_async(const Test::AMD_Thrower_throwEPtr&, const Ice::Current&);
    virtual void throwF_async(const Test::AMD_Thrower_throwFPtr&, const Ice::Current&);
    virtual void throwG_async(const Test::AMD_Thrower_throwGPtr&, const Ice::Current&);
    virtual void throwH_async(const Test::AMD_Thrower_throwHPtr&, const Ice::Current&);
#endif
};

#endif
