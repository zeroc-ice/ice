// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <slicing/objects/TestI.h>
#import <TestCommon.h>

@implementation TestSlicingObjectsServerI
-(ICEObject*) SBaseAsObject:(ICECurrent*)current
{
    return [TestSlicingObjectsServerSBase sBase:@"SBase.sb"];
}

-(TestSlicingObjectsServerSBase*) SBaseAsSBase:(ICECurrent*)current
{
    return [TestSlicingObjectsServerSBase sBase:@"SBase.sb"];
}

-(TestSlicingObjectsServerSBase*) SBSKnownDerivedAsSBase:(ICECurrent*)current
{
    return [TestSlicingObjectsServerSBSKnownDerived sbsKnownDerived:@"SBSKnownDerived.sb"
                                                              sbskd:@"SBSKnownDerived.sbskd"];
}

-(TestSlicingObjectsServerSBSKnownDerived*) SBSKnownDerivedAsSBSKnownDerived:(ICECurrent*)current
{
    return [TestSlicingObjectsServerSBSKnownDerived sbsKnownDerived:@"SBSKnownDerived.sb"
                                                              sbskd:@"SBSKnownDerived.sbskd"];
}

-(TestSlicingObjectsServerSBase*) SBSUnknownDerivedAsSBase:(ICECurrent*)current
{
    return [TestSlicingObjectsServerSBSUnknownDerived sbsUnknownDerived:@"SBSUnknownDerived.sb"
                                                                  sbsud:@"SBSUnknownDerived.sbsud"];
}

-(TestSlicingObjectsServerSBase*) SBSUnknownDerivedAsSBaseCompact:(ICECurrent*)current
{
    return [TestSlicingObjectsServerSBSUnknownDerived sbsUnknownDerived:@"SBSUnknownDerived.sb"
                                                                  sbsud:@"SBSUnknownDerived.sbsud"];
}

-(ICEObject*) SUnknownAsObject:(ICECurrent*)current
{
    return [TestSlicingObjectsServerSUnknown sUnknown:@"SUnknown.su"];
}

-(void) checkSUnknown:(ICEObject*) object current:(ICECurrent*)current
{
    if([current encoding] == ICEEncoding_1_0)
    {
        test(![object isKindOfClass:[TestSlicingObjectsServerSUnknown class]]);
    }
    else
    {
        test([object isKindOfClass:[TestSlicingObjectsServerSUnknown class]]);
        TestSlicingObjectsServerSUnknown* su = (TestSlicingObjectsServerSUnknown*)object;
        test([[su su] isEqual:@"SUnknown.su"]);
    }
}

