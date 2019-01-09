// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <ObjectsTest.h>

@interface TestObjectsBI : TestObjectsB<TestObjectsB>
{
}
@end

@interface TestObjectsCI : TestObjectsC<TestObjectsC>
{
}
@end

@interface TestObjectsDI : TestObjectsD<TestObjectsD>
{
}
@end

@interface TestObjectsEI : TestObjectsE<TestObjectsE>
@end

@interface TestObjectsFI : TestObjectsF<TestObjectsF>
@end

//@interface TestObjectsII : TestObjectsI<TestObjectsI>
//@end

@interface TestObjectsJI : TestObjectsJ<TestObjectsJ>
@end

@interface TestObjectsHI : TestObjectsH<TestObjectsH>
@end

@interface TestObjectsInitialI : TestObjectsInitial<TestObjectsInitial>
{
    TestObjectsB* _b1;
    TestObjectsB* _b2;
    TestObjectsC* _c;
    TestObjectsD* _d;
    TestObjectsE* _e;
    TestObjectsF* _f;
}
-(id) init;
-(void) shutdown:(ICECurrent*)current;
-(TestObjectsB *) getB1:(ICECurrent *)current;
-(TestObjectsB *) getB2:(ICECurrent *)current;
-(TestObjectsC *) getC:(ICECurrent *)current;
-(TestObjectsD *) getD:(ICECurrent *)current;
-(TestObjectsE *) getE:(ICECurrent *)current;
-(TestObjectsF *) getF:(ICECurrent *)current;
-(void) getAll:(TestObjectsB **)b1 b2:(TestObjectsB **)b2 theC:(TestObjectsC **)theC theD:(TestObjectsD **)theD current:(ICECurrent *)current;
-(TestObjectsI *) getI:(ICECurrent *)current;
-(TestObjectsI *) getJ:(ICECurrent *)current;
-(TestObjectsI *) getH:(ICECurrent *)current;
-(TestObjectsCompact*) getCompact:(ICECurrent*)current;
-(void) setG:(TestObjectsG *)theG current:(ICECurrent *)current;
-(void) setI:(TestObjectsI *)theI current:(ICECurrent *)current;
-(TestObjectsObjectSeq *) getObjectSeq:(TestObjectsMutableObjectSeq *)s current:(ICECurrent *)current;
-(TestObjectsObjectPrxSeq *) getObjectPrxSeq:(TestObjectsMutableObjectPrxSeq *)s current:(ICECurrent *)current;
-(TestObjectsBaseSeq *) getBaseSeq:(TestObjectsMutableBaseSeq *)s current:(ICECurrent *)current;
-(TestObjectsBasePrxSeq *) getBasePrxSeq:(TestObjectsMutableBasePrxSeq *)s current:(ICECurrent *)current;
-(TestObjectsObjectDict *) getObjectDict:(TestObjectsMutableObjectDict *)d current:(ICECurrent *)current;
-(TestObjectsObjectPrxDict *) getObjectPrxDict:(TestObjectsMutableObjectPrxDict *)d current:(ICECurrent *)current;
-(TestObjectsBaseDict *) getBaseDict:(TestObjectsMutableBaseDict *)d current:(ICECurrent *)current;
-(TestObjectsBasePrxDict *) getBasePrxDict:(TestObjectsMutableBasePrxDict *)d current:(ICECurrent *)current;
 -(TestObjectsM *) opM:(TestObjectsM *)v1 v2:(TestObjectsM **)v2 current:(ICECurrent *)current;
@end

@interface UnexpectedObjectExceptionTestI : ICEBlobject<ICEBlobject>
@end

@interface TestObjectsTestIntfI : TestObjectsTestIntf<TestObjectsTestIntf>
-(TestObjectsBase*) opDerived:(ICECurrent *)current;
-(void) throwDerived:(ICECurrent *)current;
@end
