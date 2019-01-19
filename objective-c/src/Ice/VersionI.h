//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
