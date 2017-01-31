// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <ExceptionI.h>
#import <IdentityI.h>
#import <Util.h>
#import <VersionI.h>

#import <objc/Ice/LocalException.h>

#include <Ice/LocalException.h>

#import <Foundation/NSKeyedArchiver.h>

@implementation ICEException
-(id)init
{
    return [super initWithName:[self ice_id] reason:nil userInfo:nil];
}

-(id)initWithReason:(NSString*)reason
{
    return [super initWithName:[self ice_id] reason:reason userInfo:nil];
}

-(NSString*)ice_name
{
    return [[self ice_id] substringFromIndex:2];
}

-(NSString*)ice_id
{
    NSAssert(false, @"ice_id not overriden");
    return nil;
}

-(id) initWithCoder:(NSCoder*)decoder
{
    [NSException raise:NSInvalidArchiveOperationException format:@"ICEExceptions do not support NSCoding"];
    return nil;
}

-(void) encodeWithCoder:(NSCoder*)coder
{
    [NSException raise:NSInvalidArchiveOperationException format:@"ICEExceptions do not support NSCoding"];
}

-(id) copyWithZone:(NSZone *)zone
{
    NSAssert(false, @"copyWithZone: must be overriden");
    return nil;
}

-(void) dealloc
{
    [super dealloc];
}
@end

static NSString*
localExceptionToString(const Ice::LocalException& ex)
{
    std::ostringstream os;
    os << ex;
    std::string str = os.str();
    // Remove the lengthy path part of the C++ filename.
    std::string::size_type pos = str.find(".cpp");
    if(pos != std::string::npos)
    {
        pos = str.rfind('/', pos);
        if(pos != std::string::npos)
        {
            str = str.substr(pos + 1);
        }
    }
    return [NSString stringWithUTF8String:str.c_str()];
}

@implementation ICELocalException
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithReason:localExceptionToString(ex)];
    if(!self)
    {
        return nil;
    }
    file = ex.ice_file();
    line = ex.ice_line();
    return self;
}

-(void)rethrowCxx
{
    NSAssert(false, @"rethrowCxx must be overriden");
}

+(id)localExceptionWithLocalException:(const Ice::LocalException&)ex
{
    return [[[self alloc] initWithLocalException:ex] autorelease];
}

-(NSString*) file
{
    return [toNSString(file) autorelease];
}

@synthesize line;

-(id)init:(const char*)f line:(int)l reason_:(NSString*)r
{
    return [self init:f line:l reason:r];
}

-(id)init:(const char*)f line:(int)l
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    file = f;
    line = l;
    return self;
}

-(id)init:(const char*)f line:(int)l reason:(NSString*)r
{
    self = [super initWithReason:r];
    if(!self)
    {
        return nil;
    }
    file = f;
    line = l;
    return self;
}

+(id)localException:(const char*)file line:(int)line
{
    return [[[self alloc] init:file line:line] autorelease];
}

-(NSString*)description
{
    try
    {
        [self rethrowCxx];
        return @""; // Keep the compiler happy.
    }
    catch(const Ice::LocalException& ex)
    {
        return localExceptionToString(ex);
    }
}

-(id) copyWithZone:(NSZone *)zone
{
    return [[[self class] allocWithZone:zone] init:file line:line];
}

-(void) dealloc
{
    [super dealloc];
}
@end

@implementation ICEUserException
-(BOOL)iceUsesClasses
{
    return NO;
}

-(void)iceWrite:(id<ICEOutputStream>)os
{
    [os startException:nil];
    [self iceWriteImpl:os];
    [os endException];
}

-(void) iceWriteImpl:(id<ICEOutputStream>)os
{
    NSAssert(NO, @"iceWriteImpl requires override");
}

-(void)iceRead:(id<ICEInputStream>)is
{
    [is startException];
    [self iceReadImpl:is];
    [is endException:NO];
}

-(void) iceReadImpl:(id<ICEInputStream>)is
{
    NSAssert(NO, @"iceReadImpl requires override");
}

-(id) copyWithZone:(NSZone *)zone
{
    return [[[self class] allocWithZone:zone] init];
}

-(void) dealloc
{
    [super dealloc];
}
@end

@implementation ICEInitializationException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::InitializationException*>(&ex), @"invalid local exception type");
    const Ice::InitializationException& localEx = dynamic_cast<const Ice::InitializationException&>(ex);
    reason_ = toNSString(localEx.reason);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::InitializationException(file, line, fromNSString(reason_));
}
@end

@implementation ICEPluginInitializationException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::PluginInitializationException*>(&ex), @"invalid local exception type");
    const Ice::PluginInitializationException& localEx = dynamic_cast<const Ice::PluginInitializationException&>(ex);
    reason_ = toNSString(localEx.reason);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::PluginInitializationException(file, line, fromNSString(reason_));
}
@end

