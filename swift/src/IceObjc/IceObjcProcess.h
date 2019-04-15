//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "IceObjcLocalObject.h"

NS_ASSUME_NONNULL_BEGIN

@interface ICEProcess : ICELocalObject
-(BOOL) shutdown:(NSError**)error;
-(BOOL) writeMessage:(NSString*)message fd:(int32_t)fd error:(NSError**)error;
@end

#ifdef __cplusplus

@interface ICEProcess()
@property (nonatomic, readonly) std::shared_ptr<Ice::Process> process;
-(instancetype) initWithCppProcess:(std::shared_ptr<Ice::Process>)process;
@end

#endif

NS_ASSUME_NONNULL_END
