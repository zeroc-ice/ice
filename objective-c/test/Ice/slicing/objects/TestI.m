// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice.h>
#import <slicing/objects/TestI.h>
#import <TestCommon.h>

static void breakCycles(id o)
{
    if([o isKindOfClass:[TestSlicingObjectsServerD1 class]])
    {
        TestSlicingObjectsServerD1* d1 = (TestSlicingObjectsServerD1*)o;
        if(d1.pd1 != d1)
        {
            breakCycles(d1.pd1);
        }
        d1.pd1 = nil;
    }
    if([o isKindOfClass:[TestSlicingObjectsServerD2 class]])
    {
        TestSlicingObjectsServerD2* d2 = (TestSlicingObjectsServerD2*)o;
        d2.pd2 = nil;
    }
    if([o isKindOfClass:[TestSlicingObjectsServerD4 class]])
    {
        TestSlicingObjectsServerD4* d4 = (TestSlicingObjectsServerD4*)o;
        d4.p1 = nil;
        d4.p2 = nil;
    }
    if([o isKindOfClass:[TestSlicingObjectsServerB class]])
    {
        TestSlicingObjectsServerB* b = (TestSlicingObjectsServerB*)o;
        if(b.pb != nil)
        {
            b.pb.pb = nil;
        }
        b.pb = nil;
    }
    if([o isKindOfClass:[TestSlicingObjectsServerPreserved class]])
    {
        TestSlicingObjectsServerPreserved* p = (TestSlicingObjectsServerPreserved*)o;
        [[p ice_getSlicedData] clear];
    }
    if([o isKindOfClass:[TestSlicingObjectsServerPDerived class]])
    {
        TestSlicingObjectsServerPDerived* p = (TestSlicingObjectsServerPDerived*)o;
        p.pb = nil;
    }
    if([o isKindOfClass:[TestSlicingObjectsServerCompactPDerived class]])
    {
        TestSlicingObjectsServerPDerived* p = (TestSlicingObjectsServerPDerived*)o;
        p.pb = nil;
    }
    if([o isKindOfClass:[TestSlicingObjectsServerPNode class]])
    {
        TestSlicingObjectsServerPNode* curr = o;
        while(curr && o != curr.next)
        {
            curr = curr.next;
        }
        if(curr && o == curr.next)
        {
            curr.next = nil;
        }
    }
    if([o isKindOfClass:[TestSlicingObjectsServerPSUnknown class]])
    {
        TestSlicingObjectsServerPSUnknown* p = (TestSlicingObjectsServerPSUnknown*)o;
        breakCycles(p.graph);
    }
    if([o isKindOfClass:[TestSlicingObjectsServerPSUnknown2 class]])
    {
        TestSlicingObjectsServerPSUnknown2* p = (TestSlicingObjectsServerPSUnknown2*)o;
        p.pb = nil;
    }
    if([o isKindOfClass:[TestSlicingObjectsServerBaseException class]])
    {
        TestSlicingObjectsServerBaseException* e = (TestSlicingObjectsServerBaseException*)o;
        breakCycles(e.pb);
    }
    if([o isKindOfClass:[TestSlicingObjectsServerDerivedException class]])
    {
        TestSlicingObjectsServerDerivedException* e = (TestSlicingObjectsServerDerivedException*)o;
        breakCycles(e.pd1);
    }
    if([o isKindOfClass:[TestSlicingObjectsServerUnknownDerivedException class]])
    {
        TestSlicingObjectsServerUnknownDerivedException* e = (TestSlicingObjectsServerUnknownDerivedException*)o;
        breakCycles(e.pd2);
    }
    if([o isKindOfClass:[TestSlicingObjectsServerPSUnknownException class]])
    {
        TestSlicingObjectsServerPSUnknownException* e = (TestSlicingObjectsServerPSUnknownException*)o;
        breakCycles(e.p);
    }
    if([o isKindOfClass:[TestSlicingObjectsServerSS1 class]])
    {
        TestSlicingObjectsServerSS1* s = (TestSlicingObjectsServerSS1*)o;
        breakCycles(s.s);
    }
    if([o isKindOfClass:[TestSlicingObjectsServerSS2 class]])
    {
        TestSlicingObjectsServerSS2* s = (TestSlicingObjectsServerSS2*)o;
        breakCycles(s.s);
    }
    if([o isKindOfClass:[TestSlicingObjectsServerSS class]])
    {
        TestSlicingObjectsServerSS* s = (TestSlicingObjectsServerSS*)o;
        breakCycles(s.c1);
        breakCycles(s.c2);
    }
    if([o isKindOfClass:[TestSlicingObjectsServerForward class]])
    {
        TestSlicingObjectsServerForward* f = (TestSlicingObjectsServerForward*)o;
        f.h = nil;
    }
    if([o isKindOfClass:[TestSlicingObjectsServerSUnknown class]])
    {
        TestSlicingObjectsServerSUnknown* u = (TestSlicingObjectsServerSUnknown*)o;
        u.cycle = nil;
    }
    if([o isKindOfClass:[NSArray class]])
    {
        for(id e in o)
        {
            breakCycles(e);
        }
    }
    if([o isKindOfClass:[NSDictionary class]])
    {
        for(id e in [o allValues])
        {
            breakCycles(e);
        }
    }
}

