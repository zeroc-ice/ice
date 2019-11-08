//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
#include <Test.h>

#ifdef ICE_CPP11_MAPPING
namespace Test
{

class ICE_CLASS(INTERCEPTOR_TEST_API) RetryException : public ::Ice::LocalExceptionHelper<RetryException, ::Ice::LocalException>
{
public:
    ICE_MEMBER(INTERCEPTOR_TEST_API) virtual ~RetryException();
    RetryException(const RetryException&) = default;
    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    RetryException(const char* file, int line) : ::Ice::LocalExceptionHelper<RetryException, ::Ice::LocalException>(file, line)
    {
    }
    /**
     * Obtains a tuple containing all of the exception's data members.
     * @return The data members in a tuple.
     */
    std::tuple<> ice_tuple() const
    {
        return std::tie();
    }
    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    ICE_MEMBER(INTERCEPTOR_TEST_API) static const ::std::string& ice_staticId();
    /**
     * Prints this exception to the given stream.
     * @param stream The target stream.
     */
    ICE_MEMBER(INTERCEPTOR_TEST_API) virtual void ice_print(::std::ostream& stream) const override;
};

}
#else

namespace Test
{

class INTERCEPTOR_TEST_API RetryException : public ::Ice::LocalException
{
public:
    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    RetryException(const char* file, int line);
    virtual ~RetryException() throw();
    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    virtual ::std::string ice_id() const;
    /**
     * Prints this exception to the given stream.
     * @param stream The target stream.
     */
    virtual void ice_print(::std::ostream& stream) const;
    /**
     * Polymporphically clones this exception.
     * @return A shallow copy of this exception.
     */
    virtual RetryException* ice_clone() const;
    /**
     * Throws this exception.
     */
    virtual void ice_throw() const;
};

}
#endif
