// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_SECURE_TRANSPORT_UTIL_H
#define ICESSL_SECURE_TRANSPORT_UTIL_H

#ifdef __APPLE__

#include <IceSSL/SecureTransport.h>
#include <IceSSL/Util.h>

namespace IceSSL
{

namespace SecureTransport
{

std::string sslErrorToString(CFErrorRef);
std::string sslErrorToString(OSStatus);

#  if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
//
// Retrieve a certificate property
//
CFDictionaryRef getCertificateProperty(SecCertificateRef, CFTypeRef);
#  endif

//
// Read certificate from a file.
//
CFArrayRef loadCertificateChain(const std::string&, const std::string&, const std::string&, const std::string&,
                                const std::string&, const PasswordPromptPtr&, int);

SecCertificateRef loadCertificate(const std::string&);
CFArrayRef loadCACertificates(const std::string&);
CFArrayRef findCertificateChain(const std::string&, const std::string&, const std::string&);

} // SecureTransport namespace end

} // IceSSL namespace end

#endif

#endif
