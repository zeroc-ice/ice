//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OperationsTest.h>
#import <limits.h>
#import <float.h>

#import <Foundation/Foundation.h>

@interface PerThreadContextInvokeThread : NSThread
{
    id<TestOperationsMyClassPrx> proxy;
    NSCondition* cond;
    BOOL called;
}
-(id) init:(id<TestOperationsMyClassPrx>)proxy_;
+(id) create:(id<TestOperationsMyClassPrx>)proxy_;
-(void) join;
-(void) run;
@end

@implementation PerThreadContextInvokeThread
-(id) init:(id<TestOperationsMyClassPrx>)proxy_
{
    self = [super initWithTarget:self selector:@selector(run) object:nil];
    if(self)
    {
        self->called = NO;
        self->proxy = ICE_RETAIN(proxy_);
        cond = [[NSCondition alloc] init];
    }
    return self;
}

+(id) create:(id<TestOperationsMyClassPrx>)proxy_
{
    return ICE_AUTORELEASE([[PerThreadContextInvokeThread alloc] init:proxy_]);
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [proxy release];
    [cond release];
    [super dealloc];
}
#endif

-(void) run
{
    [cond lock];
    ICEMutableContext* ctx = [[[proxy ice_getCommunicator] getImplicitContext] getContext];
    test([ctx count] == 0);
    [ctx setObject:@"UN" forKey:@"one" ];
    [[[proxy ice_getCommunicator] getImplicitContext] setContext:ctx];
    called = YES;
    [cond signal];
    [cond unlock];
}

-(void) join
{
    [cond lock];
    while(!called)
    {
        [cond wait];
    }
    [cond unlock];
}
@end

