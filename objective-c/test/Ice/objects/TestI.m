// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <objects/TestI.h>

@implementation TestObjectsBI
-(void) ice_preMarshal
{
    preMarshalInvoked = YES;
}
-(void) ice_postUnmarshal
{
    postUnmarshalInvoked = YES;
}

@end

@implementation TestObjectsCI
-(void) ice_preMarshal
{
    preMarshalInvoked = YES;
}
-(void) ice_postUnmarshal
{
    postUnmarshalInvoked = YES;
}

@end

@implementation TestObjectsDI
-(void) ice_preMarshal
{
    preMarshalInvoked = YES;
}
-(void) ice_postUnmarshal
{
    postUnmarshalInvoked = YES;
}
@end

@implementation TestObjectsEI
-(id) init
{
    return [super init:1 s:@"hello"];
}

@end

@implementation TestObjectsFI
-(id) init:(TestObjectsE*)e1_ e2:(TestObjectsE*)e2_
{
    self = [super init:e1_ e2:e2_];
    if(!self)
    {
        return nil;
    }
    return self;
}
-(BOOL) checkValues:(ICECurrent*)__unused current
{
    return e1 && e1 == e2;
}
@end

@implementation TestObjectsHI
@end

/*@implementation TestObjectsII
-(void) dealloc
{
    printf("Deallocating Object");
}
@end*/

@implementation TestObjectsJI
@end

@implementation TestObjectsInitialI

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    _b1 = [[TestObjectsBI alloc] init];
    _b2 = [[TestObjectsBI alloc] init];
    _c = [[TestObjectsCI alloc] init];
    _d = [[TestObjectsDI alloc] init];
    _e = [[TestObjectsEI alloc] init];
    _f = [[TestObjectsFI alloc] init:_e e2:_e];

    _b1.theA = _b2; // Cyclic reference to another B
    _b1.theB = _b1; // Self reference.
    _b1.theC = nil; // Null reference.

    _b2.theA = _b2; // Self reference, using base.
    _b2.theB = _b1; // Cyclic reference to another B
    _b2.theC = _c; // Cyclic reference to a C.

    _c.theB = _b2; // Cyclic reference to a B.

    _d.theA = _b1; // Reference to a B.
    _d.theB = _b2; // Reference to a B.
    _d.theC = nil; // Reference to a C.
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_b1 release];
    [_b2 release];
    [_c release];
    [_d release];
    [_e release];
    [_f release];
    [super dealloc];
}
#endif

-(void) shutdown:(ICECurrent*)__unused current
{
    _b1.theA = nil; // Break cyclic reference.
    _b1.theB = nil; // Break cyclic reference.

    _b2.theA = nil; // Break cyclic reference.
    _b2.theB = nil; // Break cyclic reference.
    _b2.theC = nil; // Break cyclic reference.

    _c.theB = nil; // Break cyclic reference.

    _d.theA = nil; // Break cyclic reference.
    _d.theB = nil; // Break cyclic reference.
    _d.theC = nil; // Break cyclic reference.
    [[current.adapter getCommunicator] shutdown];
}

-(TestObjectsB*) getB1:(ICECurrent*)__unused current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    return _b1;
}

-(TestObjectsB*) getB2:(ICECurrent*)__unused current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    return _b2;
}

-(TestObjectsC*) getC:(ICECurrent*)__unused current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    return _c;
}

-(TestObjectsD*) getD:(ICECurrent*)__unused current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    _d.preMarshalInvoked = NO;
    return _d;
}

-(TestObjectsE*) getE:(ICECurrent*)__unused current
{
    return _e;
}

-(TestObjectsF*) getF:(ICECurrent*)__unused current
{
    return _f;
}

-(void) setRecursive:(TestObjectsRecursive*)__unused recursive current:(ICECurrent*)__unused current
{
}

-(BOOL) supportsClassGraphDepthMax:(ICECurrent*)__unused current
{
    return YES;
}

-(TestObjectsB*) getMB:(ICECurrent*)__unused current
{
    return _b1;
}

-(TestObjectsB*) getAMDMB:(ICECurrent*)__unused current
{
    return _b1;
}

-(void) getAll:(TestObjectsB **)b1 b2:(TestObjectsB **)b2 theC:(TestObjectsC **)theC theD:(TestObjectsD **)theD current:(ICECurrent *)__unused current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    _d.preMarshalInvoked = NO;
    *b1 = _b1;
    *b2 = _b2;
    *theC = _c;
    *theD = _d;
}

-(TestObjectsI*) getI:(ICECurrent*)__unused current
{
    return [TestObjectsI i];
}

-(TestObjectsI*) getJ:(ICECurrent*)__unused current
{
    return (TestObjectsI*)[TestObjectsJ j];
}

-(TestObjectsI*) getH:(ICECurrent*)__unused current
{
    return (TestObjectsI*)[TestObjectsH h];
}

