//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <FacetsTest.h>

@interface TestFacetsAI : TestFacetsA<TestFacetsA>
@end

@interface TestFacetsBI : TestFacetsB<TestFacetsB>
@end

@interface TestFacetsCI : TestFacetsC<TestFacetsC>
@end

@interface TestFacetsDI : TestFacetsD<TestFacetsD>
@end

@interface TestFacetsEI : TestFacetsE<TestFacetsE>
@end

@interface TestFacetsFI : TestFacetsF<TestFacetsF>
@end

@interface TestFacetsGI : TestFacetsG<TestFacetsG>
{
    id<ICECommunicator> communicator_;
}
-(void) shutdown:(ICECurrent*)current;
-(NSString*) callG:(ICECurrent*)current;
@end

@interface TestFacetsHI : TestFacetsH<TestFacetsH>
-(NSString*) callH:(ICECurrent*)current;
@end
