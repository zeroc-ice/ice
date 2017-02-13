// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Some import files we need almost everywhere
//
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSData.h>
#import <Foundation/NSNull.h>

#import <stdlib.h>

#define ICE_DEPRECATED_API(msg) __attribute__((deprecated(msg)))
#define ICE_DECLSPEC_EXPORT __attribute__((visibility ("default")))
#define ICE_DECLSPEC_IMPORT __attribute__((visibility ("default")))

#ifndef ICE_API
#   ifdef ICE_API_EXPORTS
#       define ICE_API ICE_DECLSPEC_EXPORT
#    else
#       define ICE_API ICE_DECLSPEC_IMPORT
#    endif
#endif

//
// Don't forget to update the conversion methods from Util.h if the types below
// are changed.
//
typedef unsigned char ICEByte;
typedef short ICEShort;
typedef int ICEInt;
#if defined(__x86_64) || defined(__arm64)
typedef long ICELong;
#else
typedef long long ICELong;
#endif
typedef float ICEFloat;
typedef double ICEDouble;

#if defined(__clang__) && __has_feature(objc_arc)
#  define ICE_STRONG_QUALIFIER __strong
#  define ICE_AUTORELEASING_QUALIFIER __autoreleasing
#  define ICE_STRONG_ATTR strong
#  define ICE_AUTORELEASE(v) v
#  define ICE_RETAIN(v) v
#  define ICE_RELEASE(v)
#else
#  define ICE_STRONG_QUALIFIER
#  define ICE_AUTORELEASING_QUALIFIER
#  define ICE_STRONG_ATTR retain
#  define ICE_AUTORELEASE(v) [v autorelease]
#  define ICE_RETAIN(v) [v retain]
#  define ICE_RELEASE(v) [v release]
#endif