-(TestSlicingObjectsServerB*) oneElementCycle:(ICECurrent*)current
{
    TestSlicingObjectsServerB* b1 = [TestSlicingObjectsServerB b];
    b1.sb = @"B1.sb";
    b1.pb = b1;
    return b1;
}
-(TestSlicingObjectsServerB*) twoElementCycle:(ICECurrent*)current
{
    TestSlicingObjectsServerB* b1 = [TestSlicingObjectsServerB b];
    b1.sb = @"B1.sb";
    TestSlicingObjectsServerB* b2 = [TestSlicingObjectsServerB b];
    b2.sb = @"B2.sb";
    b2.pb = b1;
    b1.pb = b2;
    return b1;
}
-(TestSlicingObjectsServerB*) D1AsB:(ICECurrent*)current
{
    TestSlicingObjectsServerD1* d1 = [TestSlicingObjectsServerD1 d1];
    d1.sb = @"D1.sb";
    d1.sd1 = @"D1.sd1";

    TestSlicingObjectsServerD2* d2 = [TestSlicingObjectsServerD2 d2];
    d2.pb = d1;
    d2.sb = @"D2.sb";
    d2.sd2 = @"D2.sd2";
    d2.pd2 = d1;
    d1.pb = d2;
    d1.pd1 = d2;
    return d1;
}
-(TestSlicingObjectsServerD1*) D1AsD1:(ICECurrent*)current
{
    TestSlicingObjectsServerD1* d1 = [TestSlicingObjectsServerD1 d1];
    d1.sb = @"D1.sb";
    d1.sd1 = @"D1.sd1";
    TestSlicingObjectsServerD2* d2 = [TestSlicingObjectsServerD2 d2];
    d2.pb = d1;
    d2.sb = @"D2.sb";
    d2.sd2 = @"D2.sd2";
    d2.pd2 = d1;
    d1.pb = d2;
    d1.pd1 = d2;
    return d1;
}
-(TestSlicingObjectsServerB*) D2AsB:(ICECurrent*)current
{
    TestSlicingObjectsServerD2* d2 = [TestSlicingObjectsServerD2 d2];
    d2.sb = @"D2.sb";
    d2.sd2 = @"D2.sd2";
    TestSlicingObjectsServerD1* d1 = [TestSlicingObjectsServerD1 d1];
    d1.pb = d2;
    d1.sb = @"D1.sb";
    d1.sd1 = @"D1.sd1";
    d1.pd1 = d2;
    d2.pb = d1;
    d2.pd2 = d1;
    return d2;
}
-(void) paramTest1:(TestSlicingObjectsServerB**)p1 p2:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)current
{
    TestSlicingObjectsServerD1* d1 = [TestSlicingObjectsServerD1 d1];
    d1.sb = @"D1.sb";
    d1.sd1 = @"D1.sd1";
    TestSlicingObjectsServerD2* d2 = [TestSlicingObjectsServerD2 d2];
    d2.pb = d1;
    d2.sb = @"D2.sb";
    d2.sd2 = @"D2.sd2";
    d2.pd2 = d1;
    d1.pb = d2;
    d1.pd1 = d2;
    *p1 = d1;
    *p2 = d2;
}
-(void) paramTest2:(TestSlicingObjectsServerB**)p1 p1:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)current
{
    [self paramTest1:p2 p2:p1 current:current];
}
-(TestSlicingObjectsServerB*) paramTest3:(TestSlicingObjectsServerB**)p1 p2:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)current
{
    TestSlicingObjectsServerD2* d2 = [TestSlicingObjectsServerD2 d2];
    d2.sb = @"D2.sb (p1 1)";
    d2.pb = 0;
    d2.sd2 = @"D2.sd2 (p1 1)";
    *p1 = d2;

    TestSlicingObjectsServerD1* d1 = [TestSlicingObjectsServerD1 d1];
    d1.sb = @"D1.sb (p1 2)";
    d1.pb = 0;
    d1.sd1 = @"D1.sd2 (p1 2)";
    d1.pd1 = 0;
    d2.pd2 = d1;

    TestSlicingObjectsServerD2* d4 = [TestSlicingObjectsServerD2 d2];
    d4.sb = @"D2.sb (p2 1)";
    d4.pb = 0;
    d4.sd2 = @"D2.sd2 (p2 1)";
    *p2 = d4;

    TestSlicingObjectsServerD1* d3 = [TestSlicingObjectsServerD1 d1];
    d3.sb = @"D1.sb (p2 2)";
    d3.pb = 0;
    d3.sd1 = @"D1.sd2 (p2 2)";
    d3.pd1 = 0;
    d4.pd2 = d3;

    return d3;
}
-(TestSlicingObjectsServerB*) paramTest4:(TestSlicingObjectsServerB**)p1 current:(ICECurrent*)current
{
    TestSlicingObjectsServerD4* d4 = [TestSlicingObjectsServerD4 d4];
    d4.sb = @"D4.sb (1)";
    d4.pb = 0;
    d4.p1 = [TestSlicingObjectsServerB b];
    d4.p1.sb = @"B.sb (1)";
    d4.p2 = [TestSlicingObjectsServerB b];
    d4.p2.sb = @"B.sb (2)";
    *p1 = d4;
    return d4.p2;
}
-(TestSlicingObjectsServerB*) returnTest1:(TestSlicingObjectsServerB**)p1 p2:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)current
{
    [self paramTest1:p1 p2:p2 current:current];
    return *p1;
}
-(TestSlicingObjectsServerB*) returnTest2:(TestSlicingObjectsServerB**)p1 p1:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)current
{
    [self paramTest1:p2 p2:p1 current:current];
    return *p1;
}
-(TestSlicingObjectsServerB*) returnTest3:(TestSlicingObjectsServerB*)p1 p2:(TestSlicingObjectsServerB*)p2 current:(ICECurrent*)current
{
    return p1;
}
-(TestSlicingObjectsServerSS*) sequenceTest:(TestSlicingObjectsServerSS1*)p1 p2:(TestSlicingObjectsServerSS2*)p2 current:(ICECurrent*)current
{
    TestSlicingObjectsServerSS* ss = [TestSlicingObjectsServerSS ss];
    ss.c1 = p1;
    ss.c2 = p2;
    return ss;
}
-(TestSlicingObjectsServerBDict*) dictionaryTest:(TestSlicingObjectsServerMutableBDict*)bin
                                            bout:(TestSlicingObjectsServerBDict**)bout current:(ICECurrent*)current
{
    int i;
    *bout = [TestSlicingObjectsServerMutableBDict dictionary];
    for(i = 0; i < 10; ++i)
    {
        TestSlicingObjectsServerB* b = [bin objectForKey:[NSNumber numberWithInt:i]];
        TestSlicingObjectsServerD2* d2 = [TestSlicingObjectsServerD2 d2];
        d2.sb = b.sb;
        d2.pb = b.pb;
        d2.sd2 = @"D2";
        d2.pd2 = d2;
        [(NSMutableDictionary*)*bout setObject:d2 forKey:[NSNumber numberWithInt:(i * 10)]];
    }
    TestSlicingObjectsServerMutableBDict* r = [TestSlicingObjectsServerMutableBDict dictionary];
    for(i = 0; i < 10; ++i)
    {
        TestSlicingObjectsServerD1* d1 = [TestSlicingObjectsServerD1 d1];
        d1.sb = [NSString stringWithFormat:@"D1.%d",(i * 20)];
        d1.pb = (i == 0 ? nil : [r objectForKey:[NSNumber numberWithInt:((i - 1) * 20)]]);
        d1.sd1 = d1.sb;
        d1.pd1 = d1;
        [r setObject:d1 forKey:[NSNumber numberWithInt:(i * 20)]];
    }
    return r;
}