@implementation ICECollocationOptimizationException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::CollocationOptimizationException(file, line);
}
@end

@implementation ICEAlreadyRegisteredException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::AlreadyRegisteredException*>(&ex), @"invalid local exception type");
    const Ice::AlreadyRegisteredException& localEx = dynamic_cast<const Ice::AlreadyRegisteredException&>(ex);
    kindOfObject = toNSString(localEx.kindOfObject);
    id_ = toNSString(localEx.id);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::AlreadyRegisteredException(file, line, fromNSString(kindOfObject), fromNSString(id_));
}
@end

@implementation ICENotRegisteredException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::NotRegisteredException*>(&ex), @"invalid local exception type");
    const Ice::NotRegisteredException& localEx = dynamic_cast<const Ice::NotRegisteredException&>(ex);
    kindOfObject = toNSString(localEx.kindOfObject);
    id_ = toNSString(localEx.id);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::NotRegisteredException(file, line, fromNSString(kindOfObject), fromNSString(id_));
}
@end

@implementation ICETwowayOnlyException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::TwowayOnlyException*>(&ex), @"invalid local exception type");
    const Ice::TwowayOnlyException& localEx = dynamic_cast<const Ice::TwowayOnlyException&>(ex);
    operation = toNSString(localEx.operation);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::TwowayOnlyException(file, line, fromNSString(operation));
}
@end

@implementation ICEUnknownException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::UnknownException*>(&ex), @"invalid local exception type");
    const Ice::UnknownException& localEx = dynamic_cast<const Ice::UnknownException&>(ex);
    unknown = toNSString(localEx.unknown);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::UnknownException(file, line, fromNSString(unknown));
}
@end

@implementation ICEUnknownLocalException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::UnknownLocalException(file, line, fromNSString([self unknown]));
}
@end

@implementation ICEUnknownUserException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::UnknownUserException(file, line, fromNSString([self unknown]));
}
@end

@implementation ICEVersionMismatchException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::VersionMismatchException(file, line);
}
@end

@implementation ICECommunicatorDestroyedException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::CommunicatorDestroyedException(file, line);
}
@end

@implementation ICEObjectAdapterDeactivatedException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::ObjectAdapterDeactivatedException*>(&ex), @"invalid local exception type");
    const Ice::ObjectAdapterDeactivatedException& localEx = dynamic_cast<const Ice::ObjectAdapterDeactivatedException&>(ex);
    name_ = toNSString(localEx.name);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::ObjectAdapterDeactivatedException(file, line, fromNSString(name_));
}
@end

@implementation ICEObjectAdapterIdInUseException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::ObjectAdapterIdInUseException*>(&ex), @"invalid local exception type");
    const Ice::ObjectAdapterIdInUseException& localEx = dynamic_cast<const Ice::ObjectAdapterIdInUseException&>(ex);
    id_ = toNSString(localEx.id);
    return self;
}
@end

@implementation ICENoEndpointException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::NoEndpointException*>(&ex), @"invalid local exception type");
    const Ice::NoEndpointException& localEx = dynamic_cast<const Ice::NoEndpointException&>(ex);
    proxy = toNSString(localEx.proxy);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::NoEndpointException(file, line, fromNSString(proxy));
}
@end

@implementation ICEEndpointParseException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::EndpointParseException*>(&ex), @"invalid local exception type");
    const Ice::EndpointParseException& localEx = dynamic_cast<const Ice::EndpointParseException&>(ex);
    str = toNSString(localEx.str);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::EndpointParseException(file, line, fromNSString(str));
}
@end

@implementation ICEEndpointSelectionTypeParseException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::EndpointSelectionTypeParseException*>(&ex), @"invalid local exception type");
    const Ice::EndpointSelectionTypeParseException& localEx = dynamic_cast<const Ice::EndpointSelectionTypeParseException&>(ex);
    str = toNSString(localEx.str);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::EndpointSelectionTypeParseException(file, line, fromNSString(str));
}
@end

@implementation ICEIdentityParseException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::IdentityParseException*>(&ex), @"invalid local exception type");
    const Ice::IdentityParseException& localEx = dynamic_cast<const Ice::IdentityParseException&>(ex);
    str = toNSString(localEx.str);
    return self;
}
@end

@implementation ICEProxyParseException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::ProxyParseException*>(&ex), @"invalid local exception type");
    const Ice::ProxyParseException& localEx = dynamic_cast<const Ice::ProxyParseException&>(ex);
    str = toNSString(localEx.str);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::ProxyParseException(file, line, fromNSString(str));
}
@end

