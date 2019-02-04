//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <objects/TestI.h>
#import <ObjectsDerived.h>
#import <ObjectsDerivedEx.h>

@implementation TestObjectsTestIntfI
-(TestObjectsBase*) opDerived:(ICECurrent *)__unused current
{
   TestObjectsDerived *d = ICE_AUTORELEASE([[TestObjectsDerived alloc] init]);
   d.theS.str = @"S.str";
   d.str = @"str";
   d.b = @"b";
   return d;
}

-(void) throwDerived:(ICECurrent *)__unused current
{
   @throw [TestObjectsDerivedEx derivedEx:@"reason"];
}
@end
