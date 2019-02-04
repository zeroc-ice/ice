//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <optional/TestI.h>
#import <objc/Ice.h>

#import <Foundation/NSThread.h>

@implementation InitialI
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
-(ICEObject*) pingPong:(ICEObject*)obj current:(ICECurrent*)__unused current
{
    return obj;
}
-(void) opOptionalException:(id)a b:(id)b o:(id)o current:(ICECurrent *)__unused current
{
    @throw [TestOptionalOptionalException optionalException:NO a:a b:b o:o];
}
-(void) opDerivedException:(id)a b:(id)b o:(id)o current:(ICECurrent *)__unused current
{
    @throw [TestOptionalDerivedException derivedException:NO a:a b:b o:o ss:b o2:o];
}
-(void) opRequiredException:(id)a b:(id)b o:(id)o current:(ICECurrent *)__unused current
{
    TestOptionalRequiredException* ex = [TestOptionalRequiredException requiredException];
    if(a != ICENone)
    {
        ex.a = [a intValue];
    }
    else
    {
        [ex clearA];
    }
    if(b != ICENone)
    {
        ex.b = b;
    }
    else
    {
        [ex clearB];
    }
    if(o != ICENone)
    {
        ex.o = o;
        ex.o2 = o;
    }
    else
    {
        [ex clearO];
    }
    if(b != ICENone)
    {
        ex.ss = b;
    }
    @throw ex;
}
-(id) opByte:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opBool:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opShort:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opInt:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opLong:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opFloat:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opDouble:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opString:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opMyEnum:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opSmallStruct:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opFixedStruct:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opVarStruct:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opOneOptional:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opOneOptionalProxy:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opByteSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opBoolSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opShortSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opIntSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opLongSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opFloatSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opDoubleSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opStringSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opSmallStructSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opSmallStructList:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opFixedStructSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opFixedStructList:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opVarStructSeq:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opSerializable:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opIntIntDict:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opStringIntDict:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(id) opIntOneOptionalDict:(id)p1 p3:(id *)p3 current:(ICECurrent *)__unused current
{
    *p3 = p1;
    return p1;
}
-(void) opClassAndUnknownOptional:(TestOptionalA *)__unused p current:(ICECurrent *)__unused current
{
}
-(void) sendOptionalClass:(BOOL)__unused req o:(id)__unused o current:(ICECurrent *)__unused current
{
}
-(void) returnOptionalClass:(BOOL)__unused req o:(id *)o current:(ICECurrent *)__unused current
{
    *o = [TestOptionalOneOptional oneOptional:@53];
}
-(id) opG:(id)g current:(ICECurrent*)__unused current
{
    return g;
}
-(void) opVoid:(ICECurrent*)__unused current
{
}
-(id) opMStruct1:(ICECurrent *)__unused current
{
    return [TestOptionalSmallStruct smallStruct];
}
-(id) opMStruct2:(id)p1 p2:(id*)p2 current:(ICECurrent *)__unused current
{
    *p2 = p1;
    return p1;
}
-(id) opMSeq1:(ICECurrent *)__unused current
{
    return [TestOptionalStringSeq array];
}
-(id) opMSeq2:(id)p1 p2:(id*)p2 current:(ICECurrent *)__unused current
{
    *p2 = p1;
    return p1;
}
-(id) opMDict1:(ICECurrent *)__unused current
{
    return [TestOptionalStringIntDict dictionary];
}
-(id) opMDict2:(id)p1 p2:(id*)p2 current:(ICECurrent *)__unused current
{
    *p2 = p1;
    return p1;
}
-(id) opMG1:(ICECurrent *)__unused current
{
    return [TestOptionalG g];
}
-(id) opMG2:(id)p1 p2:(id*)p2 current:(ICECurrent *)__unused current
{
    *p2 = p1;
    return p1;
}
-(BOOL) supportsRequiredParams:(ICECurrent*)__unused current
{
    return NO;
}
-(BOOL) supportsJavaSerializable:(ICECurrent*)__unused current
{
    return NO;
}
-(BOOL) supportsCsharpSerializable:(ICECurrent*)__unused current
{
    return NO;
}
-(BOOL) supportsCppStringView:(ICECurrent*)__unused current
{
    return NO;
}
-(BOOL) supportsNullOptional:(ICECurrent*)__unused current
{
    return YES;
}

@end