-(TestSlicingObjectsServerPBase*) exchangePBase:(TestSlicingObjectsServerPBase*)pb
                                        current:(ICECurrent*)current
{
    return pb;
}

-(TestSlicingObjectsServerPreserved*) PBSUnknownAsPreserved:(ICECurrent*)current
{
    if([current.encoding isEqual:ICEEncoding_1_0])
    {
        //
        // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
        // from unread slice.
        //
        return ICE_AUTORELEASE([[TestSlicingObjectsServerPSUnknown alloc] init:5
                                                            ps:@"preserved"
                                                           psu:@"unknown"
                                                         graph:0
                                                            cl:nil]);
    }
    else
    {
        return ICE_AUTORELEASE([[TestSlicingObjectsServerPSUnknown alloc] init:5
                                            ps:@"preserved"
                                           psu:@"unknown"
                                         graph:0
                                            cl:ICE_AUTORELEASE([[TestSlicingObjectsServerMyClass alloc] init:15])]);
    }
}

-(void) checkPBSUnknown:(TestSlicingObjectsServerPreserved*)p current:(ICECurrent*)current
{
    if([current.encoding isEqual:ICEEncoding_1_0])
    {
        test(![p isKindOfClass:[TestSlicingObjectsServerPSUnknown class]]);
        test(p.pi == 5);
        test([p.ps isEqual:@"preserved"]);
    }
    else
    {
        test([p isKindOfClass:[TestSlicingObjectsServerPSUnknown class]]);
        TestSlicingObjectsServerPSUnknown* pu = (TestSlicingObjectsServerPSUnknown*)p;
        test(pu.pi == 5);
        test([pu.ps isEqual:@"preserved"]);
        test([pu.psu isEqual:@"unknown"]);
        test(!pu.graph);
        test(pu.cl && pu.cl.i == 15);
    }
}

-(TestSlicingObjectsServerPreserved*) PBSUnknownAsPreservedWithGraph:(ICECurrent*)current
{
    TestSlicingObjectsServerPSUnknown* r = [TestSlicingObjectsServerPSUnknown alloc];
    r.pi = 5;
    r.ps = @"preserved";
    r.psu = @"unknown";
    r.graph = [TestSlicingObjectsServerPNode alloc];
    r.graph.next = [TestSlicingObjectsServerPNode alloc];
    r.graph.next.next = [TestSlicingObjectsServerPNode alloc];
    r.graph.next.next.next = r.graph;
    return r;
}

-(void) checkPBSUnknownWithGraph:(TestSlicingObjectsServerPreserved*) p current:(ICECurrent*)current
{
    if([current.encoding isEqual:ICEEncoding_1_0])
    {
        test(![p isKindOfClass:[TestSlicingObjectsServerPSUnknown class]]);
        test(p.pi == 5);
        test([p.ps isEqual:@"preserved"]);
    }
    else
    {
        test([p isKindOfClass:[TestSlicingObjectsServerPSUnknown class]]);
        TestSlicingObjectsServerPSUnknown* pu = (TestSlicingObjectsServerPSUnknown*)p;
        test(pu.pi == 5);
        test([pu.ps isEqual:@"preserved"]);
        test([pu.psu isEqual:@"unknown"]);
        test(pu.graph != pu.graph.next);
        test(pu.graph.next != pu.graph.next.next);
        test(pu.graph.next.next.next == pu.graph);
        pu.graph.next.next.next = nil; // Break the cycle.
    }
}

