// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Version.h>

#include <Ice/Version.h>

@interface ICEProtocolVersion (ICEInternal)
+(void) load;
-(ICEProtocolVersion*)initWithProtocolVersion:(const Ice::ProtocolVersion&)arg;
-(Ice::ProtocolVersion)protocolVersion;
+(ICEProtocolVersion*)protocolVersionWithProtocolVersion:(const Ice::ProtocolVersion&)arg;
@end

@interface ICEEncodingVersion (ICEInternal)
+(void) load;
-(ICEEncodingVersion*)initWithEncodingVersion:(const Ice::EncodingVersion&)arg;
-(Ice::EncodingVersion)encodingVersion;
+(ICEEncodingVersion*)encodingVersionWithEncodingVersion:(const Ice::EncodingVersion&)arg;
@end

