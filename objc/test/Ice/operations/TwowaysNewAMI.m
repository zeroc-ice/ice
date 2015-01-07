// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OperationsTest.h>
#import <limits.h>
#import <float.h>

#import <Foundation/Foundation.h>

@interface TestNewAMIOperationsCallback : NSObject
{
    BOOL called;
    NSCondition* cond;
}
-(BOOL) check;
-(void) called;
@end

@implementation TestNewAMIOperationsCallback
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cond = [[NSCondition alloc] init];
    return self;
}

+(id) create
{
#if defined(__clang__) && __has_feature(objc_arc)
    return [[TestNewAMIOperationsCallback alloc] init];
#else
    return [[[TestNewAMIOperationsCallback alloc] init] autorelease];
#endif
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [cond release];
    [super dealloc];
}
#endif

-(BOOL) check
{
    [cond lock];
    while(!called)
    {
        if(![cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:50]])
        {
            return NO;
        }
    }
    called = NO;
    [cond unlock];
    return YES;
}
-(void) called
{
    [cond lock];
    called = YES;
    [cond signal];
    [cond unlock];
}
-(void) opVoidExResponse
{
    test(NO);
}
-(void) opVoidExException:(ICEException*)ex
{
    test([ex isKindOfClass:[ICENoEndpointException class]]);
    [self called];
}
-(void) opVoidResponse
{
    [self called];
}
-(void) opVoidException:(ICEException*)ex
{
    test(NO);
}
-(void) opByteExResponse:(ICEByte)ret p3:(ICEByte)p3
{
    test(NO);
}
-(void) opByteExException:(ICEException*)ex
{
    test([ex isKindOfClass:[ICENoEndpointException class]]);
    [self called];
}
-(void) opByteResponse:(ICEByte)ret p3:(ICEByte)p3
{
    [self called];
}
-(void) opByteException:(ICEException*)ex
{
    test(NO);
}
-(void) opBoolResponse:(BOOL)r p3:(BOOL)b
{
    test(b);
    test(!r);
    [self called];
}
-(void) opBoolException:(ICEException*)ex
{
    test(NO);
}
-(void) opShortIntLongResponse:(ICELong)r p4:(ICEShort)s p5:(ICEInt)i p6:(ICELong)l
{
    test(s == 10);
    test(i == 11);
    test(l == 12);
    test(r == 12);
    [self called];
}
-(void) opShortIntLongException:(ICEException*)ex
{
    test(NO);
}
-(void) opFloatDoubleResponse:(ICEDouble)r p3:(ICEFloat)f p4:(ICEDouble)d
{
    test(f == 3.14f);
    test(d == 1.1E10);
    test(r == 1.1E10);
    [self called];
}
-(void) opFloatDoubleException:(ICEException*)ex
{
    test(NO);
}
-(void) opStringResponse:(NSString*)r p3:(NSString*)s
{
    test([s isEqualToString:@"world hello"]);
    test([r isEqualToString:@"hello world"]);
    [self called];
}

-(void) opStringException:(ICEException*)ex
{
    test(NO);
};

-(void) opMyEnumResponse:(TestOperationsMyEnum)r p2:(TestOperationsMyEnum)e
{
    test(e == TestOperationsenum2);
    test(r == TestOperationsenum3);
    [self called];
}

-(void) opMyEnumException:(ICEException*)ex
{
    test(NO);
}

-(void) opMyClassResponse:(id<TestOperationsMyClassPrx>)r p2:(id<TestOperationsMyClassPrx>)c1 p3:(id<TestOperationsMyClassPrx>)c2
{
    test([[c1 ice_getIdentity] isEqual:[[c1 ice_getCommunicator] stringToIdentity:@"test"]]);
    test([[c2 ice_getIdentity] isEqual:[[c1 ice_getCommunicator] stringToIdentity:@"noSuchIdentity"]]);
    test([[r ice_getIdentity] isEqual:[[c1 ice_getCommunicator] stringToIdentity:@"test"]]);
    // We can't do the callbacks below in connection serialization mode.
    if([[[c1 ice_getCommunicator] getProperties] getPropertyAsInt:@"Ice.ThreadPool.Client.Serialize"])
    {
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
    }
    [self called];
}

-(void) opMyClassException:(ICEException*)ex
{
    test(NO);
}

