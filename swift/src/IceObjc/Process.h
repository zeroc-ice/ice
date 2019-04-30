//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "LocalObject.h"

NS_ASSUME_NONNULL_BEGIN

@interface ICEProcess : ICELocalObject
-(void) shutdown;
-(void) writeMessage:(NSString*)message fd:(int32_t)fd;
@end

#ifdef __cplusplus

@interface ICEProcess()
@property (nonatomic, readonly) std::shared_ptr<Ice::Process> process;
@end

#endif

NS_ASSUME_NONNULL_END
