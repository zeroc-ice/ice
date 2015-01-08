// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
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

//
// Don't forget to update the conversion methods from Util.h if the types below
// are changed.
//
typedef unsigned char ICEByte;
typedef short ICEShort;
typedef int ICEInt;
typedef long long ICELong;
typedef float ICEFloat;
typedef double ICEDouble;

#if defined(__clang__) && __has_feature(objc_arc)
#  define ICE_STRONG_QUALIFIER __strong
#  define ICE_AUTORELEASING_QUALIFIER __autoreleasing
#  define ICE_STRONG_ATTR strong
#else
#  define ICE_STRONG_QUALIFIER
#  define ICE_AUTORELEASING_QUALIFIER
#  define ICE_STRONG_ATTR retain
#endif