-(void) opStructResponse:(TestOperationsStructure*)rso p3:(TestOperationsStructure*)so
{
    test(rso.p == nil);
    test(rso.e == TestOperationsenum2);
    test([rso.s.s isEqualToString:@"def"]);
    test([so e] == TestOperationsenum3);
    test(so.p != nil);
    test([so.s.s isEqualToString:@"a new string"]);
    // We can't do the callbacks below in connection serialization mode.
    if([[[so.p ice_getCommunicator] getProperties] getPropertyAsInt:@"Ice.ThreadPool.Client.Serialize"])
    {
        [so.p opVoid];
    }
    [self called];
}

-(void) opStructException:(ICEException*)ex
{
    test(NO);
}

-(void) opByteSResponse:(TestOperationsByteS*)rso p3:(TestOperationsByteS*)bso
{
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
    [self called];
}

-(void) opByteSException:(ICEException*)ex
{
    test(NO);
}

-(void) opBoolSResponse:(TestOperationsBoolS*)rso p3:(TestOperationsBoolS*)bso
{
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
    [self called];
}

-(void) opBoolSException:(ICEException*)ex
{
    test(NO);
}

-(void) opShortIntLongSResponse:(TestOperationsLongS*)rso p4:(TestOperationsShortS*)sso p5:(TestOperationsIntS*)iso p6:(TestOperationsLongS*)lso
{
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
    [self called];
}

-(void) opShortIntLongSException:(ICEException*)ex
{
    test(NO);
}

-(void) opFloatDoubleSResponse:(TestOperationsDoubleS*)rso p3:(TestOperationsFloatS*)fso p4:(TestOperationsDoubleS*)dso
{
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
    [self called];
}

-(void) opFloatDoubleSException:(ICEException*)ex
{
    test(NO);
}

-(void) opStringSResponse:(TestOperationsStringS*)rso p3:(TestOperationsStringS*)sso
{
    test([sso count] == 4);
    test([[sso objectAtIndex:0] isEqualToString:@"abc"]);
    test([[sso objectAtIndex:1] isEqualToString:@"de"]);
    test([[sso objectAtIndex:2] isEqualToString:@"fghi"]);
    test([[sso objectAtIndex:3] isEqualToString:@"xyz"]);
    test([rso count] == 3);
    test([[rso objectAtIndex:0] isEqualToString:@"fghi"]);
    test([[rso objectAtIndex:1] isEqualToString:@"de"]);
    test([[rso objectAtIndex:2] isEqualToString:@"abc"]);
    [self called];
}

-(void) opStringSException:(ICEException*)ex
{
    test(NO);
}

-(void) opByteSSResponse:(TestOperationsByteSS*)rso p3:(TestOperationsByteSS*)bso
{
    const ICEByte *p;
    test([bso count] == 2);
    test([[bso objectAtIndex:0] length] / sizeof(ICEByte) == 1);
    p = [[bso objectAtIndex:0] bytes];
    test(p[0] == (ICEByte)0x0ff);
    test([[bso objectAtIndex:1] length] / sizeof(ICEByte) == 3);
    p = [[bso objectAtIndex:1] bytes];
    test(p[0] == (ICEByte)0x01);
    test(p[1] == (ICEByte)0x11);
    test(p[2] == (ICEByte)0x12);
    test([rso count] == 4);
    test([[rso objectAtIndex:0] length] / sizeof(ICEByte) == 3);
    p = [[rso objectAtIndex:0] bytes];
    test(p[0] == (ICEByte)0x01);
    test(p[1] == (ICEByte)0x11);
    test(p[2] == (ICEByte)0x12);
    test([[rso objectAtIndex:1] length] / sizeof(ICEByte) == 1);
    p = [[rso objectAtIndex:1] bytes];
    test(p[0] == (ICEByte)0xff);
    test([[rso objectAtIndex:2] length] / sizeof(ICEByte) == 1);
    p = [[rso objectAtIndex:2] bytes];
    test(p[0] == (ICEByte)0x0e);
    test([[rso objectAtIndex:3] length] / sizeof(ICEByte) == 2);
    p = [[rso objectAtIndex:3] bytes];
    test(p[0] == (ICEByte)0xf2);
    test(p[1] == (ICEByte)0xf1);
    [self called];
}

-(void) opByteSSException:(ICEException*)ex
{
    test(NO);
}

-(void) opBoolSSResponse:(TestOperationsBoolSS*)sso p3:(TestOperationsBoolSS*)bso
{
    [self called];
}

-(void) opBoolSSException:(ICEException*)ex
{
    test(NO);
}

