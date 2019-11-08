#include <IceGrid/SynchronizationException.h>

#ifdef ICE_CPP11_MAPPING
IceGrid::SynchronizationException::~SynchronizationException()
{
}

const ::std::string&
IceGrid::SynchronizationException::ice_staticId()
{
    static const ::std::string typeId = "::IceGrid::SynchronizationException";
    return typeId;
}

#else
IceGrid::SynchronizationException::SynchronizationException(const char* file, int line) :
    ::Ice::LocalException(file, line)
{
}

IceGrid::SynchronizationException::~SynchronizationException() throw()
{
}

::std::string
IceGrid::SynchronizationException::ice_id() const
{
    return "::IceGrid::SynchronizationException";
}

IceGrid::SynchronizationException*
IceGrid::SynchronizationException::ice_clone() const
{
    return new SynchronizationException(*this);
}

void
IceGrid::SynchronizationException::ice_throw() const
{
    throw *this;
}
#endif
