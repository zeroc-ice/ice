// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice/Object.h>

@interface ICERequest : NSObject<ICERequest>
{
    @private
        ICECurrent* current;
        id<ICEInputStream> is;
        id<ICEOutputStream> os;
        BOOL needReset;
}
+(id) request:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
-(id) init:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
-(void) callDispatch:(ICEServant*)servant;
@end
