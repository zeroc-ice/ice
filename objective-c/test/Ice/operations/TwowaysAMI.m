// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OperationsTest.h>
#import <limits.h>
#import <float.h>

#import <Foundation/Foundation.h>

@interface TestAMIOperationsCallback : NSObject
{
    BOOL called;
    NSCondition* cond;
}
-(BOOL) check;
-(void) called;
@end

@implementation TestAMIOperationsCallback
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
    return ICE_AUTORELEASE([[TestAMIOperationsCallback alloc] init]);
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
-(void) opVoidException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opByteExResponse:(ICEByte)__unused ret p3:(ICEByte)__unused p3
{
    test(NO);
}
-(void) opByteExException:(ICEException*)__unused ex
{
    test([ex isKindOfClass:[ICENoEndpointException class]]);
    [self called];
}
-(void) opByteResponse:(ICEByte)__unused ret p3:(ICEByte)__unused p3
{
    [self called];
}
-(void) opByteException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opBoolResponse:(BOOL)r p3:(BOOL)b
{
    test(b);
    test(!r);
    [self called];
}
-(void) opBoolException:(ICEException*)__unused ex
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
-(void) opShortIntLongException:(ICEException*)__unused ex
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
-(void) opFloatDoubleException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opStringResponse:(NSString*)r p3:(NSString*)s
{
    test([s isEqualToString:@"world hello"]);
    test([r isEqualToString:@"hello world"]);
    [self called];
}

-(void) opStringException:(ICEException*)__unused ex
{
    test(NO);
};

-(void) opMyEnumResponse:(TestOperationsMyEnum)r p2:(TestOperationsMyEnum)e
{
    test(e == TestOperationsenum2);
    test(r == TestOperationsenum3);
    [self called];
}

-(void) opMyEnumException:(ICEException*)__unused ex
{
    test(NO);
}

