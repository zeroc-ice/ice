//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>

#import <operations/TestI.h>
#import <TestCommon.h>

@implementation TestOperationsMyDerivedClassI
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _opByteSOnewayCallCount = 0;
    _cond = [[NSCondition alloc] init];
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_cond release];
    [super dealloc];
}
#endif

-(void) opVoid:(ICECurrent*)__unused current
{
}

-(void) opDerived:(ICECurrent*)__unused current
{
}

-(ICEByte) opByte:(ICEByte)p1 p2:(ICEByte)p2 p3:(ICEByte *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1 ^ p2;
    return p1;
}

-(BOOL) opBool:(BOOL)p1 p2:(BOOL)p2 p3:(BOOL *)p3 current:(ICECurrent*)__unused current
{
    *p3 = p1;
    return p2;
}

-(ICELong) opShortIntLong:(ICEShort)p1 p2:(ICEInt)p2 p3:(ICELong)p3
                       p4:(ICEShort *)p4 p5:(ICEInt *)p5 p6:(ICELong *)p6
                  current:(ICECurrent *)__unused current
{
    *p4 = p1;
    *p5 = p2;
    *p6 = p3;
    return p3;
}

-(ICEDouble) opFloatDouble:(ICEFloat)p1 p2:(ICEDouble)p2 p3:(ICEFloat *)p3 p4:(ICEDouble *)p4
                   current:(ICECurrent *)__unused current
{
    *p3 = p1;
    *p4 = p2;
    return p2;
}

-(NSString *) opString:(NSMutableString *)p1 p2:(NSMutableString *)p2 p3:(NSString **)p3 current:(ICECurrent *)__unused current
{
    NSMutableString *sout = [NSMutableString stringWithCapacity:([p2 length] + 1 + [p1 length])];
    [sout appendString:p2];
    [sout appendString:@" "];
    [sout appendString:p1];
    *p3 = sout;

    NSMutableString *ret = [NSMutableString stringWithCapacity:([p1 length] + 1 + [p2 length])];
    [ret appendString:p1];
    [ret appendString:@" "];
    [ret appendString:p2];
    return ret;
}

-(TestOperationsMyEnum) opMyEnum:(TestOperationsMyEnum)p1 p2:(TestOperationsMyEnum *)p2 current:(ICECurrent *)__unused current
{
    *p2 = p1;
    return TestOperationsenum3;
}

-(id<TestOperationsMyClassPrx>) opMyClass:(id<TestOperationsMyClassPrx>)p1 p2:(id<TestOperationsMyClassPrx> *)p2 p3:(id<TestOperationsMyClassPrx> *)p3
                        current:(ICECurrent *)__unused current
{
    *p2 = p1;
    *p3 = [TestOperationsMyClassPrx uncheckedCast:[current.adapter
                                           createProxy:[ICEUtil stringToIdentity:@"noSuchIdentity"]]];
    return [TestOperationsMyClassPrx uncheckedCast:[current.adapter createProxy:[current id_]]];
}

-(TestOperationsStructure *) opStruct:(TestOperationsStructure *)p1 p2:(TestOperationsStructure *)p2 p3:(TestOperationsStructure **)p3
                            current:(ICECurrent *)__unused current
{
    *p3 = ICE_AUTORELEASE([p1 copy]);
    (*p3).s.s = @"a new string";
    return p2;
}

-(TestOperationsByteS *) opByteS:(TestOperationsMutableByteS *)p1 p2:(TestOperationsMutableByteS *)p2 p3:(TestOperationsByteS **)p3
                       current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableByteS dataWithLength:[p1 length]];
    ICEByte *target = (ICEByte *)[*p3 bytes];
    ICEByte *src = (ICEByte *)[p1 bytes] + [p1 length];
    int i;
    for(i = 0; i != (int)[p1 length]; ++i)
    {
        *target++ = *--src;
    }
    TestOperationsMutableByteS *r = [TestOperationsMutableByteS dataWithData:p1];
    [r appendData:p2];
    return r;
}

