
#include <IceUtil/MutexProtocol.h>

IceUtil::MutexProtocol
IceUtil::getDefaultMutexProtocol()
{
#ifdef _WIN32
   return PrioNone;
#else
   return ICE_DEFAULT_MUTEX_PROTOCOL;
#endif
}