-(void) opMyClassResponse:(id<TestOperationsMyClassPrx>)r p2:(id<TestOperationsMyClassPrx>)c1 p3:(id<TestOperationsMyClassPrx>)c2
{
    test([[c1 ice_getIdentity] isEqual:[ICEUtil stringToIdentity:@"test"]]);
    test([[c2 ice_getIdentity] isEqual:[ICEUtil stringToIdentity:@"noSuchIdentity"]]);
    test([[r ice_getIdentity] isEqual:[ICEUtil stringToIdentity:@"test"]]);
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

-(void) opMyClassException:(ICEException*)__unused ex
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

-(void) opStructException:(ICEException*)__unused ex
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

-(void) opByteSException:(ICEException*)__unused ex
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

-(void) opBoolSException:(ICEException*)__unused ex
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

-(void) opShortIntLongSException:(ICEException*)__unused ex
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

-(void) opFloatDoubleSException:(ICEException*)__unused ex
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

-(void) opStringSException:(ICEException*)__unused ex
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

-(void) opByteSSException:(ICEException*)__unused ex
{
    test(NO);
}

-(void) opBoolSSResponse:(TestOperationsBoolSS*)__unused sso p3:(TestOperationsBoolSS*)__unused bso
{
    [self called];
}

-(void) opBoolSSException:(ICEException*)__unused ex
{
    test(NO);
}

-(void) opShortIntLongSSResponse:(TestOperationsLongSS*)__unused a p4:(TestOperationsShortSS*)__unused p4 p5:(TestOperationsIntSS*)__unused p5 p6:(TestOperationsLongSS*)__unused p6
{
    [self called];
}

-(void) opShortIntLongSSException:(ICEException*)__unused ex
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

-(void) opFloatDoubleSSException:(ICEException*)__unused ex
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

-(void) opStringSSException:(ICEException*)__unused ex
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
-(void) opStringSSSException:(ICEException*)__unused ex
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
-(void) opByteBoolDException:(ICEException*)__unused ex
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
-(void) opShortIntDException:(ICEException*)__unused ex
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
-(void) opLongFloatDException:(ICEException*)__unused ex
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
-(void) opStringStringDException:(ICEException*)__unused ex
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
-(void) opStringMyEnumDException:(ICEException*)__unused ex
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
-(void) opMyEnumStringDException:(ICEException*)__unused ex
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
-(void) opMyStructMyEnumDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opByteBoolDSResponse:(TestOperationsByteBoolDS*)ro p3:(TestOperationsByteBoolDS*)_do
{
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
    [self called];
}
-(void) opByteBoolDSException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opShortIntDSResponse:(TestOperationsShortIntDS*)ro p3:(TestOperationsShortIntDS*)_do
{
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
    [self called];
}
-(void) opShortIntDSException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opLongFloatDSResponse:(TestOperationsLongFloatDS*)ro p3:(TestOperationsLongFloatDS*)_do
{
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
    [self called];
}
-(void) opLongFloatDSException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opStringStringDSResponse:(TestOperationsStringStringDS*)ro p3:(TestOperationsStringStringDS*)_do
{
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
    [self called];
}
-(void) opStringStringDSException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opStringMyEnumDSResponse:(TestOperationsStringStringDS*)ro p3:(TestOperationsStringStringDS*)_do
{
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
    [self called];
}
-(void) opStringMyEnumDSException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opMyEnumStringDSResponse:(TestOperationsMyEnumStringDS*)ro p3:(TestOperationsMyEnumStringDS*)_do
{
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
    [self called];
}
-(void) opMyEnumStringDSException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opMyStructMyEnumDSResponse:(TestOperationsMyStructMyEnumDS*)ro p3:(TestOperationsMyStructMyEnumDS*)_do
                               s11:(TestOperationsMyStruct*)s11 s12:(TestOperationsMyStruct*)s12
                               s22:(TestOperationsMyStruct*)s22 s23:(TestOperationsMyStruct*)s23
{
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
    [self called];
}
-(void) opMyStructMyEnumDSException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opByteByteSDResponse:(TestOperationsByteByteSD*)ro p3:(TestOperationsByteByteSD*)_do
{
    const ICEByte *p;
    test([_do count] == 1);
    test([[_do objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]] length] / sizeof(ICEByte) == 2);
    p = [[_do objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]] bytes];
    test(p[0] == 0xf2);
    test(p[1] == 0xf3);
    test([ro count] == 3);
    test([[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x01]] length] / sizeof(ICEByte) == 2);
    p = [[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x01]] bytes];
    test(p[0] == 0x01);
    test(p[1] == 0x11);
    test([[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x22]] length] / sizeof(ICEByte) == 1);
    p = [[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0x22]] bytes];
    test(p[0] == 0x12);
    test([[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]] length] / sizeof(ICEByte) == 2);
    p = [[ro objectForKey:[NSNumber numberWithUnsignedChar:(ICEByte)0xf1]] bytes];
    test(p[0] == 0xf2);
    test(p[1] == 0xf3);
    [self called];
}
-(void) opByteByteSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opBoolBoolSDResponse:(TestOperationsBoolBoolSD*)ro p3:(TestOperationsBoolBoolSD*)_do
{
    const BOOL *p;
    test([_do count] == 1);
    test([[_do objectForKey:[NSNumber numberWithBool:NO]] length] / sizeof(BOOL) == 2);
    p = [[_do objectForKey:[NSNumber numberWithBool:NO]] bytes];
    test(p[0] == YES);
    test(p[1] == NO);
    test([ro count] == 2);
    test([[ro objectForKey:[NSNumber numberWithBool:NO]] length] / sizeof(BOOL) == 2);
    p = [[ro objectForKey:[NSNumber numberWithBool:NO]] bytes];
    test(p[0] == YES);
    test(p[1] == NO);
    test([[ro objectForKey:[NSNumber numberWithBool:YES]] length] / sizeof(BOOL) == 3);
    p = [[ro objectForKey:[NSNumber numberWithBool:YES]] bytes];
    test(p[0] == NO);
    test(p[1] == YES);
    test(p[2] == YES);
    [self called];
}
-(void) opBoolBoolSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opShortShortSDResponse:(TestOperationsShortShortSD*)ro p3:(TestOperationsShortShortSD*)_do
{
    const ICEShort *p;
    test([_do count] == 1);
    test([[_do objectForKey:[NSNumber numberWithShort:4]] length] / sizeof(ICEShort) == 2);
    p = [[_do objectForKey:[NSNumber numberWithShort:4]] bytes];
    test(p[0] == 6);
    test(p[1] == 7);
    test([ro count] == 3);
    test([[ro objectForKey:[NSNumber numberWithShort:1]] length] / sizeof(ICEShort) == 3);
    p = [[ro objectForKey:[NSNumber numberWithShort:1]] bytes];
    test(p[0] == 1);
    test(p[1] == 2);
    test(p[2] == 3);
    test([[ro objectForKey:[NSNumber numberWithShort:2]] length] / sizeof(ICEShort) == 2);
    p = [[ro objectForKey:[NSNumber numberWithShort:2]] bytes];
    test(p[0] == 4);
    test(p[1] == 5);
    test([[ro objectForKey:[NSNumber numberWithShort:4]] length] / sizeof(ICEShort) == 2);
    p = [[ro objectForKey:[NSNumber numberWithShort:4]] bytes];
    test(p[0] == 6);
    test(p[1] == 7);
    [self called];
}
-(void) opShortShortSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opIntIntSDResponse:(TestOperationsIntIntSD*)ro p3:(TestOperationsIntIntSD*)_do
{
    const ICEInt *p;
    test([_do count] == 1);
    test([[_do objectForKey:[NSNumber numberWithInt:400]] length] / sizeof(ICEInt) == 2);
    p = [[_do objectForKey:[NSNumber numberWithInt:400]] bytes];
    test(p[0] == 600);
    test(p[1] == 700);
    test([ro count] == 3);
    test([[ro objectForKey:[NSNumber numberWithInt:100]] length] / sizeof(ICEInt) == 3);
    p = [[ro objectForKey:[NSNumber numberWithInt:100]] bytes];
    test(p[0] == 100);
    test(p[1] == 200);
    test(p[2] == 300);
    test([[ro objectForKey:[NSNumber numberWithInt:200]] length] / sizeof(ICEInt) == 2);
    p = [[ro objectForKey:[NSNumber numberWithInt:200]] bytes];
    test(p[0] == 400);
    test(p[1] == 500);
    test([[ro objectForKey:[NSNumber numberWithInt:400]] length] / sizeof(ICEInt) == 2);
    p = [[ro objectForKey:[NSNumber numberWithInt:400]] bytes];
    test(p[0] == 600);
    test(p[1] == 700);
    [self called];
}
-(void) opIntIntSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opLongLongSDResponse:(TestOperationsLongLongSD*)ro p3:(TestOperationsLongLongSD*)_do
{
    const ICELong *p;
    test([_do count] == 1);
    test([[_do objectForKey:[NSNumber numberWithLong:999999992]] length] / sizeof(ICELong) == 2);
    p = [[_do objectForKey:[NSNumber numberWithLong:999999992]] bytes];
    test(p[0] == 999999110);
    test(p[1] == 999999120);
    test([ro count] == 3);
    test([[ro objectForKey:[NSNumber numberWithLong:999999990]] length] / sizeof(ICELong) == 3);
    p = [[ro objectForKey:[NSNumber numberWithLong:999999990]] bytes];
    test(p[0] == 999999110);
    test(p[1] == 999999111);
    test(p[2] == 999999110);
    test([[ro objectForKey:[NSNumber numberWithLong:999999991]] length] / sizeof(ICELong) == 2);
    p = [[ro objectForKey:[NSNumber numberWithLong:999999991]] bytes];
    test(p[0] == 999999120);
    test(p[1] == 999999130);
    test([[ro objectForKey:[NSNumber numberWithLong:999999992]] length] / sizeof(ICELong) == 2);
    p = [[ro objectForKey:[NSNumber numberWithLong:999999992]] bytes];
    test(p[0] == 999999110);
    test(p[1] == 999999120);
    [self called];
}
-(void) opLongLongSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opStringFloatSDResponse:(TestOperationsStringFloatSD*)ro p3:(TestOperationsStringFloatSD*)_do
{
    const ICEFloat *p;
    test([_do count] == 1);
    test([[_do objectForKey:@"aBc"] length] / sizeof(ICEFloat) == 2);
    p = [[_do objectForKey:@"aBc"] bytes];
    test(p[0] == -3.14f);
    test(p[1] == 3.14f);
    test([ro count] == 3);
    test([[ro objectForKey:@"abc"] length] / sizeof(ICEFloat) == 3);
    p = [[ro objectForKey:@"abc"] bytes];
    test(p[0] == -1.1f);
    test(p[1] == 123123.2f);
    test(p[2] == 100.0f);
    test([[ro objectForKey:@"ABC"] length] / sizeof(ICEFloat) == 2);
    p = [[ro objectForKey:@"ABC"] bytes];
    test(p[0] == 42.24f);
    test(p[1] == -1.61f);
    test([[ro objectForKey:@"aBc"] length] / sizeof(ICEFloat) == 2);
    p = [[ro objectForKey:@"aBc"] bytes];
    test(p[0] == -3.14f);
    test(p[1] == 3.14f);
    [self called];
}
-(void) opStringFloatSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opStringDoubleSDResponse:(TestOperationsStringDoubleSD*)ro p3:(TestOperationsStringDoubleSD*)_do
{
    const ICEDouble *p;
    test([_do count] == 1);
    test([[_do objectForKey:@""] length] / sizeof(ICEDouble) == 2);
    p = [[_do objectForKey:@""] bytes];
    test(p[0] == 1.6E10);
    test(p[1] == 1.7E10);
    test([ro count] == 3);
    test([[ro objectForKey:@"Hello!!"] length] / sizeof(ICEDouble) == 3);
    p = [[ro objectForKey:@"Hello!!"] bytes];
    test(p[0] == 1.1E10);
    test(p[1] == 1.2E10);
    test(p[2] == 1.3E10);
    test([[ro objectForKey:@"Goodbye"] length] / sizeof(ICEDouble) == 2);
    p = [[ro objectForKey:@"Goodbye"] bytes];
    test(p[0] == 1.4E10);
    test(p[1] == 1.5E10);
    test([[ro objectForKey:@""] length] / sizeof(ICEDouble) == 2);
    p = [[ro objectForKey:@""] bytes];
    test(p[0] == 1.6E10);
    test(p[1] == 1.7E10);
    [self called];
}
-(void) opStringDoubleSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opStringStringSDResponse:(TestOperationsStringStringSD*)ro p3:(TestOperationsStringStringSD*)_do
{
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
    [self called];
}
-(void) opStringStringSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opMyEnumMyEnumSDResponse:(TestOperationsMyEnumMyEnumSD*)ro p3:(TestOperationsMyEnumMyEnumSD*)_do
{
    const TestOperationsMyEnum *p;
    test([_do count] == 1);
    test([[_do objectForKey:@(TestOperationsenum1)] length] / sizeof(TestOperationsMyEnum) == 2);
    p = [[_do objectForKey:@(TestOperationsenum1)] bytes];
    test(p[0] == TestOperationsenum3);
    test(p[1] == TestOperationsenum3);
    test([ro count] == 3);
    test([[ro objectForKey:@(TestOperationsenum3)] length] / sizeof(TestOperationsMyEnum) == 3);
    p = [[ro objectForKey:@(TestOperationsenum3)] bytes];
    test(p[0] == TestOperationsenum1);
    test(p[1] == TestOperationsenum1);
    test(p[2] == TestOperationsenum2);
    test([[ro objectForKey:@(TestOperationsenum2)] length] / sizeof(TestOperationsMyEnum) == 2);
    p = [[ro objectForKey:@(TestOperationsenum2)] bytes];
    test(p[0] == TestOperationsenum1);
    test(p[1] == TestOperationsenum2);
    test([[ro objectForKey:@(TestOperationsenum1)] length] / sizeof(TestOperationsMyEnum) == 2);
    p = [[ro objectForKey:@(TestOperationsenum1)] bytes];
    test(p[0] == TestOperationsenum3);
    test(p[1] == TestOperationsenum3);
    [self called];
}
-(void) opMyEnumMyEnumSDException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opIntSResponse:(TestOperationsIntS*)r
{
    const ICEInt *rp = [r bytes];
    for(int j = 0; j < (int)([r length] / sizeof(ICEInt)); ++j)
    {
        test(rp[j] == -j);
    }
    [self called];
}
-(void) opIntSException:(ICEException*)__unused ex
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
-(void) opContextException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opDoubleMarshalingResponse
{
    [self called];
}
-(void) opDoubleMarshalingException:(ICEException*)__unused ex
{
    test(NO);
}
@end