-(TestOperationsBoolS *) opBoolS:(TestOperationsMutableBoolS *)p1 p2:(TestOperationsMutableBoolS *)p2 p3:(TestOperationsBoolS **)p3
                       current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableBoolS dataWithData:p1];
    [(TestOperationsMutableBoolS *)*p3 appendData:p2];

    TestOperationsMutableBoolS *r = [TestOperationsMutableBoolS dataWithLength:[p1 length] * sizeof(BOOL)];
    BOOL *target = (BOOL *)[r bytes];
    BOOL *src = (BOOL *)([p1 bytes] + [p1 length]);
    int i;
    for(i = 0; i != (int)[p1 length]; ++i)
    {
        *target++ = *--src;
    }
    return r;
}

-(TestOperationsLongS *) opShortIntLongS:(TestOperationsMutableShortS *)p1 p2:(TestOperationsMutableIntS *)p2 p3:(TestOperationsMutableLongS *)p3
                               p4:(TestOperationsShortS **)p4 p5:(TestOperationsIntS **)p5 p6:(TestOperationsLongS **)p6
                               current:(ICECurrent *)__unused current
{
    *p4 = [TestOperationsMutableShortS dataWithData:p1];
    *p5 = [TestOperationsMutableIntS dataWithLength:[p2 length]];
    ICEInt *target = (ICEInt *)[*p5 bytes];
    ICEInt *src = (ICEInt *)([p2 bytes] + [p2 length]);
    int i;
    for(i = 0; i != (int)[p2 length] / sizeof(ICEInt); ++i)
    {
        *target++ = *--src;
    }
    *p6 = [TestOperationsMutableLongS dataWithData:p3];
    [(TestOperationsMutableLongS *)*p6 appendData:p3];
    return p3;
}

-(TestOperationsDoubleS *) opFloatDoubleS:(TestOperationsMutableFloatS *)p1 p2:(TestOperationsMutableDoubleS *)p2
                                p3:(TestOperationsFloatS **)p3 p4:(TestOperationsDoubleS **)p4 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableFloatS dataWithData:p1];
    *p4 = [TestOperationsMutableDoubleS dataWithLength:[p2 length]];
    ICEDouble *target = (ICEDouble *)[*p4 bytes];
    ICEDouble *src = (ICEDouble *)([p2 bytes] + [p2 length]);
    for(size_t i = 0; i != [p2 length] / sizeof(ICEDouble); ++i)
    {
        *target++ = *--src;
    }
    TestOperationsDoubleS *r = [TestOperationsMutableDoubleS dataWithLength:([p2 length]
                                                         + ([p1 length] / sizeof(ICEFloat) * sizeof(ICEDouble)))];
    ICEDouble *rp = (ICEDouble *)[r bytes];
    ICEDouble *p2p = (ICEDouble *)[p2 bytes];
    for(size_t i = 0; i < [p2 length] / sizeof(ICEDouble); ++i)
    {
        *rp++ = *p2p++;
    }
    ICEFloat *bp1 = (ICEFloat *)[p1 bytes];
    for(size_t i = 0; i < [p1 length] / sizeof(ICEFloat); ++i)
    {
        *rp++ = bp1[i];
    }
    return r;
}

-(TestOperationsStringS *) opStringS:(TestOperationsMutableStringS *)p1 p2:(TestOperationsMutableStringS *)p2
                           p3:(TestOperationsStringS **)p3 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringS arrayWithArray:p1];
    [(TestOperationsMutableStringS *)*p3 addObjectsFromArray:p2];
    TestOperationsMutableStringS *r = [TestOperationsMutableStringS arrayWithCapacity:[p1 count]];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(NSString *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsMyEnumS *) opMyEnumS:(TestOperationsMutableMyEnumS *)p1 p2:(TestOperationsMutableMyEnumS *)p2
                           p3:(TestOperationsMyEnumS **)p3 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableMyEnumS dataWithLength:[p1 length]];
    TestOperationsMyEnum *target = (TestOperationsMyEnum *)[*p3 bytes];
    TestOperationsMyEnum *src = (TestOperationsMyEnum *)([p1 bytes] + [p1 length]);
    for(size_t i = 0; i != [p1 length] / sizeof(TestOperationsMyEnum); ++i)
    {
        *target++ = *--src;
    }
    TestOperationsMutableMyEnumS *r = [TestOperationsMutableMyEnumS dataWithData:p1];
    [r appendData:p2];
    return r;
}

