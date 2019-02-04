//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <enums/TestI.h>
#import <TestCommon.h>

@implementation TestEnumTestIntfI

-(void) shutdown:(ICECurrent*)current
{
    [[[current adapter] getCommunicator] shutdown];
}

-(TestEnumByteEnum) opByte:(TestEnumByteEnum)b1 b2:(TestEnumByteEnum*)b2 current:(ICECurrent*)__unused current
{
    *b2 = b1;
    return b1;
}

-(TestEnumShortEnum) opShort:(TestEnumShortEnum)s1 s2:(TestEnumShortEnum*)s2 current:(ICECurrent*)__unused current
{
    *s2 = s1;
    return s1;
}

-(TestEnumIntEnum) opInt:(TestEnumIntEnum)i1 i2:(TestEnumIntEnum*)i2 current:(ICECurrent*)__unused current
{
    *i2 = i1;
    return i1;
}

-(TestEnumSimpleEnum) opSimple:(TestEnumSimpleEnum)s1 s2:(TestEnumSimpleEnum*)s2 current:(ICECurrent*)__unused current
{
    *s2 = s1;
    return s1;
}

-(TestEnumByteEnumSeq*) opByteSeq:(TestEnumByteEnumSeq*)b1 b2:(TestEnumByteEnumSeq**)b2 current:(ICECurrent*)__unused current
{
    *b2 = b1;
    return b1;
}

-(TestEnumShortEnumSeq*) opShortSeq:(TestEnumShortEnumSeq*)s1 s2:(TestEnumShortEnumSeq**)s2 current:(ICECurrent*)__unused current
{
    *s2 = s1;
    return s1;
}

-(TestEnumIntEnumSeq*) opIntSeq:(TestEnumIntEnumSeq*)i1 i2:(TestEnumIntEnumSeq**)i2 current:(ICECurrent*)__unused current
{
    *i2 = i1;
    return i1;
}

-(TestEnumSimpleEnumSeq*) opSimpleSeq:(TestEnumSimpleEnumSeq*)s1 s2:(TestEnumSimpleEnumSeq**)s2 current:(ICECurrent*)__unused current
{
    *s2 = s1;
    return s1;
}

@end
