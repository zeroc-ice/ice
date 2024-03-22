//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ICESSL_H
#define ICESSL_ICESSL_H

#include "Config.h"
#include "Plugin.h"

#if defined(_WIN32)
#    include "SChannel.h"
#elif defined(__APPLE__)
#    include "SecureTransport.h"
#else
#    include "OpenSSL.h"
#endif

#include "EndpointInfo.h"
#include "ConnectionInfo.h"

#endif