-(TestOperationsMyClassS *) opMyClassS:(TestOperationsMutableMyClassS *)p1 p2:(TestOperationsMutableMyClassS *)p2
                          p3:(TestOperationsMyClassS **)p3 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableMyClassS arrayWithArray:p1];
    [(TestOperationsMutableMyClassS *)*p3 addObjectsFromArray:p2];
    TestOperationsMutableMyClassS *r = [TestOperationsMutableMyClassS arrayWithCapacity:[p1 count]];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(NSString *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsByteSS *) opByteSS:(TestOperationsMutableByteSS *)p1 p2:(TestOperationsMutableByteSS *)p2 p3:(TestOperationsByteSS * *)p3
                         current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableByteSS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsByteS *element in enumerator)
    {
        [(TestOperationsMutableByteSS *)*p3 addObject:element];
    }

    TestOperationsMutableByteSS *r = [TestOperationsMutableByteSS arrayWithArray:p1];
    [r addObjectsFromArray:p2];
    return r;
}

-(TestOperationsBoolSS *) opBoolSS:(TestOperationsMutableBoolSS *)p1 p2:(TestOperationsMutableBoolSS *)p2 p3:(TestOperationsBoolSS * *)p3
                         current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableBoolSS arrayWithArray:p1];
    [(TestOperationsMutableBoolSS *)*p3 addObjectsFromArray:p2];

    TestOperationsMutableBoolSS *r = [TestOperationsMutableBoolSS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsBoolS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsLongSS *) opShortIntLongSS:(TestOperationsMutableShortSS *)p1 p2:(TestOperationsMutableIntSS *)p2 p3:(TestOperationsMutableLongSS *)p3
                                 p4:(TestOperationsShortSS **)p4 p5:(TestOperationsIntSS **)p5 p6:(TestOperationsLongSS **)p6
                                 current:(ICECurrent *)__unused current
{
    *p4 = [TestOperationsShortSS arrayWithArray:p1];
    *p5 = [TestOperationsMutableIntSS array];
    NSEnumerator *enumerator = [p2 reverseObjectEnumerator];
    for(TestOperationsIntS *element in enumerator)
    {
        [(TestOperationsMutableIntSS *)*p5 addObject:element];
    }
    *p6 = [TestOperationsMutableLongSS arrayWithArray:p3];
    [(TestOperationsMutableLongSS *)*p6 addObjectsFromArray:p3];
    return p3;
}

-(TestOperationsDoubleSS *) opFloatDoubleSS:(TestOperationsMutableFloatSS *)p1 p2:(TestOperationsMutableDoubleSS *)p2
                                  p3:(TestOperationsFloatSS **)p3 p4:(TestOperationsDoubleSS **)p4 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsFloatSS arrayWithArray:p1];
    *p4 = [TestOperationsMutableDoubleSS array];
    NSEnumerator *enumerator = [p2 reverseObjectEnumerator];
    for(TestOperationsDoubleS *element in enumerator)
    {
        [(TestOperationsMutableDoubleSS *)*p4 addObject:element];
    }
    TestOperationsMutableDoubleSS *r = [TestOperationsMutableDoubleSS arrayWithArray:p2];
    [r addObjectsFromArray:p2];
    return r;
}