-(void) opShortIntLongSSResponse:(TestOperationsLongSS*)a p4:(TestOperationsShortSS*)p4 p5:(TestOperationsIntSS*)p5 p6:(TestOperationsLongSS*)p6
{
    [self called];
}

-(void) opShortIntLongSSException:(ICEException*)ex
{
    test(NO);
}

-(void) opFloatDoubleSSResponse:(TestOperationsDoubleSS*)rso p3:(TestOperationsFloatSS*)fso p4:(TestOperationsDoubleSS*)dso
{
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
    [self called];
}

-(void) opFloatDoubleSSException:(ICEException*)ex
{
    test(NO);
}

-(void) opStringSSResponse:(TestOperationsStringSS*)rso p3:(TestOperationsStringSS*)sso
{
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
    [self called];
}

-(void) opStringSSException:(ICEException*)ex
{
    test(NO);
}

-(void) opStringSSSResponse:(TestOperationsStringSS*)rsso p3:(TestOperationsStringSS*)ssso
{
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
    [self called];
}
-(void) opStringSSSException:(ICEException*)ex
{
    test(NO);
}
-(void) opByteBoolDResponse:(TestOperationsMutableByteBoolD*)ro p3:(TestOperationsMutableByteBoolD*)_do
{
    test([_do count] == 2);
    test([[_do objectForKey:[NSNumber numberWithUnsignedChar:10]] boolValue] == YES);
    test([[_do objectForKey:[NSNumber numberWithUnsignedChar:100]] boolValue] == NO);
    test([ro count] == 4);
    test([[ro objectForKey:[NSNumber numberWithUnsignedChar:10]] boolValue] == YES);
    test([[ro objectForKey:[NSNumber numberWithUnsignedChar:11]] boolValue] == NO);
    test([[ro objectForKey:[NSNumber numberWithUnsignedChar:100]] boolValue] == NO);
    test([[ro objectForKey:[NSNumber numberWithUnsignedChar:101]] boolValue] == YES);
    [self called];
}
-(void) opByteBoolDException:(ICEException*)ex
{
    test(NO);
}
-(void) opShortIntDResponse:(TestOperationsShortIntD*)ro p3:(TestOperationsShortIntD*)_do
{
    test([_do count] == 2);
    test([[_do objectForKey:[NSNumber numberWithShort:110]] intValue] == -1);
    test([[_do objectForKey:[NSNumber numberWithShort:1100]] intValue] == 123123);
    test([ro count] == 4);
    test([[ro objectForKey:[NSNumber numberWithShort:110]] intValue] == -1);
    test([[ro objectForKey:[NSNumber numberWithShort:111]] intValue] == -100);
    test([[ro objectForKey:[NSNumber numberWithShort:1100]] intValue] == 123123);
    test([[ro objectForKey:[NSNumber numberWithShort:1101]] intValue] == 0);
    [self called];
}
-(void) opShortIntDException:(ICEException*)ex
{
    test(NO);
}
-(void) opLongFloatDResponse:(TestOperationsLongFloatD*)ro p3:(TestOperationsLongFloatD*)_do
{
    test([_do count] == 2);
    test((ICEFloat)[[_do objectForKey:[NSNumber numberWithLong:999999110]] floatValue] == -1.1f);
    test((ICEFloat)[[_do objectForKey:[NSNumber numberWithLong:999999111]] floatValue] == 123123.2f);
    test([ro count] == 4);
    test((ICEFloat)[[ro objectForKey:[NSNumber numberWithLong:999999110]] floatValue] == -1.1f);
    test((ICEFloat)[[ro objectForKey:[NSNumber numberWithLong:999999120]] floatValue] == -100.4f);
    test((ICEFloat)[[ro objectForKey:[NSNumber numberWithLong:999999111]] floatValue] == 123123.2f);
    test((ICEFloat)[[ro objectForKey:[NSNumber numberWithLong:999999130]] floatValue] == 0.5f);
    [self called];
}
-(void) opLongFloatDException:(ICEException*)ex
{
    test(NO);
}
-(void) opStringStringDResponse:(TestOperationsStringStringD*)ro p3:(TestOperationsStringStringD*)_do
{
    test([_do count] == 2);
    test([[_do objectForKey:@"foo"] isEqualToString:@"abc -1.1"]);
    test([[_do objectForKey:@"bar"] isEqualToString:@"abc 123123.2"]);
    test([ro count] == 4);
    test([[ro objectForKey:@"foo"] isEqualToString:@"abc -1.1"]);
    test([[ro objectForKey:@"FOO"] isEqualToString:@"abc -100.4"]);
    test([[ro objectForKey:@"bar"] isEqualToString:@"abc 123123.2"]);
    test([[ro objectForKey:@"BAR"] isEqualToString:@"abc 0.5"]);
    [self called];
}
-(void) opStringStringDException:(ICEException*)ex
{
    test(NO);
}
-(void) opStringMyEnumDResponse:(TestOperationsStringMyEnumD*)ro p3:(TestOperationsStringMyEnumD*)_do
{
    test([_do count] == 2);
    test([[_do objectForKey:@"abc"] intValue] == TestOperationsenum1);
    test([[_do objectForKey:@""] intValue] == TestOperationsenum2);
    test([ro count] == 4);
    test([[ro objectForKey:@"abc"] intValue] == TestOperationsenum1);
    test([[ro objectForKey:@"querty"] intValue] == TestOperationsenum3);
    test([[ro objectForKey:@""] intValue] == TestOperationsenum2);
    test([[ro objectForKey:@"Hello!!"] intValue] == TestOperationsenum2);
    [self called];
}
-(void) opStringMyEnumDException:(ICEException*)ex
{
    test(NO);
}
-(void) opMyEnumStringDResponse:(TestOperationsMyEnumStringD*)ro p3:(TestOperationsMyEnumStringD*)_do
{
    test([_do count] == 1);
    test([ro count] == 3);
    test([[ro objectForKey:@(TestOperationsenum1)] isEqualToString:@"abc"]);
    test([[ro objectForKey:@(TestOperationsenum2)] isEqualToString:@"Hello!!"]);
    test([[ro objectForKey:@(TestOperationsenum3)] isEqualToString:@"querty"]);
    [self called];
}
-(void) opMyEnumStringDException:(ICEException*)ex
{
    test(NO);
}
-(void) opMyStructMyEnumDResponse:(TestOperationsMyStructMyEnumD*)ro p3:(TestOperationsMyStructMyEnumD*)_do
                              s11:(TestOperationsMyStruct*)s11 s12:(TestOperationsMyStruct*)s12
                              s22:(TestOperationsMyStruct*)s22 s23:(TestOperationsMyStruct*)s23
{
    test([_do count] == 2);
    test([ro count] == 4);
    test([[ro objectForKey:s11] isEqual:@(TestOperationsenum1)]);
    test([[ro objectForKey:s12] isEqual:@(TestOperationsenum2)]);
    test([[ro objectForKey:s22] isEqual:@(TestOperationsenum3)]);
    test([[ro objectForKey:s23] isEqual:@(TestOperationsenum2)]);
    [self called];
}
-(void) opMyStructMyEnumDException:(ICEException*)ex
{
    test(NO);
}
-(void) opIntSResponse:(TestOperationsIntS*)r
{
    const ICEInt *rp = [r bytes];
    int j;
    for(j = 0; j < [r length] / sizeof(ICEInt); ++j)
    {
        test(rp[j] == -j);
    }
    [self called];
}
-(void) opIntSException:(ICEException*)ex
{
    test(NO);
}
-(void) opEmptyContextResponse:(ICEContext*)ctx
{
    test([ctx count] == 0);
    [self called];
}
-(void) opNonEmptyContextResponse:(ICEContext*)ctx
{
    test([ctx count] == 3);
    test([[ctx objectForKey:@"one"] isEqualToString:@"ONE"]);
    test([[ctx objectForKey:@"two"] isEqualToString:@"TWO"]);
    test([[ctx objectForKey:@"three"] isEqualToString:@"THREE"]);
    [self called];
}
-(void) opContextException:(ICEException*)ex
{
    test(NO);
}
-(void) opDoubleMarshalingResponse
{
    [self called];
}
-(void) opDoubleMarshalingException:(ICEException*)ex
{
    test(NO);
}
@end

