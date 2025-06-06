// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_CONFIG
#define ICE_SSL_CONFIG

#if defined(_WIN32)
#    define ICE_USE_SCHANNEL
// We need to include windows.h before wincrypt.h.
// clang-format off
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#    include <wincrypt.h>
// clang-format on

// SECURITY_WIN32 or SECURITY_KERNEL are defined before including security.h indicating who is compiling the code.
#    ifdef SECURITY_WIN32
#        undef SECURITY_WIN32
#    endif
#    ifdef SECURITY_KERNEL
#        undef SECURITY_KERNEL
#    endif
#    define SECURITY_WIN32 1

// See SCH_CREDENTIALS requirements:
// https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials#remarks
#    ifndef SCHANNEL_USE_BLACKLISTS
#        define SCHANNEL_USE_BLACKLISTS 1
#    endif
#    include <SubAuth.h>

#    include <schannel.h>
#    include <security.h>
#    include <sspi.h>
#    undef SECURITY_WIN32
#elif defined(__APPLE__)
#    include <TargetConditionals.h>
#    define ICE_USE_SECURE_TRANSPORT
#    if TARGET_OS_IPHONE != 0
#        define ICE_USE_SECURE_TRANSPORT_IOS 1
#    else
#        define ICE_USE_SECURE_TRANSPORT_MACOS 1
#    endif
#    include <Security/SecureTransport.h>
#    include <Security/Security.h>
#else
#    define ICE_USE_OPENSSL
#    include <openssl/err.h>
#    include <openssl/ssl.h>
#    include <openssl/x509v3.h>
#endif

#endif