-(TestOperationsStringSS *) opStringSS:(TestOperationsMutableStringSS *)p1 p2:(TestOperationsMutableStringSS *)p2 p3:(TestOperationsStringSS **)p3
                             current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringSS arrayWithArray:p1];
    [(TestOperationsMutableStringSS *)*p3 addObjectsFromArray:p2];
    TestOperationsMutableStringSS *r = [TestOperationsMutableStringSS array];
    NSEnumerator *enumerator = [p2 reverseObjectEnumerator];
    for(TestOperationsStringS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsStringSSS *) opStringSSS:(TestOperationsMutableStringSSS *)p1 p2:(TestOperationsMutableStringSSS *)p2 p3:(TestOperationsStringSSS **)p3
                               current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringSSS arrayWithArray:p1];
    [(TestOperationsMutableStringSSS *)*p3 addObjectsFromArray:p2];
    TestOperationsMutableStringSSS *r = [TestOperationsMutableStringSSS array];
    NSEnumerator *enumerator = [p2 reverseObjectEnumerator];
    for(TestOperationsStringSS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsByteBoolD *) opByteBoolD:(TestOperationsMutableByteBoolD *)p1 p2:(TestOperationsMutableByteBoolD *)p2 p3:(TestOperationsByteBoolD **)p3
                               current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableByteBoolD dictionaryWithDictionary:p1];
    TestOperationsMutableByteBoolD *r = [TestOperationsMutableByteBoolD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsShortIntD *) opShortIntD:(TestOperationsMutableShortIntD *)p1 p2:(TestOperationsMutableShortIntD *)p2 p3:(TestOperationsShortIntD **)p3
                               current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableShortIntD dictionaryWithDictionary:p1];
    TestOperationsMutableShortIntD *r = [TestOperationsMutableShortIntD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsLongFloatD *) opLongFloatD:(TestOperationsMutableLongFloatD *)p1 p2:(TestOperationsMutableLongFloatD *)p2 p3:(TestOperationsLongFloatD **)p3
                               current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableLongFloatD dictionaryWithDictionary:p1];
    TestOperationsMutableLongFloatD *r = [TestOperationsMutableLongFloatD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsStringStringD *) opStringStringD:(TestOperationsMutableStringStringD *)p1 p2:(TestOperationsMutableStringStringD *)p2
                                       p3:(TestOperationsStringStringD **)p3 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringStringD dictionaryWithDictionary:p1];
    TestOperationsMutableStringStringD *r = [TestOperationsMutableStringStringD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsStringMyEnumD *) opStringMyEnumD:(TestOperationsMutableStringMyEnumD *)p1
                                              p2:(TestOperationsMutableStringMyEnumD *)p2
                                              p3:(TestOperationsStringMyEnumD **)p3 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringMyEnumD dictionaryWithDictionary:p1];
    TestOperationsMutableStringMyEnumD *r = [TestOperationsMutableStringMyEnumD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsMyEnumStringD *) opMyEnumStringD:(TestOperationsMyEnumStringD*)p1
                                              p2:(TestOperationsMyEnumStringD*)p2
                                              p3:(TestOperationsMyEnumStringD**)p3
                                         current:(ICECurrent*)__unused current
{
    *p3 = [TestOperationsMutableMyEnumStringD dictionaryWithDictionary:p1];
    TestOperationsMutableMyEnumStringD *r = [TestOperationsMutableMyEnumStringD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsMyStructMyEnumD *) opMyStructMyEnumD:(TestOperationsMyStructMyEnumD*)p1
                                                  p2:(TestOperationsMyStructMyEnumD*)p2
                                                  p3:(TestOperationsMyStructMyEnumD**)p3
                                             current:(ICECurrent*)__unused current
{
    *p3 = [TestOperationsMutableMyStructMyEnumD dictionaryWithDictionary:p1];
    TestOperationsMutableMyStructMyEnumD *r = [TestOperationsMutableMyStructMyEnumD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsByteBoolDS*) opByteBoolDS:(TestOperationsMutableByteBoolDS*)p1
                             p2:(TestOperationsMutableByteBoolDS*)p2
                             p3:(TestOperationsByteBoolDS**)p3
                             current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableByteBoolDS arrayWithArray:p2];
    [(TestOperationsMutableByteBoolDS *)*p3 addObjectsFromArray:p1];
    TestOperationsMutableByteBoolDS *r = [TestOperationsMutableByteBoolDS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsByteBoolDS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsShortIntDS*) opShortIntDS:(TestOperationsMutableShortIntDS*)p1
                             p2:(TestOperationsMutableShortIntDS*)p2
                             p3:(TestOperationsShortIntDS**)p3
                             current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableShortIntDS arrayWithArray:p2];
    [(TestOperationsMutableShortIntDS *)*p3 addObjectsFromArray:p1];
    TestOperationsMutableShortIntDS *r = [TestOperationsMutableShortIntDS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsShortIntDS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsLongFloatDS*) opLongFloatDS:(TestOperationsMutableLongFloatDS*)p1
                              p2:(TestOperationsMutableLongFloatDS*)p2
                              p3:(TestOperationsLongFloatDS**)p3
                              current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableLongFloatDS arrayWithArray:p2];
    [(TestOperationsMutableLongFloatDS *)*p3 addObjectsFromArray:p1];
    TestOperationsMutableLongFloatDS *r = [TestOperationsMutableLongFloatDS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsLongFloatDS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsStringStringDS*) opStringStringDS:(TestOperationsMutableStringStringDS*)p1
                                 p2:(TestOperationsMutableStringStringDS*)p2
                                 p3:(TestOperationsStringStringDS**)p3
                                 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringStringDS arrayWithArray:p2];
    [(TestOperationsMutableStringStringDS *)*p3 addObjectsFromArray:p1];
    TestOperationsMutableStringStringDS *r = [TestOperationsMutableStringStringDS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsStringStringDS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsStringMyEnumDS*) opStringMyEnumDS:(TestOperationsMutableStringMyEnumDS*)p1
                                 p2:(TestOperationsMutableStringMyEnumDS*)p2
                                 p3:(TestOperationsStringMyEnumDS**)p3
                                 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringMyEnumDS arrayWithArray:p2];
    [(TestOperationsMutableStringMyEnumDS *)*p3 addObjectsFromArray:p1];
    TestOperationsMutableStringMyEnumDS *r = [TestOperationsMutableStringMyEnumDS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsStringMyEnumDS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsMyEnumStringDS*) opMyEnumStringDS:(TestOperationsMutableMyEnumStringDS*)p1
                                 p2:(TestOperationsMutableMyEnumStringDS*)p2
                                 p3:(TestOperationsMyEnumStringDS**)p3
                                 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableMyEnumStringDS arrayWithArray:p2];
    [(TestOperationsMutableMyEnumStringDS *)*p3 addObjectsFromArray:p1];
    TestOperationsMutableMyEnumStringDS *r = [TestOperationsMutableMyEnumStringDS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsMyEnumStringDS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsMyStructMyEnumDS*) opMyStructMyEnumDS:(TestOperationsMutableMyStructMyEnumDS*)p1
                                   p2:(TestOperationsMutableMyStructMyEnumDS*)p2
                                   p3:(TestOperationsMyStructMyEnumDS**)p3
                                   current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableMyStructMyEnumDS arrayWithArray:p2];
    [(TestOperationsMutableMyStructMyEnumDS *)*p3 addObjectsFromArray:p1];
    TestOperationsMutableMyStructMyEnumDS *r = [TestOperationsMutableMyStructMyEnumDS array];
    NSEnumerator *enumerator = [p1 reverseObjectEnumerator];
    for(TestOperationsMyStructMyEnumDS *element in enumerator)
    {
        [r addObject:element];
    }
    return r;
}

