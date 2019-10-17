//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Logger.h"
#import "Convert.h"

@implementation ICELogger
-(std::shared_ptr<Ice::Logger>) logger
{
    return std::static_pointer_cast<Ice::Logger>(self.cppObject);
}

-(void) print:(NSString*)message
{
    self.logger->print(fromNSString(message));
}

-(void) trace:(NSString*)category message:(NSString*)message
{
    self.logger->trace(fromNSString(category), fromNSString(message));
}

-(void) warning:(NSString*)message
{
    self.logger->warning(fromNSString(message));
}

-(void) error:(NSString*)message
{
    self.logger->error(fromNSString(message));
}

-(NSString*) getPrefix
{
    return toNSString(self.logger->getPrefix());
}

-(id) cloneWithPrefix:(NSString*)prefix
{
    return [ICELogger getHandle:self.logger->cloneWithPrefix(fromNSString(prefix))];
}
@end
