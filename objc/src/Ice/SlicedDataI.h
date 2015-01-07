// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in
// the ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/SlicedData.h>

#include <IceCpp/SlicedData.h>

@interface ICESlicedData : NSObject<ICESlicedData>
{
@private
    Ice::SlicedData* slicedData__;
}
-(id) initWithSlicedData:(Ice::SlicedData*)slicedData;
-(Ice::SlicedData*) slicedData;
@end