-(TestOperationsByteByteSD*) opByteByteSD:(TestOperationsMutableByteByteSD*)p1
                             p2:(TestOperationsMutableByteByteSD*)p2
                             p3:(TestOperationsByteByteSD**)p3
                             current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableByteByteSD dictionaryWithDictionary:p2];
    TestOperationsMutableByteByteSD *r = [TestOperationsMutableByteByteSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsBoolBoolSD*) opBoolBoolSD:(TestOperationsMutableBoolBoolSD*)p1
                             p2:(TestOperationsMutableBoolBoolSD*)p2
                             p3:(TestOperationsBoolBoolSD**)p3
                             current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableBoolBoolSD dictionaryWithDictionary:p2];
    TestOperationsMutableBoolBoolSD *r = [TestOperationsMutableBoolBoolSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsShortShortSD*) opShortShortSD:(TestOperationsMutableShortShortSD*)p1
                               p2:(TestOperationsMutableShortShortSD*)p2
                               p3:(TestOperationsShortShortSD**)p3
                               current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableShortShortSD dictionaryWithDictionary:p2];
    TestOperationsMutableShortShortSD *r = [TestOperationsMutableShortShortSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsIntIntSD*) opIntIntSD:(TestOperationsMutableIntIntSD*)p1
                           p2:(TestOperationsMutableIntIntSD*)p2
                           p3:(TestOperationsIntIntSD**)p3
                           current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableIntIntSD dictionaryWithDictionary:p2];
    TestOperationsMutableIntIntSD *r = [TestOperationsMutableIntIntSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsLongLongSD*) opLongLongSD:(TestOperationsMutableLongLongSD*)p1
                             p2:(TestOperationsMutableLongLongSD*)p2
                             p3:(TestOperationsLongLongSD**)p3
                             current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableLongLongSD dictionaryWithDictionary:p2];
    TestOperationsMutableLongLongSD *r = [TestOperationsMutableLongLongSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsStringFloatSD*) opStringFloatSD:(TestOperationsMutableStringFloatSD*)p1
                                p2:(TestOperationsMutableStringFloatSD*)p2
                                p3:(TestOperationsStringFloatSD**)p3
                                current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringFloatSD dictionaryWithDictionary:p2];
    TestOperationsMutableStringFloatSD *r = [TestOperationsMutableStringFloatSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsStringDoubleSD*) opStringDoubleSD:(TestOperationsMutableStringDoubleSD*)p1
                                 p2:(TestOperationsMutableStringDoubleSD*)p2
                                 p3:(TestOperationsStringDoubleSD**)p3
                                 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringDoubleSD dictionaryWithDictionary:p2];
    TestOperationsMutableStringDoubleSD *r = [TestOperationsMutableStringDoubleSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsStringStringSD*) opStringStringSD:(TestOperationsMutableStringStringSD*)p1
                                 p2:(TestOperationsMutableStringStringSD*)p2
                                 p3:(TestOperationsStringStringSD**)p3
                                 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableStringStringSD dictionaryWithDictionary:p2];
    TestOperationsMutableStringStringSD *r = [TestOperationsMutableStringStringSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsMyEnumMyEnumSD*) opMyEnumMyEnumSD:(TestOperationsMutableMyEnumMyEnumSD*)p1
                                 p2:(TestOperationsMutableMyEnumMyEnumSD*)p2
                                 p3:(TestOperationsMyEnumMyEnumSD**)p3
                                 current:(ICECurrent *)__unused current
{
    *p3 = [TestOperationsMutableMyEnumMyEnumSD dictionaryWithDictionary:p2];
    TestOperationsMutableMyEnumMyEnumSD *r = [TestOperationsMutableMyEnumMyEnumSD dictionaryWithDictionary:p1];
    [r addEntriesFromDictionary:p2];
    return r;
}

