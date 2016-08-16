// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OptionalTest.h>

#import <Foundation/Foundation.h>

@interface TestObjectReader : ICEObject
{
}
@end

@implementation TestObjectReader
-(void) read__:(id<ICEInputStream>)is
{
    [is startValue];
    [is startSlice];
    [is endSlice];
    [is endValue:NO];
}
@end

@interface BObjectReader : ICEObject
{
}
@end

@implementation BObjectReader
-(void) read__:(id<ICEInputStream>)is
{
    [is startValue];
    // ::Test::B
    [is startSlice];
    [is readInt];
    [is endSlice];
    // ::Test::A
    [is startSlice];
    [is readInt];
    [is endSlice];
    [is endValue:NO];
};
@end

@interface CObjectReader : ICEObject
{
}
@end

@implementation CObjectReader
-(void) read__:(id<ICEInputStream>)is
{
    [is startValue];
    // ::Test::C
    [is startSlice];
    [is skipSlice];
    // ::Test::B
    [is startSlice];
    [is readInt];
    [is endSlice];
    // ::Test::A
    [is startSlice];
    [is readInt];
    [is endSlice];
    [is endValue:NO];
};
@end

@interface DObjectWriter : ICEObject
{
}
@end

@implementation DObjectWriter
-(void) write__:(id<ICEOutputStream>)os
{
    [os startValue:0];
    // ::Test::D
    [os startSlice:@"::Test::D" compactId:-1 lastSlice:NO];
    [os writeString:@"test"];
    if([os writeOptional:1 format:ICEOptionalFormatVSize])
    {
        ICEMutableStringSeq* o = [ICEMutableStringSeq array];
        [o addObject:@"test1"];
        [o addObject:@"test2"];
        [o addObject:@"test3"];
        [o addObject:@"test4"];
        [ICEStringSeqHelper write:o stream:os];
    }
    if([os writeOptional:1000 format:ICEOptionalFormatClass])
    {
        TestOptionalA* a = [TestOptionalA a];
        a.mc = 18;
        [os writeValue:a];
    }
    [os endSlice];
    // ::Test::B
    [os startSlice:@"::Test::B" compactId:-1 lastSlice:NO];
    [os writeInt:14];
    [os endSlice];
    // ::Test::A
    [os startSlice:@"::Test::A" compactId:-1 lastSlice:YES];
    [os writeInt:14];
    [os endSlice];
    [os endValue];
}
@end

@interface DObjectReader : ICEObject
{
    TestOptionalA* a_;
}
@end

@implementation DObjectReader
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [a_ release];
    [super dealloc];
}
#endif
-(void) read__:(id<ICEInputStream>)is
{
    [is startValue];
    // ::Test::D
    [is startSlice];
    NSString* s = [is readString];
    test([s isEqualToString:@"test"]);
    test([is readOptional:1 format:ICEOptionalFormatVSize]);
    NSMutableArray* o = [ICEStringSeqHelper read:is];
    test(o != nil && [o count] == 4 &&
         [[o objectAtIndex:0] isEqualToString:@"test1"] &&
         [[o objectAtIndex:1] isEqualToString:@"test2"] &&
         [[o objectAtIndex:2] isEqualToString:@"test3"] &&
         [[o objectAtIndex:3] isEqualToString:@"test4"]);
    test([is readOptional:1000 format:ICEOptionalFormatClass]);
    [is newValue:(ICEObject**)&a_ expectedType:[TestOptionalA class]];
    [is endSlice];

    // ::Test::B
    [is startSlice];
    [is readInt];
    [is endSlice];
    // ::Test::A
    [is startSlice];
    [is readInt];
    [is endSlice];
    [is endValue:NO];
}
-(void) check
{
    test(a_.mc == 18);
}
@end

@interface FObjectReader : ICEObject
{
    TestOptionalF* f_;
}
@end

@implementation FObjectReader
-(id) init
{
    self = [super init];
    if(self)
    {
        f_ = nil;
    }
    return self;
}

-(void) read__:(id<ICEInputStream>)is
{
    if(f_ != nil)
    {
        ICE_RELEASE(f_);
    }
    f_ = [TestOptionalF new];
    [is startValue];
    [is startSlice];
    // Don't read optional af on purpose
    //[is_ readValue:(ICEObject**)&self->af expectedType:[TestOptionalA class]];
    [is endSlice];
    [is startSlice];
    TestOptionalA* ICE_AUTORELEASING_QUALIFIER ae;
    [is readValue:(ICEObject**)&ae expectedType:[TestOptionalA class]];
    [is endSlice];
    [is endValue:NO];
    f_.ae = ae;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [f_ release];
    [super dealloc];
}
#endif

-(TestOptionalF*) getF
{
    return f_;
}
@end

@interface FactoryI : ICEObject
{
    BOOL enabled_;
}
+(FactoryI*) factoryI;
-(void) setEnabled:(BOOL)enabled;
@end

@implementation FactoryI
+(FactoryI*) factoryI
{
    return ICE_AUTORELEASE([[FactoryI alloc] init]);
}
-(id) init
{
    self = [super init];
    if(self)
    {
        self->enabled_ = NO;
    }
    return self;
}
-(ICEObject*) create:(NSString*)typeId
{
    if(!enabled_)
    {
        return nil;
    }
    if([typeId isEqualToString:@"::Test::OneOptional"])
    {
        return [TestObjectReader new];
    }
    else if([typeId isEqualToString:@"::Test::MultiOptional"])
    {
        return [TestObjectReader new];
    }
    else if([typeId isEqualToString:@"::Test::B"])
    {
        return [BObjectReader new];
    }
    else if([typeId isEqualToString:@"::Test::C"])
    {
        return [CObjectReader new];
    }
    else if([typeId isEqualToString:@"::Test::D"])
    {
        return [DObjectReader new];
    }
    else if([typeId isEqualToString:@"::Test::F"])
    {
        return [FObjectReader new];
    }

    return nil;
}
-(void) setEnabled:(BOOL)enabled
{
    self->enabled_ = enabled;
}
@end