void
twowaysNewAMI(id<ICECommunicator> communicator, id<TestOperationsMyClassPrx> p)
{
    {
        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opByte:(ICEByte)0xff p2:(ICEByte)0x0f response:^(ICEByte ret, ICEByte p3) { [cb opByteResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opByteException:ex]; }];
        test([cb check]);
    }

    {
        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opBool:YES p2:NO response:^(BOOL ret, BOOL p3) { [cb opBoolResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opBoolException:ex]; }];
        test([cb check]);
    }

    {
        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opShortIntLong:10 p2:11 p3:12 response:^(ICELong ret, ICEShort p4, ICEInt p5, ICELong p6) { [cb opShortIntLongResponse:ret p4:p4 p5:p5 p6:p6]; } exception:^(ICEException* ex) { [cb opShortIntLongException:ex]; }];
        test([cb check]);
    }

    {
        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opFloatDouble:3.14f p2:1.1E10 response:^(ICEDouble ret, ICEFloat p3, ICEDouble p4) { [cb opFloatDoubleResponse:ret p3:p3 p4:p4]; } exception:^(ICEException* ex) { [cb opFloatDoubleException:ex]; }];
        test([cb check]);
    }

    {
        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opString:@"hello" p2:@"world" response:^(NSMutableString* ret, NSMutableString* p3) { [cb opStringResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStringException:ex]; }];
        test([cb check]);
    }

    {
        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opMyEnum:TestOperationsenum2 response:^(TestOperationsMyEnum ret, TestOperationsMyEnum p2) { [cb opMyEnumResponse:ret p2:p2]; } exception:^(ICEException* ex) { [cb opMyEnumException:ex]; }];
        test([cb check]);
    }
    
    {
        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opMyClass:p response:^(id<TestOperationsMyClassPrx> ret, id<TestOperationsMyClassPrx>p2, id<TestOperationsMyClassPrx> p3) { [cb opMyClassResponse:ret p2:p2 p3:p3]; } exception:^(ICEException* ex) { [cb opMyClassException:ex]; }];
        test([cb check]);
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

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opStruct:si1 p2:si2 response:^(TestOperationsStructure* ret, TestOperationsStructure* p3) { [cb opStructResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStructException:ex]; }];
        test([cb check]);
    }

    {
        ICEByte buf1[] = { 0x01, 0x11, 0x12, 0x22 };
        ICEByte buf2[] = { 0xf1, 0xf2, 0xf3, 0xf4 };
        
        TestOperationsMutableByteS *bsi1 = [TestOperationsMutableByteS data];
        TestOperationsMutableByteS *bsi2 = [TestOperationsMutableByteS data];

        [bsi1 appendBytes:buf1 length:sizeof(buf1)];
        [bsi2 appendBytes:buf2 length:sizeof(buf2)];

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opByteS:bsi1 p2:bsi2 response:^(TestOperationsMutableByteS* ret, TestOperationsMutableByteS* p3) { [cb opByteSResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opByteSException:ex]; }];
        test([cb check]);
    }

    {
        BOOL buf1[] = { YES, YES, NO };
        BOOL buf2[] = { NO };
        
        TestOperationsMutableBoolS *bsi1 = [TestOperationsMutableBoolS data];
        TestOperationsMutableBoolS *bsi2 = [TestOperationsMutableBoolS data];
        
        [bsi1 appendBytes:buf1 length:sizeof(buf1)];
        [bsi2 appendBytes:buf2 length:sizeof(buf2)];

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opBoolS:bsi1 p2:bsi2 response:^(TestOperationsMutableBoolS* ret, TestOperationsMutableBoolS* p3) { [cb opBoolSResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opBoolSException:ex]; }];
        test([cb check]);
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

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opShortIntLongS:ssi p2:isi p3:lsi response:^(TestOperationsMutableLongS* ret, TestOperationsMutableShortS* p4, TestOperationsMutableIntS* p5, TestOperationsMutableLongS* p6) { [cb opShortIntLongSResponse:ret p4:p4 p5:p5 p6:p6]; } exception:^(ICEException* ex) { [cb opShortIntLongSException:ex]; }];
        test([cb check]);
    }

    {
        ICEFloat buf1[] = { 3.14f, 1.11f };
        ICEDouble buf2[] = { 1.1E10, 1.2E10, 1.3E10 };

        TestOperationsMutableFloatS *fsi = [TestOperationsMutableFloatS data];
        TestOperationsMutableDoubleS *dsi = [TestOperationsMutableDoubleS data];

        [fsi appendBytes:buf1 length:sizeof(buf1)];
        [dsi appendBytes:buf2 length:sizeof(buf2)];

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opFloatDoubleS:fsi p2:dsi response:^(TestOperationsMutableDoubleS* ret, TestOperationsMutableFloatS* p3, TestOperationsMutableDoubleS* p4) { [cb opFloatDoubleSResponse:ret p3:p3 p4:p4]; } exception:^(ICEException* ex) { [cb opFloatDoubleSException:ex]; }];
        test([cb check]);
    }

    {
        TestOperationsMutableStringS *ssi1 = [TestOperationsMutableStringS arrayWithCapacity:3];
        TestOperationsMutableStringS *ssi2 = [TestOperationsMutableStringS arrayWithCapacity:1];

        [ssi1 addObject:@"abc"];
        [ssi1 addObject:@"de"];
        [ssi1 addObject:@"fghi"];

        [ssi2 addObject:@"xyz"];


        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opStringS:ssi1 p2:ssi2 response:^(TestOperationsMutableStringS* ret, TestOperationsMutableStringS* p3) { [cb opStringSResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStringSException:ex]; }];
        test([cb check]);
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

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opByteSS:bsi1 p2:bsi2 response:^(TestOperationsMutableByteSS* ret, TestOperationsMutableByteSS* p3) { [cb opByteSSResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opByteSSException:ex]; }];
        test([cb check]);
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

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opFloatDoubleSS:fsi p2:dsi response:^(TestOperationsMutableDoubleSS* ret, TestOperationsMutableFloatSS* p3, TestOperationsMutableDoubleSS* p4) { [cb opFloatDoubleSSResponse:ret p3:p3 p4:p4]; } exception:^(ICEException* ex) { [cb opFloatDoubleSSException:ex]; }];
        test([cb check]);
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

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opStringSS:ssi1 p2:ssi2 response:^(TestOperationsMutableStringSS* ret, TestOperationsMutableStringSS* p3) { [cb opStringSSResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStringSSException:ex]; }];
        test([cb check]);
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

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opStringSSS:sssi1 p2:sssi2 response:^(TestOperationsMutableStringSS* ret, TestOperationsMutableStringSS* p3) { [cb opStringSSSResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStringSSSException:ex]; }];
        test([cb check]);
    }

    {
        TestOperationsMutableByteBoolD *di1 = [TestOperationsMutableByteBoolD dictionary];
        [di1 setObject:[NSNumber numberWithBool:YES] forKey:[NSNumber numberWithUnsignedChar:10]];
        [di1 setObject:[NSNumber numberWithBool:NO] forKey:[NSNumber numberWithUnsignedChar:100]];
            TestOperationsMutableByteBoolD *di2 = [TestOperationsMutableByteBoolD dictionary];
        [di2 setObject:[NSNumber numberWithBool:YES] forKey:[NSNumber numberWithUnsignedChar:10]];
        [di2 setObject:[NSNumber numberWithBool:NO] forKey:[NSNumber numberWithUnsignedChar:11]];
        [di2 setObject:[NSNumber numberWithBool:TRUE] forKey:[NSNumber numberWithUnsignedChar:101]];

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opByteBoolD:di1 p2:di2 response:^(TestOperationsMutableByteBoolD* ret, TestOperationsMutableByteBoolD* p3) { [cb opByteBoolDResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opByteBoolDException:ex]; }];
        test([cb check]);
    }

    {
        TestOperationsMutableShortIntD *di1 = [TestOperationsMutableShortIntD dictionary];
        [di1 setObject:[NSNumber numberWithInt:-1] forKey:[NSNumber numberWithShort:110]];
        [di1 setObject:[NSNumber numberWithInt:123123] forKey:[NSNumber numberWithShort:1100]];
            TestOperationsMutableShortIntD *di2 = [TestOperationsMutableShortIntD dictionary];
        [di2 setObject:[NSNumber numberWithInt:-1] forKey:[NSNumber numberWithShort:110]];
        [di2 setObject:[NSNumber numberWithInt:-100] forKey:[NSNumber numberWithShort:111]];
        [di2 setObject:[NSNumber numberWithInt:0] forKey:[NSNumber numberWithShort:1101]];

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opShortIntD:di1 p2:di2 response:^(TestOperationsMutableShortIntD* ret, TestOperationsMutableShortIntD* p3) { [cb opShortIntDResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opShortIntDException:ex]; }];
        test([cb check]);
    }

    {
        TestOperationsMutableLongFloatD *di1 = [TestOperationsMutableLongFloatD dictionary];
        [di1 setObject:[NSNumber numberWithFloat:-1.1f] forKey:[NSNumber numberWithLong:999999110]];
        [di1 setObject:[NSNumber numberWithFloat:123123.2f] forKey:[NSNumber numberWithLong:999999111]];
            TestOperationsMutableLongFloatD *di2 = [TestOperationsMutableLongFloatD dictionary];
        [di2 setObject:[NSNumber numberWithFloat:-1.1f] forKey:[NSNumber numberWithLong:999999110]];
        [di2 setObject:[NSNumber numberWithFloat:-100.4f] forKey:[NSNumber numberWithLong:999999120]];
        [di2 setObject:[NSNumber numberWithFloat:0.5f] forKey:[NSNumber numberWithLong:999999130]];

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opLongFloatD:di1 p2:di2 response:^(TestOperationsMutableLongFloatD* ret, TestOperationsMutableLongFloatD* p3) { [cb opLongFloatDResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opLongFloatDException:ex]; }];
        test([cb check]);
    }

    {
        TestOperationsMutableStringStringD *di1 = [TestOperationsMutableStringStringD dictionary];
        [di1 setObject:@"abc -1.1" forKey:@"foo"];
        [di1 setObject:@"abc 123123.2" forKey:@"bar"];
            TestOperationsMutableStringStringD *di2 = [TestOperationsMutableStringStringD dictionary];
        [di2 setObject:@"abc -1.1" forKey:@"foo"];
        [di2 setObject:@"abc -100.4" forKey:@"FOO"];
        [di2 setObject:@"abc 0.5" forKey:@"BAR"];


        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opStringStringD:di1 p2:di2 response:^(TestOperationsMutableStringStringD* ret, TestOperationsMutableStringStringD* p3) { [cb opStringStringDResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStringStringDException:ex]; }];
        test([cb check]);
    }

    {
        TestOperationsMutableStringMyEnumD *di1 = [TestOperationsMutableStringMyEnumD dictionary];
        [di1 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:@"abc"];
        [di1 setObject:[NSNumber numberWithInt:TestOperationsenum2] forKey:@""];
            TestOperationsMutableStringMyEnumD *di2 = [TestOperationsMutableStringMyEnumD dictionary];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:@"abc"];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum3] forKey:@"querty"];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum2] forKey:@"Hello!!"];

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opStringMyEnumD:di1 p2:di2 response:^(TestOperationsMutableStringMyEnumD* ret, TestOperationsMutableStringMyEnumD* p3) { [cb opStringMyEnumDResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStringMyEnumDException:ex]; }];
        test([cb check]);
    }

    {
        TestOperationsMutableMyEnumStringD *di1 = [TestOperationsMutableMyEnumStringD dictionary];
	[di1 setObject:@"abc" forKey:@(TestOperationsenum1)];
        TestOperationsMutableMyEnumStringD *di2 = [TestOperationsMutableMyEnumStringD dictionary];
	[di2 setObject:@"Hello!!" forKey:@(TestOperationsenum2)];
	[di2 setObject:@"querty" forKey:@(TestOperationsenum3)];

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opMyEnumStringD:di1 
                              p2:di2 
                        response:^(TestOperationsMutableMyEnumStringD* ret, 
                                   TestOperationsMutableMyEnumStringD* p3) { [cb opMyEnumStringDResponse:ret 
                                                                                                      p3:p3]; }
                       exception:^(ICEException* ex) { [cb opMyEnumStringDException:ex]; }];
        [cb check];
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

        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opMyStructMyEnumD:di1 
                                p2:di2 
                          response:^(TestOperationsMutableMyStructMyEnumD* ret, 
                                     TestOperationsMutableMyStructMyEnumD* p3) { [cb opMyStructMyEnumDResponse:ret 
                                                                                                            p3:p3
                                                                                                           s11:s11
                                                                                                           s12:s12
                                                                                                           s22:s22
                                                                                                           s23:s23]; }
                         exception:^(ICEException* ex) { [cb opMyStructMyEnumDException:ex]; }];
        [cb check];
    }

    {
        const int lengths[] = { 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 };

        int l;
        for(l = 0; l != sizeof(lengths) / sizeof(*lengths); ++l)
        {
            TestOperationsMutableIntS *s = [TestOperationsMutableIntS dataWithLength:(lengths[l] * sizeof(ICEInt))];
            ICEInt *ip = (ICEInt *)[s bytes];
            int i;
            for(i = 0; i < lengths[l]; ++i)
            {
                *ip++ = i;
            }

            TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
            [p begin_opIntS:s response:^(TestOperationsMutableIntS* ret) { [cb opIntSResponse:ret]; } exception:^(ICEException* ex) { [cb opIntSException:ex]; }];
            test([cb check]);
        }
    }

    {
        ICEMutableContext *ctx = [ICEMutableContext dictionary];
        [ctx setObject:@"ONE" forKey:@"one"];
        [ctx setObject:@"TWO" forKey:@"two"];
        [ctx setObject:@"THREE" forKey:@"three"];
        {
                TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
                [p begin_opContext:^(ICEMutableContext* ctx) { [cb opEmptyContextResponse:ctx]; } exception:^(ICEException* ex) { [cb opContextException:ex]; }];
                test([cb check]);
        }
        {
            TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
            [p begin_opContext:ctx response:^(ICEMutableContext* ctx) { [cb opNonEmptyContextResponse:ctx]; } exception:^(ICEException* ex) { [cb opContextException:ex]; }];
            test([cb check]);
        }
        {
            id<TestOperationsMyClassPrx> p2 = [TestOperationsMyClassPrx checkedCast:[p ice_context:ctx]];
            test([[p2 ice_getContext] isEqual:ctx]);
            TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
            [p2 begin_opContext:^(ICEMutableContext* ctx) { [cb opNonEmptyContextResponse:ctx]; } exception:^(ICEException* ex) { [cb opContextException:ex]; }];
            test([cb check]);


            cb = [TestNewAMIOperationsCallback create];
            [p2 begin_opContext:ctx response:^(ICEMutableContext* ctx) { [cb opNonEmptyContextResponse:ctx]; } exception:^(ICEException* ex) { [cb opContextException:ex]; }];
            test([cb check]);
	}
    }

    {
        //
        // TestOperations implicit context propagation
        //
        
        NSString *impls[] = {@"Shared", @"PerThread"};
        for(int i = 0; i < 2; i++)
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            [initData setProperties:[[communicator getProperties] clone]];
            [initData.properties setProperty:@"Ice.ImplicitContext" value:impls[i]];
            
            id<ICECommunicator> ic = [ICEUtil createCommunicator:initData];

            ICEMutableContext *ctx = [ICEMutableContext dictionary];
            [ctx setObject:@"ONE" forKey:@"one" ];
            [ctx setObject:@"TWO" forKey:@"two" ];
            [ctx setObject:@"THREE" forKey:@"three"];

            id<TestOperationsMyClassPrx> p = [TestOperationsMyClassPrx uncheckedCast:
                                                                    [ic stringToProxy:@"test:default -p 12010"]];
            
            [[ic getImplicitContext] setContext:(ctx)];
            test([[[ic getImplicitContext] getContext] isEqualToDictionary:ctx]);
            {
                id<ICEAsyncResult> r = [p begin_opContext];
                ICEContext* c = [p end_opContext:r];
                test([c isEqualToDictionary:ctx]);
            }

            test([[ic getImplicitContext] get:@"zero"] == nil);
            [[ic getImplicitContext] put:@"zero" value:@"ZERO"];
            test([[[ic getImplicitContext] get:@"zero"] isEqualToString:@"ZERO"]);
      
            ctx = [[ic getImplicitContext] getContext];
            {
                id<ICEAsyncResult> r = [p begin_opContext];
                ICEContext* c = [p end_opContext:r];
                test([c isEqualToDictionary:ctx]);
            }
            
            ICEMutableContext *prxContext = [ICEMutableContext dictionary];
            [prxContext setObject:@"UN" forKey:@"one"];
            [prxContext setObject:@"QUATRE" forKey:@"four"];

            ICEMutableContext *combined = [ICEMutableContext dictionaryWithDictionary:ctx];
            [combined addEntriesFromDictionary:prxContext];
            
            p = [TestOperationsMyClassPrx uncheckedCast:[p ice_context:prxContext]];
            
            [[ic getImplicitContext] setContext:[ICEMutableContext dictionary]];
            {
                id<ICEAsyncResult> r = [p begin_opContext];
                ICEContext* c = [p end_opContext:r];
                test([c isEqualToDictionary:prxContext]);
            }

            [[ic getImplicitContext] setContext:ctx];
            {
                id<ICEAsyncResult> r = [p begin_opContext];
                ICEContext* c = [p end_opContext:r];
                test([c isEqualToDictionary:combined]);
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
        TestNewAMIOperationsCallback* cb = [TestNewAMIOperationsCallback create];
        [p begin_opDoubleMarshaling:d p2:ds response:^() { [cb opDoubleMarshalingResponse]; } exception:^(ICEException* ex) { [cb opDoubleMarshalingException:ex]; }];
        test([cb check]);
    }

    // Marshaling tests for NSNull are present only in synchronous test because testing asynchronously
    // would only test the same marshaling code that's been tested already.
}