-(TestOperationsIntS *) opIntS:(TestOperationsMutableIntS *)p1 current:(ICECurrent *)__unused current
{
    NSUInteger count = [p1 length] / sizeof(ICEInt);
    TestOperationsMutableIntS *r = [TestOperationsMutableIntS dataWithLength:[p1 length]];
    const int *src = [p1 bytes];
    int *target = (int *)[r bytes];
    while(count-- > 0)
    {
        *target++ = -*src++;
    }
    return r;
}

-(void) opByteSOneway:(TestOperationsMutableByteS *)__unused p1 current:(ICECurrent *)__unused current
{
    [_cond lock];
    ++_opByteSOnewayCallCount;
    [_cond unlock];
}

-(ICEInt) opByteSOnewayCallCount:(ICECurrent *)__unused current
{
    [_cond lock];
    ICEInt count = _opByteSOnewayCallCount;
    _opByteSOnewayCallCount = 0;
    @try
    {
        return count;
    }
    @finally
    {
        [_cond unlock];
    }
}

-(ICEContext *) opContext:(ICECurrent *)__unused current
{
    return current.ctx;
}

-(void) opDoubleMarshaling:(ICEDouble)p1 p2:(TestOperationsMutableDoubleS *)p2 current:(ICECurrent *)__unused current
{
    ICEDouble d = 1278312346.0 / 13.0;
    test(p1 == d);
    const ICEDouble *p = [p2 bytes];
    for(size_t i = 0; i < [p2 length] / sizeof(ICEDouble); ++i)
    {
        test(p[i] == d);
    }
}

