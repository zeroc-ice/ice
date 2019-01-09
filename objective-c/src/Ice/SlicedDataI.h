// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice/SlicedData.h>

#include <Ice/SlicedData.h>

@interface ICESlicedData : NSObject<ICESlicedData>
{
@private
    Ice::SlicedData* slicedData_;
}
-(id) initWithSlicedData:(Ice::SlicedData*)slicedData;
-(Ice::SlicedData*) slicedData;
@end
