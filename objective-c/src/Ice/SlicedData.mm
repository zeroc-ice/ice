// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <SlicedDataI.h>
#import <Util.h>

#import <Ice/SlicedData.h>

@implementation ICESlicedData

-(id) initWithSlicedData:(Ice::SlicedData*)slicedData
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->slicedData_ = slicedData;
    self->slicedData_->__incRef();
    return self;
}

-(void) dealloc
{
    self->slicedData_->__decRef();
    [super dealloc];
}

-(Ice::SlicedData*) slicedData
{
    return slicedData_;
}

-(void) clear
{
    slicedData_->clear();
}
@end

@implementation ICEUnknownSlicedValue

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->unknownTypeId_ = nil;
    self->slicedData_ = nil;
    return self;
}

-(void) dealloc
{
    [slicedData_ release];
    [unknownTypeId_ release];
    [super dealloc];
}

-(id<ICESlicedData>) ice_getSlicedData
{
    return [[slicedData_ retain] autorelease];
}

-(NSString*) ice_id
{
    return [[unknownTypeId_ retain] autorelease];
}

-(void) iceWrite:(id<ICEOutputStream>)os
{
    [os startValue:slicedData_];
    [os endValue];
}

-(void) iceRead:(id<ICEInputStream>)is
{
    [is startValue];
    slicedData_ = [is endValue:YES];

    // Initialize unknown type ID to type ID of first slice.
    Ice::SlicedData* slicedData = [((ICESlicedData*)slicedData_) slicedData];
    assert(!slicedData->slices.empty());
    unknownTypeId_ = toNSString(slicedData->slices[0]->typeId);
}

@end
