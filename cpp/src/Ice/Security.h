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
    SECURITY_PROTOCOL_DEBUG
} SecurityTraceLevel;

}

#define SECURITY_LEVEL_METHODS (_traceLevels->security >= IceSecurity::SECURITY_METHODS)
#define SECURITY_LEVEL_PARSEWARNINGS (_traceLevels->security >= IceSecurity::SECURITY_PARSE_WARNINGS)
#define SECURITY_LEVEL_WARNINGS (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
#define SECURITY_LEVEL_EXCEPTIONS (_traceLevels->security >= IceSecurity::SECURITY_EXCEPTIONS)
#define SECURITY_LEVEL_PROTOCOL (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL)
#define SECURITY_LEVEL_PROTOCOL_DEBUG (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL_DEBUG)

#define SECURITY_LOGGER(s) _logger->trace(_traceLevels->securityCat, s);

#define METHOD_INV(s) \
    if (_traceLevels->security >= IceSecurity::SECURITY_METHODS) \
    { \
	_logger->trace(_traceLevels->securityCat, "INV " + string(s)); \
    }

#define METHOD_INS(s) \
    if (_traceLevels->security >= IceSecurity::SECURITY_METHODS) \
    { \
	_logger->trace(_traceLevels->securityCat, "INS " + string(s)); \
    }

#define METHOD_RET(s) \
    if (_traceLevels->security >= IceSecurity::SECURITY_METHODS) \
    { \
	_logger->trace(_traceLevels->securityCat, "RET " + string(s)); \
    }

#define PARSE_WARNING(s) \
    if (_traceLevels->security >= IceSecurity::SECURITY_PARSE_WARNINGS) \
    { \
	_logger->trace(_traceLevels->securityCat, "PWN " + string(s)); \
    }

#define WARNING(s) \
    if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS) \
    { \
	_logger->trace(_traceLevels->securityCat, "WRN " + string(s)); \
    }

#define EXCEPTION(s) \
    if (_traceLevels->security >= IceSecurity::SECURITY_EXCEPTIONS) \
    { \
	_logger->trace(_traceLevels->securityCat, "EXC " + string(s)); \
    }

#define PROTOCOL(s) \
    if (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL) \
    { \
	_logger->trace(_traceLevels->securityCat, "PTL " + string(s)); \
    }

#define PROTOCOL_DEBUG(s) \
    if (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL_DEBUG) \
    { \
	_logger->trace(_traceLevels->securityCat, "DBG " + string(s)); \
    }
#endif
