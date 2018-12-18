// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>

#import <exceptions/TestI.h>
#import <TestCommon.h>
#import <Foundation/NSException.h>

@interface FooException : NSException
@end

@implementation FooException
-(id)init
{
    self = [super initWithName:@"FooException" reason:@"no reason" userInfo:nil];
    if(!self)
    {
        return nil;
    }
    return self;
}
@end

@implementation ThrowerI

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}

-(BOOL) supportsUndeclaredExceptions:(ICECurrent*)__unused current
{
    return YES;
}

-(BOOL) supportsAssertException:(ICECurrent*)__unused current
{
    return NO;
}

-(void) throwAasA:(ICEInt)a current:(ICECurrent*)__unused current
{
    @throw [TestExceptionsA a:a];
}

-(void) throwAorDasAorD:(ICEInt)a current:(ICECurrent*)__unused current
{
    if(a > 0)
    {
        @throw [TestExceptionsA a:a];
    }
    else
    {
        @throw [TestExceptionsD d:a];
    }
}

-(void) throwBasA:(ICEInt)a b:(ICEInt)b current:(ICECurrent*)__unused current
{
    [self throwBasB:a b:b current:current];
}

-(void) throwCasA:(ICEInt)a b:(ICEInt)b c:(ICEInt) c current:(ICECurrent*)__unused current
{
    [self throwCasC:a b:b c:c current:current];
}

-(void) throwBasB:(ICEInt)a b:(ICEInt)b current:(ICECurrent*)__unused current
{
    @throw [TestExceptionsB b:a bMem:b];
}

-(void) throwCasB:(ICEInt)a b:(ICEInt)b c:(ICEInt)c current:(ICECurrent*)__unused current
{
    [self throwCasC:a b:b c:c current:current];
}

-(void) throwCasC:(ICEInt)a b:(ICEInt)b c:(ICEInt)c current:(ICECurrent*)__unused current
{
    @throw [TestExceptionsC c:a bMem:b cMem:c];
}

-(void) throwModA:(ICEInt)a a2:(ICEInt)a2 current:(ICECurrent*)__unused current
{
    @throw [TestExceptionsModA a:a a2Mem:a2];
}

-(void) throwUndeclaredA:(ICEInt)a current:(ICECurrent*)__unused current
{
    @throw [TestExceptionsA a:a];
}

-(void) throwUndeclaredB:(ICEInt)a b:(ICEInt)b current:(ICECurrent*)__unused current
{
    @throw [TestExceptionsB b:a bMem:b];
}

-(void) throwUndeclaredC:(ICEInt)a b:(ICEInt)b c:(ICEInt)c current:(ICECurrent*)__unused current
{
    @throw [TestExceptionsC c:a bMem:b cMem:c];
}

-(void) throwLocalException:(ICECurrent*)__unused current
{
    @throw [ICETimeoutException timeoutException:__FILE__ line:__LINE__];
}

-(ICEByteSeq*) throwMemoryLimitException:(ICEMutableByteSeq*)__unused bs current:(ICECurrent*)__unused current
{
    int limit = 20 * 1024;
    ICEMutableByteSeq *r = [NSMutableData dataWithLength:limit];
    ICEByte *p = (ICEByte *)[r bytes];
    while(--limit > 0)
    {
        *p++ = limit % 0x80;
    }
    return r;
}

-(void) throwNonIceException:(ICECurrent*)__unused current
{
    @throw ICE_AUTORELEASE([[FooException alloc] init]);
}

-(void) throwAssertException:(ICECurrent*)__unused current
{
    // Not supported.
}

-(void) throwLocalExceptionIdempotent:(ICECurrent*)__unused current
{
    @throw [ICETimeoutException timeoutException:__FILE__ line:__LINE__];
}
-(void) throwAfterResponse:(ICECurrent*)__unused current
{
    // Only relevant for AMD
}
-(void) throwAfterException:(ICECurrent*)__unused current
{
    // Only relevant for AMD
    @throw [TestExceptionsA a:12345];
}
@end