@implementation TestSlicingObjectsServerI
+(id) serverI
{
    TestSlicingObjectsServerI* impl = [[TestSlicingObjectsServerI alloc] init];
    impl->objects_ = [[NSMutableArray alloc] init];
    return ICE_AUTORELEASE(impl);
}
-(void) dealloc
{
    for(id o in objects_)
    {
        breakCycles(o);
    }
#if !__has_feature(objc_arc)
    [objects_ release];
    [super dealloc];
#endif
}
-(ICEObject*) SBaseAsObject:(ICECurrent*)__unused current
{
    return [TestSlicingObjectsServerSBase sBase:@"SBase.sb"];
}

-(TestSlicingObjectsServerSBase*) SBaseAsSBase:(ICECurrent*)__unused current
{
    return [TestSlicingObjectsServerSBase sBase:@"SBase.sb"];
}

-(TestSlicingObjectsServerSBase*) SBSKnownDerivedAsSBase:(ICECurrent*)__unused current
{
    return [TestSlicingObjectsServerSBSKnownDerived sbsKnownDerived:@"SBSKnownDerived.sb"
                                                              sbskd:@"SBSKnownDerived.sbskd"];
}

-(TestSlicingObjectsServerSBSKnownDerived*) SBSKnownDerivedAsSBSKnownDerived:(ICECurrent*)__unused current
{
    return [TestSlicingObjectsServerSBSKnownDerived sbsKnownDerived:@"SBSKnownDerived.sb"
                                                              sbskd:@"SBSKnownDerived.sbskd"];
}

-(TestSlicingObjectsServerSBase*) SBSUnknownDerivedAsSBase:(ICECurrent*)__unused current
{
    return [TestSlicingObjectsServerSBSUnknownDerived sbsUnknownDerived:@"SBSUnknownDerived.sb"
                                                                  sbsud:@"SBSUnknownDerived.sbsud"];
}

-(TestSlicingObjectsServerSBase*) SBSUnknownDerivedAsSBaseCompact:(ICECurrent*)__unused current
{
    return [TestSlicingObjectsServerSBSUnknownDerived sbsUnknownDerived:@"SBSUnknownDerived.sb"
                                                                  sbsud:@"SBSUnknownDerived.sbsud"];
}

-(ICEObject*) SUnknownAsObject:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerSUnknown* s = [TestSlicingObjectsServerSUnknown sUnknown:@"SUnknown.su" cycle:nil];
    s.cycle = s;
    [objects_ addObject:s];
    return s;
}

-(void) checkSUnknown:(ICEObject*)object current:(ICECurrent*)__unused current
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
    [objects_ addObject:object];
}

-(TestSlicingObjectsServerB*) oneElementCycle:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerB* b1 = [TestSlicingObjectsServerB b];
    b1.sb = @"B1.sb";
    b1.pb = b1;
    [objects_ addObject:b1];
    return b1;
}
-(TestSlicingObjectsServerB*) twoElementCycle:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerB* b1 = [TestSlicingObjectsServerB b];
    b1.sb = @"B1.sb";
    TestSlicingObjectsServerB* b2 = [TestSlicingObjectsServerB b];
    b2.sb = @"B2.sb";
    b2.pb = b1;
    b1.pb = b2;
    [objects_ addObject:b1];
    return b1;
}
-(TestSlicingObjectsServerB*) D1AsB:(ICECurrent*)__unused current
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

    [objects_ addObject:d1];
    return d1;
}
-(TestSlicingObjectsServerD1*) D1AsD1:(ICECurrent*)__unused current
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

    [objects_ addObject:d1];
    return d1;
}
-(TestSlicingObjectsServerB*) D2AsB:(ICECurrent*)__unused current
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

    [objects_ addObject:d2];
    return d2;
}
-(void) paramTest1:(TestSlicingObjectsServerB**)p1 p2:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)__unused current
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

    [objects_ addObject:d1];
    [objects_ addObject:d2];
}
-(void) paramTest2:(TestSlicingObjectsServerB**)p1 p1:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)__unused current
{
    [self paramTest1:p2 p2:p1 current:current];
}
-(TestSlicingObjectsServerB*) paramTest3:(TestSlicingObjectsServerB**)p1 p2:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)__unused current
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

    [objects_ addObject:d2];
    [objects_ addObject:d3];
    [objects_ addObject:d4];
    return d3;
}
-(TestSlicingObjectsServerB*) paramTest4:(TestSlicingObjectsServerB**)p1 current:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerD4* d4 = [TestSlicingObjectsServerD4 d4];
    d4.sb = @"D4.sb (1)";
    d4.pb = 0;
    d4.p1 = [TestSlicingObjectsServerB b];
    d4.p1.sb = @"B.sb (1)";
    d4.p2 = [TestSlicingObjectsServerB b];
    d4.p2.sb = @"B.sb (2)";
    *p1 = d4;
    [objects_ addObject:d4];
    return d4.p2;
}
-(TestSlicingObjectsServerB*) returnTest1:(TestSlicingObjectsServerB**)p1 p2:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)__unused current
{
    [self paramTest1:p1 p2:p2 current:current];
    return *p1;
}
-(TestSlicingObjectsServerB*) returnTest2:(TestSlicingObjectsServerB**)p1 p1:(TestSlicingObjectsServerB**)p2 current:(ICECurrent*)__unused current
{
    [self paramTest1:p2 p2:p1 current:current];
    return *p1;
}
-(TestSlicingObjectsServerB*) returnTest3:(TestSlicingObjectsServerB*)p1 p2:(TestSlicingObjectsServerB*)p2 current:(ICECurrent*)__unused current
{
    [objects_ addObject:p1];
    [objects_ addObject:p2];
    return p1;
}
-(TestSlicingObjectsServerSS*) sequenceTest:(TestSlicingObjectsServerSS1*)p1 p2:(TestSlicingObjectsServerSS2*)p2 current:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerSS* ss = [TestSlicingObjectsServerSS ss];
    ss.c1 = p1;
    ss.c2 = p2;
    [objects_ addObject:p1];
    [objects_ addObject:p2];
    [objects_ addObject:ss];
    return ss;
}
-(TestSlicingObjectsServerBDict*) dictionaryTest:(TestSlicingObjectsServerMutableBDict*)bin
                                            bout:(TestSlicingObjectsServerBDict**)bout current:(ICECurrent*)__unused current
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
    [objects_ addObject:*bout];
    [objects_ addObject:r];
    return r;
}

