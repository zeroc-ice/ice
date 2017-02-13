// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <objects/TestI.h>
#import <ObjectsDerived.h>
#import <ObjectsDerivedEx.h>

@implementation TestObjectsTestIntfI
-(TestObjectsBase*) opDerived:(ICECurrent *)current
{
   TestObjectsDerived *d = ICE_AUTORELEASE([[TestObjectsDerived alloc] init]);
   d.theS.str = @"S.str";
   d.str = @"str";
   d.b = @"b";
   return d;
}

-(void) throwDerived:(ICECurrent *)current
{
   @throw [TestObjectsDerivedEx derivedEx:@"reason"];
}
@end