-(void) opIdempotent:(ICECurrent*)__unused current
{
    test([current mode] == ICEIdempotent);
}

-(void) opNonmutating:(ICECurrent*)__unused current
{
    test([current mode] == ICENonmutating);
}

-(ICEByte) opByte1:(ICEByte)p current:(ICECurrent*)__unused current
{
    return p;
}

-(ICEShort) opShort1:(ICEShort)p current:(ICECurrent*)__unused current
{
    return p;
}

-(ICEInt) opInt1:(ICEInt)p current:(ICECurrent*)__unused current
{
    return p;
}

-(ICELong) opLong1:(ICELong)p current:(ICECurrent*)__unused current
{
    return p;
}

-(ICEFloat) opFloat1:(ICEFloat)p current:(ICECurrent*)__unused current
{
    return p;
}

-(ICEDouble) opDouble1:(ICEDouble)p current:(ICECurrent*)__unused current
{
    return p;
}

-(NSString*) opString1:(NSString*)p current:(ICECurrent*)__unused current
{
    return p;
}

-(TestOperationsStringS*) opStringS1:(TestOperationsStringS*)p current:(ICECurrent*)__unused current
{
    return p;
}

-(TestOperationsByteBoolD*) opByteBoolD1:(TestOperationsByteBoolD*)p current:(ICECurrent*)__unused current
{
    return p;
}

-(TestOperationsStringS*) opStringS2:(TestOperationsStringS*)p current:(ICECurrent*)__unused current
{
    return p;
}

-(TestOperationsByteBoolD*) opByteBoolD2:(TestOperationsByteBoolD*)p current:(ICECurrent*)__unused current
{
    return p;
}

-(TestOperationsStringS *) opStringLiterals:(ICECurrent *)__unused current
{
    TestOperationsStringS *s = [NSArray arrayWithObjects:TestOperationss0,
                                                         TestOperationss1,
                                                         TestOperationss2,
                                                         TestOperationss3,
                                                         TestOperationss4,
                                                         TestOperationss5,
                                                         TestOperationss6,
                                                         TestOperationss7,
                                                         TestOperationss8,
                                                         TestOperationss9,
                                                         TestOperationss10,

                                                         TestOperationssw0,
                                                         TestOperationssw1,
                                                         TestOperationssw2,
                                                         TestOperationssw3,
                                                         TestOperationssw4,
                                                         TestOperationssw5,
                                                         TestOperationssw6,
                                                         TestOperationssw7,
                                                         TestOperationssw8,
                                                         TestOperationssw9,
                                                         TestOperationssw10,

                                                         TestOperationsss0,
                                                         TestOperationsss1,
                                                         TestOperationsss2,
                                                         TestOperationsss3,
                                                         TestOperationsss4,
                                                         TestOperationsss5,

                                                         TestOperationssu0,
                                                         TestOperationssu1,
                                                         TestOperationssu2,
                                                         nil];
    return s;
}