-(TestSlicingObjectsServerPBase*) exchangePBase:(TestSlicingObjectsServerPBase*)pb
                                        current:(ICECurrent*)__unused current
{
    [objects_ addObject:pb];
    return pb;
}

-(TestSlicingObjectsServerPreserved*) PBSUnknownAsPreserved:(ICECurrent*)__unused current
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

-(void) checkPBSUnknown:(TestSlicingObjectsServerPreserved*)p current:(ICECurrent*)__unused current
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

-(TestSlicingObjectsServerPreserved*) PBSUnknownAsPreservedWithGraph:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerPSUnknown* r = [TestSlicingObjectsServerPSUnknown alloc];
    r.pi = 5;
    r.ps = @"preserved";
    r.psu = @"unknown";
    r.graph = [TestSlicingObjectsServerPNode alloc];
    r.graph.next = [TestSlicingObjectsServerPNode alloc];
    r.graph.next.next = [TestSlicingObjectsServerPNode alloc];
    r.graph.next.next.next = r.graph;
    [objects_ addObject:r];
    return r;
}

-(void) checkPBSUnknownWithGraph:(TestSlicingObjectsServerPreserved*) p current:(ICECurrent*)__unused current
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
    [objects_ addObject:p];
}

-(TestSlicingObjectsServerPreserved*) PBSUnknown2AsPreservedWithGraph:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerPSUnknown2* r = [TestSlicingObjectsServerPSUnknown2 alloc];
    r.pi = 5;
    r.ps = @"preserved";
    r.pb = r;
    [objects_ addObject:r];
    return r;
}

-(void) checkPBSUnknown2WithGraph:(TestSlicingObjectsServerPreserved*)p current:(ICECurrent*)__unused current
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
        pu.pb = nil; // Break the cycle.
    }
    [objects_ addObject:p];
}

-(TestSlicingObjectsServerPNode*) exchangePNode:(TestSlicingObjectsServerPNode*)pn current:(ICECurrent*)__unused current
{
    [objects_ addObject:pn];
    return pn;
}

-(void) throwBaseAsBase:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerBaseException* be = [TestSlicingObjectsServerBaseException baseException];
    be.sbe = @"sbe";
    be.pb = [TestSlicingObjectsServerB b];
    be.pb.sb = @"sb";
    be.pb.pb = be.pb;
    [objects_ addObject:be];
    @throw be;
}
-(void) throwDerivedAsBase:(ICECurrent*)__unused current
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
    [objects_ addObject:de];
    @throw de;
}
-(void) throwDerivedAsDerived:(ICECurrent*)__unused current
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
    [objects_ addObject:de];
    @throw de;
}
-(void) throwUnknownDerivedAsBase:(ICECurrent*)__unused current
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
    [objects_ addObject:ude];
    @throw ude;
}
-(void) throwPreservedException:(ICECurrent*)__unused current
{
    TestSlicingObjectsServerPSUnknownException* ue = [TestSlicingObjectsServerPSUnknownException psUnknownException];
    ue.p = [TestSlicingObjectsServerPSUnknown2 psUnknown2];
    ue.p.pi = 5;
    ue.p.ps = @"preserved";
    ue.p.pb = ue.p;
    [objects_ addObject:ue];
    @throw ue;
}
-(void) useForward:(TestSlicingObjectsServerForward**)f current:(ICECurrent*)__unused current
{
    *f = [TestSlicingObjectsServerForward forward];
    (*f).h = [TestSlicingObjectsServerHidden hidden];
    (*f).h.f = *f;
    [objects_ addObject:*f];
}
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end