@implementation ICEIllegalIdentityException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::IllegalIdentityException*>(&ex), @"invalid local exception type");
    const Ice::IllegalIdentityException& localEx = dynamic_cast<const Ice::IllegalIdentityException&>(ex);
    id_ = [[ICEIdentity alloc] initWithIdentity:localEx.id];
    return self;
}
-(void) rethrowCxx
{
    Ice::Identity ident = { fromNSString(id_.name), fromNSString(id_.category) };
    throw Ice::IllegalIdentityException(file, line, ident);
}
@end

@implementation ICERequestFailedException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::RequestFailedException*>(&ex), @"invalid local exception type");
    const Ice::RequestFailedException& localEx = dynamic_cast<const Ice::RequestFailedException&>(ex);
    id_ = [[ICEIdentity alloc] initWithIdentity:localEx.id];
    facet = toNSString(localEx.facet);
    operation = toNSString(localEx.operation);
    return self;
}
-(void) rethrowCxx
{
    Ice::Identity ident = { fromNSString(id_.name), fromNSString(id_.category) };
    throw Ice::RequestFailedException(file, line, ident, fromNSString(facet), fromNSString(operation));
}
@end

@implementation ICEObjectNotExistException (ICEInternal)
-(void) rethrowCxx
{
    Ice::Identity ident = { fromNSString([self id_].name), fromNSString([self id_].category) };
    throw Ice::ObjectNotExistException(file, line, ident, fromNSString([self facet]), fromNSString([self operation]));
}
@end

@implementation ICEFacetNotExistException (ICEInternal)
-(void) rethrowCxx
{
    Ice::Identity ident = { fromNSString([self id_].name), fromNSString([self id_].category) };
    throw Ice::FacetNotExistException(file, line, ident, fromNSString([self facet]), fromNSString([self operation]));
}
@end

@implementation ICEOperationNotExistException (ICEInternal)
-(void) rethrowCxx
{
    Ice::Identity ident = { fromNSString([self id_].name), fromNSString([self id_].category) };
    throw Ice::OperationNotExistException(file, line, ident, fromNSString([self facet]), fromNSString([self operation]));
}
@end

@implementation ICESyscallException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::SyscallException*>(&ex), @"invalid local exception type");
    const Ice::SyscallException& localEx = dynamic_cast<const Ice::SyscallException&>(ex);
    error = localEx.error;
    return self;
}
-(void) rethrowCxx
{
    throw Ice::SyscallException(file, line, error);
}
@end

@implementation ICESocketException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::SocketException(file, line, [self error]);
}
@end

@implementation ICEConnectFailedException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::ConnectFailedException(file, line, [self error]);
}
@end

@implementation ICEConnectionRefusedException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::ConnectionRefusedException(file, line, [self error]);
}
@end

@implementation ICEConnectionLostException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::ConnectionLostException(file, line, [self error]);
}
@end

@implementation ICEFileException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::FileException*>(&ex), @"invalid local exception type");
    const Ice::FileException& localEx = dynamic_cast<const Ice::FileException&>(ex);
    path = toNSString(localEx.path);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::FileException(file, line, [self error], fromNSString(path));
}
@end

@implementation ICEDNSException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::DNSException*>(&ex), @"invalid local exception type");
    const Ice::DNSException& localEx = dynamic_cast<const Ice::DNSException&>(ex);
    error = localEx.error;
    host = toNSString(localEx.host);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::DNSException(file, line, [self error], fromNSString(host));
}
@end

@implementation ICETimeoutException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::TimeoutException(file, line);
}
@end

@implementation ICEConnectTimeoutException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::ConnectTimeoutException(file, line);
}
@end

@implementation ICECloseTimeoutException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::CloseTimeoutException(file, line);
}
@end

@implementation ICEConnectionTimeoutException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::ConnectionTimeoutException(file, line);
}
@end

@implementation ICEInvocationTimeoutException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::InvocationTimeoutException(file, line);
}
@end

@implementation ICEInvocationCanceledException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::InvocationCanceledException(file, line);
}
@end

@implementation ICEProtocolException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::ProtocolException*>(&ex), @"invalid local exception type");
    const Ice::ProtocolException& localEx = dynamic_cast<const Ice::ProtocolException&>(ex);
    reason_ = toNSString(localEx.reason);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::ProtocolException(file, line, fromNSString(reason_));
}
@end

@implementation ICEUnknownMessageException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::UnknownMessageException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEConnectionNotValidatedException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::ConnectionNotValidatedException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEUnknownRequestIdException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::UnknownRequestIdException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEUnknownReplyStatusException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::UnknownReplyStatusException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICECloseConnectionException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::CloseConnectionException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEIllegalMessageSizeException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::IllegalMessageSizeException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICECompressionException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::CompressionException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEDatagramLimitException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::DatagramLimitException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEMarshalException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::MarshalException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEProxyUnmarshalException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::ProxyUnmarshalException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEUnmarshalOutOfBoundsException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::UnmarshalOutOfBoundsException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEMemoryLimitException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::MemoryLimitException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEStringConversionException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::StringConversionException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEEncapsulationException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::EncapsulationException(file, line, fromNSString([self reason_]));
}
@end