id<TestOptionalInitialPrx>
optionalAllTests(id<ICECommunicator> communicator)
{
    FactoryI* factory = [FactoryI factoryI];
    [[communicator getValueFactoryManager] add:^(id s) { return [factory create:s]; } sliceId:@""];

    tprintf("testing stringToProxy... ");
    NSString* sref = @"initial:default -p 12010";
    id<ICEObjectPrx> base = [communicator stringToProxy:sref];
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    id<TestOptionalInitialPrx> initial = [TestOptionalInitialPrx checkedCast:base];
    test(initial != nil);
    test([initial isEqual:base]);
    tprintf("ok\n");

    tprintf("testing constructor, copy constructor, and assignment operator... ");

    TestOptionalOneOptional* oo1 = [TestOptionalOneOptional oneOptional];
    test(![oo1 hasA]);
    oo1.a = 15;
    test([oo1 hasA] && oo1.a == 15);

    TestOptionalOneOptional* oo2 = [TestOptionalOneOptional oneOptional:@16];
    test([oo2 hasA] && oo2.a == 16);

    TestOptionalOneOptional* oon = [TestOptionalOneOptional oneOptional:ICENone];
    test(![oon hasA]);

    TestOptionalMultiOptional* mo1 = [TestOptionalMultiOptional multiOptional];
    mo1.a = 15;
    mo1.b = true;
    mo1.c = 19;
    mo1.d = 78;
    mo1.e = 99;
    mo1.f = 5.5f;
    mo1.g = 1.0;
    mo1.h = @"test";
    mo1.i = TestOptionalMyEnumMember;
    mo1.j = [TestOptionalMultiOptionalPrx uncheckedCast:[communicator stringToProxy:@"test"]];
    //mo1.k = mo1;
    ICEByte bsa[] = { 0x05 };
    mo1.bs = [TestOptionalByteSeq dataWithBytes:bsa length:1];
    mo1.ss = [TestOptionalStringSeq arrayWithObjects:@"test", @"test2", nil];
    mo1.iid = [TestOptionalIntIntDict dictionaryWithObjectsAndKeys:@3, @4, nil];
    mo1.sid = [TestOptionalStringIntDict dictionaryWithObjectsAndKeys:@10, @"test", nil];
    TestOptionalFixedStruct* fs = [TestOptionalFixedStruct fixedStruct];
    fs.m = 78;
    mo1.fs = fs;
    TestOptionalVarStruct* vs = [TestOptionalVarStruct varStruct];
    vs.m = @"hello";
    mo1.vs = vs;

    ICEShort shs[] = { 1 };
    mo1.shs = [TestOptionalShortSeq dataWithBytes:shs length:sizeof(shs)];

    TestOptionalMyEnum es[] = { TestOptionalMyEnumMember, TestOptionalMyEnumMember };
    mo1.es = [TestOptionalMyEnumSeq dataWithBytes:es length:sizeof(es)];
    mo1.fss = [TestOptionalFixedStructSeq arrayWithObject:fs];
    mo1.vss = [TestOptionalVarStructSeq arrayWithObject:vs];
    mo1.oos = [TestOptionalOneOptionalSeq arrayWithObject:oo1];
    id<TestOptionalOneOptionalPrx> oneOptionalProxy =
        [TestOptionalOneOptionalPrx uncheckedCast:[communicator stringToProxy:@"test"]];
    mo1.oops = [TestOptionalOneOptionalPrxSeq arrayWithObject:oneOptionalProxy];
    mo1.ied = [TestOptionalIntEnumDict dictionaryWithObjectsAndKeys:@(TestOptionalMyEnumMember), @4, nil];
    mo1.ifsd = [TestOptionalIntFixedStructDict dictionaryWithObjectsAndKeys:fs, @4, nil];
    mo1.ivsd = [TestOptionalIntVarStructDict dictionaryWithObjectsAndKeys:vs, @4, nil];
    mo1.iood = [TestOptionalIntOneOptionalDict dictionaryWithObjectsAndKeys:[TestOptionalOneOptional oneOptional:@15],
                                               @5, nil];
    mo1.ioopd = [TestOptionalIntOneOptionalPrxDict dictionaryWithObjectsAndKeys:oneOptionalProxy, @5, nil];
    BOOL bos[] = { NO, YES, NO };
    mo1.bos = [TestOptionalBoolSeq dataWithBytes:bos length:sizeof(bos)];

    TestOptionalMultiOptional* mo3 = ICE_AUTORELEASE([mo1 copy]);
    test(mo3.a == 15);
    test(mo3.b == YES);
    test(mo3.c == 19);
    test(mo3.d == 78);
    test(mo3.e == 99);
    test(mo3.f == 5.5f);
    test(mo3.g == 1.0);
    test([mo3.h isEqualToString:@"test"]);
    test(mo3.i == TestOptionalMyEnumMember);
    test([mo3.j isEqual:[TestOptionalMultiOptionalPrx uncheckedCast:[communicator stringToProxy:@"test"]]]);
    //test(mo3.k == mo1);
    test(mo3.bs == mo1.bs);
    test(mo3.ss == mo1.ss);
    test(mo3.iid == mo1.iid);
    test(mo3.sid == mo1.sid);
    test(mo3.fs == mo1.fs);
    test(mo3.vs == mo1.vs);

    test(mo3.shs == mo1.shs);
    test(mo3.es == mo1.es);
    test(mo3.fss == mo1.fss);
    test(mo3.vss == mo1.vss);
    test(mo3.oos == mo1.oos);
    test(mo3.oops == mo1.oops);

    test(mo3.ied == mo1.ied);
    test(mo3.ifsd == mo1.ifsd);
    test(mo3.ivsd == mo1.ivsd);
    test(mo3.iood == mo1.iood);
    test(mo3.ioopd == mo1.ioopd);

    test(mo3.bos == mo1.bos);

    tprintf("ok\n");

//     tprintf("testing comparison operators... ");

//     test(mo1->a == 15 && 15 == mo1->a && mo1->a != 16 && 16 != mo1->a);
//     test(mo1->a < 16 && mo1->a > 14 && mo1->a <= 15 && mo1->a >= 15 && mo1->a <= 16 && mo1->a >= 14);
//     test(mo1->a > IceUtil::Optional<int>() && IceUtil::Optional<int>() < mo1->a);
//     test(14 > IceUtil::Optional<int>() && IceUtil::Optional<int>() < 14);

//     test(mo1->h == "test" && "test" == mo1->h && mo1->h != "testa" && "testa" != mo1->h);
//     test(mo1->h < "test1" && mo1->h > "tesa" && mo1->h <= "test");
//     test(mo1->h >= "test" && mo1->h <= "test1" && mo1->h >= "tesa");
//     test(mo1->h > IceUtil::Optional<string>() && IceUtil::Optional<string>() < mo1->h);
//     test("test1" > IceUtil::Optional<string>() && IceUtil::Optional<string>() < "test1");

//     tprintf("ok\n");

    tprintf("testing marshalling... ");
    TestOptionalOneOptional* oo4 = (TestOptionalOneOptional*)[initial pingPong:[TestOptionalOneOptional oneOptional]];
    test(![oo4 hasA]);

    TestOptionalOneOptional* oo5 = (TestOptionalOneOptional*)[initial pingPong:oo1];
    test(oo1.a == oo5.a);

    TestOptionalMultiOptional* mo4 = (TestOptionalMultiOptional*)
        [initial pingPong:[TestOptionalMultiOptional multiOptional]];
    test(![mo4 hasA]);
    test(![mo4 hasB]);
    test(![mo4 hasC]);
    test(![mo4 hasD]);
    test(![mo4 hasE]);
    test(![mo4 hasF]);
    test(![mo4 hasG]);
    test(![mo4 hasH]);
    test(![mo4 hasI]);
    test(![mo4 hasJ]);
    test(![mo4 hasK]);
    test(![mo4 hasBs]);
    test(![mo4 hasSs]);
    test(![mo4 hasIid]);
    test(![mo4 hasSid]);
    test(![mo4 hasFs]);
    test(![mo4 hasVs]);

    test(![mo4 hasShs]);
    test(![mo4 hasEs]);
    test(![mo4 hasFss]);
    test(![mo4 hasVss]);
    test(![mo4 hasOos]);
    test(![mo4 hasOops]);

    test(![mo4 hasIed]);
    test(![mo4 hasIfsd]);
    test(![mo4 hasIvsd]);
    test(![mo4 hasIood]);
    test(![mo4 hasIoopd]);

    test(![mo4 hasBos]);

    //mo1.k = mo1;
    TestOptionalMultiOptional* mo5 = (TestOptionalMultiOptional*)[initial pingPong:mo1];
    test(mo5.a == mo1.a);
    test(mo5.b == mo1.b);
    test(mo5.c == mo1.c);
    test(mo5.d == mo1.d);
    test(mo5.e == mo1.e);
    test(mo5.f == mo1.f);
    test(mo5.g == mo1.g);
    test([mo5.h isEqualToString:mo1.h]);
    test(mo5.i == mo1.i);
    test([mo5.j isEqual:mo1.j]);
    //test(mo5.k == mo5);
    test([mo5.bs isEqual:mo1.bs]);
    test([mo5.ss isEqual:mo1.ss]);
    test([mo5.iid isEqual: mo1.iid]);
    test([mo5.sid isEqual:mo1.sid]);
    test([mo5.fs isEqual:mo1.fs]);
    test([mo5.vs isEqual:mo1.vs]);

    test([mo5.shs isEqual:mo1.shs]);
    test([mo5.es isEqual:mo1.es]);
    test([mo5.fss isEqual:mo1.fss]);
    test([mo5.vss isEqual:mo1.vss]);
    test([mo5.oos count] > 0 && ((TestOptionalOneOptional*)[mo5.oos objectAtIndex:0]).a == oo1.a);
    test([mo5.oops isEqual:mo1.oops]);

    test([mo5.ied isEqual:mo1.ied]);
    test([mo5.ifsd isEqual:mo1.ifsd]);
    test([mo5.ivsd isEqual:mo1.ivsd]);
    test([mo5.iood count] > 0 && ((TestOptionalOneOptional*)[mo5.iood objectForKey:@5]).a == 15);
    test([mo5.ioopd isEqual:mo1.ioopd]);

    test([mo5.bos isEqual:mo1.bos]);

    // Clear the first half of the optional parameters
    TestOptionalMultiOptional* mo6 = ICE_AUTORELEASE([mo5 copy]);
    [mo6 clearA];
    [mo6 clearC];
    [mo6 clearE];
    [mo6 clearG];
    [mo6 clearI];
    [mo6 clearK];
    [mo6 clearSs];
    [mo6 clearSid];
    [mo6 clearVs];

    [mo6 clearEs];
    [mo6 clearVss];
    [mo6 clearOops];

    [mo6 clearIed];
    [mo6 clearIvsd];
    [mo6 clearIoopd];

    TestOptionalMultiOptional* mo7 = (TestOptionalMultiOptional*)[initial pingPong:mo6];
    test(![mo7 hasA]);
    test(mo7.b == mo1.b);
    test(![mo7 hasC]);
    test(mo7.d == mo1.d);
    test(![mo7 hasE]);
    test(mo7.f == mo1.f);
    test(![mo7 hasG]);
    test([mo7.h isEqual:mo1.h]);
    test(![mo7 hasI]);
    test([mo7.j isEqual:mo1.j]);
    test(![mo7 hasK]);
    test([mo7.bs isEqual:mo1.bs]);
    test(![mo7 hasSs]);
    test([mo7.iid isEqual:mo1.iid]);
    test(![mo7 hasSid]);
    test([mo7.fs isEqual:mo1.fs]);
    test(![mo7 hasVs]);

    test([mo7.shs isEqual:mo1.shs]);
    test(![mo7 hasEs]);
    test([mo7.fss isEqual:mo1.fss]);
    test(![mo7 hasVss]);
    test([mo7.oos count] > 0 && ((TestOptionalOneOptional*)[mo7.oos objectAtIndex:0]).a == oo1.a);
    test(![mo7 hasOops]);

    test(![mo7 hasIed]);
    test([mo7.ifsd isEqual:mo1.ifsd]);
    test(![mo7 hasIvsd]);
    test([mo7.iood count] > 0 && ((TestOptionalOneOptional*)[mo7.iood objectForKey:@5]).a == 15);
    test(![mo7 hasIoopd]);

    // Clear the second half of the optional parameters
    TestOptionalMultiOptional* mo8 = ICE_AUTORELEASE([mo5 copy]);
    [mo8 clearB];
    [mo8 clearD];
    [mo8 clearF];
    [mo8 clearH];
    [mo8 clearJ];
    [mo8 clearBs];
    [mo8 clearIid];
    [mo8 clearFs];

    [mo8 clearShs];
    [mo8 clearFss];
    [mo8 clearOos];

    [mo8 clearIfsd];
    [mo8 clearIood];

    TestOptionalMultiOptional* mo9 = (TestOptionalMultiOptional*)[initial pingPong:mo8];
    test(mo9.a == mo1.a);
    test(![mo9 hasB]);
    test(mo9.c == mo1.c);
    test(![mo9 hasD]);
    test(mo9.e == mo1.e);
    test(![mo9 hasF]);
    test(mo9.g == mo1.g);
    test(![mo9 hasH]);
    test(mo9.i == mo1.i);
    test(![mo9 hasJ]);
    //test(mo9.k == mo9.k);
    test(![mo9 hasBs]);
    test([mo9.ss isEqual:mo1.ss]);
    test(![mo9 hasIid]);
    test([mo9.sid isEqual:mo1.sid]);
    test(![mo9 hasFs]);
    test([mo9.vs isEqual:mo1.vs]);

    test(![mo8 hasShs]);
    test([mo8.es isEqual:mo1.es]);
    test(![mo8 hasFss]);
    test([mo8.vss isEqual:mo1.vss]);
    test(![mo8 hasOos]);
    test([mo8.oops isEqual:mo1.oops]);

    test([mo8.ied isEqual:mo1.ied]);
    test(![mo8 hasIfsd]);
    test([mo8.ivsd isEqual:mo1.ivsd]);
    test(![mo8 hasIood]);

    //
    // Send a request using blobjects. Upon receival, we don't read
    // any of the optional members. This ensures the optional members
    // are skipped even if the receiver knows nothing about them.
    //
    [factory setEnabled:YES];
    id<ICEOutputStream> os = [ICEUtil createOutputStream:communicator];
    [os startEncapsulation];
    [os writeValue:oo1];
    [os endEncapsulation];
    ICEByteSeq* inEncaps = [os finished];
    ICEMutableByteSeq* outEncaps;
    test([initial ice_invoke:@"pingPong" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
    id<ICEInputStream> is = [ICEUtil createInputStream:communicator data:outEncaps];
    [is startEncapsulation];
    ICEObject* obj;
    [is readValue:&obj];
    [is endEncapsulation];
    test(obj != nil && [obj isKindOfClass:[TestObjectReader class]]);

    os = [ICEUtil createOutputStream:communicator];
    [os startEncapsulation];
    [os writeValue:mo1];
    [os endEncapsulation];
    inEncaps = [os finished];
    test([initial ice_invoke:@"pingPong" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
    is = [ICEUtil createInputStream:communicator data:outEncaps];
    [is startEncapsulation];
    [is readValue:&obj];
    [is endEncapsulation];
    test(obj != nil && [obj isKindOfClass:[TestObjectReader class]]);
    [factory setEnabled:false];

    //
    // Use the 1.0 encoding with operations whose only class parameters are optional.
    //
    id oo = [TestOptionalOneOptional oneOptional:@53];
    [initial sendOptionalClass:YES o:oo];
    [[initial ice_encodingVersion:ICEEncoding_1_0] sendOptionalClass:YES o:oo];

    [initial returnOptionalClass:YES o:&oo];
    test(oo != nil && oo != ICENone);
    [[initial ice_encodingVersion:ICEEncoding_1_0] returnOptionalClass:YES o:&oo];
    test(oo == ICENone);

    TestOptionalG* g = [TestOptionalG g];
    g.gg1 = [TestOptionalG1 g1:@"g1"];
    g.gg1Opt = [TestOptionalG1 g1:@"g1opt"];
    g.gg2 = [TestOptionalG2 g2:10];
    g.gg2Opt = [TestOptionalG2 g2:20];
    TestOptionalG* r = [initial opG:g];

    test([r.gg1.a isEqualToString:@"g1"]);
    test([r.gg1Opt.a isEqualToString:@"g1opt"]);
    test(r.gg2.a == 10);
    test(r.gg2Opt.a == 20);

    os = [ICEUtil createOutputStream:communicator];
    [os startEncapsulation];
    [ICEIntHelper writeOptional:@15 stream:os tag:1];
    [ICEStringHelper writeOptional:@"test" stream:os tag:2];
    [os endEncapsulation];
    inEncaps = [os finished];
    test([initial ice_invoke:@"opVoid" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);

    tprintf("ok\n");

    tprintf("testing marshalling of large containers with fixed size elements...");
    TestOptionalMultiOptional* mc = [TestOptionalMultiOptional multiOptional];

    mc.bs = [TestOptionalMutableByteSeq dataWithLength:1000];
    mc.shs = [TestOptionalMutableShortSeq dataWithLength:300 * sizeof(ICEShort)];
    mc.fss = [TestOptionalMutableFixedStructSeq array];
    for(int i = 0; i < 300; ++i)
    {
        [(TestOptionalMutableFixedStructSeq*)mc.fss addObject:[TestOptionalFixedStruct fixedStruct]];
    }

    mc.ifsd = [TestOptionalMutableIntFixedStructDict dictionary];
    for(int i = 0; i < 300; ++i)
    {
        [(TestOptionalMutableIntFixedStructDict*)mc.ifsd setObject:[TestOptionalFixedStruct fixedStruct] forKey:@(i)];
    }

    mc = (TestOptionalMultiOptional*)[initial pingPong:mc];
    test([mc.bs length] == 1000);
    test([mc.shs length] == 300 * sizeof(ICEShort));
    test([mc.fss count] == 300);
    test([mc.ifsd count] == 300);

    [factory setEnabled:YES];
    os = [ICEUtil createOutputStream:communicator];
    [os startEncapsulation];
    [os writeValue:mc];
    [os endEncapsulation];
    inEncaps = [os finished];

    test([initial ice_invoke:@"pingPong" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
    is = [ICEUtil createInputStream:communicator data:outEncaps];
    [is startEncapsulation];
    [is readValue:&obj];
    [is endEncapsulation];
    test(obj != nil && [obj isKindOfClass:[TestObjectReader class]]);
    [factory setEnabled:NO];

    tprintf("ok\n");

    tprintf("testing tag marshalling... ");
    TestOptionalB* b = [TestOptionalB b];
    TestOptionalB* b2 = (TestOptionalB*)[initial pingPong:b];
    test(![b2 hasMa]);
    test(![b2 hasMb]);
    test(![b2 hasMc]);

    b.ma = 10;
    b.mb = 11;
    b.mc = 12;
    b.md = 13;

    b2 = (TestOptionalB*)[initial pingPong:b];
    test(b2.ma == 10);
    test(b2.mb == 11);
    test(b2.mc == 12);
    test(b2.md == 13);

    [factory setEnabled:YES];
    os = [ICEUtil createOutputStream:communicator];
    [os startEncapsulation];
    [os writeValue:b];
    [os endEncapsulation];
    inEncaps = [os finished];
    test([initial ice_invoke:@"pingPong" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
    is = [ICEUtil createInputStream:communicator data:outEncaps];
    [is startEncapsulation];
    [is readValue:&obj];
    [is endEncapsulation];
    test(obj != nil);
    [factory setEnabled:NO];

    tprintf("ok\n");

    tprintf("testing marshalling of objects with optional objects...");
    {
        TestOptionalF* f = [TestOptionalF f];

        f.af = [TestOptionalA a];
        f.ae = f.af;

        TestOptionalF* rf = (TestOptionalF*)[initial pingPong:f];
        test(rf.ae == rf.af);

        [factory setEnabled:YES];
        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [os writeValue:f];
        [os endEncapsulation];
        inEncaps = [os finished];
        is = [ICEUtil createInputStream:communicator data:inEncaps];
        [is startEncapsulation];
        [is readValue:&obj];
        [is endEncapsulation];
        [factory setEnabled:NO];

        test(obj != nil && [obj isKindOfClass:[FObjectReader class]]);
        rf = [(FObjectReader*)obj getF];
        test(rf.ae != nil && ![rf hasAf]);
    }
    tprintf("ok\n");

    tprintf("testing optional with default values... ");
    TestOptionalWD* wd = (TestOptionalWD*)[initial pingPong:[TestOptionalWD wd]];
    test(wd.a == 5);
    test([wd.s isEqualToString:@"test"]);
    [wd clearA];
    [wd clearS];
    wd = (TestOptionalWD*)[initial pingPong:wd];
    test(![wd hasA]);
    test(![wd hasS]);
    tprintf("ok\n");

    if([[communicator getProperties] getPropertyAsInt:@"Ice.Default.SlicedFormat"] > 0)
    {
        tprintf("testing marshalling with unknown class slices... ");
        {
            TestOptionalC* c = [TestOptionalC c];
            c.ss = @"test";
            c.ms = @"testms";
            os = [ICEUtil createOutputStream:communicator];
            [os startEncapsulation];
            [os writeValue:c];
            [os endEncapsulation];
            inEncaps = [os finished];
            [factory setEnabled:YES];
            test([initial ice_invoke:@"pingPong" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
            is = [ICEUtil createInputStream:communicator data:outEncaps];
            [is startEncapsulation];
            [is readValue:&obj];
            [is endEncapsulation];
            test(obj != nil && [obj isKindOfClass:[CObjectReader class]]);
            [factory setEnabled:NO];

            [factory setEnabled:YES];
            os = [ICEUtil createOutputStream:communicator];
            [os startEncapsulation];
            ICEObject* d = [DObjectWriter new];
            [os writeValue:d];
            ICE_RELEASE(d);
            [os endEncapsulation];
            inEncaps = [os finished];
            test([initial ice_invoke:@"pingPong" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
            is = [ICEUtil createInputStream:communicator data:outEncaps];
            [is startEncapsulation];
            [is readValue:&obj];
            [is endEncapsulation];
            test(obj != nil && [obj isKindOfClass:[DObjectReader class]]);
            [(DObjectReader*)obj check];
            [factory setEnabled:NO];
        }
        tprintf("ok\n");

        tprintf("testing optionals with unknown classes...");
        {
            TestOptionalA* a = [TestOptionalA a];

            os = [ICEUtil createOutputStream:communicator];
            [os startEncapsulation];
            [os writeValue:a];
            DObjectWriter* writer = [DObjectWriter new];
            [ICEObjectHelper writeOptional:writer stream:os tag:1];
            ICE_RELEASE(writer);
            [os endEncapsulation];
            inEncaps = [os finished];
            test([initial ice_invoke:@"opClassAndUnknownOptional" mode:ICENormal inEncaps:inEncaps
                           outEncaps:&outEncaps]);
            is = [ICEUtil createInputStream:communicator data:outEncaps];
            [is startEncapsulation];
            [is endEncapsulation];
        }
        tprintf("ok\n");
    }

    tprintf("testing optional parameters... ");
    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opByte:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @0x56;
        p2 = [initial opByte:p1 p3:&p3];
        test([p2 isEqual:@0x56] && [p3 isEqual:@0x56]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEByteHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opByte" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [ICEByteHelper readOptional:is tag:1];
        p3 = [ICEByteHelper readOptional:is tag:3];

        id p4 = @0x08;
        p4 = [ICEByteHelper readOptional:is tag:89];

        [is endEncapsulation];
        test([p2 isEqual:@0x56] && [p3 isEqual:@0x56] && [p4 isEqual:ICENone]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opBool:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @YES;
        p2 = [initial opBool:p1 p3:&p3];
        test([p2 isEqual:@YES] && [p3 isEqual:@YES]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEBoolHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opBool" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [ICEBoolHelper readOptional:is tag:1];
        p3 = [ICEBoolHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:@YES] && [p3 isEqual:@YES]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opShort:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @56;
        p2 = [initial opShort:p1 p3:&p3];
        test([p2 isEqual:@56] && [p3 isEqual:@56]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEShortHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opShort" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [ICEShortHelper readOptional:is tag:1];
        p3 = [ICEShortHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:@56] && [p3 isEqual:@56]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opInt:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @56;
        p2 = [initial opInt:p1 p3:&p3];
        test([p2 isEqual:@56] && [p3 isEqual:@56]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEIntHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opInt" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [ICEIntHelper readOptional:is tag:1];
        p3 = [ICEIntHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:@56] && [p3 isEqual:@56]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opLong:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @56;
        p2 = [initial opLong:p1 p3:&p3];
        test([p2 isEqual:@56] && [p3 isEqual:@56]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICELongHelper writeOptional:p1 stream:os tag:1];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opLong" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p3 = [ICELongHelper readOptional:is tag:2];
        p2 = [ICELongHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:@56] && [p3 isEqual:@56]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opFloat:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @1.0f;
        p2 = [initial opFloat:p1 p3:&p3];
        test([p2 isEqual:@1.0f] && [p3 isEqual:@1.0f]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEFloatHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opFloat" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p3 = [ICEFloatHelper readOptional:is tag:1];
        p2 = [ICEFloatHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:@1.0f] && [p3 isEqual:@1.0f]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opDouble:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @1.0;
        p2 = [initial opDouble:p1 p3:&p3];
        test([p2 isEqual:@1.0] && [p3 isEqual:@1.0]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEDoubleHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opDouble" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p3 = [ICEDoubleHelper readOptional:is tag:1];
        p2 = [ICEDoubleHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:@1.0] && [p3 isEqual:@1.0]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opString:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @"test";
        p2 = [initial opString:p1 p3:&p3];
        test([p2 isEqualToString:@"test"] && [p3 isEqualToString:@"test"]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEStringHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opString" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [ICEStringHelper readOptional:is tag:1];
        p3 = [ICEStringHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqualToString:@"test"] && [p3 isEqualToString:@"test"]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

   {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opMyEnum:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = @(TestOptionalMyEnumMember);
        p2 = [initial opMyEnum:p1 p3:&p3];
        test([p2 isEqual:@(TestOptionalMyEnumMember)] && [p3 isEqual:@(TestOptionalMyEnumMember)]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalMyEnumHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opMyEnum" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalMyEnumHelper readOptional:is tag:1];
        p3 = [TestOptionalMyEnumHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:@(TestOptionalMyEnumMember)] && [p3 isEqual:@(TestOptionalMyEnumMember)]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opSmallStruct:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = [TestOptionalSmallStruct smallStruct:56];
        p2 = [initial opSmallStruct:p1 p3:&p3];
        test(((TestOptionalSmallStruct*)p2).m == 56 && ((TestOptionalSmallStruct*)p3).m== 56);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalSmallStructHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opSmallStruct" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalSmallStructHelper readOptional:is tag:1];
        p3 = [TestOptionalSmallStructHelper readOptional:is tag:3];
        [is endEncapsulation];
        test(((TestOptionalSmallStruct*)p2).m == 56 && ((TestOptionalSmallStruct*)p3).m== 56);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opFixedStruct:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = [TestOptionalFixedStruct fixedStruct:56];
        p2 = [initial opFixedStruct:p1 p3:&p3];
        test(((TestOptionalFixedStruct*)p2).m == 56 && ((TestOptionalFixedStruct*)p3).m== 56);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalFixedStructHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opFixedStruct" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalFixedStructHelper readOptional:is tag:1];
        p3 = [TestOptionalFixedStructHelper readOptional:is tag:3];
        [is endEncapsulation];
        test(((TestOptionalFixedStruct*)p2).m == 56 && ((TestOptionalFixedStruct*)p3).m== 56);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opVarStruct:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = [TestOptionalVarStruct varStruct:@"test"];
        p2 = [initial opVarStruct:p1 p3:&p3];
        test([((TestOptionalVarStruct*)p2).m isEqual:@"test"] && [((TestOptionalVarStruct*)p3).m isEqual:@"test"]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalVarStructHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opVarStruct" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalVarStructHelper readOptional:is tag:1];
        p3 = [TestOptionalVarStructHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([((TestOptionalVarStruct*)p2).m isEqual:@"test"] && [((TestOptionalVarStruct*)p3).m isEqual:@"test"]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opOneOptional:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = [TestOptionalOneOptional oneOptional:@58];
        p2 = [initial opOneOptional:p1 p3:&p3];
        test([p2 isKindOfClass:[TestOptionalOneOptional class]] && [p3 isKindOfClass:[TestOptionalOneOptional class]]);
        test(((TestOptionalOneOptional*)p2).a == 58 && ((TestOptionalOneOptional*)p3).a == 58);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEObjectHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opOneOptional" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [ICEObjectHelper readOptional:&p2 stream:is tag:1];
        [ICEObjectHelper readOptional:&p3 stream:is tag:3];
        [is endEncapsulation];
        test([p2 isKindOfClass:[TestOptionalOneOptional class]] && [p3 isKindOfClass:[TestOptionalOneOptional class]]);
        test(((TestOptionalOneOptional*)p2).a == 58 && ((TestOptionalOneOptional*)p3).a == 58);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opOneOptionalProxy:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p1 = [TestOptionalOneOptionalPrx uncheckedCast:[communicator stringToProxy:@"test"]];
        p2 = [initial opOneOptionalProxy:p1 p3:&p3];
        test([p2 isKindOfClass:[TestOptionalOneOptionalPrx class]] &&
             [p3 isKindOfClass:[TestOptionalOneOptionalPrx class]]);
        test([p2 isEqual:p1] && [p3 isEqual:p1]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [ICEProxyHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opOneOptionalProxy" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalOneOptionalPrxHelper readOptional:is tag:1];
        p3 = [TestOptionalOneOptionalPrxHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isKindOfClass:[TestOptionalOneOptionalPrx class]] &&
             [p3 isKindOfClass:[TestOptionalOneOptionalPrx class]]);
        test([p2 isEqual:p1] && [p3 isEqual:p1]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        TestOptionalF* f = [TestOptionalF f];
        f.af = [TestOptionalA a];
        f.af.requiredA = 56;
        f.ae = f.af;

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalFHelper writeOptional:f stream:os tag:1];
        [TestOptionalFHelper writeOptional:f.ae stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];

        is = [ICEUtil createInputStream:communicator data:inEncaps];
        [is startEncapsulation];
        id a;
        [TestOptionalAHelper readOptional:&a stream:is tag:2];
        [is endEncapsulation];
        test(a != nil && [a isKindOfClass:[TestOptionalA class]] && ((TestOptionalA*)a).requiredA == 56);
    }
    tprintf("ok\n");

    tprintf("testing optional parameters and sequences... ");
    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opByteSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        TestOptionalByteSeq* bs = [TestOptionalMutableByteSeq dataWithLength:100];
        p1 = bs;
        p2 = [initial opByteSeq:p1 p3:&p3];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalByteSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opByteSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalByteSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalByteSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opBoolSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        TestOptionalBoolSeq* bs = [TestOptionalMutableBoolSeq dataWithLength:100];
        p1 = bs;
        p2 = [initial opBoolSeq:p1 p3:&p3];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalBoolSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opBoolSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalBoolSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalBoolSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opShortSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        TestOptionalShortSeq* bs = [TestOptionalMutableShortSeq dataWithLength:100 * sizeof(ICEShort)];
        p1 = bs;
        p2 = [initial opShortSeq:p1 p3:&p3];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalShortSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opShortSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalShortSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalShortSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opIntSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        TestOptionalIntSeq* bs = [TestOptionalMutableIntSeq dataWithLength:100 * sizeof(ICEInt)];
        p1 = bs;
        p2 = [initial opIntSeq:p1 p3:&p3];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalIntSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opIntSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalIntSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalIntSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opLongSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        TestOptionalLongSeq* bs = [TestOptionalMutableLongSeq dataWithLength:100 * sizeof(ICELong)];
        p1 = bs;
        p2 = [initial opLongSeq:p1 p3:&p3];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalLongSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opLongSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalLongSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalLongSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opFloatSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        TestOptionalFloatSeq* bs = [TestOptionalMutableFloatSeq dataWithLength:100 * sizeof(ICEFloat)];
        p1 = bs;
        p2 = [initial opFloatSeq:p1 p3:&p3];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalFloatSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opFloatSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalFloatSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalFloatSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opDoubleSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        TestOptionalDoubleSeq* bs = [TestOptionalMutableDoubleSeq dataWithLength:100 * sizeof(ICEDouble)];
        p1 = bs;
        p2 = [initial opDoubleSeq:p1 p3:&p3];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalDoubleSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opDoubleSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalDoubleSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalDoubleSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:bs] && [p3 isEqual:bs]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opStringSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        TestOptionalMutableStringSeq* ss = [TestOptionalMutableStringSeq array];
        [ss addObject:@"test1"];
        p1 = ss;
        p2 = [initial opStringSeq:p1 p3:&p3];
        test([p2 isEqual:ss] && [p3 isEqual:ss]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalStringSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opStringSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalStringSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalStringSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test([p2 isEqual:ss] && [p3 isEqual:ss]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opFixedStructSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p2 = [initial opFixedStructSeq:[TestOptionalMutableFixedStructSeq array] p3:&p3];
        test(p2 != nil && p3 != nil && [p2 count] == 0 && [p3 count] == 0);

        TestOptionalMutableFixedStructSeq* fss = [TestOptionalMutableFixedStructSeq array];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:1]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:2]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:3]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:4]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:5]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:6]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:7]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:8]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:9]];
        [fss addObject:[TestOptionalFixedStruct fixedStruct:10]];
        p1 = fss;
        p2 = [initial opFixedStructSeq:p1 p3:&p3];
        test(p2 != nil && p3 != nil);
        test([p2 isEqual:fss] && [p3 isEqual:fss]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalFixedStructSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opFixedStructSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalFixedStructSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalFixedStructSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test(p2 != nil && p3 != nil);
        test([p2 isEqual:fss] && [p3 isEqual:fss]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }

    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opVarStructSeq:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p2 = [initial opVarStructSeq:[TestOptionalMutableVarStructSeq array] p3:&p3];
        test(p2 != nil && p3 != nil && [p2 count] == 0 && [p3 count] == 0);

        TestOptionalMutableVarStructSeq* fss = [TestOptionalMutableVarStructSeq array];
        [fss addObject:[TestOptionalVarStruct varStruct:@"1"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"2"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"3"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"4"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"5"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"6"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"7"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"8"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"9"]];
        [fss addObject:[TestOptionalVarStruct varStruct:@"10"]];
        p1 = fss;
        p2 = [initial opVarStructSeq:p1 p3:&p3];
        test(p2 != nil && p3 != nil);
        test([p2 isEqual:fss] && [p3 isEqual:fss]);

        os = [ICEUtil createOutputStream:communicator];
        [os startEncapsulation];
        [TestOptionalVarStructSeqHelper writeOptional:p1 stream:os tag:2];
        [os endEncapsulation];
        inEncaps = [os finished];
        [initial ice_invoke:@"opVarStructSeq" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        p2 = [TestOptionalVarStructSeqHelper readOptional:is tag:1];
        p3 = [TestOptionalVarStructSeqHelper readOptional:is tag:3];
        [is endEncapsulation];
        test(p2 != nil && p3 != nil);
        test([p2 isEqual:fss] && [p3 isEqual:fss]);

        is = [ICEUtil createInputStream:communicator data:outEncaps];
        [is startEncapsulation];
        [is endEncapsulation];
    }
    tprintf("ok\n");

    tprintf("testing optional parameters and dictionaries... ");
    {
        id p1 = ICENone;
        id p3 = ICENone;
        id p2 = [initial opIntIntDict:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p2 = [initial opIntIntDict:[TestOptionalIntIntDict dictionary] p3:&p3];
        test(p2 != nil && p3 != nil && [p2 count] == 0 && [p3 count] == 0);

        TestOptionalMutableIntIntDict* iid = [TestOptionalMutableIntIntDict dictionary];
        [iid setObject:@45 forKey:@1];
        p1 = iid;
        p2 = [initial opIntIntDict:p1 p3:&p3];
        test(p2 != nil && p3 != nil);
        test([p2 isEqual:iid] && [p3 isEqual:iid]);

        p1 = ICENone;
        p3 = ICENone;
        p2 = [initial opStringIntDict:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p2 = [initial opIntIntDict:[TestOptionalStringIntDict dictionary] p3:&p3];
        test(p2 != nil && p3 != nil && [p2 count] == 0 && [p3 count] == 0);

        TestOptionalMutableStringIntDict* sid = [TestOptionalMutableStringIntDict dictionary];
        [sid setObject:@45 forKey:@"1"];
        p1 = sid;
        p2 = [initial opStringIntDict:p1 p3:&p3];
        test(p2 != nil && p3 != nil);
        test([p2 isEqual:sid] && [p3 isEqual:sid]);

        p1 = ICENone;
        p3 = ICENone;
        p2 = [initial opIntOneOptionalDict:p1 p3:&p3];
        test(p2 == ICENone && p3 == ICENone);

        p2 = [initial opIntOneOptionalDict:[TestOptionalStringIntDict dictionary] p3:&p3];
        test(p2 != nil && p3 != nil && [p2 count] == 0 && [p3 count] == 0);

        TestOptionalMutableIntOneOptionalDict* iod = [TestOptionalMutableIntOneOptionalDict dictionary];
        TestOptionalOneOptional* oneOpt = [TestOptionalOneOptional oneOptional:@58];
        [iod setObject:oneOpt forKey:@1];
        p1 = iod;
        p2 = [initial opIntOneOptionalDict:p1 p3:&p3];
        test(p2 != nil && p3 != nil);
        test(((TestOptionalOneOptional*)[p2 objectForKey:@1]).a == 58 &&
             ((TestOptionalOneOptional*)[p3 objectForKey:@1]).a == 58);
    }
    tprintf("ok\n");

    tprintf("testing exception optionals... ");
    {
        @try
        {
            [initial opOptionalException:ICENone b:ICENone o:ICENone];
            test(NO);
        }
        @catch(TestOptionalOptionalException* ex)
        {
            test(![ex hasA]);
            test(![ex hasB]);
            test(![ex hasO]);
        }

        @try
        {
            [initial opOptionalException:@30 b:@"test" o:[TestOptionalOneOptional oneOptional:@53]];
            test(NO);
        }
        @catch(TestOptionalOptionalException* ex)
        {
            test(ex.a == 30);
            test([ex.b isEqualToString:@"test"]);
            test(ex.o.a == 53);
        }

        @try
        {
            //
            // Use the 1.0 encoding with an exception whose only class members are optional.
            //
            [[initial ice_encodingVersion:ICEEncoding_1_0]
               opOptionalException:@30 b:@"test" o:[TestOptionalOneOptional oneOptional:@53]];
            test(NO);
        }
        @catch(TestOptionalOptionalException* ex)
        {
            test(![ex hasA]);
            test(![ex hasB]);
            test(![ex hasO]);
        }

        @try
        {
            id a = ICENone;
            id b = ICENone;
            id o = ICENone;
            [initial opDerivedException:a b:b o:o];
            test(NO);
        }
        @catch(TestOptionalDerivedException* ex)
        {
            test(![ex hasA]);
            test(![ex hasB]);
            test(![ex hasO]);
            test(![ex hasSs]);
            test(![ex hasO2]);
        }
        @catch(TestOptionalOptionalException* ex)
        {
            test(NO);
        }

        @try
        {
            id a = @30;
            id b = @"test2";
            TestOptionalOneOptional* o = [TestOptionalOneOptional oneOptional:@53];
            [initial opDerivedException:a b:b o:o];
            test(NO);
        }
        @catch(TestOptionalDerivedException* ex)
        {
            test(ex.a == 30);
            test([ex.b isEqualToString:@"test2"]);
            test(ex.o.a == 53);
            test([ex.ss isEqualToString:@"test2"]);
            test(ex.o2.a == 53);
        }
        @catch(TestOptionalOptionalException* ex)
        {
            test(NO);
        }

        @try
        {
            id a = ICENone;
            id b = ICENone;
            id o = ICENone;
            [initial opRequiredException:a b:b o:o];
            test(NO);
        }
        @catch(TestOptionalRequiredException* ex)
        {
            test(![ex hasA]);
            test(![ex hasB]);
            test(![ex hasO]);
            test([ex.ss isEqualToString:@"test"]);
        }
        @catch(TestOptionalOptionalException* ex)
        {
            test(NO);
        }

        @try
        {
            id a = @30;
            id b = @"test2";
            id o = [TestOptionalOneOptional oneOptional:@53];
            [initial opRequiredException:a b:b o:o];
            test(NO);
        }
        @catch(TestOptionalRequiredException* ex)
        {
            test(ex.a == 30);
            test([ex.b isEqualToString:@"test2"]);
            test(ex.o.a == 53);
            test([ex.ss isEqualToString:@"test2"]);
            test(ex.o2.a == 53);
        }
        @catch(TestOptionalOptionalException* ex)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("testing optionals with marshaled results... ");
    {
        test([initial opMStruct1]);
        test([initial opMDict1]);
        test([initial opMSeq1]);
        test([initial opMG1]);

        {
            id p1, p2, p3;
            p3 = [initial opMStruct2:ICENone p2:&p2];
            test(p2 == ICENone && p3 == ICENone);

            p1 = [TestOptionalSmallStruct smallStruct];
            p3 = [initial opMStruct2:p1 p2:&p2];
            test([p2 isEqual:p1] && [p3 isEqual:p1]);
        }
        {
            id p1, p2, p3;
            p3 = [initial opMSeq2:ICENone p2:&p2];
            test(p2 == ICENone && p3 == ICENone);

            p1 = [TestOptionalStringSeq arrayWithObject:@"hello"];
            p3 = [initial opMSeq2:p1 p2:&p2];
            test([[p2 objectAtIndex:0] isEqualToString:@"hello"] &&
                 [[p3 objectAtIndex:0] isEqualToString:@"hello"]);
        }
        {
            id p1, p2, p3;
            p3 = [initial opMDict2:ICENone p2:&p2];
            test(p2 == ICENone && p3 == ICENone);

            p1 = [TestOptionalStringIntDict dictionaryWithObjectsAndKeys:@54, @"test", nil];
            p3 = [initial opMDict2:p1 p2:&p2];
            test([[p2 objectForKey:@"test"] isEqual:@54] && [[p3 objectForKey:@"test"] isEqual:@54]);
        }
        {
            id p1, p2, p3;
            p3 = [initial opMG2:ICENone p2:&p2];
            test(p2 == ICENone && p3 == ICENone);

            p1 = [TestOptionalG g];
            p3 = [initial opMG2:p1 p2:&p2];
            test(p2 != ICENone && p3 != ICENone && p3 == p2);
        }
    }
    tprintf("ok\n");

    return initial;
}
