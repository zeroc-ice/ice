//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ICESSL_H
#define ICESSL_ICESSL_H

#if defined(_WIN32)
#    include "SChannel.h"
#elif defined(__APPLE__)
#    include "SecureTransport.h"
#else
#    include "OpenSSL.h"
#endif

#include "ConnectionInfo.h"
#include "EndpointInfo.h"

#endif
