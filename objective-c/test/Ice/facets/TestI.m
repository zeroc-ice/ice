// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <facets/TestI.h>

@implementation TestFacetsAI
-(NSString*) callA:(ICECurrent*)current
{
    return @"A";
}
@end

@implementation TestFacetsBI
-(NSString*) callA:(ICECurrent*)current
{
    return @"A";
}
-(NSString*) callB:(ICECurrent*)current
{
    return @"B";
}
@end

@implementation TestFacetsCI
-(NSString*) callA:(ICECurrent*)current
{
    return @"A";
}
-(NSString*) callC:(ICECurrent*)current
{
    return @"C";
}
@end

@implementation TestFacetsDI
-(NSString*) callA:(ICECurrent*)current
{
    return @"A";
}
-(NSString*) callB:(ICECurrent*)current
{
    return @"B";
}
-(NSString*) callC:(ICECurrent*)current
{
    return @"C";
}
-(NSString*) callD:(ICECurrent*)current
{
    return @"D";
}
@end

@implementation TestFacetsEI
-(NSString*) callE:(ICECurrent*)current
{
    return @"E";
}
@end

@implementation TestFacetsFI
-(NSString*) callE:(ICECurrent*)current
{
    return @"E";
}
-(NSString*) callF:(ICECurrent*)current
{
    return @"F";
}
@end

@implementation TestFacetsGI
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
-(NSString*) callG:(ICECurrent*)current
{
    return @"G";
}
@end

@implementation TestFacetsHI
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
-(NSString*) callG:(ICECurrent*)current
{
    return @"G";
}
-(NSString*) callH:(ICECurrent*)current
{
    return @"H";
}
@end
