// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <FacetsTest.h>


@interface FacetsEmptyI : TestFacetsEmpty
@end

@implementation FacetsEmptyI
@end

id<TestFacetsGPrx>
facetsAllTests(id<ICECommunicator> communicator)
{
//     tprintf("testing Ice.Admin.Facets property... ");
//     test([[[communicator getProperties] getPropertyAsList:@"Ice.Admin.Facets"] length] > 0);
//     [[communicator getProperties] setProperty:@"Ice.Admin.Facets" value:@"foobar"];
//     ICEStringSeq facetFilter = [[communicator getProperties] getPropertyAsList:@"Ice.Admin.Facets"];
//     test([[facetFilter count] == 1 && [facetFilter objectAtIndex:0] isEqualToString:@"foobar"]);
//     [[communicator getProperties] setProperty:@"Ice.Admin.Facets" value:@"foo'bar"];
//     facetFilter = [[communicator getProperties] getPropertyAsList:@"Ice.Admin.Facets"];
//     test([[facetFilter count] == 1 && [facetFilter objectAtIndex:0] isEqualToString:@"foo'bar"]);
//     [[communicator getProperties] setProperty:@"Ice.Admin.Facets" value:@"'foo bar' toto 'titi'"];
//     facetFilter = [[communicator getProperties] getPropertyAsList:@"Ice.Admin.Facets"];
//     test([[facetFilter count] == 3 && [facetFilter objectAtIndex:0] isEqualToString:@"foo bar" && [facetFilter objectAtIndex:1]:isEqualToString:@"toto" && [facetFilter objectAtIndex:2]:isEqualToString:@"titi"]);
//     [[communicator getProperties] setProperty:@"Ice.Admin.Facets" value:@"'foo bar\\' toto' 'titi'"];
//     facetFilter = [[communicator getProperties] getPropertyAsList:@"Ice.Admin.Facets"];
//     test([[facetFilter count] == 2 && [facetFilter objectAtIndex:0] isEqualToString:@"foo bar' toto" && [facetFilter objectAtIndex:1]:isEqualToString:@"titi"]);
//     // [[communicator getProperties] setProperty:@"Ice.Admin.Facets" value:@"'foo bar' 'toto titi"];
//     // facetFilter = [[communicator getProperties] getPropertyAsList:@"Ice.Admin.Facets"];
//     // test([facetFilter count] == 0);
//     [[communicator getProperties] setProperty:@"Ice.Admin.Facets" value:@""];
//     tprintf("ok\n");

    tprintf("testing facet registration exceptions... ");
    [[communicator getProperties] setProperty:@"FacetExceptionTestAdapter.Endpoints" value:@"default"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"FacetExceptionTestAdapter"];
    ICEObject* obj = [FacetsEmptyI empty];
    [adapter add:obj identity:[ICEUtil stringToIdentity:@"d"]];
    [adapter addFacet:obj identity:[ICEUtil stringToIdentity:@"d"] facet:@"facetABCD"];
    @try
    {
        [adapter addFacet:obj identity:[ICEUtil stringToIdentity:@"d"] facet:@"facetABCD"];
        test(NO);
    }
    @catch(ICEAlreadyRegisteredException*)
    {
    }
    [adapter removeFacet:[ICEUtil stringToIdentity:@"d"] facet:@"facetABCD"];
    @try
    {
        [adapter removeFacet:[ICEUtil stringToIdentity:@"d"] facet:@"facetABCD"];
        test(NO);
    }
    @catch(ICENotRegisteredException*)
    {
    }
    tprintf("ok\n");

    tprintf("testing removeAllFacets... ");
    ICEObject* obj1 = [FacetsEmptyI empty];
    ICEObject* obj2 = [FacetsEmptyI empty];

    [adapter addFacet:obj1 identity:[ICEUtil stringToIdentity:@"id1"] facet:@"f1"];
    [adapter addFacet:obj2 identity:[ICEUtil stringToIdentity:@"id1"] facet:@"f2"];

    ICEObject* obj3 = [FacetsEmptyI empty];

    [adapter addFacet:obj1 identity:[ICEUtil stringToIdentity:@"id2"] facet:@"f1"];
    [adapter addFacet:obj2 identity:[ICEUtil stringToIdentity:@"id2"] facet:@"f2"];
    [adapter addFacet:obj3 identity:[ICEUtil stringToIdentity:@"id2"] facet:@""];
    NSDictionary* fm = [adapter removeAllFacets:[ICEUtil stringToIdentity:@"id1"]];
    test([fm count] == 2);
    test([fm objectForKey:@"f1"] == obj1);
    test([fm objectForKey:@"f2"] == obj2);
    @try
    {
        [adapter removeAllFacets:[ICEUtil stringToIdentity:@"id1"]];
        test(NO);
    }
    @catch(ICENotRegisteredException*)
    {
    }
    fm = [adapter removeAllFacets:[ICEUtil stringToIdentity:@"id2"]];
    test([fm count] == 3);
    test([fm objectForKey:@"f1"] == obj1);
    test([fm objectForKey:@"f2"] == obj2);
    test([fm objectForKey:@""] == obj3);
    tprintf("ok\n");

    [adapter deactivate];

    tprintf("testing stringToProxy... ");
    NSString* ref = @"d:default -p 12010";
    id<ICEObjectPrx> db = [communicator stringToProxy:ref];
    test(db);
    tprintf("ok\n");

    tprintf("testing unchecked cast... ");
    id<ICEObjectPrx> prx = [ICEObjectPrx uncheckedCast:db];
    test([[prx ice_getFacet] length] == 0);
    prx = [ICEObjectPrx uncheckedCast:db facet:@"facetABCD"];
    test([[prx ice_getFacet] isEqualToString:@"facetABCD"]);
    id<ICEObjectPrx> prx2 = [ICEObjectPrx uncheckedCast:prx];
    test([[prx2 ice_getFacet] isEqualToString:@"facetABCD"]);
    id<ICEObjectPrx> prx3 = [ICEObjectPrx uncheckedCast:prx facet:@""];
    test([[prx3 ice_getFacet] length] == 0);
    id<TestFacetsDPrx> d = [TestFacetsDPrx uncheckedCast:db];
    test([[d ice_getFacet] length] == 0);
    id<TestFacetsDPrx> df = [TestFacetsDPrx uncheckedCast:db facet:@"facetABCD"];
    test([[df ice_getFacet] isEqualToString:@"facetABCD"]);
    id<TestFacetsDPrx> df2 = [TestFacetsDPrx uncheckedCast:df];
    test([[df2 ice_getFacet] isEqualToString:@"facetABCD"]);
    id<TestFacetsDPrx> df3 = [TestFacetsDPrx uncheckedCast:df facet:@""];
    test([[df3 ice_getFacet] length] == 0);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    prx = [ICEObjectPrx checkedCast:db];
    test([[prx ice_getFacet] length] == 0);
    prx = [ICEObjectPrx checkedCast:db facet:@"facetABCD"];
    test([[prx ice_getFacet] isEqualToString:@"facetABCD"]);
    prx2 = [ICEObjectPrx checkedCast:prx];
    test([[prx2 ice_getFacet] isEqualToString:@"facetABCD"]);
    prx3 = [ICEObjectPrx checkedCast:prx facet:@""];
    test([[prx3 ice_getFacet] length] == 0);
    d = [TestFacetsDPrx checkedCast:db];
    test([[d ice_getFacet] length] == 0);
    df = [TestFacetsDPrx checkedCast:db facet:@"facetABCD"];
    test([[df ice_getFacet] isEqualToString:@"facetABCD"]);
    df2 = [TestFacetsDPrx checkedCast:df];
    test([[df2 ice_getFacet] isEqualToString:@"facetABCD"]);
    df3 = [TestFacetsDPrx checkedCast:df facet:@""];
    test([[df3 ice_getFacet] length] == 0);
    tprintf("ok\n");

    tprintf("testing non-facets A, B, C, and D... ");
    d = [TestFacetsDPrx checkedCast:db];
    test(d);
    test([d isEqual:db]);
    test([[d callA] isEqualToString:@"A"]);
    test([[d callB] isEqualToString:@"B"]);
    test([[d callC] isEqualToString:@"C"]);
    test([[d callD] isEqualToString:@"D"]);
    tprintf("ok\n");

    tprintf("testing facets A, B, C, and D... ");
    df = [TestFacetsDPrx checkedCast:d facet:@"facetABCD"];
    test(df);
    test([[df callA] isEqualToString:@"A"]);
    test([[df callB] isEqualToString:@"B"]);
    test([[df callC] isEqualToString:@"C"]);
    test([[df callD] isEqualToString:@"D"]);
    tprintf("ok\n");

    tprintf("testing facets E and F... ");
    id<TestFacetsFPrx> ff = [TestFacetsFPrx checkedCast:d facet:@"facetEF"];
    test(ff);
    test([[ff callE] isEqualToString:@"E"]);
    test([[ff callF] isEqualToString:@"F"]);
    tprintf("ok\n");

    tprintf("testing facet G... ");
    id<TestFacetsGPrx> gf = [TestFacetsGPrx checkedCast:ff facet:@"facetGH"];
    test(gf);
    test([[gf callG] isEqualToString:@"G"]);
    tprintf("ok\n");

    tprintf("testing whether casting preserves the facet... ");
    id<TestFacetsHPrx> hf = [TestFacetsHPrx checkedCast:gf];
    test(hf);
    test([[hf callG] isEqualToString:@"G"]);
    test([[hf callH] isEqualToString:@"H"]);
    tprintf("ok\n");

    return gf;
}
