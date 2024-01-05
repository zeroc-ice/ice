//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <inheritance/TestI.h>

@implementation IAI
-(id<TestInheritanceMAIAPrx>) iaop:(id<TestInheritanceMAIAPrx>)p current:(ICECurrent*)__unused current
{
    return p;
}
@end

@implementation IB1I
-(id<TestInheritanceMAIAPrx>) iaop:(id<TestInheritanceMAIAPrx>)p current:(ICECurrent*)__unused current
{
    return p;
}
-(id<TestInheritanceMBIB1Prx>) ib1op:(id<TestInheritanceMBIB1Prx>)p current:(ICECurrent*)__unused current
{
    return p;
}
@end

@implementation IB2I
-(id<TestInheritanceMAIAPrx>) iaop:(id<TestInheritanceMAIAPrx>)p current:(ICECurrent*)__unused current
{
    return p;
}
-(id<TestInheritanceMBIB2Prx>) ib2op:(id<TestInheritanceMBIB2Prx>)p current:(ICECurrent*)__unused current
{
    return p;
}
@end

@implementation ICI
-(id<TestInheritanceMAIAPrx>) iaop:(id<TestInheritanceMAIAPrx>)p current:(ICECurrent*)__unused current
{
    return p;
}
-(id<TestInheritanceMBIB1Prx>) ib1op:(id<TestInheritanceMBIB1Prx>)p current:(ICECurrent*)__unused current
{
    return p;
}
-(id<TestInheritanceMBIB2Prx>) ib2op:(id<TestInheritanceMBIB2Prx>)p current:(ICECurrent*)__unused current
{
    return p;
}
-(id<TestInheritanceMAICPrx>) icop:(id<TestInheritanceMAICPrx>)p current:(ICECurrent*)__unused current
{
    return p;
}
@end

@implementation TestInheritanceInitialI
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    ia_ = [TestInheritanceMAIAPrx uncheckedCast:[adapter addWithUUID:[IAI ia]]];
    ib1_ = [TestInheritanceMBIB1Prx uncheckedCast:[adapter addWithUUID:[IB1I ib1]]];
    ib2_ = [TestInheritanceMBIB2Prx uncheckedCast:[adapter addWithUUID:[IB2I ib2]]];
    ic_ = [TestInheritanceMAICPrx uncheckedCast:[adapter addWithUUID:[ICI ic]]];

    return self;
}
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}

-(id<TestInheritanceMAIAPrx>) iaop:(ICECurrent*)__unused current
{
    return ia_;
}

-(id<TestInheritanceMBIB1Prx>) ib1op:(ICECurrent*)__unused current
{
    return ib1_;
}

-(id<TestInheritanceMBIB2Prx>) ib2op:(ICECurrent*)__unused current
{
    return ib2_;
}

-(id<TestInheritanceMAICPrx>) icop:(ICECurrent*)__unused current
{
    return ic_;
}
@end