@implementation ICEBadMagicException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::BadMagicException*>(&ex), @"invalid local exception type");
    const Ice::BadMagicException& localEx = dynamic_cast<const Ice::BadMagicException&>(ex);
    badMagic = toNSData(localEx.badMagic);
    return self;
}
-(void) rethrowCxx
{
    Ice::ByteSeq s;
    throw Ice::BadMagicException(file, line, fromNSString([self reason_]), fromNSData(badMagic, s));
}
@end

@implementation ICEUnsupportedProtocolException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::UnsupportedProtocolException*>(&ex), @"invalid local exception type");
    const Ice::UnsupportedProtocolException& localEx = dynamic_cast<const Ice::UnsupportedProtocolException&>(ex);
    bad = [[ICEProtocolVersion protocolVersionWithProtocolVersion:localEx.bad] retain];
    supported = [[ICEProtocolVersion protocolVersionWithProtocolVersion:localEx.supported] retain];
    return self;
}
-(void) rethrowCxx
{
    Ice::ProtocolVersion badVersion = [bad protocolVersion];
    Ice::ProtocolVersion supportedVersion = [supported protocolVersion];
    throw Ice::UnsupportedProtocolException(file, line, fromNSString([self reason_]), badVersion, supportedVersion);
}
@end

@implementation ICEUnsupportedEncodingException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::UnsupportedEncodingException*>(&ex), @"invalid local exception type");
    const Ice::UnsupportedEncodingException& localEx = dynamic_cast<const Ice::UnsupportedEncodingException&>(ex);
    bad = [[ICEEncodingVersion encodingVersionWithEncodingVersion:localEx.bad] retain];
    supported = [[ICEEncodingVersion encodingVersionWithEncodingVersion:localEx.supported] retain];
    return self;
}
-(void) rethrowCxx
{
    Ice::EncodingVersion badVersion = [bad encodingVersion];
    Ice::EncodingVersion supportedVersion = [supported encodingVersion];
    throw Ice::UnsupportedEncodingException(file, line, fromNSString([self reason_]), badVersion, supportedVersion);
}
@end

@implementation ICENoValueFactoryException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::NoValueFactoryException*>(&ex), @"invalid local exception type");
    const Ice::NoValueFactoryException& localEx = dynamic_cast<const Ice::NoValueFactoryException&>(ex);
    type = toNSString(localEx.type);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::NoValueFactoryException(file, line, fromNSString([self reason_]), fromNSString(type));
}
@end

@implementation ICEUnexpectedObjectException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::UnexpectedObjectException*>(&ex), @"invalid local exception type");
    const Ice::UnexpectedObjectException& localEx = dynamic_cast<const Ice::UnexpectedObjectException&>(ex);
    type = toNSString(localEx.type);
    expectedType = toNSString(localEx.expectedType);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::UnexpectedObjectException(file, line, fromNSString([self reason_]), fromNSString(type), fromNSString(expectedType));
}
@end

@implementation ICEFeatureNotSupportedException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::FeatureNotSupportedException*>(&ex), @"invalid local exception type");
    const Ice::FeatureNotSupportedException& localEx = dynamic_cast<const Ice::FeatureNotSupportedException&>(ex);
    unsupportedFeature = toNSString(localEx.unsupportedFeature);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::FeatureNotSupportedException(file, line, fromNSString(unsupportedFeature));
}
@end

@implementation ICESecurityException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::SecurityException*>(&ex), @"invalid local exception type");
    const Ice::SecurityException& localEx = dynamic_cast<const Ice::SecurityException&>(ex);
    reason_ = toNSString(localEx.reason);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::SecurityException(file, line, fromNSString(reason_));
}
@end

@implementation ICEFixedProxyException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::FixedProxyException(file, line);
}
@end

@implementation ICEResponseSentException (ICEInternal)
-(void) rethrowCxx
{
    throw Ice::ResponseSentException(file, line);
}
@end

#ifdef ICE_USE_CFSTREAM
@implementation ICECFNetworkException (ICEInternal)
-(id)initWithLocalException:(const Ice::LocalException&)ex
{
    self = [super initWithLocalException:ex];
    if(!self)
    {
        return nil;
    }
    NSAssert(dynamic_cast<const Ice::CFNetworkException*>(&ex), @"invalid local exception type");
    const Ice::CFNetworkException& localEx = dynamic_cast<const Ice::CFNetworkException&>(ex);
    domain = toNSString(localEx.domain);
    return self;
}
-(void) rethrowCxx
{
    throw Ice::CFNetworkException(file, line, fromNSString(domain));
}
@end
#endif
