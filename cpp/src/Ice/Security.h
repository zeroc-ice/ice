// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************
#ifndef ICE_SECURITY_H
#define ICE_SECURITY_H

// NOTE: Define USE_SOCKETS in order to turn on use of sockets in OpenSSL
#ifndef USE_SOCKETS
#define USE_SOCKETS
#endif

#include <openssl/ssl.h>
#include <time.h>

namespace IceSecurity
{

typedef enum
{
    NO_SECURITY_TRACE = 0,
    SECURITY_WARNINGS,
    SECURITY_PARSE_WARNINGS,
    SECURITY_METHODS,
    SECURITY_EXCEPTIONS,
    SECURITY_PROTOCOL,
    SECURITY_DEV_DEBUG,
    SECURITY_PROTOCOL_DEBUG
} SecurityTraceLevel;

}

#define ICE_SECURITY_DEBUG

#ifdef WIN32
#define GETTHREADID GetCurrentThreadId()
#else
#define GETTHREADID getpid()
#endif

#define ICE_SECURITY_DISPLAYTHREADS

/*
    time_t ltime; \
    time(&ltime); \
    thread << " " << dec << ltime << " " << hex << (void *)this << " Thread(" << dec << GETTHREADID << ") "; \
*/

#ifdef ICE_SECURITY_DISPLAYTHREADS
#define ICE_SECURITY_LOGGER(s) \
    ostringstream thread; \
    thread << hex << (void *)this << " Thread(" << dec << GETTHREADID << ") "; \
    _logger->trace(_traceLevels->securityCat, thread.str() + s);
#else
#define ICE_SECURITY_LOGGER(s) _logger->trace(_traceLevels->securityCat, s);
#endif


#ifdef ICE_SECURITY_DEBUG

#define ICE_SECURITY_LEVEL_METHODS (_traceLevels->security >= IceSecurity::SECURITY_METHODS)
#define ICE_SECURITY_LEVEL_PARSEWARNINGS (_traceLevels->security >= IceSecurity::SECURITY_PARSE_WARNINGS)
#define ICE_SECURITY_LEVEL_WARNINGS (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
#define ICE_SECURITY_LEVEL_EXCEPTIONS (_traceLevels->security >= IceSecurity::SECURITY_EXCEPTIONS)
#define ICE_SECURITY_LEVEL_PROTOCOL (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL)
#define ICE_SECURITY_LEVEL_PROTOCOL_DEBUG (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL_DEBUG)
#define ICE_SECURITY_LEVEL_DEV_DEBUG (_traceLevels->security >= IceSecurity::SECURITY_DEV_DEBUG)

#define ICE_SECURITY_LEVEL_PROTOCOL_GLOBAL \
    (IceSecurity::Ssl::OpenSSL::System::_globalTraceLevels->security >= IceSecurity::SECURITY_PROTOCOL)

#define ICE_METHOD_INV(s) \
    if (ICE_SECURITY_LEVEL_METHODS) \
    { \
	ICE_SECURITY_LOGGER("INV " + string(s)); \
    }

#define ICE_METHOD_INS(s) \
    if (ICE_SECURITY_LEVEL_METHODS) \
    { \
	ICE_SECURITY_LOGGER("INS " + string(s)); \
    }

#define ICE_METHOD_RET(s) \
    if (ICE_SECURITY_LEVEL_METHODS) \
    { \
	ICE_SECURITY_LOGGER("RET " + string(s)); \
    }

#define ICE_PARSE_WARNING(s) \
    if (ICE_SECURITY_LEVEL_PARSEWARNINGS) \
    { \
	ICE_SECURITY_LOGGER("PWN " + string(s)); \
    }

#define ICE_WARNING(s) \
    if (ICE_SECURITY_LEVEL_WARNINGS) \
    { \
	ICE_SECURITY_LOGGER("WRN " + string(s)); \
    }

#define ICE_EXCEPTION(s) \
    if (ICE_SECURITY_LEVEL_EXCEPTIONS) \
    { \
	ICE_SECURITY_LOGGER("EXC " + string(s)); \
    }

#define ICE_PROTOCOL(s) \
    if (ICE_SECURITY_LEVEL_PROTOCOL) \
    { \
	ICE_SECURITY_LOGGER("PTL " + string(s)); \
    }

#define ICE_PROTOCOL_DEBUG(s) \
    if (ICE_SECURITY_LEVEL_PROTOCOL_DEBUG) \
    { \
	ICE_SECURITY_LOGGER("PDB " + string(s)); \
    }

#define ICE_DEV_DEBUG(s) \
    if (ICE_SECURITY_LEVEL_DEV_DEBUG) \
    { \
	ICE_SECURITY_LOGGER("DBG " + string(s)); \
    }

#else

#define ICE_SECURITY_LEVEL_METHODS false
#define ICE_SECURITY_LEVEL_PARSEWARNINGS (_traceLevels->security >= IceSecurity::SECURITY_PARSE_WARNINGS)
#define ICE_SECURITY_LEVEL_WARNINGS (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
#define ICE_SECURITY_LEVEL_EXCEPTIONS false
#define ICE_SECURITY_LEVEL_PROTOCOL false
#define ICE_SECURITY_LEVEL_PROTOCOL_DEBUG false
#define ICE_SECURITY_LEVEL_PROTOCOL_GLOBAL false
#define ICE_SECURITY_LEVEL_DEV_DEBUG false

#define ICE_METHOD_INV(s)
#define ICE_METHOD_INS(s)
#define ICE_METHOD_RET(s)

#define ICE_PARSE_WARNING(s) \
    if (ICE_SECURITY_LEVEL_PARSEWARNINGS) \
    { \
	ICE_SECURITY_LOGGER("PWN " + string(s)); \
    }

#define ICE_WARNING(s) \
    if (ICE_SECURITY_LEVEL_WARNINGS) \
    { \
	ICE_SECURITY_LOGGER("WRN " + string(s)); \
    }

#define ICE_EXCEPTION(s)
#define ICE_PROTOCOL(s)
#define ICE_PROTOCOL_DEBUG(s)
#define ICE_DEV_DEBUG(s)

#endif

#define ICE_SSLERRORS(s) \
    if (!s.empty()) \
    { \
        s += "\n"; \
        s += sslGetErrors(); \
    } \
    else \
    { \
        s = sslGetErrors(); \
    }

#endif