void
twowaysAMI(id<ICECommunicator> communicator, id<TestOperationsMyClassPrx> p)
{
    {
        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opByte:(ICEByte)0xff p2:(ICEByte)0x0f response:^(ICEByte ret, ICEByte p3) { [cb opByteResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opByteException:ex]; }];
        test([cb check]);
    }

    {
        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opBool:YES p2:NO response:^(BOOL ret, BOOL p3) { [cb opBoolResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opBoolException:ex]; }];
        test([cb check]);
    }

    {
        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opShortIntLong:10 p2:11 p3:12 response:^(ICELong ret, ICEShort p4, ICEInt p5, ICELong p6) { [cb opShortIntLongResponse:ret p4:p4 p5:p5 p6:p6]; } exception:^(ICEException* ex) { [cb opShortIntLongException:ex]; }];
        test([cb check]);
    }

    {
        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opFloatDouble:3.14f p2:1.1E10 response:^(ICEDouble ret, ICEFloat p3, ICEDouble p4) { [cb opFloatDoubleResponse:ret p3:p3 p4:p4]; } exception:^(ICEException* ex) { [cb opFloatDoubleException:ex]; }];
        test([cb check]);
    }

    {
        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opString:@"hello" p2:@"world" response:^(NSMutableString* ret, NSMutableString* p3) { [cb opStringResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStringException:ex]; }];
        test([cb check]);
    }

    {
        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opMyEnum:TestOperationsenum2 response:^(TestOperationsMyEnum ret, TestOperationsMyEnum p2) { [cb opMyEnumResponse:ret p2:p2]; } exception:^(ICEException* ex) { [cb opMyEnumException:ex]; }];
        test([cb check]);
    }

    {
        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opStringMyEnumD:di1 p2:di2 response:^(TestOperationsMutableStringMyEnumD* ret, TestOperationsMutableStringMyEnumD* p3) { [cb opStringMyEnumDResponse:ret p3:p3]; } exception:^(ICEException* ex) { [cb opStringMyEnumDException:ex]; }];
        test([cb check]);
    }

    {
        TestOperationsMutableMyEnumStringD *di1 = [TestOperationsMutableMyEnumStringD dictionary];
        [di1 setObject:@"abc" forKey:@(TestOperationsenum1)];
        TestOperationsMutableMyEnumStringD *di2 = [TestOperationsMutableMyEnumStringD dictionary];
        [di2 setObject:@"Hello!!" forKey:@(TestOperationsenum2)];
        [di2 setObject:@"querty" forKey:@(TestOperationsenum3)];

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opByteBoolDS:dsi1
                           p2:dsi2
                     response:^(TestOperationsMutableByteBoolDS* ret,
                                TestOperationsMutableByteBoolDS* p3) { [cb opByteBoolDSResponse:ret
                                                                                             p3:p3]; }
                    exception:^(ICEException* ex) { [cb opByteBoolDSException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opShortIntDS:dsi1
                           p2:dsi2
                     response:^(TestOperationsMutableShortIntDS* ret,
                                TestOperationsMutableShortIntDS* p3) { [cb opShortIntDSResponse:ret
                                                                                             p3:p3]; }
                    exception:^(ICEException* ex) { [cb opShortIntDSException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opLongFloatDS:dsi1
                           p2:dsi2
                     response:^(TestOperationsMutableLongFloatDS* ret,
                                TestOperationsMutableLongFloatDS* p3) { [cb opLongFloatDSResponse:ret
                                                                                               p3:p3]; }
                    exception:^(ICEException* ex) { [cb opLongFloatDSException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opStringStringDS:dsi1
                               p2:dsi2
                         response:^(TestOperationsMutableStringStringDS* ret,
                                    TestOperationsMutableStringStringDS* p3) { [cb opStringStringDSResponse:ret
                                                                                                         p3:p3]; }
                        exception:^(ICEException* ex) { [cb opStringStringDSException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opStringMyEnumDS:dsi1
                           p2:dsi2
                     response:^(TestOperationsMutableStringMyEnumDS* ret,
                                TestOperationsMutableStringMyEnumDS* p3) { [cb opStringMyEnumDSResponse:ret
                                                                                                     p3:p3]; }
                    exception:^(ICEException* ex) { [cb opStringMyEnumDSException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opMyEnumStringDS:dsi1
                           p2:dsi2
                     response:^(TestOperationsMutableMyEnumStringDS* ret,
                                TestOperationsMutableMyEnumStringDS* p3) { [cb opMyEnumStringDSResponse:ret
                                                                                                     p3:p3]; }
                    exception:^(ICEException* ex) { [cb opMyEnumStringDSException:ex]; }];
        [cb check];
    }

    {
        TestOperationsMutableMyStructMyEnumDS *dsi1 = [TestOperationsMutableMyStructMyEnumDS array];
        TestOperationsMutableMyStructMyEnumDS *dsi2 = [TestOperationsMutableMyStructMyEnumDS array];

        TestOperationsMyStruct* s11 = [TestOperationsMyStruct myStruct:1 j:1];
        TestOperationsMyStruct* s12 = [TestOperationsMyStruct myStruct:1 j:2];
        TestOperationsMutableMyStructMyEnumD* di1 = [TestOperationsMutableMyStructMyEnumD dictionary];
        [di1 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:s11];
        [di1 setObject:[NSNumber numberWithInt:TestOperationsenum2] forKey:s12];

        TestOperationsMyStruct* s22 = [TestOperationsMyStruct myStruct:2 j:2];
        TestOperationsMyStruct* s23 = [TestOperationsMyStruct myStruct:2 j:3];
        TestOperationsMutableMyStructMyEnumD* di2 = [TestOperationsMutableMyStructMyEnumD dictionary];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum1] forKey:s11];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum3] forKey:s22];
        [di2 setObject:[NSNumber numberWithInt:TestOperationsenum2] forKey:s23];

        TestOperationsMutableMyStructMyEnumD* di3 = [TestOperationsMutableMyStructMyEnumD dictionary];
        [di3 setObject:[NSNumber numberWithInt:TestOperationsenum3] forKey:s23];

        [dsi1 addObject:di1];
        [dsi1 addObject:di2];
        [dsi2 addObject:di3];

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opMyStructMyEnumDS:dsi1
                                 p2:dsi2
                           response:^(TestOperationsMutableMyStructMyEnumDS* ret,
                                     TestOperationsMutableMyStructMyEnumDS* p3) { [cb opMyStructMyEnumDSResponse:ret
                                                                                                             p3:p3
                                                                                                            s11:s11
                                                                                                            s12:s12
                                                                                                            s22:s22
                                                                                                            s23:s23]; }
                          exception:^(ICEException* ex) { [cb opMyStructMyEnumDSException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opByteByteSD:sdi1
                           p2:sdi2
                     response:^(TestOperationsMutableByteByteSD* ret,
                                TestOperationsMutableByteByteSD* p3) { [cb opByteByteSDResponse:ret
                                                                                             p3:p3]; }
                    exception:^(ICEException* ex) { [cb opByteByteSDException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opBoolBoolSD:sdi1
                           p2:sdi2
                     response:^(TestOperationsMutableBoolBoolSD* ret,
                                TestOperationsMutableBoolBoolSD* p3) { [cb opBoolBoolSDResponse:ret
                                                                                             p3:p3]; }
                    exception:^(ICEException* ex) { [cb opBoolBoolSDException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opShortShortSD:sdi1
                           p2:sdi2
                     response:^(TestOperationsMutableShortShortSD* ret,
                                TestOperationsMutableShortShortSD* p3) { [cb opShortShortSDResponse:ret
                                                                                             p3:p3]; }
                    exception:^(ICEException* ex) { [cb opShortShortSDException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opIntIntSD:sdi1
                           p2:sdi2
                     response:^(TestOperationsMutableIntIntSD* ret,
                                TestOperationsMutableIntIntSD* p3) { [cb opIntIntSDResponse:ret
                                                                                         p3:p3]; }
                    exception:^(ICEException* ex) { [cb opIntIntSDException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opLongLongSD:sdi1
                           p2:sdi2
                     response:^(TestOperationsMutableLongLongSD* ret,
                                TestOperationsMutableLongLongSD* p3) { [cb opLongLongSDResponse:ret
                                                                                             p3:p3]; }
                    exception:^(ICEException* ex) { [cb opLongLongSDException:ex]; }];
        [cb check];
    }

    {
        TestOperationsMutableStringFloatSD *sdi1 = [TestOperationsMutableStringFloatSD dictionary];
        TestOperationsMutableStringFloatSD *sdi2 = [TestOperationsMutableStringFloatSD dictionary];

        ICEFloat buf1[] = { -1.1f, 123123.2f, 100.0f };
        ICEFloat buf2[] = { 42.24f, -1.61f };
        ICEFloat buf3[] = { -3.14, 3.14 };

        TestOperationsMutableFloatS *si1 = [TestOperationsMutableFloatS data];
        TestOperationsMutableFloatS *si2 = [TestOperationsMutableFloatS data];
        TestOperationsMutableFloatS *si3 = [TestOperationsMutableFloatS data];

        [si1 appendBytes:buf1 length:sizeof(buf1)];
        [si2 appendBytes:buf2 length:sizeof(buf2)];
        [si3 appendBytes:buf3 length:sizeof(buf3)];

        [sdi1 setObject:si1 forKey:@"abc"];
        [sdi1 setObject:si2 forKey:@"ABC"];
        [sdi2 setObject:si3 forKey:@"aBc"];

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opStringFloatSD:sdi1
                              p2:sdi2
                        response:^(TestOperationsMutableStringFloatSD* ret,
                                  TestOperationsMutableStringFloatSD* p3) { [cb opStringFloatSDResponse:ret
                                                                                                     p3:p3]; }
                       exception:^(ICEException* ex) { [cb opStringFloatSDException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opStringDoubleSD:sdi1
                               p2:sdi2
                         response:^(TestOperationsMutableStringDoubleSD* ret,
                                    TestOperationsMutableStringDoubleSD* p3) { [cb opStringDoubleSDResponse:ret
                                                                                                         p3:p3]; }
                        exception:^(ICEException* ex) { [cb opStringDoubleSDException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opStringStringSD:sdi1
                               p2:sdi2
                         response:^(TestOperationsMutableStringStringSD* ret,
                                    TestOperationsMutableStringStringSD* p3) { [cb opStringStringSDResponse:ret
                                                                                                         p3:p3]; }
                        exception:^(ICEException* ex) { [cb opStringStringSDException:ex]; }];
        [cb check];
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

        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opMyEnumMyEnumSD:sdi1
                               p2:sdi2
                         response:^(TestOperationsMutableMyEnumMyEnumSD* ret,
                                    TestOperationsMutableMyEnumMyEnumSD* p3) { [cb opMyEnumMyEnumSDResponse:ret
                                                                                                         p3:p3]; }
                        exception:^(ICEException* ex) { [cb opMyEnumMyEnumSDException:ex]; }];
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

            TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
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
                TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
                [p begin_opContext:^(ICEMutableContext* ctxP) { [cb opEmptyContextResponse:ctxP]; } exception:^(ICEException* ex) { [cb opContextException:ex]; }];
                test([cb check]);
        }
        {
            TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
            [p begin_opContext:ctx response:^(ICEMutableContext* ctxP) { [cb opNonEmptyContextResponse:ctxP]; } exception:^(ICEException* ex) { [cb opContextException:ex]; }];
            test([cb check]);
        }
        {
            id<TestOperationsMyClassPrx> p2 = [TestOperationsMyClassPrx checkedCast:[p ice_context:ctx]];
            test([[p2 ice_getContext] isEqual:ctx]);
            TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
            [p2 begin_opContext:^(ICEMutableContext* ctxP) { [cb opNonEmptyContextResponse:ctxP]; } exception:^(ICEException* ex) { [cb opContextException:ex]; }];
            test([cb check]);

            cb = [TestAMIOperationsCallback create];
            [p2 begin_opContext:ctx response:^(ICEMutableContext* ctxP) { [cb opNonEmptyContextResponse:ctxP]; } exception:^(ICEException* ex) { [cb opContextException:ex]; }];
            test([cb check]);
        }
    }

    if([p ice_getConnection])
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

            id<TestOperationsMyClassPrx> pc = [TestOperationsMyClassPrx uncheckedCast:
                                                                    [ic stringToProxy:@"test:default -p 12010"]];

            [[ic getImplicitContext] setContext:(ctx)];
            test([[[ic getImplicitContext] getContext] isEqualToDictionary:ctx]);
            {
                id<ICEAsyncResult> r = [pc begin_opContext];
                ICEContext* c = [pc end_opContext:r];
                test([c isEqualToDictionary:ctx]);
            }

            test([[ic getImplicitContext] get:@"zero"] == nil);
            [[ic getImplicitContext] put:@"zero" value:@"ZERO"];
            test([[[ic getImplicitContext] get:@"zero"] isEqualToString:@"ZERO"]);

            ctx = [[ic getImplicitContext] getContext];
            {
                id<ICEAsyncResult> r = [pc begin_opContext];
                ICEContext* c = [pc end_opContext:r];
                test([c isEqualToDictionary:ctx]);
            }

            ICEMutableContext *prxContext = [ICEMutableContext dictionary];
            [prxContext setObject:@"UN" forKey:@"one"];
            [prxContext setObject:@"QUATRE" forKey:@"four"];

            ICEMutableContext *combined = [ICEMutableContext dictionaryWithDictionary:ctx];
            [combined addEntriesFromDictionary:prxContext];

            pc = [TestOperationsMyClassPrx uncheckedCast:[pc ice_context:prxContext]];

            [[ic getImplicitContext] setContext:[ICEMutableContext dictionary]];
            {
                id<ICEAsyncResult> r = [pc begin_opContext];
                ICEContext* c = [pc end_opContext:r];
                test([c isEqualToDictionary:prxContext]);
            }

            [[ic getImplicitContext] setContext:ctx];
            {
                id<ICEAsyncResult> r = [pc begin_opContext];
                ICEContext* c = [pc end_opContext:r];
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
        TestAMIOperationsCallback* cb = [TestAMIOperationsCallback create];
        [p begin_opDoubleMarshaling:d p2:ds response:^() { [cb opDoubleMarshalingResponse]; } exception:^(ICEException* ex) { [cb opDoubleMarshalingException:ex]; }];
        test([cb check]);
    }

    // Marshaling tests for NSNull are present only in synchronous test because testing asynchronously
    // would only test the same marshaling code that's been tested already.
}
