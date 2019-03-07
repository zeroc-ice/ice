// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcLogger.h"

#include "IceObjcUtil.h"

@implementation ICELogger
-(instancetype) initWithCppLogger:(std::shared_ptr<Ice::Logger>)logger
{
    self = [super initWithLocalObject:logger.get()];
    if(self)
    {
        self->_logger = logger;
    }
    return self;
}

-(void) print:(NSString*)message
{
    _logger->print(fromNSString(message));
}

-(void) trace:(NSString*)category message:(NSString*)message
{
    _logger->trace(fromNSString(category), fromNSString(message));
}

-(void) warning:(NSString*)message
{
    _logger->warning(fromNSString(message));
}

-(void) error:(NSString*)message
{
    _logger->error(fromNSString(message));
}

-(NSString*) getPrefix
{
    return toNSString(_logger->getPrefix());
}

-(id) cloneWithPrefix:(NSString*)prefix
{
    auto l = _logger->cloneWithPrefix(fromNSString(prefix));
    return [[ICELogger alloc] initWithCppLogger:l];
}
@end
