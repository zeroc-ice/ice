//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ICESSL_H
#define ICESSL_ICESSL_H

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/Config.h>
#include <IceSSL/Plugin.h>

#if defined(_WIN32)
#  if defined(ICE_OS_UWP)
#    include <IceSSL/UWP.h>
#  else
#    include <IceSSL/SChannel.h>
#  endif
#elif defined(__APPLE__)
#  include <IceSSL/SecureTransport.h>
#else
#  include <IceSSL/OpenSSL.h>
#endif

#include <IceSSL/EndpointInfo.h>
#include <IceSSL/ConnectionInfo.h>
#include <IceUtil/PopDisableWarnings.h>

#endif
