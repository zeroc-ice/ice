//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/LocalException.h>

namespace IceGrid
{

class SynchronizationException : public Ice::LocalExceptionHelper<SynchronizationException, Ice::LocalException>
{
public:

    SynchronizationException(const SynchronizationException&) = default;

    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    SynchronizationException(const char* file, int line) : Ice::LocalExceptionHelper<SynchronizationException, Ice::LocalException>(file, line)
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
    static const ::std::string& ice_staticId();
};

}
