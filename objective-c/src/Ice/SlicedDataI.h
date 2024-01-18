//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/SlicedData.h>

#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <Ice/SlicedData.h>

@interface ICESlicedData : NSObject<ICESlicedData>
{
@private
    Ice::SlicedDataPtr slicedData_;
}
-(id) initWithSlicedData:(Ice::SlicedDataPtr)slicedData;
-(Ice::SlicedDataPtr) slicedData;
@end
