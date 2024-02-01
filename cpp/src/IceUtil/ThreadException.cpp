//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/ThreadException.h>

using namespace std;

IceUtil::ThreadSyscallException::ThreadSyscallException(const char* file, int line, int err ):
    SyscallExceptionHelper<ThreadSyscallException>(file, line, err)
{
}

string
IceUtil::ThreadSyscallException::ice_id() const
{
    return "::IceUtil::ThreadSyscallException";
}

IceUtil::ThreadLockedException::ThreadLockedException(const char* file, int line) :
    ExceptionHelper<ThreadLockedException>(file, line)
{
}

string
IceUtil::ThreadLockedException::ice_id() const
{
    return "::IceUtil::ThreadLockedException";
}

IceUtil::ThreadStartedException::ThreadStartedException(const char* file, int line) :
    ExceptionHelper<ThreadStartedException>(file, line)
{
}

string
IceUtil::ThreadStartedException::ice_id() const
{
    return "::IceUtil::ThreadStartedException";
}

IceUtil::ThreadNotStartedException::ThreadNotStartedException(const char* file, int line) :
    ExceptionHelper<ThreadNotStartedException>(file, line)
{
}

string
IceUtil::ThreadNotStartedException::ice_id() const
{
    return "::IceUtil::ThreadNotStartedException";
}

IceUtil::BadThreadControlException::BadThreadControlException(const char* file, int line) :
    ExceptionHelper<BadThreadControlException>(file, line)
{
}

string
IceUtil::BadThreadControlException::ice_id() const
{
    return "::IceUtil::BadThreadControlException";
}

IceUtil::InvalidTimeoutException::InvalidTimeoutException(const char* file, int line,
                                                          const IceUtil::Time& timeout) :
    ExceptionHelper<InvalidTimeoutException>(file, line),
    _timeout(timeout)
{
}

string
IceUtil::InvalidTimeoutException::ice_id() const
{
    return "::IceUtil::InvalidTimeoutException";
}

void
IceUtil::InvalidTimeoutException::ice_print(ostream& os) const
{
    Exception::ice_print(os);
    os << ":\ninvalid timeout: " << _timeout << " seconds";
}