-(TestSlicingObjectsServerPreserved*) PBSUnknown2AsPreservedWithGraph:(ICECurrent*)current
{
    TestSlicingObjectsServerPSUnknown2* r = [TestSlicingObjectsServerPSUnknown2 alloc];
    r.pi = 5;
    r.ps = @"preserved";
    r.pb = r;
    return r;
}

-(void) checkPBSUnknown2WithGraph:(TestSlicingObjectsServerPreserved*) p current:(ICECurrent*)current
{
    if([current.encoding isEqual:ICEEncoding_1_0])
    {
        test(![p isKindOfClass:[TestSlicingObjectsServerPSUnknown2 class]]);
        test(p.pi == 5);
        test([p.ps isEqual:@"preserved"]);
    }
    else
    {
        test([p isKindOfClass:[TestSlicingObjectsServerPSUnknown2 class]]);
        TestSlicingObjectsServerPSUnknown2* pu = (TestSlicingObjectsServerPSUnknown2*)p;
        test(pu.pi == 5);
        test([pu.ps isEqual:@"preserved"]);
        test(pu.pb == pu);
        pu.pb = 0; // Break the cycle.
    }
}

-(TestSlicingObjectsServerPNode*) exchangePNode:(TestSlicingObjectsServerPNode*)pn current:(ICECurrent*)current
{
    return pn;
}

-(void) throwBaseAsBase:(ICECurrent*)current
{
    TestSlicingObjectsServerBaseException* be = [TestSlicingObjectsServerBaseException baseException];
    be.sbe = @"sbe";
    be.pb = [TestSlicingObjectsServerB b];
    be.pb.sb = @"sb";
    be.pb.pb = be.pb;
    @throw be;
}
-(void) throwDerivedAsBase:(ICECurrent*)current
{
    TestSlicingObjectsServerDerivedException* de = [TestSlicingObjectsServerDerivedException derivedException];
    de.sbe = @"sbe";
    de.pb = [TestSlicingObjectsServerB b];
    de.pb.sb = @"sb1";
    de.pb.pb = de.pb;
    de.sde = @"sde1";
    de.pd1 = [TestSlicingObjectsServerD1 d1];
    de.pd1.sb = @"sb2";
    de.pd1.pb = de.pd1;
    de.pd1.sd1 = @"sd2";
    de.pd1.pd1 = de.pd1;
    @throw de;
}
-(void) throwDerivedAsDerived:(ICECurrent*)current
{
    TestSlicingObjectsServerDerivedException* de = [TestSlicingObjectsServerDerivedException derivedException];
    de.sbe = @"sbe";
    de.pb = [TestSlicingObjectsServerB b];
    de.pb.sb = @"sb1";
    de.pb.pb = de.pb;
    de.sde = @"sde1";
    de.pd1 = [TestSlicingObjectsServerD1 d1];
    de.pd1.sb = @"sb2";
    de.pd1.pb = de.pd1;
    de.pd1.sd1 = @"sd2";
    de.pd1.pd1 = de.pd1;
    @throw de;
}
-(void) throwUnknownDerivedAsBase:(ICECurrent*)current
{
    TestSlicingObjectsServerD2* d2 = [TestSlicingObjectsServerD2 d2];
    d2.sb = @"sb d2";
    d2.pb = d2;
    d2.sd2 = @"sd2 d2";
    d2.pd2 = d2;

    TestSlicingObjectsServerUnknownDerivedException* ude = [TestSlicingObjectsServerUnknownDerivedException unknownDerivedException];
    ude.sbe = @"sbe";
    ude.pb = d2;
    ude.sude = @"sude";
    ude.pd2 = d2;
    @throw ude;
}
-(void) throwPreservedException:(ICECurrent*)current
{
    TestSlicingObjectsServerPSUnknownException* ue = [TestSlicingObjectsServerPSUnknownException psUnknownException];
    ue.p = [TestSlicingObjectsServerPSUnknown2 psUnknown2];
    ue.p.pi = 5;
    ue.p.ps = @"preserved";
    ue.p.pb = ue.p;
    if(ex_ != nil)
    {
        ex_.p.pb = nil;
        ICE_RELEASE(ex_);
    }
    ex_ = ICE_RETAIN(ue);
    @throw ue;
}
-(void) useForward:(TestSlicingObjectsServerForward**)f current:(ICECurrent*)current
{
    *f = [TestSlicingObjectsServerForward forward];
    (*f).h = [TestSlicingObjectsServerHidden hidden];
    (*f).h.f = *f;
}
-(void) shutdown:(ICECurrent*)current
{
    if(ex_ != nil)
    {
        ex_.p.pb = nil;
        ICE_RELEASE(ex_);
    }
    [[current.adapter getCommunicator] shutdown];
}
@end
