// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <VersionI.h>
#import <Util.h>

ICE_API ICEEncodingVersion* ICEEncoding_1_0;
ICE_API ICEEncodingVersion* ICEEncoding_1_1;
ICE_API ICEEncodingVersion* ICECurrentEncoding;

@implementation ICEEncodingVersion (ICEInternal)

+(void) load
{
    ICEEncoding_1_0 = [[ICEEncodingVersion alloc] init:1 minor:0];
    ICEEncoding_1_1 = [[ICEEncodingVersion alloc] init:1 minor:1];
    ICECurrentEncoding = [[ICEEncodingVersion alloc] init:IceInternal::encodingMajor minor:IceInternal::encodingMinor];
}

-(ICEEncodingVersion*) initWithEncodingVersion:(const Ice::EncodingVersion&)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    major = arg.major;
    minor = arg.minor;
    return self;
}

-(Ice::EncodingVersion) encodingVersion
{
    Ice::EncodingVersion v;
    v.major = major;
    v.minor = minor;
    return v;
}

+(ICEEncodingVersion*) encodingVersionWithEncodingVersion:(const Ice::EncodingVersion&)arg
{
    if(arg == Ice::Encoding_1_0)
    {
        return ICEEncoding_1_0;
    }
    else if(arg == Ice::Encoding_1_1)
    {
        return ICEEncoding_1_1;
    }
    else if(arg == Ice::currentEncoding)
    {
        return ICECurrentEncoding;
    }
    else
    {
        return [[[ICEEncodingVersion alloc] initWithEncodingVersion:arg] autorelease];
    }
}

-(NSString*) description
{
    return [toNSString(Ice::encodingVersionToString([self encodingVersion])) autorelease];
}
@end

ICEProtocolVersion* ICEProtocol_1_0;
ICEProtocolVersion* ICECurrentProtocol;
ICEEncodingVersion* ICECurrentProtocolEncoding;

@implementation ICEProtocolVersion (ICEInternal)

+(void) load
{
    ICEProtocol_1_0 = [[ICEProtocolVersion alloc] init:1 minor:0];
    ICECurrentProtocol = [[ICEProtocolVersion alloc] init:IceInternal::protocolMajor minor:IceInternal::protocolMinor];
    ICECurrentProtocolEncoding = [[ICEEncodingVersion alloc] init:IceInternal::protocolEncodingMajor
                                                            minor:IceInternal::protocolEncodingMinor];
}

-(ICEProtocolVersion*) initWithProtocolVersion:(const Ice::ProtocolVersion&)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    major = arg.major;
    minor = arg.minor;
    return self;
}

-(Ice::ProtocolVersion) protocolVersion
{
    Ice::ProtocolVersion v;
    v.major = major;
    v.minor = minor;
    return v;
}

+(ICEProtocolVersion*) protocolVersionWithProtocolVersion:(const Ice::ProtocolVersion&)arg
{
    if(arg == Ice::Protocol_1_0)
    {
        return ICEProtocol_1_0;
    }
    else if(arg == Ice::currentProtocol)
    {
        return ICECurrentProtocol;
    }
    else
    {
        return [[[ICEProtocolVersion alloc] initWithProtocolVersion:arg] autorelease];
    }
}

-(NSString*) description
{
    return [toNSString(Ice::protocolVersionToString([self protocolVersion])) autorelease];
}
@end
