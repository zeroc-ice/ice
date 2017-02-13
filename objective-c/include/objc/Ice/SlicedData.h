// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in
// the ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>

#import <objc/Ice/Object.h>

ICE_API @protocol ICESlicedData<NSObject>
@end

ICE_API @interface ICEUnknownSlicedValue : ICEObject
{
@private
    NSString* unknownTypeId_;
    id<ICESlicedData> slicedData_;
}
-(NSString*) getUnknownTypeId;
-(id<ICESlicedData>) getSlicedData;
@end
