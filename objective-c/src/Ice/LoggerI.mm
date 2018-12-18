// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <LoggerI.h>
#import <Util.h>
#import <LocalObjectI.h>

#import <Foundation/NSDate.h>

namespace
{

class LoggerWrapperI : public Ice::Logger
{
public:

    // We must explicitely CFRetain/CFRelease so that the garbage
    // collector does not trash the logger.
    LoggerWrapperI(id<ICELogger> logger) : _logger(logger)
    {
        CFRetain(_logger);
    }

    virtual ~LoggerWrapperI()
    {
        CFRelease(_logger);
    }

    virtual void
    print(const std::string& msg)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            NSString* s = toNSString(msg);
            @try
            {
                [_logger print:s];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
            @finally
            {
                [s release];
            }
        }
        if(ex != nil)
        {
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

    virtual void
    trace(const std::string& category, const std::string& msg)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            NSString* s1 = toNSString(category);
            NSString* s2 = toNSString(msg);
            @try
            {
                [_logger trace:s1 message:s2];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
            @finally
            {
                [s1 release];
                [s2 release];
            }
        }
        if(ex != nil)
        {
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

    virtual void
    warning(const std::string& msg)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            NSString* s = toNSString(msg);
            @try
            {
                [_logger warning:s];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
            @finally
            {
                [s release];
            }
        }
        if(ex != nil)
        {
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

    virtual void
    error(const std::string& msg)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            NSString* s = toNSString(msg);
            @try
            {
                [_logger error:s];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
            @finally
            {
                [s release];
            }
        }
        if(ex != nil)
        {
            rethrowCxxException(ex, true); // True = release the exception.
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
typedef IceUtil::Handle<LoggerWrapperI> LoggerWrapperIPtr;

}

@implementation ICELoggerWrapper
+(id) loggerWithLogger:(Ice::Logger*)cxxObject
{
    LoggerWrapperI* impl = dynamic_cast<LoggerWrapperI*>(cxxObject);
    if(impl)
    {
        return [[impl->getLogger() retain] autorelease];
    }
    else
    {
        ICELoggerWrapper* wrapper = [self localObjectWithCxxObject:static_cast<IceUtil::Shared*>(cxxObject)];
        wrapper->logger_ = cxxObject;
        return wrapper;
    }
}
-(void) print:(NSString*)message
{
    logger_->print(fromNSString(message));
}
-(void) trace:(NSString*)category message:(NSString*)message
{
    logger_->trace(fromNSString(category), fromNSString(message));
}
-(void) warning:(NSString*)message
{
    logger_->warning(fromNSString(message));
}
-(void) error:(NSString*)message
{
    logger_->error(fromNSString(message));
}
-(NSMutableString*) getPrefix
{
    return toNSMutableString(logger_->getPrefix());
}
-(id<ICELogger>) cloneWithPrefix:(NSString*)prefix
{
    Ice::LoggerPtr logger = logger_->cloneWithPrefix(fromNSString(prefix));
    if(logger.get() != logger_)
    {
        return [ICELoggerWrapper loggerWithLogger:logger.get()];
    }
    return self;
}
@end

@implementation ICELogger
+(Ice::Logger*) loggerWithLogger:(id<ICELogger>)logger
{
    if(logger == 0)
    {
        id<ICELogger> l = [[self alloc] init];
        Ice::Logger* impl = new LoggerWrapperI(l);
        [l release];
        return impl;
    }
    else
    {
        return new LoggerWrapperI(logger);
    }
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
