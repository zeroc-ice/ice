//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

#ifdef __cplusplus

#define ICE_CPP11_MAPPING

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>

#if TARGET_OS_IPHONE
#   include <IceIAP/IceIAP.h>
#endif

#endif

# define ICE_SWIFT_UNAVAILABLE(msg) __attribute__((unavailable(msg)))
