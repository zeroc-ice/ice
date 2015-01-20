// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <LoggerI.h>
#import <Util.h>

#import <Foundation/NSDate.h>

namespace
{

class LoggerI : public Ice::Logger
{
public:

// We must explicitely CFRetain/CFRelease so that the garbage
// collector does not trash the logger.
LoggerI(id<ICELogger> logger) : _logger(logger)
{
    CFRetain(_logger);
}

virtual ~LoggerI()
{
    CFRelease(_logger);
}

virtual void 
print(const std::string& msg)
{
    NSString* s = toNSString(msg);
    @try
    {
        [_logger print:s];
    }
    @catch(id ex)
    {
        rethrowCxxException(ex);
    }
    @finally
    {
        [s release];
    }
}

virtual void 
trace(const std::string& category, const std::string& msg)
{
    NSString* s1 = toNSString(category);
    NSString* s2 = toNSString(msg);
    @try
    {
        [_logger trace:s1 message:s2];
    }
    @catch(id ex)
    {
        rethrowCxxException(ex);
    }
    @finally
    {
        [s1 release];
        [s2 release];
    }
}

virtual void 
warning(const std::string& msg)
{
    NSString* s = toNSString(msg);
    @try
    {
        [_logger warning:s];
    }
    @catch(id ex)
    {
        rethrowCxxException(ex);
    }
    @finally
    {
        [s release];
    }
}

virtual void 
error(const std::string& msg)
{
    NSString* s = toNSString(msg);
    @try
    {
        [_logger error:s];
    }
    @catch(id ex)
    {
        rethrowCxxException(ex);
    }
    @finally
    {
        [s release];
    }
}

virtual Ice::LoggerPtr
cloneWithPrefix(const std::string& prefix)
{
    NSString* s = toNSString(prefix);
    @try
    {
        return [ICELogger loggerWithLogger:[_logger cloneWithPrefix:s]];
    }
    @finally
    {
        [s release];
    }
}

virtual std::string
getPrefix()
{
    return fromNSString([_logger getPrefix]);
}

id<ICELogger>
getLogger()
{
    return _logger;
}

private:

id<ICELogger> _logger;

};
typedef IceUtil::Handle<LoggerI> LoggerIPtr;

}

@implementation ICELogger
+(Ice::Logger*) loggerWithLogger:(id<ICELogger>)logger
{
    if(logger == 0)
    {
        id<ICELogger> l = [[self alloc] init];
        Ice::Logger* impl = new LoggerI(l);
        [l release];
        return impl;
    }
    else
    {
        return new LoggerI(logger);
    }
}
+(id) localObjectWithCxxObject:(IceUtil::Shared*)cxxObject
{
    LoggerI* impl = dynamic_cast<LoggerI*>(cxxObject);
    assert(impl);
    return [[impl->getLogger() retain] autorelease];
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->prefix_ = @"";
    self->formattedPrefix_ = @"";
    return self;
}
-(id) initWithPrefix:(NSString*)prefix
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->prefix_ = prefix == nil ? @"" : [prefix retain];
    if(self->prefix_.length > 0)
    {
        self->formattedPrefix_ = [[NSString alloc] initWithFormat:@"%@ :", prefix];
    }
    else
    {
        self->formattedPrefix_ = @"";
    }
    return self;
}
-(void) dealloc
{
    [self->prefix_ release];
    [self->formattedPrefix_ release];
    [super dealloc];
}

//
// @protocol Logger methods.
//

-(void) print:(NSString*)message
{
    NSLog(@"%@", message);
}

-(void) trace:(NSString*)cat message:(NSString*)msg
{
    NSMutableString* s = [[NSMutableString alloc] initWithFormat:@"%@[%@: %@]", self->formattedPrefix_, cat, msg];
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    [s replaceOccurrencesOfString:@"\n" withString:@" " options:0 range:NSMakeRange(0, s.length)];
#endif
    [self print:s];
    [s release];
}

-(void) warning:(NSString*)message
{
    NSString* s = [[NSString alloc] initWithFormat:@"%@warning: %@", self->formattedPrefix_, message]; 
    [self print:s];
    [s release];
}

-(void) error:(NSString*)message
{
    NSString* s = [[NSString alloc] initWithFormat:@"%@error: %@", self->formattedPrefix_, message];
    [self print:s];
    [s release];
}

-(NSMutableString*) getPrefix
{
    return [[self->prefix_ mutableCopy] autorelease];
}

-(id<ICELogger>) cloneWithPrefix:(NSString*)prefix
{
    return [[[ICELogger alloc] initWithPrefix:prefix] autorelease];
}

@end
