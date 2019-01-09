// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice.h>
#import <facets/TestI.h>

@implementation TestFacetsAI
-(NSString*) callA:(ICECurrent*)__unused current
{
    return @"A";
}
@end

@implementation TestFacetsBI
-(NSString*) callA:(ICECurrent*)__unused current
{
    return @"A";
}
-(NSString*) callB:(ICECurrent*)__unused current
{
    return @"B";
}
@end

@implementation TestFacetsCI
-(NSString*) callA:(ICECurrent*)__unused current
{
    return @"A";
}
-(NSString*) callC:(ICECurrent*)__unused current
{
    return @"C";
}
@end

@implementation TestFacetsDI
-(NSString*) callA:(ICECurrent*)__unused current
{
    return @"A";
}
-(NSString*) callB:(ICECurrent*)__unused current
{
    return @"B";
}
-(NSString*) callC:(ICECurrent*)__unused current
{
    return @"C";
}
-(NSString*) callD:(ICECurrent*)__unused current
{
    return @"D";
}
@end

@implementation TestFacetsEI
-(NSString*) callE:(ICECurrent*)__unused current
{
    return @"E";
}
@end

@implementation TestFacetsFI
-(NSString*) callE:(ICECurrent*)__unused current
{
    return @"E";
}
-(NSString*) callF:(ICECurrent*)__unused current
{
    return @"F";
}
@end

@implementation TestFacetsGI
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
-(NSString*) callG:(ICECurrent*)__unused current
{
    return @"G";
}
@end

@implementation TestFacetsHI
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
-(NSString*) callG:(ICECurrent*)__unused current
{
    return @"G";
}
-(NSString*) callH:(ICECurrent*)__unused current
{
    return @"H";
}
@end
