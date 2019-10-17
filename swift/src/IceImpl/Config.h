//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

#ifndef ICEIMPL_API
#   define ICEIMPL_API __attribute__((visibility ("default")))
#endif

#ifdef __cplusplus

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>

#if TARGET_OS_IPHONE
#   include <IceIAP/IceIAP.h>
#endif

#endif

# define ICE_SWIFT_UNAVAILABLE(msg) __attribute__((unavailable(msg)))
