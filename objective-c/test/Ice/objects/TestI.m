// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <objects/TestI.h>

@implementation TestObjectsBI
-(BOOL) postUnmarshalInvoked:(ICECurrent*)current
{
    return _postUnmarshalInvoked;
}
-(void) ice_preMarshal
{
    preMarshalInvoked = YES;
}
-(void) ice_postUnmarshal
{
    _postUnmarshalInvoked = YES;
}

@end

@implementation TestObjectsCI
-(BOOL) postUnmarshalInvoked:(ICECurrent*)current
{
    return _postUnmarshalInvoked;
}
-(void) ice_preMarshal
{
    preMarshalInvoked = YES;
}
-(void) ice_postUnmarshal
{
    _postUnmarshalInvoked = YES;
}

@end

@implementation TestObjectsDI
-(BOOL) postUnmarshalInvoked:(ICECurrent*)current
{
    return _postUnmarshalInvoked;
}
-(void) ice_preMarshal
{
    preMarshalInvoked = YES;
}
-(void) ice_postUnmarshal
{
    _postUnmarshalInvoked = YES;
}
@end

@implementation TestObjectsEI
-(id) init
{
    return [super init:1 s:@"hello"];
}
-(BOOL) checkValues:(ICECurrent*)current
{
    return i == 1 && [s isEqualToString:@"hello"];
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
-(BOOL) checkValues:(ICECurrent*)current
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

-(void) shutdown:(ICECurrent*)current
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

-(TestObjectsB*) getB1:(ICECurrent*)current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    return _b1;
}

-(TestObjectsB*) getB2:(ICECurrent*)current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    return _b2;
}


-(TestObjectsC*) getC:(ICECurrent*)current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    return _c;
}


-(TestObjectsD*) getD:(ICECurrent*)current
{
    _b1.preMarshalInvoked = NO;
    _b2.preMarshalInvoked = NO;
    _c.preMarshalInvoked = NO;
    _d.preMarshalInvoked = NO;
    return _d;
}


-(TestObjectsE*) getE:(ICECurrent*)current
{
    return _e;
}


-(TestObjectsF*) getF:(ICECurrent*)current
{
    return _f;
}


-(void) getAll:(TestObjectsB **)b1 b2:(TestObjectsB **)b2 theC:(TestObjectsC **)theC theD:(TestObjectsD **)theD current:(ICECurrent *)current;
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

-(TestObjectsI*) getI:(ICECurrent*)current
{
    return [TestObjectsI i];
}

-(TestObjectsI*) getJ:(ICECurrent*)current
{
    return (TestObjectsI*)[TestObjectsJ j];
}

-(TestObjectsI*) getH:(ICECurrent*)current
{
    return (TestObjectsI*)[TestObjectsH h];
}

-(TestObjectsI*) getD1:(TestObjectsI*)d1 current:(ICECurrent*)current
{
    return d1;
}

-(void) throwEDerived:(ICECurrent*)current
{
    @throw [TestObjectsEDerived eDerived:[TestObjectsA1 a1:@"a1"]
                                      a2:[TestObjectsA1 a1:@"a2"]
                                      a3:[TestObjectsA1 a1:@"a3"]
                                      a4:[TestObjectsA1 a1:@"a4"]];
}

-(TestObjectsBaseSeq*) opBaseSeq:(TestObjectsMutableBaseSeq*)inSeq outSeq:(TestObjectsBaseSeq**)outSeq
                         current:(ICECurrent*)current
{
    *outSeq = inSeq;
    return inSeq;
}

-(TestObjectsCompact*) getCompact:(ICECurrent*)current
{
    return (TestObjectsCompact*)[TestObjectsCompactExt compactExt];
}

-(TestInnerA*) getInnerA:(ICECurrent *)current
{
    return [TestInnerA a:_b1];
}

-(TestInnerSubA*) getInnerSubA:(ICECurrent *)current
{
    return [TestInnerSubA a:[TestInnerA a:_b1]];
}

-(void) throwInnerEx:(ICECurrent *)current
{
    @throw [TestInnerEx ex:@"Inner::Ex"];
}

-(void) throwInnerSubEx:(ICECurrent *)current
{
    @throw [TestInnerSubEx ex:@"Inner::Sub::Ex"];
}

-(void) setI:(TestObjectsI*)i current:(ICECurrent*)current
{
}

-(TestObjectsObjectSeq *) getObjectSeq:(TestObjectsMutableObjectSeq *)s current:(ICECurrent*)current
{
    return s;
}

-(TestObjectsObjectPrxSeq *) getObjectPrxSeq:(TestObjectsMutableObjectPrxSeq *)s current:(ICECurrent*)current
{
    return s;
}

-(TestObjectsBaseSeq *) getBaseSeq:(TestObjectsMutableBaseSeq *)s current:(ICECurrent*)current
{
    return s;
}

-(TestObjectsBasePrxSeq *) getBasePrxSeq:(TestObjectsMutableBasePrxSeq *)s current:(ICECurrent*)current
{
    return s;
}

-(TestObjectsObjectDict *) getObjectDict:(TestObjectsMutableObjectDict *)d current:(ICECurrent*)current
{
    return d;
}

-(TestObjectsObjectPrxDict *) getObjectPrxDict:(TestObjectsMutableObjectPrxDict *)d current:(ICECurrent*)current
{
    return d;
}

-(TestObjectsBaseDict *) getBaseDict:(TestObjectsMutableBaseDict *)d current:(ICECurrent*)current
{
    return d;
}

-(TestObjectsBasePrxDict *) getBasePrxDict:(TestObjectsMutableBasePrxDict *)d current:(ICECurrent*)current
{
    return d;
}
@end

@implementation UnexpectedObjectExceptionTestI
-(BOOL)ice_invoke:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps current:(ICECurrent*)current
{
    id<ICECommunicator> communicator = [current.adapter getCommunicator];
    id<ICEOutputStream> o = [ICEUtil createOutputStream:communicator];
    [o startEncapsulation];
    TestObjectsAlsoEmpty* ae = [TestObjectsAlsoEmpty alsoEmpty];
    [o writeObject:ae];
    [o writePendingObjects];
    [o endEncapsulation];
    *outEncaps = [o finished];
    return YES;
}
@end