-(ICEObject*) getK:(ICECurrent*)__unused current
{
    return [[TestObjectsK alloc] init:[[TestObjectsL alloc] init:@"l"]];
}

-(ICEObject*) opValue:(ICEObject*)v1 v2:(ICEObject**)v2 current:(ICECurrent*)__unused current
{
    *v2 = v1;
    return v1;
}

-(TestObjectsValueSeq*) opValueSeq:(TestObjectsMutableValueSeq*)v1 v2:(TestObjectsValueSeq**)v2 current:(ICECurrent*)__unused current
{
    *v2 = v1;
    return v1;
}

-(TestObjectsValueMap*) opValueMap:(TestObjectsMutableValueMap*)v1 v2:(TestObjectsValueMap**)v2 current:(ICECurrent*)__unused current
{
    *v2 = v1;
    return v1;
}

-(TestObjectsI*) getD1:(TestObjectsI*)d1 current:(ICECurrent*)__unused current
{
    return d1;
}

-(void) throwEDerived:(ICECurrent*)__unused current
{
    @throw [TestObjectsEDerived eDerived:[TestObjectsA1 a1:@"a1"]
                                      a2:[TestObjectsA1 a1:@"a2"]
                                      a3:[TestObjectsA1 a1:@"a3"]
                                      a4:[TestObjectsA1 a1:@"a4"]];
}

-(TestObjectsBaseSeq*) opBaseSeq:(TestObjectsMutableBaseSeq*)inSeq outSeq:(TestObjectsBaseSeq**)outSeq
                         current:(ICECurrent*)__unused current
{
    *outSeq = inSeq;
    return inSeq;
}

-(TestObjectsCompact*) getCompact:(ICECurrent*)__unused current
{
    return (TestObjectsCompact*)[TestObjectsCompactExt compactExt];
}

-(TestInnerA*) getInnerA:(ICECurrent *)__unused current
{
    return [TestInnerA a:_b1];
}

-(TestInnerSubA*) getInnerSubA:(ICECurrent *)__unused current
{
    return [TestInnerSubA a:[TestInnerA a:_b1]];
}

-(void) throwInnerEx:(ICECurrent *)__unused current
{
    @throw [TestInnerEx ex:@"Inner::Ex"];
}

-(void) throwInnerSubEx:(ICECurrent *)__unused current
{
    @throw [TestInnerSubEx ex:@"Inner::Sub::Ex"];
}

-(void) setG:(TestObjectsG*)__unused g current:(ICECurrent*)__unused current
{
}

-(void) setI:(TestObjectsI*)__unused i current:(ICECurrent*)__unused current
{
}

-(TestObjectsObjectSeq *) getObjectSeq:(TestObjectsMutableObjectSeq *)s current:(ICECurrent*)__unused current
{
    return s;
}

-(TestObjectsObjectPrxSeq *) getObjectPrxSeq:(TestObjectsMutableObjectPrxSeq *)s current:(ICECurrent*)__unused current
{
    return s;
}

-(TestObjectsBaseSeq *) getBaseSeq:(TestObjectsMutableBaseSeq *)s current:(ICECurrent*)__unused current
{
    return s;
}

-(TestObjectsBasePrxSeq *) getBasePrxSeq:(TestObjectsMutableBasePrxSeq *)s current:(ICECurrent*)__unused current
{
    return s;
}

-(TestObjectsObjectDict *) getObjectDict:(TestObjectsMutableObjectDict *)d current:(ICECurrent*)__unused current
{
    return d;
}

-(TestObjectsObjectPrxDict *) getObjectPrxDict:(TestObjectsMutableObjectPrxDict *)d current:(ICECurrent*)__unused current
{
    return d;
}

-(TestObjectsBaseDict *) getBaseDict:(TestObjectsMutableBaseDict *)d current:(ICECurrent*)__unused current
{
    return d;
}

-(TestObjectsBasePrxDict *) getBasePrxDict:(TestObjectsMutableBasePrxDict *)d current:(ICECurrent*)__unused current
{
    return d;
}

-(TestObjectsM *) opM:(TestObjectsM *)v1 v2:(TestObjectsM **)v2 current:(ICECurrent *)__unused current
{
    *v2 = v1;
    return v1;
}
@end

@implementation UnexpectedObjectExceptionTestI
-(BOOL)ice_invoke:(NSData*)__unused inEncaps outEncaps:(NSMutableData**)outEncaps current:(ICECurrent*)current
{
    id<ICECommunicator> communicator = [current.adapter getCommunicator];
    id<ICEOutputStream> o = [ICEUtil createOutputStream:communicator];
    [o startEncapsulation];
    TestObjectsAlsoEmpty* ae = [TestObjectsAlsoEmpty alsoEmpty];
    [o writeValue:ae];
    [o writePendingValues];
    [o endEncapsulation];
    *outEncaps = [o finished];
    return YES;
}
@end