-(TestOperationsStringS *) opWStringLiterals:(ICECurrent *)__unused current
{
    return [self opStringLiterals:current];
}

-(TestOperationsStructure*) opMStruct1:(ICECurrent *)__unused current
{
    return [TestOperationsStructure structure];
}
-(TestOperationsStructure*) opMStruct2:(TestOperationsStructure*)p1 p2:(TestOperationsStructure**)p2
                               current:(ICECurrent *)__unused current
{
    *p2 = p1;
    return p1;
}

-(TestOperationsStringS*) opMSeq1:(ICECurrent *)__unused current
{
    return [TestOperationsStringS array];
}

-(TestOperationsStringS*) opMSeq2:(TestOperationsMutableStringS*)p1 p2:(TestOperationsStringS**)p2
                          current:(ICECurrent *)__unused current
{
    *p2 = p1;
    return p1;
}

-(TestOperationsStringStringD*) opMDict1:(ICECurrent *)__unused current
{
    return [TestOperationsStringStringD dictionary];
}

-(TestOperationsStringStringD*) opMDict2:(TestOperationsMutableStringStringD*)p1 p2:(TestOperationsStringStringD**)p2
                                 current:(ICECurrent *)__unused current
{
    *p2 = p1;
    return p1;
}

-(TestOperationsMyClass1*) opMyClass1:(TestOperationsMyClass1*)p current:(ICECurrent*)__unused current
{
    return p;
}

-(TestOperationsMyStruct1*) opMyStruct1:(TestOperationsMyStruct1*)p current:(ICECurrent*)__unused current
{
    return p;
}

-(TestOperationsStringS *) getNSNullStringSeq:(ICECurrent *)__unused current
{
    return [NSArray arrayWithObjects:@"first", [NSNull null], nil];
}

-(TestOperationsMyClassS *) getNSNullASeq:(ICECurrent *)__unused current
{
    return [NSArray arrayWithObjects:[TestOperationsA a:99], [NSNull null], nil];
}

-(TestOperationsStructS *) getNSNullStructSeq:(ICECurrent *)__unused current
{
    TestOperationsStructure *s = [TestOperationsStructure structure:nil e:TestOperationsenum2 s:[TestOperationsAnotherStruct anotherStruct:@"Hello"]];
    return [NSArray arrayWithObjects:s, [NSNull null], nil];
}

-(TestOperationsStringSS *) getNSNullStringSeqSeq:(ICECurrent *)__unused current
{
    TestOperationsStringSS *s = [NSArray arrayWithObjects:@"first", nil];
    return [NSArray arrayWithObjects:s, [NSNull null], nil];
}

-(TestOperationsStringStringD *) getNSNullStringStringDict:(ICECurrent *)__unused current
{
    TestOperationsMutableStringStringD *d = [TestOperationsMutableStringStringD dictionary];
    [d setObject:@"ONE" forKey:@"one"];
    [d setObject:[NSNull null] forKey:@"two"];
    return d;
}

-(void) putNSNullStringStringDict:(TestOperationsMutableStringStringD *)__unused d current:(ICECurrent *)__unused current
{
    // Nothing to do because this tests that an exception is thrown on the client side.
}

-(void) putNSNullShortIntDict:(TestOperationsMutableShortIntD *)__unused d current:(ICECurrent *)__unused current
{
    // Nothing to do because this tests that an exception is thrown on the client side.
}

-(void) putNSNullStringMyEnumDict:(TestOperationsMutableStringMyEnumD *)__unused d current:(ICECurrent *)__unused current
{
    // Nothing to do because this tests that an exception is thrown on the client side.
}

-(void) shutdown:(ICECurrent*)__unused current
{
    [[current.adapter getCommunicator] shutdown];
}

-(BOOL) supportsCompress:(ICECurrent*)__unused current
{
    return YES;
}
@end



@implementation TestOperationsBI

-(void) opB:(ICECurrent*)__unused current
{
}

-(void) opIntf:(ICECurrent*)__unused current
{
}

@end