void
twoways(id<ICECommunicator> communicator, id<TestOperationsMyClassPrx> p, id<TestOperationsMBPrx> bprx)
{
    {
        TestOperationsStringS *literals = [p opStringLiterals];

        test([TestOperationss0  isEqualToString:[literals objectAtIndex: 0]]);

        test([TestOperationss0 isEqualToString:@"\\"] &&
             [TestOperationss0 isEqualToString:TestOperationssw0] &&
             [TestOperationss0 isEqualToString:[literals objectAtIndex:0]] &&
             [TestOperationss0 isEqualToString:[literals objectAtIndex:11]]);

        test([TestOperationss1 isEqualToString:@"A"] &&
             [TestOperationss1 isEqualToString:TestOperationssw1] &&
             [TestOperationss1 isEqualToString:[literals objectAtIndex:1]] &&
             [TestOperationss1 isEqualToString:[literals objectAtIndex:12]]);

        test([TestOperationss2 isEqualToString:@"Ice"] &&
             [TestOperationss2 isEqualToString:TestOperationssw2] &&
             [TestOperationss2 isEqualToString:[literals objectAtIndex:2]] &&
             [TestOperationss2 isEqualToString:[literals objectAtIndex:13]]);

        test([TestOperationss3 isEqualToString:@"A21"] &&
             [TestOperationss3 isEqualToString:TestOperationssw3] &&
             [TestOperationss3 isEqualToString:[literals objectAtIndex:3]] &&
             [TestOperationss3 isEqualToString:[literals objectAtIndex:14]]);

        test([TestOperationss4 isEqualToString:@"\\u0041 \\U00000041"] &&
             [TestOperationss4 isEqualToString:TestOperationssw4] &&
             [TestOperationss4 isEqualToString:[literals objectAtIndex:4]] &&
             [TestOperationss4 isEqualToString:[literals objectAtIndex:15]]);

        test([TestOperationss5 isEqualToString:@"ÿ"] &&
             [TestOperationss5 isEqualToString:TestOperationssw5] &&
             [TestOperationss5 isEqualToString:[literals objectAtIndex:5]] &&
             [TestOperationss5 isEqualToString:[literals objectAtIndex:16]]);

        test([TestOperationss6 isEqualToString:@"Ͽ"] &&
             [TestOperationss6 isEqualToString:TestOperationssw6] &&
             [TestOperationss6 isEqualToString:[literals objectAtIndex:6]] &&
             [TestOperationss6 isEqualToString:[literals objectAtIndex:17]]);

        test([TestOperationss7 isEqualToString:@"װ"] &&
             [TestOperationss7 isEqualToString:TestOperationssw7] &&
             [TestOperationss7 isEqualToString:[literals objectAtIndex:7]] &&
             [TestOperationss7 isEqualToString:[literals objectAtIndex:18]]);

        test([TestOperationss8 isEqualToString:@"\U00010000"] &&
             [TestOperationss8 isEqualToString:TestOperationssw8] &&
             [TestOperationss8 isEqualToString:[literals objectAtIndex:8]] &&
             [TestOperationss8 isEqualToString:[literals objectAtIndex:19]]);

        test([TestOperationss9 isEqualToString:@"\U0001F34C"] &&
             [TestOperationss9 isEqualToString:TestOperationssw9] &&
             [TestOperationss9 isEqualToString:[literals objectAtIndex:9]] &&
             [TestOperationss9 isEqualToString:[literals objectAtIndex:20]]);

        test([TestOperationss10 isEqualToString:@"ට"] &&
             [TestOperationss10 isEqualToString:TestOperationssw10] &&
             [TestOperationss10 isEqualToString:[literals objectAtIndex:10]] &&
             [TestOperationss10 isEqualToString:[literals objectAtIndex:21]]);

        test([TestOperationsss0 isEqualToString:@"\'\"\?\\\a\b\f\n\r\t\v\6"] &&
             [TestOperationsss0 isEqualToString:TestOperationsss1] &&
             [TestOperationsss0 isEqualToString:TestOperationsss2] &&
             [TestOperationsss0 isEqualToString:[literals objectAtIndex:22]] &&
             [TestOperationsss0 isEqualToString:[literals objectAtIndex:23]] &&
             [TestOperationsss0 isEqualToString:[literals objectAtIndex:24]]);

        test([TestOperationsss3 isEqualToString:@"\\\\U\\u\\"] &&
             [TestOperationsss3 isEqualToString:[literals objectAtIndex:25]]);

        test([TestOperationsss4 isEqualToString:@"\\A\\"] &&
             [TestOperationsss4 isEqualToString:[literals objectAtIndex:26]]);

        test([TestOperationsss5 isEqualToString:@"\\u0041\\"] &&
             [TestOperationsss5 isEqualToString:[literals objectAtIndex:27]]);

        test([TestOperationssu0 isEqualToString:TestOperationssu1] &&
             [TestOperationssu0 isEqualToString:TestOperationssu1] &&
             [TestOperationssu0 isEqualToString:[literals objectAtIndex:28]] &&
             [TestOperationssu0 isEqualToString:[literals objectAtIndex:29]] &&
             [TestOperationssu0 isEqualToString:[literals objectAtIndex:30]]);
    }

    {
        [p opVoid];
    }

    {
        ICEByte b;
        ICEByte r;

        r = [p opByte:(ICEByte)0xff p2:(ICEByte)0x0f p3:&b];
        test(b == (ICEByte)0xf0);
        test(r == (ICEByte)0xff);
    }

    {
        BOOL b;
        BOOL r;

        r = [p opBool:YES p2:NO p3:&b];
        test(b);
        test(!r);
    }

    {
        ICEShort s;
        ICEInt i;
        ICELong l;
        ICELong r;

        r = [p opShortIntLong:10 p2:11 p3:12 p4:&s p5:&i p6:&l];
        test(s == 10);
        test(i == 11);
        test(l == 12);
        test(r == 12);

#ifndef SHORT_MIN
#define SHORT_MIN (-0x7fff)
#endif
        r = [p opShortIntLong:SHORT_MIN p2:INT_MIN p3:LONG_MIN p4:&s p5:&i p6:&l];
        test(s == SHORT_MIN);
        test(i == INT_MIN);
        test(l == LONG_MIN);
        test(r == LONG_MIN);

#ifndef SHORT_MAX
#define SHORT_MAX 0x7fff
#endif
        r = [p opShortIntLong:SHORT_MAX p2:INT_MAX p3:LONG_MAX p4:&s p5:&i p6:&l];
        test(s == SHORT_MAX);
        test(i == INT_MAX);
        test(l == LONG_MAX);
        test(r == LONG_MAX);
    }

    {
        ICEFloat f;
        ICEDouble d;
        ICEDouble r;

        r = [p opFloatDouble:3.14f p2:1.1E10 p3:&f p4:&d];
        test(f == 3.14f);
        test(d == 1.1E10);
        test(r == 1.1E10);

        r = [p opFloatDouble:FLT_MIN p2:DBL_MIN p3:&f p4:&d];
        test(f == FLT_MIN);
        test(d == DBL_MIN);
        test(r == DBL_MIN);

        r = [p opFloatDouble:FLT_MAX p2:DBL_MAX p3:&f p4:&d];
        test(f == FLT_MAX);
        test(d == DBL_MAX);
        test(r == DBL_MAX);
    }

    {
        NSMutableString *s = nil;
        NSMutableString *r;

        r = [p opString:@"hello" p2:@"world" p3:&s];
        r = [p opString:@"hello" p2:@"world" p3:&s];
        r = [p opString:@"hello" p2:@"world" p3:&s];
        r = [p opString:@"hello" p2:@"world" p3:&s];
        test([s isEqualToString:@"world hello"]);
        test([r isEqualToString:@"hello world"]);
    }

    {
        TestOperationsMyEnum e;
        TestOperationsMyEnum r;

        r = [p opMyEnum:TestOperationsenum2 p2:&e];
        test(e == TestOperationsenum2);
        test(r == TestOperationsenum3);
    }

    {
        id<TestOperationsMyClassPrx> c1;
        id<TestOperationsMyClassPrx> c2;
        id<TestOperationsMyClassPrx> r;

        r = [p opMyClass:p p2:&c1 p3:&c2];
        test([c1 compareIdentityAndFacet:p] == NSOrderedSame);
        test([c2 compareIdentityAndFacet:p] != NSOrderedSame);
        test([r compareIdentityAndFacet:p] == NSOrderedSame);
        test([[c1 ice_getIdentity] isEqual:[ICEUtil stringToIdentity:@"test"]]);
        test([[c2 ice_getIdentity]isEqual:[ICEUtil stringToIdentity:@"noSuchIdentity"]]);
        test([[r ice_getIdentity] isEqual:[ICEUtil stringToIdentity:@"test"]]);
        [r opVoid];
        [c1 opVoid];
        @try
        {
            [c2 opVoid];
            test(NO);
        }
        @catch(ICEObjectNotExistException*)
        {
        }

        r = [p opMyClass:0 p2:&c1 p3:&c2];
        test(c1 == nil);
        test(c2 != nil);
        test([r compareIdentityAndFacet:p] == NSOrderedSame);
        [r opVoid];
    }

    {
        TestOperationsStructure *si1 = [TestOperationsStructure structure];
        si1.p = p;
        si1.e = TestOperationsenum3;
        si1.s = [TestOperationsAnotherStruct anotherStruct];
        si1.s.s = @"abc";
        TestOperationsStructure *si2 = [TestOperationsStructure structure];
        si2.p = nil;
        si2.e = TestOperationsenum2;
        si2.s = [TestOperationsAnotherStruct anotherStruct];
        si2.s.s = @"def";

        TestOperationsStructure *so;
        TestOperationsStructure *rso = [p opStruct:si1 p2:si2 p3:&so];

        test(rso.p == nil);
        test(rso.e == TestOperationsenum2);
        test([rso.s.s isEqualToString:@"def"]);
        test(so.p != nil);
        test([so.p isEqual:p]);
        test(so.e == TestOperationsenum3);
        test([so.s.s isEqualToString:@"a new string"]);
        [so.p opVoid];

        // Testing null struct
        si1 = [TestOperationsStructure structure];
        si2 = nil;
        rso = [p opStruct:si1 p2:si2 p3:&so];

        test(rso.p == nil);
        test(rso.e == TestOperationsenum1);
        test([rso.s.s isEqualToString:@""]);
        test(so.p == nil);
        test(so.e == TestOperationsenum1);
        test([so.s.s isEqualToString:@"a new string"]);
    }

    {
        ICEByte buf1[] = { 0x01, 0x11, 0x12, 0x22 };
        ICEByte buf2[] = { 0xf1, 0xf2, 0xf3, 0xf4 };

        TestOperationsMutableByteS *bsi1 = [TestOperationsMutableByteS data];
        TestOperationsMutableByteS *bsi2 = [TestOperationsMutableByteS data];

        [bsi1 appendBytes:buf1 length:sizeof(buf1)];
        [bsi2 appendBytes:buf2 length:sizeof(buf2)];

        TestOperationsMutableByteS *bso;
        TestOperationsByteS *rso;

        rso = [p opByteS:bsi1 p2:bsi2 p3:&bso];

        test([bso length] == 4);
        ICEByte *bbso = (ICEByte *)[bso bytes];
        test(bbso[0] == 0x22);
        test(bbso[1] == 0x12);
        test(bbso[2] == 0x11);
        test(bbso[3] == 0x01);
        test([rso length] == 8);
        ICEByte *brso = (ICEByte *)[rso bytes];
        test(brso[0] == 0x01);
        test(brso[1] == 0x11);
        test(brso[2] == 0x12);
        test(brso[3] == 0x22);
        test(brso[4] == 0xf1);
        test(brso[5] == 0xf2);
        test(brso[6] == 0xf3);
        test(brso[7] == 0xf4);
    }

    {
        BOOL buf1[] = { YES, YES, NO };
        BOOL buf2[] = { NO };

        TestOperationsMutableBoolS *bsi1 = [TestOperationsMutableBoolS data];
        TestOperationsMutableBoolS *bsi2 = [TestOperationsMutableBoolS data];

        [bsi1 appendBytes:buf1 length:sizeof(buf1)];
        [bsi2 appendBytes:buf2 length:sizeof(buf2)];

        TestOperationsMutableBoolS *bso;
        TestOperationsBoolS *rso;

        rso = [p opBoolS:bsi1 p2:bsi2 p3:&bso];

        test([bso length] == 4 * sizeof(BOOL));
        BOOL *bbso = (BOOL *)[bso bytes];
        test(bbso[0]);
        test(bbso[1]);
        test(!bbso[2]);
        test(!bbso[3]);
        test([rso length] == 3 * sizeof(BOOL));
        BOOL *brso = (BOOL *)[rso bytes];
        test(!brso[0]);
        test(brso[1]);
        test(brso[2]);
    }

    {
        ICEShort buf1[] = { 1, 2, 3 };
        ICEInt buf2[] = { 5, 6, 7, 8 };
        ICELong buf3[] = { 10, 30, 20 };

        TestOperationsMutableShortS *ssi = [TestOperationsMutableShortS data];
        TestOperationsMutableIntS *isi = [TestOperationsMutableIntS data];
        TestOperationsMutableLongS *lsi = [TestOperationsMutableLongS data];

        [ssi appendBytes:buf1 length:sizeof(buf1)];
        [isi appendBytes:buf2 length:sizeof(buf2)];
        [lsi appendBytes:buf3 length:sizeof(buf3)];

        TestOperationsMutableShortS *sso;
        TestOperationsMutableIntS *iso;
        TestOperationsMutableLongS *lso;
        TestOperationsLongS *rso;

        rso = [p opShortIntLongS:ssi p2:isi p3:lsi p4:&sso p5:&iso p6:&lso];

        test([sso length] == 3 * sizeof(ICEShort));
        ICEShort *bsso = (ICEShort *)[sso bytes];
        test(bsso[0] == 1);
        test(bsso[1] == 2);
        test(bsso[2] == 3);
        test([iso length] == 4 * sizeof(ICEInt));
        ICEInt *biso = (ICEInt *)[iso bytes];
        test(biso[0] == 8);
        test(biso[1] == 7);
        test(biso[2] == 6);
        test(biso[3] == 5);
        test([lso length] == 6 * sizeof(ICELong));
        ICELong *blso = (ICELong *)[lso bytes];
        test(blso[0] == 10);
        test(blso[1] == 30);
        test(blso[2] == 20);
        test(blso[3] == 10);
        test(blso[4] == 30);
        test(blso[5] == 20);
        test([rso length] == 3 * sizeof(ICELong));
        ICELong *brso = (ICELong *)[rso bytes];
        test(brso[0] == 10);
        test(brso[1] == 30);
        test(brso[2] == 20);
    }

    {
        ICEFloat buf1[] = { 3.14f, 1.11f };
        ICEDouble buf2[] = { 1.1E10, 1.2E10, 1.3E10 };

        TestOperationsMutableFloatS *fsi = [TestOperationsMutableFloatS data];
        TestOperationsMutableDoubleS *dsi = [TestOperationsMutableDoubleS data];

        [fsi appendBytes:buf1 length:sizeof(buf1)];
        [dsi appendBytes:buf2 length:sizeof(buf2)];

        TestOperationsMutableFloatS *fso;
        TestOperationsMutableDoubleS *dso;
        TestOperationsDoubleS *rso;

        rso = [p opFloatDoubleS:fsi p2:dsi p3:&fso p4:&dso];

        test([fso length] == 2 * sizeof(ICEFloat));
        ICEFloat *bfso = (ICEFloat *)[fso bytes];
        test(bfso[0] == 3.14f);
        test(bfso[1] == 1.11f);
        test([dso length] == 3 * sizeof(ICEDouble));
        ICEDouble *bdso = (ICEDouble *)[dso bytes];
        test(bdso[0] == 1.3E10);
        test(bdso[1] == 1.2E10);
        test(bdso[2] == 1.1E10);
        test([rso length] == 5 * sizeof(ICEDouble));
        ICEDouble *brso = (ICEDouble *)[rso bytes];
        test(brso[0] == 1.1E10);
        test(brso[1] == 1.2E10);
        test(brso[2] == 1.3E10);
        test((ICEFloat)brso[3] == 3.14f);
        test((ICEFloat)brso[4] == 1.11f);
    }

    {
        TestOperationsMutableStringS *ssi1 = [TestOperationsMutableStringS arrayWithCapacity:3];
        TestOperationsMutableStringS *ssi2 = [TestOperationsMutableStringS arrayWithCapacity:1];

        [ssi1 addObject:@"abc"];
        [ssi1 addObject:@"de"];
        [ssi1 addObject:@"fghi"];

        [ssi2 addObject:@"xyz"];

        TestOperationsMutableStringS *sso;
        TestOperationsStringS *rso;

        rso = [p opStringS:ssi1 p2:ssi2 p3:&sso];

        test([sso count] == 4);
        test([[sso objectAtIndex:0] isEqualToString:@"abc"]);
        test([[sso objectAtIndex:1] isEqualToString:@"de"]);
        test([[sso objectAtIndex:2] isEqualToString:@"fghi"]);
        test([[sso objectAtIndex:3] isEqualToString:@"xyz"]);
        test([rso count] == 3);
        test([[rso objectAtIndex:0] isEqualToString:@"fghi"]);
        test([[rso objectAtIndex:1] isEqualToString:@"de"]);
        test([[rso objectAtIndex:2] isEqualToString:@"abc"]);
    }

    @try
    {
        TestOperationsMutableMyClassS *ssi1 = [TestOperationsMutableMyClassS arrayWithCapacity:3];
        TestOperationsMutableMyClassS *ssi2 = [TestOperationsMutableMyClassS arrayWithCapacity:1];

        ICEIdentity *i1 = [ICEIdentity identity:@"abc" category:@""];
        ICEIdentity *i2 = [ICEIdentity identity:@"de" category:@""];
        ICEIdentity *i3 = [ICEIdentity identity:@"fhgi" category:@""];
        ICEIdentity *i4 = [ICEIdentity identity:@"xyz" category:@""];

        [ssi1 addObject:[p ice_identity:i1]];
        [ssi1 addObject:[p ice_identity:i2]];
        [ssi1 addObject:[p ice_identity:i3]];

        [ssi2 addObject:[p ice_identity:i4]];

        TestOperationsMutableMyClassS *sso;
        TestOperationsMyClassS *rso;

        rso = [p opMyClassS:ssi1 p2:ssi2 p3:&sso];

        test([sso count] == 4);
        test([[[sso objectAtIndex:0] ice_getIdentity] isEqual:i1]);
        test([[[sso objectAtIndex:1] ice_getIdentity] isEqual:i2]);
        test([[[sso objectAtIndex:2] ice_getIdentity] isEqual:i3]);
        test([[[sso objectAtIndex:3] ice_getIdentity] isEqual:i4]);
        test([rso count] == 3);
        test([[[rso objectAtIndex:0] ice_getIdentity] isEqual:i3]);
        test([[[rso objectAtIndex:1] ice_getIdentity] isEqual:i2]);
        test([[[rso objectAtIndex:2] ice_getIdentity] isEqual:i1]);
    }
    @catch(ICEOperationNotExistException*)
    {
        // Some mapping don't implement this method.
    }

    @try
    {
        TestOperationsMyEnum buf1[] = { TestOperationsenum2, TestOperationsenum3, TestOperationsenum3 };
        TestOperationsMyEnum buf2[] = { TestOperationsenum1 };

        TestOperationsMutableMyEnumS *esi1 = [TestOperationsMutableMyEnumS data];
        TestOperationsMutableMyEnumS *esi2 = [TestOperationsMutableMyEnumS data];

        [esi1 appendBytes:buf1 length:sizeof(buf1)];
        [esi2 appendBytes:buf2 length:sizeof(buf2)];

        TestOperationsMutableMyEnumS *eso;
        TestOperationsMyEnumS *rso;

        rso = [p opMyEnumS:esi1 p2:esi2 p3:&eso];

        test([eso length] / sizeof(TestOperationsMyEnum) == 3);
        const TestOperationsMyEnum *beso = (const TestOperationsMyEnum *)[eso bytes];
        test(beso[0] == TestOperationsenum3);
        test(beso[1] == TestOperationsenum3);
        test(beso[2] == TestOperationsenum2);
        test([rso length] / sizeof(TestOperationsMyEnum) == 4);
        const TestOperationsMyEnum *brso = (const TestOperationsMyEnum *)[rso bytes];
        test(brso[0] == TestOperationsenum2);
        test(brso[1] == TestOperationsenum3);
        test(brso[2] == TestOperationsenum3);
        test(brso[3] == TestOperationsenum1);
    }
    @catch(ICEOperationNotExistException*)
    {
        // Some mapping don't implement this method.
    }

    {
        TestOperationsMutableByteSS *bsi1 = [TestOperationsMutableByteSS array];
        TestOperationsMutableByteSS *bsi2 = [TestOperationsMutableByteSS array];

        ICEByte b;
        TestOperationsMutableByteS *tmp = [TestOperationsMutableByteS data];

        b = 0x01;
        [tmp appendBytes:&b length:sizeof(b)];
        b = 0x11;
        [tmp appendBytes:&b length:sizeof(b)];
        b = 0x12;
        [tmp appendBytes:&b length:sizeof(b)];
        [bsi1 addObject:tmp];

        tmp = [TestOperationsMutableByteS data];
        b = 0xff;
        [tmp appendBytes:&b length:sizeof(b)];
        [bsi1 addObject:tmp];

        tmp = [TestOperationsMutableByteS data];
        b = 0x0e;
        [tmp appendBytes:&b length:sizeof(b)];
        [bsi2 addObject:tmp];

        tmp = [TestOperationsMutableByteS data];
        b = 0xf2;
        [tmp appendBytes:&b length:sizeof(b)];
        b = 0xf1;
        [tmp appendBytes:&b length:sizeof(b)];
        [bsi2 addObject:tmp];

        TestOperationsMutableByteSS *bso;
        TestOperationsByteSS *rso;

        rso = [p opByteSS:bsi1 p2:bsi2 p3:&bso];

        const ICEByte *pb;
        test([bso count] == 2);
        test([[bso objectAtIndex:0] length] / sizeof(ICEByte) == 1);
        pb = [[bso objectAtIndex:0] bytes];
        test(pb[0] == (ICEByte)0x0ff);
        test([[bso objectAtIndex:1] length] / sizeof(ICEByte) == 3);
        pb = [[bso objectAtIndex:1] bytes];
        test(pb[0] == (ICEByte)0x01);
        test(pb[1] == (ICEByte)0x11);
        test(pb[2] == (ICEByte)0x12);
        test([rso count] == 4);
        test([[rso objectAtIndex:0] length] / sizeof(ICEByte) == 3);
        pb = [[rso objectAtIndex:0] bytes];
        test(pb[0] == (ICEByte)0x01);
        test(pb[1] == (ICEByte)0x11);
        test(pb[2] == (ICEByte)0x12);
        test([[rso objectAtIndex:1] length] / sizeof(ICEByte) == 1);
        pb = [[rso objectAtIndex:1] bytes];
        test(pb[0] == (ICEByte)0xff);
        test([[rso objectAtIndex:2] length] / sizeof(ICEByte) == 1);
        pb = [[rso objectAtIndex:2] bytes];
        test(pb[0] == (ICEByte)0x0e);
        test([[rso objectAtIndex:3] length] / sizeof(ICEByte) == 2);
        pb = [[rso objectAtIndex:3] bytes];
        test(pb[0] == (ICEByte)0xf2);
        test(pb[1] == (ICEByte)0xf1);
    }

    {
        TestOperationsMutableFloatSS *fsi = [TestOperationsMutableFloatSS array];
        TestOperationsMutableDoubleSS *dsi = [TestOperationsMutableDoubleSS array];

        ICEFloat f;
        TestOperationsMutableFloatS *ftmp;

        ftmp = [TestOperationsMutableFloatS data];
        f = 3.14f;
        [ftmp appendBytes:&f length:sizeof(f)];
        [fsi addObject:ftmp];
        ftmp = [TestOperationsMutableFloatS data];
        f = 1.11f;
        [ftmp appendBytes:&f length:sizeof(f)];
        [fsi addObject:ftmp];
        ftmp = [TestOperationsMutableFloatS data];
        [fsi addObject:ftmp];

        ICEDouble d;
        TestOperationsMutableDoubleS *dtmp;

        dtmp = [TestOperationsMutableDoubleS data];
        d = 1.1E10;
        [dtmp appendBytes:&d length:sizeof(d)];
        d = 1.2E10;
        [dtmp appendBytes:&d length:sizeof(d)];
        d = 1.3E10;
        [dtmp appendBytes:&d length:sizeof(d)];
        [dsi addObject:dtmp];

        TestOperationsMutableFloatSS *fso;
        TestOperationsMutableDoubleSS *dso;
        TestOperationsDoubleSS *rso;

        rso = [p opFloatDoubleSS:fsi p2:dsi p3:&fso p4:&dso];

        const ICEFloat *fp;
        const ICEDouble *dp;

        test([fso count] == 3);
        test([[fso objectAtIndex:0] length] / sizeof(ICEFloat) == 1);
        fp = [[fso objectAtIndex:0] bytes];
        test(fp[0] == 3.14f);
        test([[fso objectAtIndex:1] length] / sizeof(ICEFloat) == 1);
        fp = [[fso objectAtIndex:1] bytes];
        test(fp[0] == 1.11f);
        test([[fso objectAtIndex:2] length] / sizeof(ICEFloat) == 0);
        test([dso count] == 1);
        test([[dso objectAtIndex:0] length] / sizeof(ICEDouble) == 3);
        dp = [[dso objectAtIndex:0] bytes];
        test(dp[0] == 1.1E10);
        test(dp[1] == 1.2E10);
        test(dp[2] == 1.3E10);
        test([rso count] == 2);
        test([[rso objectAtIndex:0] length] / sizeof(ICEDouble) == 3);
        dp = [[rso objectAtIndex:0] bytes];
        test(dp[0] == 1.1E10);
        test(dp[1] == 1.2E10);
        test(dp[2] == 1.3E10);
        test([[rso objectAtIndex:1] length] / sizeof(ICEDouble) == 3);
        dp = [[rso objectAtIndex:1] bytes];
        test(dp[0] == 1.1E10);
        test(dp[1] == 1.2E10);
        test(dp[2] == 1.3E10);
    }

    {
        TestOperationsMutableStringSS * ssi1 = [TestOperationsMutableStringSS array];
        TestOperationsMutableStringSS * ssi2 = [TestOperationsMutableStringSS array];

        TestOperationsMutableStringS *tmp;

        tmp = [TestOperationsMutableStringS array];
        [tmp addObject:@"abc"];
        [ssi1 addObject:tmp];
        tmp = [TestOperationsMutableStringS array];
        [tmp addObject:@"de"];
        [tmp addObject:@"fghi"];
        [ssi1 addObject:tmp];

        [ssi2 addObject:[TestOperationsStringS array]];
        [ssi2 addObject:[TestOperationsStringS array]];
        tmp = [TestOperationsMutableStringS array];
        [tmp addObject:@"xyz"];
        [ssi2 addObject:tmp];

        TestOperationsMutableStringSS *sso;
        TestOperationsStringSS *rso;

        rso = [p opStringSS:ssi1 p2:ssi2 p3:&sso];

        test([sso count] == 5);
        test([[sso objectAtIndex:0] count] == 1);
        test([[[sso objectAtIndex:0] objectAtIndex:0] isEqualToString:@"abc"]);
        test([[sso objectAtIndex:1] count] == 2);
        test([[[sso objectAtIndex:1] objectAtIndex:0] isEqualToString:@"de"]);
        test([[[sso objectAtIndex:1] objectAtIndex:1] isEqualToString:@"fghi"]);
        test([[sso objectAtIndex:2] count] == 0);
        test([[sso objectAtIndex:3] count] == 0);
        test([[sso objectAtIndex:4] count] == 1);
        test([[[sso objectAtIndex:4] objectAtIndex:0] isEqualToString:@"xyz"]);
        test([rso count] == 3);
        test([[rso objectAtIndex:0] count] == 1);
        test([[[rso objectAtIndex:0] objectAtIndex:0] isEqualToString:@"xyz"]);
        test([[rso objectAtIndex:1] count] == 0);
        test([[rso objectAtIndex:2] count] == 0);
    }

    {
        TestOperationsMutableStringSSS *sssi1 = [TestOperationsMutableStringSSS array];
        TestOperationsMutableStringSSS *sssi2 = [TestOperationsMutableStringSSS array];

        TestOperationsMutableStringSS *tmpss;
        TestOperationsMutableStringS *tmps;

        tmps = [TestOperationsMutableStringS array];
        [tmps addObject:@"abc"];
        [tmps addObject:@"de"];
        tmpss = [TestOperationsMutableStringSS array];
        [tmpss addObject:tmps];
        tmps = [TestOperationsMutableStringS array];
        [tmps addObject:@"xyz"];
        [tmpss addObject:tmps];
        [sssi1 addObject:tmpss];
        tmps = [TestOperationsMutableStringS array];
        [tmps addObject:@"hello"];
        tmpss = [TestOperationsMutableStringSS array];
        [tmpss addObject:tmps];
        [sssi1 addObject:tmpss];

        tmps = [TestOperationsMutableStringS array];
        [tmps addObject:@""];
        [tmps addObject:@""];
        tmpss = [TestOperationsMutableStringSS array];
        [tmpss addObject:tmps];
        tmps = [TestOperationsMutableStringS array];
        [tmps addObject:@"abcd"];
        [tmpss addObject:tmps];
        [sssi2 addObject:tmpss];
        tmps = [TestOperationsMutableStringS array];
        [tmps addObject:@""];
        tmpss = [TestOperationsMutableStringSS array];
        [tmpss addObject:tmps];
        [sssi2 addObject:tmpss];
        tmpss = [TestOperationsMutableStringSS array];
        [sssi2 addObject:tmpss];

        TestOperationsMutableStringSSS *ssso;
        TestOperationsStringSSS *rsso;

        rsso = [p opStringSSS:sssi1 p2:sssi2 p3:&ssso];

        test([ssso count] == 5);
        test([[ssso objectAtIndex:0] count] == 2);
        test([[[ssso objectAtIndex:0] objectAtIndex:0] count] == 2);
        test([[[ssso objectAtIndex:0] objectAtIndex:1] count] == 1);
        test([[ssso objectAtIndex:1] count] == 1);
        test([[[ssso objectAtIndex:1] objectAtIndex:0] count] == 1);
        test([[ssso objectAtIndex:2] count] == 2);
        test([[[ssso objectAtIndex:2] objectAtIndex:0] count] == 2);
        test([[[ssso objectAtIndex:2] objectAtIndex:1] count] == 1);
        test([[ssso objectAtIndex:3] count] == 1);
        test([[[ssso objectAtIndex:3] objectAtIndex:0] count] == 1);
        test([[ssso objectAtIndex:4] count] == 0);
        test([[[[ssso objectAtIndex:0] objectAtIndex:0] objectAtIndex:0] isEqualToString:@"abc"]);
        test([[[[ssso objectAtIndex:0] objectAtIndex:0] objectAtIndex:1] isEqualToString:@"de"]);
        test([[[[ssso objectAtIndex:0] objectAtIndex:1] objectAtIndex:0] isEqualToString:@"xyz"]);
        test([[[[ssso objectAtIndex:1] objectAtIndex:0] objectAtIndex:0] isEqualToString:@"hello"]);
        test([[[[ssso objectAtIndex:2] objectAtIndex:0] objectAtIndex:0] isEqualToString:@""]);
        test([[[[ssso objectAtIndex:2] objectAtIndex:0] objectAtIndex:1] isEqualToString:@""]);
        test([[[[ssso objectAtIndex:2] objectAtIndex:1] objectAtIndex:0] isEqualToString:@"abcd"]);
        test([[[[ssso objectAtIndex:3] objectAtIndex:0] objectAtIndex:0] isEqualToString:@""]);

        test([rsso count] == 3);
        test([[rsso objectAtIndex:0] count] == 0);
        test([[rsso objectAtIndex:1] count] == 1);
        test([[[rsso objectAtIndex:1] objectAtIndex:0] count] == 1);
        test([[rsso objectAtIndex:2] count] == 2);
        test([[[rsso objectAtIndex:2] objectAtIndex:0] count] == 2);
        test([[[rsso objectAtIndex:2] objectAtIndex:1] count] == 1);
        test([[[[rsso objectAtIndex:1] objectAtIndex:0] objectAtIndex:0] isEqualToString:@""]);
        test([[[[rsso objectAtIndex:2] objectAtIndex:0] objectAtIndex:0] isEqualToString:@""]);
        test([[[[rsso objectAtIndex:2] objectAtIndex:0] objectAtIndex:1] isEqualToString:@""]);
        test([[[[rsso objectAtIndex:2] objectAtIndex:1] objectAtIndex:0] isEqualToString:@"abcd"]);
    }

    {
        TestOperationsMutableByteBoolD *di1 = [TestOperationsMutableByteBoolD dictionary];
        [di1 setObject:[NSNumber numberWithBool:YES] forKey:[NSNumber numberWithUnsignedChar:10]];
        [di1 setObject:[NSNumber numberWithBool:NO] forKey:[NSNumber numberWithUnsignedChar:100]];
        TestOperationsMutableByteBoolD *di2 = [TestOperationsMutableByteBoolD dictionary];
        [di2 setObject:[NSNumber numberWithBool:YES] forKey:[NSNumber numberWithUnsignedChar:10]];
        [di2 setObject:[NSNumber numberWithBool:NO] forKey:[NSNumber numberWithUnsignedChar:11]];
        [di2 setObject:[NSNumber numberWithBool:TRUE] forKey:[NSNumber numberWithUnsignedChar:101]];

        TestOperationsMutableByteBoolD *_do;
        TestOperationsMutableByteBoolD *ro = [p opByteBoolD:di1 p2:di2 p3:&_do];

        test([_do isEqualToDictionary:di1]);
        test([ro count] == 4);
        test([[ro objectForKey:[NSNumber numberWithUnsignedChar:10]] boolValue] == YES);
        test([[ro objectForKey:[NSNumber numberWithUnsignedChar:11]] boolValue] == NO);
        test([[ro objectForKey:[NSNumber numberWithUnsignedChar:100]] boolValue] == NO);
        test([[ro objectForKey:[NSNumber numberWithUnsignedChar:101]] boolValue] == YES);
    }

    {
        TestOperationsMutableShortIntD *di1 = [TestOperationsMutableShortIntD dictionary];
        [di1 setObject:[NSNumber numberWithInt:-1] forKey:[NSNumber numberWithShort:110]];
        [di1 setObject:[NSNumber numberWithInt:123123] forKey:[NSNumber numberWithShort:1100]];
        TestOperationsMutableShortIntD *di2 = [TestOperationsMutableShortIntD dictionary];
        [di2 setObject:[NSNumber numberWithInt:-1] forKey:[NSNumber numberWithShort:110]];
        [di2 setObject:[NSNumber numberWithInt:-100] forKey:[NSNumber numberWithShort:111]];
        [di2 setObject:[NSNumber numberWithInt:0] forKey:[NSNumber numberWithShort:1101]];

        TestOperationsMutableShortIntD *_do;
        TestOperationsMutableShortIntD *ro = [p opShortIntD:di1 p2:di2 p3:&_do];

        test([_do isEqualToDictionary:di1]);
        test([ro count] == 4);
        test([[ro objectForKey:[NSNumber numberWithShort:110]] intValue] == -1);
        test([[ro objectForKey:[NSNumber numberWithShort:111]] intValue] == -100);
        test([[ro objectForKey:[NSNumber numberWithShort:1100]] intValue] == 123123);
        test([[ro objectForKey:[NSNumber numberWithShort:1101]] intValue] == 0);
    }

    {
        TestOperationsMutableLongFloatD *di1 = [TestOperationsMutableLongFloatD dictionary];
        [di1 setObject:[NSNumber numberWithFloat:-1.1f] forKey:[NSNumber numberWithLong:999999110]];
        [di1 setObject:[NSNumber numberWithFloat:123123.2f] forKey:[NSNumber numberWithLong:999999111]];
        TestOperationsMutableLongFloatD *di2 = [TestOperationsMutableLongFloatD dictionary];
        [di2 setObject:[NSNumber numberWithFloat:-1.1f] forKey:[NSNumber numberWithLong:999999110]];
        [di2 setObject:[NSNumber numberWithFloat:-100.4f] forKey:[NSNumber numberWithLong:999999120]];
        [di2 setObject:[NSNumber numberWithFloat:0.5f] forKey:[NSNumber numberWithLong:999999130]];

        TestOperationsMutableLongFloatD *_do;
        TestOperationsMutableLongFloatD *ro = [p opLongFloatD:di1 p2:di2 p3:&_do];

        test([_do isEqualToDictionary:di1]);
        test([ro count] == 4);
        test((ICEFloat)[[ro objectForKey:[NSNumber numberWithLong:999999110]] floatValue] == -1.1f);
        test((ICEFloat)[[ro objectForKey:[NSNumber numberWithLong:999999120]] floatValue] == -100.4f);
        test((ICEFloat)[[ro objectForKey:[NSNumber numberWithLong:999999111]] floatValue] == 123123.2f);
        test((ICEFloat)[[ro objectForKey:[NSNumber numberWithLong:999999130]] floatValue] == 0.5f);
    }

    {
        TestOperationsMutableStringStringD *di1 = [TestOperationsMutableStringStringD dictionary];
        [di1 setObject:@"abc -1.1" forKey:@"foo"];
        [di1 setObject:@"abc 123123.2" forKey:@"bar"];
        TestOperationsMutableStringStringD *di2 = [TestOperationsMutableStringStringD dictionary];
        [di2 setObject:@"abc -1.1" forKey:@"foo"];
        [di2 setObject:@"abc -100.4" forKey:@"FOO"];
        [di2 setObject:@"abc 0.5" forKey:@"BAR"];

        TestOperationsMutableStringStringD *_do;
        TestOperationsMutableStringStringD *ro = [p opStringStringD:di1 p2:di2 p3:&_do];

        test([_do isEqualToDictionary:di1]);
        test([ro count] == 4);
        test([[ro objectForKey:@"foo"] isEqualToString:@"abc -1.1"]);
        test([[ro objectForKey:@"FOO"] isEqualToString:@"abc -100.4"]);
        test([[ro objectForKey:@"bar"] isEqualToString:@"abc 123123.2"]);
        test([[ro objectForKey:@"BAR"] isEqualToString:@"abc 0.5"]);
    }

    {
        TestOperationsMutableStringMyEnumD *di1 = [TestOperationsMutableStringMyEnumD dictionary];
        [di1 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:@"abc"];
        [di1 setObject:[NSNumber numberWithInt:TestOperationsenum2] forKey:@""];
        TestOperationsMutableStringMyEnumD *di2 = [TestOperationsMutableStringMyEnumD dictionary];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:@"abc"];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum3] forKey:@"querty"];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum2] forKey:@"Hello!!"];

        TestOperationsMutableStringMyEnumD *_do;
        TestOperationsMutableStringMyEnumD *ro = [p opStringMyEnumD:di1 p2:di2 p3:&_do];

        test([_do isEqualToDictionary:di1]);
        test([ro count] == 4);
        test([[ro objectForKey:@"abc"] intValue] == TestOperationsenum1);
        test([[ro objectForKey:@"querty"] intValue] == TestOperationsenum3);
        test([[ro objectForKey:@""] intValue] == TestOperationsenum2);
        test([[ro objectForKey:@"Hello!!"] intValue] == TestOperationsenum2);
    }

    {
        TestOperationsMutableMyEnumStringD *di1 = [TestOperationsMutableMyEnumStringD dictionary];
        [di1 setObject:@"abc" forKey:@(TestOperationsenum1)];
        TestOperationsMutableMyEnumStringD *di2 = [TestOperationsMutableMyEnumStringD dictionary];
        [di2 setObject:@"Hello!!" forKey:@(TestOperationsenum2)];
        [di2 setObject:@"querty" forKey:@(TestOperationsenum3)];

        TestOperationsMutableMyEnumStringD *_do;
        TestOperationsMutableMyEnumStringD *ro = [p opMyEnumStringD:di1 p2:di2 p3:&_do];

        test([_do isEqualToDictionary:di1]);
        test([ro count] == 3);
        test([[ro objectForKey:@(TestOperationsenum1)] isEqualToString:@"abc"]);
        test([[ro objectForKey:@(TestOperationsenum3)] isEqualToString:@"querty"]);
        test([[ro objectForKey:@(TestOperationsenum2)] isEqualToString:@"Hello!!"]);
    }

    {
        TestOperationsMyStruct* s11 = [TestOperationsMyStruct myStruct:1 j:1];
        TestOperationsMyStruct* s12 = [TestOperationsMyStruct myStruct:1 j:2];
        TestOperationsMutableMyStructMyEnumD* di1 = [TestOperationsMutableMyStructMyEnumD dictionary];
        [di1 setObject:@(TestOperationsenum1) forKey:s11];
        [di1 setObject:@(TestOperationsenum2) forKey:s12];

        TestOperationsMyStruct* s22 = [TestOperationsMyStruct myStruct:2 j:2];
        TestOperationsMyStruct* s23 = [TestOperationsMyStruct myStruct:2 j:3];
        TestOperationsMutableMyStructMyEnumD* di2 = [TestOperationsMutableMyStructMyEnumD dictionary];
        [di2 setObject:@(TestOperationsenum1) forKey:s11];
        [di2 setObject:@(TestOperationsenum3) forKey:s22];
        [di2 setObject:@(TestOperationsenum2) forKey:s23];

        TestOperationsMutableMyStructMyEnumD* _do;
        TestOperationsMyStructMyEnumD* ro = [p opMyStructMyEnumD:di1 p2:di2 p3:&_do];

        test([_do isEqual:di1]);
        test([ro count] == 4);
        test([[ro objectForKey:s11] isEqual:@(TestOperationsenum1)]);
        test([[ro objectForKey:s12] isEqual:@(TestOperationsenum2)]);
        test([[ro objectForKey:s22] isEqual:@(TestOperationsenum3)]);
        test([[ro objectForKey:s23] isEqual:@(TestOperationsenum2)]);
    }

    {
        TestOperationsMutableByteBoolDS* dsi1 = [TestOperationsMutableByteBoolDS array];
        TestOperationsMutableByteBoolDS* dsi2 = [TestOperationsMutableByteBoolDS array];

        TestOperationsMutableByteBoolD *di1 = [TestOperationsMutableByteBoolD dictionary];
        [di1 setObject:[NSNumber numberWithBool:YES] forKey:[NSNumber numberWithUnsignedChar:10]];
        [di1 setObject:[NSNumber numberWithBool:NO] forKey:[NSNumber numberWithUnsignedChar:100]];
        TestOperationsMutableByteBoolD *di2 = [TestOperationsMutableByteBoolD dictionary];
        [di2 setObject:[NSNumber numberWithBool:YES] forKey:[NSNumber numberWithUnsignedChar:10]];
        [di2 setObject:[NSNumber numberWithBool:NO] forKey:[NSNumber numberWithUnsignedChar:11]];
        [di2 setObject:[NSNumber numberWithBool:TRUE] forKey:[NSNumber numberWithUnsignedChar:101]];
        TestOperationsMutableByteBoolD *di3 = [TestOperationsMutableByteBoolD dictionary];
        [di3 setObject:[NSNumber numberWithBool:NO] forKey:[NSNumber numberWithUnsignedChar:100]];
        [di3 setObject:[NSNumber numberWithBool:NO] forKey:[NSNumber numberWithUnsignedChar:101]];

        [dsi1 addObject:di1];
        [dsi1 addObject:di2];
        [dsi2 addObject:di3];

        TestOperationsMutableByteBoolDS* _do;
        TestOperationsByteBoolDS* ro = [p opByteBoolDS:dsi1 p2:dsi2 p3:&_do];

        test([ro count] == 2);
        test([[ro objectAtIndex:0] count] == 3);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithUnsignedChar:10]] boolValue] == YES);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithUnsignedChar:11]] boolValue] == NO);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithUnsignedChar:101]] boolValue] == YES);
        test([[ro objectAtIndex:1] count] == 2);
        test([[[ro objectAtIndex:1] objectForKey:[NSNumber numberWithUnsignedChar:10]] boolValue] == YES);
        test([[[ro objectAtIndex:1] objectForKey:[NSNumber numberWithUnsignedChar:100]] boolValue] == NO);
        test([_do count] == 3);
        test([[_do objectAtIndex:0] count] == 2);
        test([[[_do objectAtIndex:0] objectForKey:[NSNumber numberWithUnsignedChar:100]] boolValue] == NO);
        test([[[_do objectAtIndex:0] objectForKey:[NSNumber numberWithUnsignedChar:101]] boolValue] == NO);
        test([[_do objectAtIndex:1] count] == 2);
        test([[[_do objectAtIndex:1] objectForKey:[NSNumber numberWithUnsignedChar:10]] boolValue] == YES);
        test([[[_do objectAtIndex:1] objectForKey:[NSNumber numberWithUnsignedChar:100]] boolValue] == NO);
        test([[_do objectAtIndex:2] count] == 3);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithUnsignedChar:10]] boolValue] == YES);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithUnsignedChar:11]] boolValue] == NO);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithUnsignedChar:101]] boolValue] == YES);
    }

    {
        TestOperationsMutableShortIntDS *dsi1 = [TestOperationsMutableShortIntDS array];
        TestOperationsMutableShortIntDS *dsi2 = [TestOperationsMutableShortIntDS array];

        TestOperationsMutableShortIntD *di1 = [TestOperationsMutableShortIntD dictionary];
        [di1 setObject:[NSNumber numberWithInt:-1] forKey:[NSNumber numberWithShort:110]];
        [di1 setObject:[NSNumber numberWithInt:123123] forKey:[NSNumber numberWithShort:1100]];
        TestOperationsMutableShortIntD *di2 = [TestOperationsMutableShortIntD dictionary];
        [di2 setObject:[NSNumber numberWithInt:-1] forKey:[NSNumber numberWithShort:110]];
        [di2 setObject:[NSNumber numberWithInt:-100] forKey:[NSNumber numberWithShort:111]];
        [di2 setObject:[NSNumber numberWithInt:0] forKey:[NSNumber numberWithShort:1101]];
        TestOperationsMutableShortIntD *di3 = [TestOperationsMutableShortIntD dictionary];
        [di3 setObject:[NSNumber numberWithInt:-1001] forKey:[NSNumber numberWithShort:100]];

        [dsi1 addObject:di1];
        [dsi1 addObject:di2];
        [dsi2 addObject:di3];

        TestOperationsMutableShortIntDS *_do;
        TestOperationsShortIntDS *ro = [p opShortIntDS:dsi1 p2:dsi2 p3:&_do];

        test([ro count] == 2);
        test([[ro objectAtIndex:0] count] == 3);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithShort:110]] intValue] == -1);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithShort:111]] intValue] == -100);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithShort:1101]] intValue] == 0);
        test([[ro objectAtIndex:1] count] == 2);
        test([[[ro objectAtIndex:1] objectForKey:[NSNumber numberWithShort:110]] intValue] == -1);
        test([[[ro objectAtIndex:1] objectForKey:[NSNumber numberWithShort:1100]] intValue] == 123123);
        test([_do count] == 3);
        test([[_do objectAtIndex:0] count] == 1);
        test([[[_do objectAtIndex:0] objectForKey:[NSNumber numberWithShort:100]] intValue] == -1001);
        test([[_do objectAtIndex:1] count] == 2);
        test([[[_do objectAtIndex:1] objectForKey:[NSNumber numberWithShort:110]] intValue] == -1);
        test([[[_do objectAtIndex:1] objectForKey:[NSNumber numberWithShort:1100]] intValue] == 123123);
        test([[_do objectAtIndex:2] count] == 3);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithShort:110]] intValue] == -1);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithShort:111]] intValue] == -100);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithShort:1101]] intValue] == 0);
    }

    {
        TestOperationsMutableLongFloatDS *dsi1 = [TestOperationsMutableLongFloatDS array];
        TestOperationsMutableLongFloatDS *dsi2 = [TestOperationsMutableLongFloatDS array];

        TestOperationsMutableLongFloatD *di1 = [TestOperationsMutableLongFloatD dictionary];
        [di1 setObject:[NSNumber numberWithFloat:-1.1f] forKey:[NSNumber numberWithLong:999999110]];
        [di1 setObject:[NSNumber numberWithFloat:123123.2f] forKey:[NSNumber numberWithLong:999999111]];
        TestOperationsMutableLongFloatD *di2 = [TestOperationsMutableLongFloatD dictionary];
        [di2 setObject:[NSNumber numberWithFloat:-1.1f] forKey:[NSNumber numberWithLong:999999110]];
        [di2 setObject:[NSNumber numberWithFloat:-100.4f] forKey:[NSNumber numberWithLong:999999120]];
        [di2 setObject:[NSNumber numberWithFloat:0.5f] forKey:[NSNumber numberWithLong:999999130]];
        TestOperationsMutableLongFloatD *di3 = [TestOperationsMutableLongFloatD dictionary];
        [di3 setObject:[NSNumber numberWithFloat:3.14f] forKey:[NSNumber numberWithLong:999999140]];

        [dsi1 addObject:di1];
        [dsi1 addObject:di2];
        [dsi2 addObject:di3];

        TestOperationsMutableLongFloatDS *_do;
        TestOperationsLongFloatDS *ro = [p opLongFloatDS:dsi1 p2:dsi2 p3:&_do];

        test([ro count] == 2);
        test([[ro objectAtIndex:0] count] == 3);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithLong:999999110]] floatValue] == -1.1f);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithLong:999999120]] floatValue] == -100.4f);
        test([[[ro objectAtIndex:0] objectForKey:[NSNumber numberWithLong:999999130]] floatValue] == 0.5f);
        test([[ro objectAtIndex:1] count] == 2);
        test([[[ro objectAtIndex:1] objectForKey:[NSNumber numberWithLong:999999110]] floatValue] == -1.1f);
        test([[[ro objectAtIndex:1] objectForKey:[NSNumber numberWithLong:999999111]] floatValue] == 123123.2f);
        test([_do count] == 3);
        test([[_do objectAtIndex:0] count] == 1);
        test([[[_do objectAtIndex:0] objectForKey:[NSNumber numberWithLong:999999140]] floatValue] == 3.14f);
        test([[_do objectAtIndex:1] count] == 2);
        test([[[_do objectAtIndex:1] objectForKey:[NSNumber numberWithLong:999999110]] floatValue] == -1.1f);
        test([[[_do objectAtIndex:1] objectForKey:[NSNumber numberWithLong:999999111]] floatValue] == 123123.2f);
        test([[_do objectAtIndex:2] count] == 3);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithLong:999999110]] floatValue] == -1.1f);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithLong:999999120]] floatValue] == -100.4f);
        test([[[_do objectAtIndex:2] objectForKey:[NSNumber numberWithLong:999999130]] floatValue] == 0.5f);
    }

    {
        TestOperationsMutableStringStringDS *dsi1 = [TestOperationsMutableStringStringDS array];
        TestOperationsMutableStringStringDS *dsi2 = [TestOperationsMutableStringStringDS array];

        TestOperationsMutableStringStringD *di1 = [TestOperationsMutableStringStringD dictionary];
        [di1 setObject:@"abc -1.1" forKey:@"foo"];
        [di1 setObject:@"abc 123123.2" forKey:@"bar"];
        TestOperationsMutableStringStringD *di2 = [TestOperationsMutableStringStringD dictionary];
        [di2 setObject:@"abc -1.1" forKey:@"foo"];
        [di2 setObject:@"abc -100.4" forKey:@"FOO"];
        [di2 setObject:@"abc 0.5" forKey:@"BAR"];
        TestOperationsMutableStringStringD *di3 = [TestOperationsMutableStringStringD dictionary];
        [di3 setObject:@"ABC -3.14" forKey:@"f00"];

        [dsi1 addObject:di1];
        [dsi1 addObject:di2];
        [dsi2 addObject:di3];

        TestOperationsMutableStringStringDS *_do;
        TestOperationsStringStringDS *ro = [p opStringStringDS:dsi1 p2:dsi2 p3:&_do];

        test([ro count] == 2);
        test([[ro objectAtIndex:0] count] == 3);
        test([[[ro objectAtIndex:0] objectForKey:@"foo"] isEqualToString:@"abc -1.1"]);
        test([[[ro objectAtIndex:0] objectForKey:@"FOO"] isEqualToString:@"abc -100.4"]);
        test([[[ro objectAtIndex:0] objectForKey:@"BAR"] isEqualToString:@"abc 0.5"]);
        test([[ro objectAtIndex:1] count] == 2);
        test([[[ro objectAtIndex:1] objectForKey:@"foo"] isEqualToString:@"abc -1.1"]);
        test([[[ro objectAtIndex:1] objectForKey:@"bar"] isEqualToString:@"abc 123123.2"]);
        test([_do count] == 3);
        test([[_do objectAtIndex:0] count] == 1);
        test([[[_do objectAtIndex:0] objectForKey:@"f00"] isEqualToString:@"ABC -3.14"]);
        test([[_do objectAtIndex:1] count] == 2);
        test([[[_do objectAtIndex:1] objectForKey:@"foo"] isEqualToString:@"abc -1.1"]);
        test([[[_do objectAtIndex:1] objectForKey:@"bar"] isEqualToString:@"abc 123123.2"]);
        test([[_do objectAtIndex:2] count] == 3);
        test([[[_do objectAtIndex:2] objectForKey:@"foo"] isEqualToString:@"abc -1.1"]);
        test([[[_do objectAtIndex:2] objectForKey:@"FOO"] isEqualToString:@"abc -100.4"]);
        test([[[_do objectAtIndex:2] objectForKey:@"BAR"] isEqualToString:@"abc 0.5"]);
    }

    {
        TestOperationsMutableStringMyEnumDS *dsi1 = [TestOperationsMutableStringMyEnumDS array];
        TestOperationsMutableStringMyEnumDS *dsi2 = [TestOperationsMutableStringMyEnumDS array];

        TestOperationsMutableStringMyEnumD *di1 = [TestOperationsMutableStringMyEnumD dictionary];
        [di1 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:@"abc"];
        [di1 setObject:[NSNumber numberWithInt:TestOperationsenum2] forKey:@""];
        TestOperationsMutableStringMyEnumD *di2 = [TestOperationsMutableStringMyEnumD dictionary];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:@"abc"];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum3] forKey:@"querty"];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum2] forKey:@"Hello!!"];
        TestOperationsMutableStringMyEnumD *di3 = [TestOperationsMutableStringMyEnumD dictionary];
        [di3 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:@"Goodbye"];

        [dsi1 addObject:di1];
        [dsi1 addObject:di2];
        [dsi2 addObject:di3];

        TestOperationsMutableStringMyEnumDS *_do;
        TestOperationsStringMyEnumDS *ro = [p opStringMyEnumDS:dsi1 p2:dsi2 p3:&_do];

        test([ro count] == 2);
        test([[ro objectAtIndex:0] count] == 3);
        test([[[ro objectAtIndex:0] objectForKey:@"abc"] intValue] == TestOperationsenum1);
        test([[[ro objectAtIndex:0] objectForKey:@"querty"] intValue] == TestOperationsenum3);
        test([[[ro objectAtIndex:0] objectForKey:@"Hello!!"] intValue] == TestOperationsenum2);
        test([[ro objectAtIndex:1] count] == 2);
        test([[[ro objectAtIndex:1] objectForKey:@"abc"] intValue] == TestOperationsenum1);
        test([[[ro objectAtIndex:1] objectForKey:@""] intValue] == TestOperationsenum2);
        test([_do count] == 3);
        test([[_do objectAtIndex:0] count] == 1);
        test([[[_do objectAtIndex:0] objectForKey:@"Goodbye"] intValue] == TestOperationsenum1);
        test([[_do objectAtIndex:1] count] == 2);
        test([[[_do objectAtIndex:1] objectForKey:@"abc"] intValue] == TestOperationsenum1);
        test([[[_do objectAtIndex:1] objectForKey:@""] intValue] == TestOperationsenum2);
        test([[_do objectAtIndex:2] count] == 3);
        test([[[_do objectAtIndex:2] objectForKey:@"abc"] intValue] == TestOperationsenum1);
        test([[[_do objectAtIndex:2] objectForKey:@"querty"] intValue] == TestOperationsenum3);
        test([[[_do objectAtIndex:2] objectForKey:@"Hello!!"] intValue] == TestOperationsenum2);
    }

    {
        TestOperationsMutableMyEnumStringDS *dsi1 = [TestOperationsMutableMyEnumStringDS array];
        TestOperationsMutableMyEnumStringDS *dsi2 = [TestOperationsMutableMyEnumStringDS array];

        TestOperationsMutableMyEnumStringD *di1 = [TestOperationsMutableMyEnumStringD dictionary];
        [di1 setObject:@"abc" forKey:@(TestOperationsenum1)];
        TestOperationsMutableMyEnumStringD *di2 = [TestOperationsMutableMyEnumStringD dictionary];
        [di2 setObject:@"Hello!!" forKey:@(TestOperationsenum2)];
        [di2 setObject:@"querty" forKey:@(TestOperationsenum3)];
        TestOperationsMutableMyEnumStringD *di3 = [TestOperationsMutableMyEnumStringD dictionary];
        [di3 setObject:@"Goodbye" forKey:@(TestOperationsenum1)];

        [dsi1 addObject:di1];
        [dsi1 addObject:di2];
        [dsi2 addObject:di3];

        TestOperationsMutableMyEnumStringDS *_do;
        TestOperationsMyEnumStringDS *ro = [p opMyEnumStringDS:dsi1 p2:dsi2 p3:&_do];

        test([ro count] == 2);
        test([[ro objectAtIndex:0] count] == 2);
        test([[[ro objectAtIndex:0] objectForKey:@(TestOperationsenum2)] isEqualToString:@"Hello!!"]);
        test([[[ro objectAtIndex:0] objectForKey:@(TestOperationsenum3)] isEqualToString:@"querty"]);
        test([[ro objectAtIndex:1] count] == 1);
        test([[[ro objectAtIndex:1] objectForKey:@(TestOperationsenum1)] isEqualToString:@"abc"]);
        test([_do count] == 3);
        test([[_do objectAtIndex:0] count] == 1);
        test([[[_do objectAtIndex:0] objectForKey:@(TestOperationsenum1)] isEqualToString:@"Goodbye"]);
        test([[_do objectAtIndex:1] count] == 1);
        test([[[_do objectAtIndex:1] objectForKey:@(TestOperationsenum1)] isEqualToString:@"abc"]);
        test([[_do objectAtIndex:2] count] == 2);
        test([[[_do objectAtIndex:2] objectForKey:@(TestOperationsenum2)] isEqualToString:@"Hello!!"]);
        test([[[_do objectAtIndex:2] objectForKey:@(TestOperationsenum3)] isEqualToString:@"querty"]);
    }

    {
        TestOperationsMutableMyStructMyEnumDS *dsi1 = [TestOperationsMutableMyStructMyEnumDS array];
        TestOperationsMutableMyStructMyEnumDS *dsi2 = [TestOperationsMutableMyStructMyEnumDS array];

        TestOperationsMyStruct* s11 = [TestOperationsMyStruct myStruct:1 j:1];
        TestOperationsMyStruct* s12 = [TestOperationsMyStruct myStruct:1 j:2];
        TestOperationsMutableMyStructMyEnumD* di1 = [TestOperationsMutableMyStructMyEnumD dictionary];
        [di1 setObject:@(TestOperationsenum1) forKey:s11];
        [di1 setObject:@(TestOperationsenum2) forKey:s12];

        TestOperationsMyStruct* s22 = [TestOperationsMyStruct myStruct:2 j:2];
        TestOperationsMyStruct* s23 = [TestOperationsMyStruct myStruct:2 j:3];
        TestOperationsMutableMyStructMyEnumD* di2 = [TestOperationsMutableMyStructMyEnumD dictionary];
        [di2 setObject:@(TestOperationsenum1) forKey:s11];
        [di2 setObject:@(TestOperationsenum3) forKey:s22];
        [di2 setObject:@(TestOperationsenum2) forKey:s23];

        TestOperationsMutableMyStructMyEnumD* di3 = [TestOperationsMutableMyStructMyEnumD dictionary];
        [di3 setObject:@(TestOperationsenum3) forKey:s23];

        [dsi1 addObject:di1];
        [dsi1 addObject:di2];
        [dsi2 addObject:di3];

        TestOperationsMutableMyStructMyEnumDS* _do;
        TestOperationsMyStructMyEnumDS* ro = [p opMyStructMyEnumDS:dsi1 p2:dsi2 p3:&_do];

        test([ro count] == 2);
        test([[ro objectAtIndex:0] count] == 3);
        test([[[ro objectAtIndex:0] objectForKey:s11] intValue] == TestOperationsenum1);
        test([[[ro objectAtIndex:0] objectForKey:s22] intValue] == TestOperationsenum3);
        test([[[ro objectAtIndex:0] objectForKey:s23] intValue] == TestOperationsenum2);
        test([[ro objectAtIndex:1] count] == 2);
        test([[[ro objectAtIndex:1] objectForKey:s11] intValue] == TestOperationsenum1);
        test([[[ro objectAtIndex:1] objectForKey:s12] intValue] == TestOperationsenum2);
        test([_do count] == 3);
        test([[_do objectAtIndex:0] count] == 1);
        test([[[_do objectAtIndex:0] objectForKey:s23] intValue] == TestOperationsenum3);
        test([[_do objectAtIndex:1] count] == 2);
        test([[[_do objectAtIndex:1] objectForKey:s11] intValue] == TestOperationsenum1);
        test([[[_do objectAtIndex:1] objectForKey:s12] intValue] == TestOperationsenum2);
        test([[_do objectAtIndex:2] count] == 3);
        test([[[_do objectAtIndex:2] objectForKey:s11] intValue] == TestOperationsenum1);
        test([[[_do objectAtIndex:2] objectForKey:s22] intValue] == TestOperationsenum3);
        test([[[_do objectAtIndex:2] objectForKey:s23] intValue] == TestOperationsenum2);
    }

    {
        TestOperationsMutableByteByteSD *sdi1 = [TestOperationsMutableByteByteSD dictionary];
        TestOperationsMutableByteByteSD *sdi2 = [TestOperationsMutableByteByteSD dictionary];

        ICEByte buf1[] = { 0x01, 0x11 };
        ICEByte buf2[] = { 0x12 };
        ICEByte buf3[] = { 0xf2, 0xf3 };

        TestOperationsMutableByteS *si1 = [TestOperationsMutableBoolS data];
        TestOperationsMutableBoolS *si2 = [TestOperationsMutableBoolS data];
        TestOperationsMutableBoolS *si3 = [TestOperationsMutableBoolS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];
        [si3 appendBytes:buf3 length:sizeof(buf3)];

        [sdi1 setObject:si1 forKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x01]];
        [sdi1 setObject:si2 forKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x22]];
        [sdi2 setObject:si3 forKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]];

        TestOperationsMutableByteByteSD *_do;
        TestOperationsByteByteSD *ro = [p opByteByteSD:sdi1 p2:sdi2 p3:&_do];

        const ICEByte *pb;
        test([_do count] == 1);
        test([[_do objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]] length] / sizeof(ICEByte) == 2);
        pb = [[_do objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]] bytes];
        test(pb[0] == 0xf2);
        test(pb[1] == 0xf3);
        test([ro count] == 3);
        test([[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x01]] length] / sizeof(ICEByte) == 2);
        pb = [[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x01]] bytes];
        test(pb[0] == 0x01);
        test(pb[1] == 0x11);
        test([[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x22]] length] / sizeof(ICEByte) == 1);
        pb = [[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x22]] bytes];
        test(pb[0] == 0x12);
        test([[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]] length] / sizeof(ICEByte) == 2);
        pb = [[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]] bytes];
        test(pb[0] == 0xf2);
        test(pb[1] == 0xf3);
    }

    {
        TestOperationsMutableBoolBoolSD *sdi1 = [TestOperationsMutableBoolBoolSD dictionary];
        TestOperationsMutableBoolBoolSD *sdi2 = [TestOperationsMutableBoolBoolSD dictionary];

        BOOL buf1[] = { YES, NO };
        BOOL buf2[] = { NO, YES, YES };

        TestOperationsMutableBoolS *si1 = [TestOperationsMutableBoolS data];
        TestOperationsMutableBoolS *si2 = [TestOperationsMutableBoolS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];

        [sdi1 setObject:si1 forKey:[NSNumber numberWithBool:NO]];
        [sdi1 setObject:si2 forKey:[NSNumber numberWithBool:YES]];
        [sdi2 setObject:si1 forKey:[NSNumber numberWithBool:NO]];

        TestOperationsMutableBoolBoolSD *_do;
        TestOperationsBoolBoolSD *ro = [p opBoolBoolSD:sdi1 p2:sdi2 p3:&_do];

        const BOOL *pb;
        test([_do count] == 1);
        test([[_do objectForKey:[NSNumber numberWithBool:NO]] length] / sizeof(BOOL) == 2);
        pb = [[_do objectForKey:[NSNumber numberWithBool:NO]] bytes];
        test(pb[0] == YES);
        test(pb[1] == NO);
        test([ro count] == 2);
        test([[ro objectForKey:[NSNumber numberWithBool:NO]] length] / sizeof(BOOL) == 2);
        pb = [[ro objectForKey:[NSNumber numberWithBool:NO]] bytes];
        test(pb[0] == YES);
        test(pb[1] == NO);
        test([[ro objectForKey:[NSNumber numberWithBool:YES]] length] / sizeof(BOOL) == 3);
        pb = [[ro objectForKey:[NSNumber numberWithBool:YES]] bytes];
        test(pb[0] == NO);
        test(pb[1] == YES);
        test(pb[2] == YES);
    }

    {
        TestOperationsMutableShortShortSD *sdi1 = [TestOperationsMutableShortShortSD dictionary];
        TestOperationsMutableShortShortSD *sdi2 = [TestOperationsMutableShortShortSD dictionary];

        ICEShort buf1[] = { 1, 2, 3 };
        ICEShort buf2[] = { 4, 5 };
        ICEShort buf3[] = { 6, 7 };

        TestOperationsMutableShortS *si1 = [TestOperationsMutableShortS data];
        TestOperationsMutableShortS *si2 = [TestOperationsMutableShortS data];
        TestOperationsMutableShortS *si3 = [TestOperationsMutableShortS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];
        [si3 appendBytes:buf3 length:sizeof(buf3)];

        [sdi1 setObject:si1 forKey:[NSNumber numberWithShort:1]];
        [sdi1 setObject:si2 forKey:[NSNumber numberWithShort:2]];
        [sdi2 setObject:si3 forKey:[NSNumber numberWithShort:4]];

        TestOperationsMutableShortShortSD *_do;
        TestOperationsShortShortSD *ro = [p opShortShortSD:sdi1 p2:sdi2 p3:&_do];

        const ICEShort *ps;
        test([_do count] == 1);
        test([[_do objectForKey:[NSNumber numberWithShort:4]] length] / sizeof(ICEShort) == 2);
        ps = [[_do objectForKey:[NSNumber numberWithShort:4]] bytes];
        test(ps[0] == 6);
        test(ps[1] == 7);
        test([ro count] == 3);
        test([[ro objectForKey:[NSNumber numberWithShort:1]] length] / sizeof(ICEShort) == 3);
        ps = [[ro objectForKey:[NSNumber numberWithShort:1]] bytes];
        test(ps[0] == 1);
        test(ps[1] == 2);
        test(ps[2] == 3);
        test([[ro objectForKey:[NSNumber numberWithShort:2]] length] / sizeof(ICEShort) == 2);
        ps = [[ro objectForKey:[NSNumber numberWithShort:2]] bytes];
        test(ps[0] == 4);
        test(ps[1] == 5);
        test([[ro objectForKey:[NSNumber numberWithShort:4]] length] / sizeof(ICEShort) == 2);
        ps = [[ro objectForKey:[NSNumber numberWithShort:4]] bytes];
        test(ps[0] == 6);
        test(ps[1] == 7);
    }

    {
        TestOperationsMutableIntIntSD *sdi1 = [TestOperationsMutableIntIntSD dictionary];
        TestOperationsMutableIntIntSD *sdi2 = [TestOperationsMutableIntIntSD dictionary];

        ICEInt buf1[] = { 100, 200, 300 };
        ICEInt buf2[] = { 400, 500 };
        ICEInt buf3[] = { 600, 700 };

        TestOperationsMutableIntS *si1 = [TestOperationsMutableIntS data];
        TestOperationsMutableIntS *si2 = [TestOperationsMutableIntS data];
        TestOperationsMutableIntS *si3 = [TestOperationsMutableIntS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];
        [si3 appendBytes:buf3 length:sizeof(buf3)];

        [sdi1 setObject:si1 forKey:[NSNumber numberWithInt:100]];
        [sdi1 setObject:si2 forKey:[NSNumber numberWithInt:200]];
        [sdi2 setObject:si3 forKey:[NSNumber numberWithInt:400]];

        TestOperationsMutableIntIntSD *_do;
        TestOperationsIntIntSD *ro = [p opIntIntSD:sdi1 p2:sdi2 p3:&_do];

        const ICEInt *pint;
        test([_do count] == 1);
        test([[_do objectForKey:[NSNumber numberWithInt:400]] length] / sizeof(ICEInt) == 2);
        pint = [[_do objectForKey:[NSNumber numberWithInt:400]] bytes];
        test(pint[0] == 600);
        test(pint[1] == 700);
        test([ro count] == 3);
        test([[ro objectForKey:[NSNumber numberWithInt:100]] length] / sizeof(ICEInt) == 3);
        pint = [[ro objectForKey:[NSNumber numberWithInt:100]] bytes];
        test(pint[0] == 100);
        test(pint[1] == 200);
        test(pint[2] == 300);
        test([[ro objectForKey:[NSNumber numberWithInt:200]] length] / sizeof(ICEInt) == 2);
        pint = [[ro objectForKey:[NSNumber numberWithInt:200]] bytes];
        test(pint[0] == 400);
        test(pint[1] == 500);
        test([[ro objectForKey:[NSNumber numberWithInt:400]] length] / sizeof(ICEInt) == 2);
        pint = [[ro objectForKey:[NSNumber numberWithInt:400]] bytes];
        test(pint[0] == 600);
        test(pint[1] == 700);
    }

    {
        TestOperationsMutableLongLongSD *sdi1 = [TestOperationsMutableLongLongSD dictionary];
        TestOperationsMutableLongLongSD *sdi2 = [TestOperationsMutableLongLongSD dictionary];

        ICELong buf1[] = { 999999110, 999999111, 999999110 };
        ICELong buf2[] = { 999999120, 999999130 };
        ICELong buf3[] = { 999999110, 999999120 };

        TestOperationsMutableLongS *si1 = [TestOperationsMutableLongS data];
        TestOperationsMutableLongS *si2 = [TestOperationsMutableLongS data];
        TestOperationsMutableLongS *si3 = [TestOperationsMutableLongS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];
        [si3 appendBytes:buf3 length:sizeof(buf3)];

        [sdi1 setObject:si1 forKey:[NSNumber numberWithLong:999999990]];
        [sdi1 setObject:si2 forKey:[NSNumber numberWithLong:999999991]];
        [sdi2 setObject:si3 forKey:[NSNumber numberWithLong:999999992]];

        TestOperationsMutableLongLongSD *_do;
        TestOperationsLongLongSD *ro = [p opLongLongSD:sdi1 p2:sdi2 p3:&_do];

        const ICELong *pl;
        test([_do count] == 1);
        test([[_do objectForKey:[NSNumber numberWithLong:999999992]] length] / sizeof(ICELong) == 2);
        pl = [[_do objectForKey:[NSNumber numberWithLong:999999992]] bytes];
        test(pl[0] == 999999110);
        test(pl[1] == 999999120);
        test([ro count] == 3);
        test([[ro objectForKey:[NSNumber numberWithLong:999999990]] length] / sizeof(ICELong) == 3);
        pl = [[ro objectForKey:[NSNumber numberWithLong:999999990]] bytes];
        test(pl[0] == 999999110);
        test(pl[1] == 999999111);
        test(pl[2] == 999999110);
        test([[ro objectForKey:[NSNumber numberWithLong:999999991]] length] / sizeof(ICELong) == 2);
        pl = [[ro objectForKey:[NSNumber numberWithLong:999999991]] bytes];
        test(pl[0] == 999999120);
        test(pl[1] == 999999130);
        test([[ro objectForKey:[NSNumber numberWithLong:999999992]] length] / sizeof(ICELong) == 2);
        pl = [[ro objectForKey:[NSNumber numberWithLong:999999992]] bytes];
        test(pl[0] == 999999110);
        test(pl[1] == 999999120);
    }

    {
        TestOperationsMutableStringFloatSD *sdi1 = [TestOperationsMutableStringFloatSD dictionary];
        TestOperationsMutableStringFloatSD *sdi2 = [TestOperationsMutableStringFloatSD dictionary];

        ICEFloat buf1[] = { -1.1f, 123123.2f, 100.0f };
        ICEFloat buf2[] = { 42.24f, -1.61f };
        ICEFloat buf3[] = { -3.14f, 3.14f };

        TestOperationsMutableFloatS *si1 = [TestOperationsMutableFloatS data];
        TestOperationsMutableFloatS *si2 = [TestOperationsMutableFloatS data];
        TestOperationsMutableFloatS *si3 = [TestOperationsMutableFloatS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];
        [si3 appendBytes:buf3 length:sizeof(buf3)];

        [sdi1 setObject:si1 forKey:@"abc"];
        [sdi1 setObject:si2 forKey:@"ABC"];
        [sdi2 setObject:si3 forKey:@"aBc"];

        TestOperationsMutableStringFloatSD *_do;
        TestOperationsStringFloatSD *ro = [p opStringFloatSD:sdi1 p2:sdi2 p3:&_do];

        const ICEFloat *pf;
        test([_do count] == 1);
        test([[_do objectForKey:@"aBc"] length] / sizeof(ICEFloat) == 2);
        pf = [[_do objectForKey:@"aBc"] bytes];
        test(pf[0] == -3.14f);
        test(pf[1] == 3.14f);
        test([ro count] == 3);
        test([[ro objectForKey:@"abc"] length] / sizeof(ICEFloat) == 3);
        pf = [[ro objectForKey:@"abc"] bytes];
        test(pf[0] == -1.1f);
        test(pf[1] == 123123.2f);
        test(pf[2] == 100.0f);
        test([[ro objectForKey:@"ABC"] length] / sizeof(ICEFloat) == 2);
        pf = [[ro objectForKey:@"ABC"] bytes];
        test(pf[0] == 42.24f);
        test(pf[1] == -1.61f);
        test([[ro objectForKey:@"aBc"] length] / sizeof(ICEFloat) == 2);
        pf = [[ro objectForKey:@"aBc"] bytes];
        test(pf[0] == -3.14f);
        test(pf[1] == 3.14f);
    }

    {
        TestOperationsMutableStringDoubleSD *sdi1 = [TestOperationsMutableStringDoubleSD dictionary];
        TestOperationsMutableStringDoubleSD *sdi2 = [TestOperationsMutableStringDoubleSD dictionary];

        ICEDouble buf1[] = { 1.1E10, 1.2E10, 1.3E10 };
        ICEDouble buf2[] = { 1.4E10, 1.5E10 };
        ICEDouble buf3[] = { 1.6E10, 1.7E10 };

        TestOperationsMutableDoubleS *si1 = [TestOperationsMutableDoubleS data];
        TestOperationsMutableDoubleS *si2 = [TestOperationsMutableDoubleS data];
        TestOperationsMutableDoubleS *si3 = [TestOperationsMutableDoubleS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];
        [si3 appendBytes:buf3 length:sizeof(buf3)];

        [sdi1 setObject:si1 forKey:@"Hello!!"];
        [sdi1 setObject:si2 forKey:@"Goodbye"];
        [sdi2 setObject:si3 forKey:@""];

        TestOperationsMutableStringDoubleSD *_do;
        TestOperationsStringDoubleSD *ro = [p opStringDoubleSD:sdi1 p2:sdi2 p3:&_do];

        const ICEDouble *pd;
        test([_do count] == 1);
        test([[_do objectForKey:@""] length] / sizeof(ICEDouble) == 2);
        pd = [[_do objectForKey:@""] bytes];
        test(pd[0] == 1.6E10);
        test(pd[1] == 1.7E10);
        test([ro count] == 3);
        test([[ro objectForKey:@"Hello!!"] length] / sizeof(ICEDouble) == 3);
        pd = [[ro objectForKey:@"Hello!!"] bytes];
        test(pd[0] == 1.1E10);
        test(pd[1] == 1.2E10);
        test(pd[2] == 1.3E10);
        test([[ro objectForKey:@"Goodbye"] length] / sizeof(ICEDouble) == 2);
        pd = [[ro objectForKey:@"Goodbye"] bytes];
        test(pd[0] == 1.4E10);
        test(pd[1] == 1.5E10);
        test([[ro objectForKey:@""] length] / sizeof(ICEDouble) == 2);
        pd = [[ro objectForKey:@""] bytes];
        test(pd[0] == 1.6E10);
        test(pd[1] == 1.7E10);
    }

    {
        TestOperationsMutableStringStringSD *sdi1 = [TestOperationsMutableStringStringSD dictionary];
        TestOperationsMutableStringStringSD *sdi2 = [TestOperationsMutableStringStringSD dictionary];

        TestOperationsMutableStringS *si1 = [TestOperationsMutableStringS array];
        TestOperationsMutableStringS *si2 = [TestOperationsMutableStringS array];
        TestOperationsMutableStringS *si3 = [TestOperationsMutableStringS array];

        [si1 addObject:@"abc"];
        [si1 addObject:@"de"];
        [si1 addObject:@"fghi"];
        [si2 addObject:@"xyz"];
        [si2 addObject:@"or"];
        [si3 addObject:@"and"];
        [si3 addObject:@"xor"];

        [sdi1 setObject:si1 forKey:@"abc"];
        [sdi1 setObject:si2 forKey:@"def"];
        [sdi2 setObject:si3 forKey:@"ghi"];

        TestOperationsMutableStringStringSD *_do;
        TestOperationsStringStringSD *ro = [p opStringStringSD:sdi1 p2:sdi2 p3:&_do];

        test([[_do objectForKey:@"ghi"] count] == 2);
        test([[[_do objectForKey:@"ghi"] objectAtIndex:0] isEqualToString:@"and"]);
        test([[[_do objectForKey:@"ghi"] objectAtIndex:1] isEqualToString:@"xor"]);
        test([ro count] == 3);
        test([[ro objectForKey:@"abc"] count] == 3);
        test([[[ro objectForKey:@"abc"] objectAtIndex:0] isEqualToString:@"abc"]);
        test([[[ro objectForKey:@"abc"] objectAtIndex:1] isEqualToString:@"de"]);
        test([[[ro objectForKey:@"abc"] objectAtIndex:2] isEqualToString:@"fghi"]);
        test([[ro objectForKey:@"def"] count] == 2);
        test([[[ro objectForKey:@"def"] objectAtIndex:0] isEqualToString:@"xyz"]);
        test([[[ro objectForKey:@"def"] objectAtIndex:1] isEqualToString:@"or"]);
        test([[ro objectForKey:@"ghi"] count] == 2);
        test([[[ro objectForKey:@"ghi"] objectAtIndex:0] isEqualToString:@"and"]);
        test([[[ro objectForKey:@"ghi"] objectAtIndex:1] isEqualToString:@"xor"]);
    }

    {
        TestOperationsMutableMyEnumMyEnumSD *sdi1 = [TestOperationsMutableMyEnumMyEnumSD dictionary];
        TestOperationsMutableMyEnumMyEnumSD *sdi2 = [TestOperationsMutableMyEnumMyEnumSD dictionary];

        TestOperationsMyEnum buf1[] = { TestOperationsenum1, TestOperationsenum1, TestOperationsenum2 };
        TestOperationsMyEnum buf2[] = { TestOperationsenum1, TestOperationsenum2 };
        TestOperationsMyEnum buf3[] = { TestOperationsenum3, TestOperationsenum3 };

        TestOperationsMutableMyEnumS *si1 = [TestOperationsMutableMyEnumS data];
        TestOperationsMutableMyEnumS *si2 = [TestOperationsMutableMyEnumS data];
        TestOperationsMutableMyEnumS *si3 = [TestOperationsMutableMyEnumS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];
        [si3 appendBytes:buf3 length:sizeof(buf3)];

        [sdi1 setObject:si1 forKey:@(TestOperationsenum3)];
        [sdi1 setObject:si2 forKey:@(TestOperationsenum2)];
        [sdi2 setObject:si3 forKey:@(TestOperationsenum1)];

        TestOperationsMutableMyEnumMyEnumSD *_do;
        TestOperationsMyEnumMyEnumSD *ro = [p opMyEnumMyEnumSD:sdi1 p2:sdi2 p3:&_do];

        const TestOperationsMyEnum *pe;
        test([_do count] == 1);
        test([[_do objectForKey:@(TestOperationsenum1)] length] / sizeof(TestOperationsMyEnum) == 2);
        pe = [[_do objectForKey:@(TestOperationsenum1)] bytes];
        test(pe[0] == TestOperationsenum3);
        test(pe[1] == TestOperationsenum3);
        test([ro count] == 3);
        test([[ro objectForKey:@(TestOperationsenum3)] length] / sizeof(TestOperationsMyEnum) == 3);
        pe = [[ro objectForKey:@(TestOperationsenum3)] bytes];
        test(pe[0] == TestOperationsenum1);
        test(pe[1] == TestOperationsenum1);
        test(pe[2] == TestOperationsenum2);
        test([[ro objectForKey:@(TestOperationsenum2)] length] / sizeof(TestOperationsMyEnum) == 2);
        pe = [[ro objectForKey:@(TestOperationsenum2)] bytes];
        test(pe[0] == TestOperationsenum1);
        test(pe[1] == TestOperationsenum2);
        test([[ro objectForKey:@(TestOperationsenum1)] length] / sizeof(TestOperationsMyEnum) == 2);
        pe = [[ro objectForKey:@(TestOperationsenum1)] bytes];
        test(pe[0] == TestOperationsenum3);
        test(pe[1] == TestOperationsenum3);
    }

    {
        const NSUInteger lengths[] = { 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 };

        NSUInteger l;
        for(l = 0; l != sizeof(lengths) / sizeof(*lengths); ++l)
        {
            TestOperationsMutableIntS *s = [TestOperationsMutableIntS dataWithLength:(lengths[l] * sizeof(ICEInt))];
            ICEInt *ip = (ICEInt *)[s bytes];
            NSUInteger i;
            for(i = 0; i < lengths[l]; ++i)
            {
                *ip++ = (ICEInt)i;
            }
            TestOperationsIntS *r = [p opIntS:s];
            test([r length] == lengths[l] * sizeof(ICEInt));
            const ICEInt *rp = [r bytes];
            for(int j = 0; j < (int)([r length] / sizeof(ICEInt)); ++j)
            {
                test(rp[j] == -j);
            }
        }
    }

    {
        ICEMutableContext *ctx = [ICEMutableContext dictionary];
        [ctx setObject:@"ONE" forKey:@"one"];
        [ctx setObject:@"TWO" forKey:@"two"];
        [ctx setObject:@"THREE" forKey:@"three"];
        {
            ICEContext *r = [p opContext];
            test([[p ice_getContext] count] == 0);
            test(![r isEqual:ctx]);
        }
        {
            ICEContext *r = [p opContext:ctx];
            test([[p ice_getContext] count] == 0);
            test([r isEqual:ctx]);
        }
        {
            id<TestOperationsMyClassPrx> p2 = [TestOperationsMyClassPrx checkedCast:[p ice_context:ctx]];
            test([[p2 ice_getContext] isEqual:ctx]);
            ICEContext *r = [p2 opContext];
            test([r isEqual:ctx]);
            r = [p2 opContext:ctx];
            test([r isEqual:ctx]);
        }
    }

    if([p ice_getConnection])
    {
        //
        // TestOperations implicit context propagation
        //
        NSString *impls[] = { @"Shared", @"PerThread" };
        int i;
        for(i = 0; i < 2; i++)
        {
            ICEInitializationData *initData = [ICEInitializationData initializationData];
            initData.properties = [[communicator getProperties] clone];
            [initData.properties setProperty:@"Ice.ImplicitContext" value:impls[i]];

            id<ICECommunicator> ic = [ICEUtil createCommunicator:initData];

            ICEMutableContext *ctx = [ICEMutableContext dictionary];
            [ctx setObject:@"ONE" forKey:@"one" ];
            [ctx setObject:@"TWO" forKey:@"two" ];
            [ctx setObject:@"THREE" forKey:@"three"];

            id<TestOperationsMyClassPrx> pc = [TestOperationsMyClassPrx uncheckedCast:[ic stringToProxy:@"test:default -p 12010"]];

            [[ic getImplicitContext] setContext:ctx];
            test([[[ic getImplicitContext] getContext] isEqual:ctx]);
            test([[pc opContext] isEqual:ctx]);

            test([[ic getImplicitContext] get:@"zero"] == nil);
            [[ic getImplicitContext] put:@"zero" value:@"ZERO"];
            test([[[ic getImplicitContext] get:@"zero"] isEqualToString:@"ZERO"]);

            ctx = [[ic getImplicitContext] getContext];
            test([[pc opContext] isEqual:ctx]);

            ICEMutableContext *prxContext = [ICEMutableContext dictionary];
            [prxContext setObject:@"UN" forKey:@"one"];
            [prxContext setObject:@"QUATRE" forKey:@"four"];

            ICEMutableContext *combined = [ICEMutableContext dictionaryWithDictionary:ctx];
            [combined addEntriesFromDictionary:prxContext];

            pc = [TestOperationsMyClassPrx uncheckedCast:[pc ice_context:prxContext]];

            [[ic getImplicitContext] setContext:[ICEContext dictionary]];
            test([[pc opContext] isEqualToDictionary:prxContext]);

            [[ic getImplicitContext] setContext:ctx];
            test([[pc opContext] isEqualToDictionary:combined]);

            test([[[ic getImplicitContext] get:@"one"] isEqualToString:@"ONE"]);
            [[ic getImplicitContext] remove:@"one"];

            if([impls[i] isEqualToString:@"PerThread"])
            {
                PerThreadContextInvokeThread* thread = [PerThreadContextInvokeThread create:[pc ice_context:[ICEMutableContext dictionary]]];
                [thread start];
                [thread join];
            }

            [[ic getImplicitContext] setContext:[ICEContext dictionary]];
            [ic destroy];
        }
    }

    {
        ICEDouble d = 1278312346.0 / 13.0;
        TestOperationsMutableDoubleS *ds = [TestOperationsMutableDoubleS dataWithLength:(5 * sizeof(ICEDouble))];
        ICEDouble *pb = (ICEDouble *)[ds bytes];
        int i = 5;
        while(i-- > 0)
        {
            *pb++ = d;
        }
        [p opDoubleMarshaling:d p2:ds];
    }

    [p opIdempotent];

    [p opNonmutating];

    test([p opByte1:0xFF] == 0xFF);
    test([p opShort1:0x7FFF] == 0x7FFF);
    test([p opInt1:0x7FFFFFFF] == 0x7FFFFFFF);
    test([p opLong1:0x7FFFFFFFFFFFFFFF] == 0x7FFFFFFFFFFFFFFF);
    test([p opFloat1:1.0] == 1.0);
    test([p opDouble1:1.0] == 1.0);
    test([[p opString1:@"opString1"] isEqualToString:@"opString1"]);

    id<TestOperationsMyDerivedClassPrx> derived = [TestOperationsMyDerivedClassPrx uncheckedCast:p];

    {
        TestOperationsMyStruct1* s =
            [TestOperationsMyStruct1 myStruct1:@"Test::MyStruct1::s" myClass:nil myStruct1:@"Test::MyStruct1::myStruct1"];
        s = [derived opMyStruct1:s];
        test([s.tesT isEqualToString:@"Test::MyStruct1::s"]);
        test(s.myClass == 0);
        test([s.myStruct1 isEqualToString:@"Test::MyStruct1::myStruct1"]);

        TestOperationsMyClass1* c =
            [TestOperationsMyClass1 myClass1:@"Test::MyClass1::testT" myClass:nil myClass1:@"Test::MyClass1::myClass1"];
        c = [derived opMyClass1:c];
        test([c.tesT isEqualToString:@"Test::MyClass1::testT"]);
        test(c.myClass == nil);
        test([c.myClass1 isEqualToString:@"Test::MyClass1::myClass1"]);
    }

    [p opStringS1:[TestOperationsStringS array]];
    [p opByteBoolD1:[TestOperationsByteBoolD dictionary]];

    {
        TestOperationsStructure* p1 = [p opMStruct1];
        p1.e = TestOperationsenum3;
        TestOperationsStructure* p2,* p3;
        p3 = [p opMStruct2:p1 p2:&p2];
        test([p2 isEqual:p1] && [p3 isEqual:p1]);
    }

    {
        [p opMSeq1];

        TestOperationsMutableStringS* p1 = [TestOperationsMutableStringS arrayWithCapacity:1];
        [p1 addObject:@"test"];
        TestOperationsMutableStringS* p2,* p3;
        p3 = [p opMSeq2:p1 p2:&p2];
        test([[p2 objectAtIndex:0] isEqualToString:@"test"] && [[p3 objectAtIndex:0] isEqualToString:@"test"]);
    }

    {
        [p opMDict1];

        TestOperationsMutableStringStringD* p1 = [TestOperationsMutableStringStringD dictionary];
        [p1 setObject:@"test" forKey:@"test"];
        TestOperationsMutableStringStringD* p2,* p3;
        p3 = [p opMDict2:p1 p2:&p2];
        test([[p2 objectForKey:@"test"] isEqualToString:@"test"] && [[p3 objectForKey:@"test"] isEqualToString:@"test"]);
    }

    //
    // TestOperationss below are for Objective-C only. They test that we do the right thing if NSNull
    // is passed as part of the sequence or dictionary.
    //
    @try
    {
        {
            TestOperationsStringS *s = [p getNSNullStringSeq];
            test([s count] == 2);
            test([[s objectAtIndex:0] isEqualToString:@"first"]);
            test([[s objectAtIndex:1] isEqualToString:@""]);
        }

        {
            TestOperationsAS *s = [p getNSNullASeq];
            test([s count] == 2);
            test(((TestOperationsA *)[s objectAtIndex:0]).i == 99);
            test([s objectAtIndex:1] == [NSNull null]);
        }

        {
            TestOperationsStructS *seq = [p getNSNullStructSeq];
            test([seq count] == 2);
            TestOperationsStructure *s = [seq objectAtIndex:0];
            test(s.p == nil);
            test(s.e == TestOperationsenum2);
            test([s.s.s isEqualToString:@"Hello"]);
            s = [seq objectAtIndex:1];
            test(s.p == nil);
            test(s.e == TestOperationsenum1);
            test([s.s.s isEqualToString:@""]);
        }

        {
            TestOperationsStringSS *seq = [p getNSNullStringSeqSeq];
            test([seq count] == 2);
            TestOperationsStringS *s = [seq objectAtIndex:0];
            test([s count] == 1);
            test([(NSString *)[s objectAtIndex:0] isEqualToString:@"first"]);
            s = [seq objectAtIndex:1];
            test([s count] == 0);
        }

        {
            TestOperationsStringStringD *d = [p getNSNullStringStringDict];
            test([d count] == 2);
            test([(NSString *)[d objectForKey:@"one"] isEqualToString:@"ONE"]);
            test([(NSString *)[d objectForKey:@"two"] isEqualToString:@""]);
        }

        {
            @try
            {
                TestOperationsMutableStringStringD *d = [TestOperationsMutableStringStringD dictionary];
                [d setObject:@"bad key" forKey:[NSNull null]];
                [p putNSNullStringStringDict:d];
                test(NO);
            }
            @catch(ICEMarshalException *)
            {
                // Expected
            }
        }

        {
            @try
            {
                TestOperationsMutableShortIntD *d = [TestOperationsMutableShortIntD dictionary];
                [d setObject:[NSNull null] forKey:[NSNumber numberWithInt:1]];
                [p putNSNullShortIntDict:d];
                test(NO);
            }
            @catch(ICEMarshalException *)
            {
                // Expected
            }
        }

        {
            @try
            {
                TestOperationsMutableStringMyEnumD *d = [TestOperationsMutableStringMyEnumD dictionary];
                [d setObject:[NSNull null] forKey:@"key"];
                [p putNSNullStringMyEnumDict:d];
                test(NO);
            }
            @catch(ICEMarshalException *)
            {
                // Expected
            }
        }
    }
    @catch(ICEOperationNotExistException *)
    {
       // Client is talking to non-Objective-C server.
    }

    {
        [bprx opB];
        [bprx opIntf];
    }
}
