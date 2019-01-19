//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <InheritanceTest.h>

@interface TestInheritanceInitialI : TestInheritanceInitial<TestInheritanceInitial>
{
    id<ICEObjectAdapter> adapter_;
    id<TestInheritanceMACAPrx> ca_;
    id<TestInheritanceMBCBPrx> cb_;
    id<TestInheritanceMACCPrx> cc_;
    id<TestInheritanceMACDPrx> cd_;
    id<TestInheritanceMAIAPrx> ia_;
    id<TestInheritanceMBIB1Prx> ib1_;
    id<TestInheritanceMBIB2Prx> ib2_;
    id<TestInheritanceMAICPrx> ic_;
}
-(id)initWithAdapter:(id<ICEObjectAdapter>) adapter;
@end

@interface CAI : TestInheritanceMACA<TestInheritanceMACA>
@end

@interface CBI : TestInheritanceMBCB<TestInheritanceMBCB>
@end

@interface CCI : TestInheritanceMACC<TestInheritanceMACC>
@end

@interface IAI : TestInheritanceMAIA<TestInheritanceMAIA>
@end

@interface IB1I : TestInheritanceMBIB1<TestInheritanceMBIB1>
@end

@interface IB2I : TestInheritanceMBIB2<TestInheritanceMBIB2>
@end

@interface ICI : TestInheritanceMAIC<TestInheritanceMAIC>
@end

@interface CDI : TestInheritanceMACD<TestInheritanceMACD>
@end
