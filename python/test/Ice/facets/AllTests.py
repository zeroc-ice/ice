#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice, Test, sys

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class EmptyI(Test.Empty):
    pass

def allTests(helper, communicator):

    sys.stdout.write("testing Ice.Admin.Facets property... ")
    sys.stdout.flush()
    test(len(communicator.getProperties().getPropertyAsList("Ice.Admin.Facets")) == 0)
    communicator.getProperties().setProperty("Ice.Admin.Facets", "foobar");
    facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
    test(len(facetFilter) == 1 and facetFilter[0] == "foobar");
    communicator.getProperties().setProperty("Ice.Admin.Facets", "foo\\'bar");
    facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
    test(len(facetFilter) == 1 and facetFilter[0] == "foo'bar");
    communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
    facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
    test(len(facetFilter) == 3 and facetFilter[0] == "foo bar" and facetFilter[1] == "toto"
         and facetFilter[2] == "titi");
    communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
    facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
    test(len(facetFilter) == 2 and facetFilter[0] == "foo bar' toto" and facetFilter[1] == "titi");
    # communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
    # facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
    # test(len(facetFilter) == 0);
    communicator.getProperties().setProperty("Ice.Admin.Facets", "");
    print("ok")

    sys.stdout.write("testing facet registration exceptions... ")
    sys.stdout.flush()
    communicator.getProperties().setProperty("FacetExceptionTestAdapter.Endpoints", "tcp -h *")
    adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter")
    obj = EmptyI()
    adapter.add(obj, Ice.stringToIdentity("d"))
    adapter.addFacet(obj, Ice.stringToIdentity("d"), "facetABCD")
    try:
        adapter.addFacet(obj, Ice.stringToIdentity("d"), "facetABCD")
        test(False)
    except Ice.AlreadyRegisteredException:
        pass
    adapter.removeFacet(Ice.stringToIdentity("d"), "facetABCD")
    try:
        adapter.removeFacet(Ice.stringToIdentity("d"), "facetABCD")
        test(False)
    except Ice.NotRegisteredException:
        pass
    print("ok")

    sys.stdout.write("testing removeAllFacets... ")
    sys.stdout.flush()
    obj1 = EmptyI()
    obj2 = EmptyI()
    adapter.addFacet(obj1, Ice.stringToIdentity("id1"), "f1")
    adapter.addFacet(obj2, Ice.stringToIdentity("id1"), "f2")
    obj3 = EmptyI()
    adapter.addFacet(obj1, Ice.stringToIdentity("id2"), "f1")
    adapter.addFacet(obj2, Ice.stringToIdentity("id2"), "f2")
    adapter.addFacet(obj3, Ice.stringToIdentity("id2"), "")
    fm = adapter.removeAllFacets(Ice.stringToIdentity("id1"))
    test(len(fm) == 2)
    test(fm["f1"] == obj1)
    test(fm["f2"] == obj2)
    try:
        adapter.removeAllFacets(Ice.stringToIdentity("id1"))
        test(False)
    except Ice.NotRegisteredException:
        pass
    fm = adapter.removeAllFacets(Ice.stringToIdentity("id2"))
    test(len(fm) == 3)
    test(fm["f1"] == obj1)
    test(fm["f2"] == obj2)
    test(fm[""] == obj3)
    print("ok")

    adapter.deactivate()

    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    ref = "d:{0}".format(helper.getTestEndpoint())
    db = communicator.stringToProxy(ref)
    test(db)
    print("ok")

    sys.stdout.write("testing unchecked cast... ")
    sys.stdout.flush()
    obj = Ice.ObjectPrx.uncheckedCast(db)
    test(obj.ice_getFacet() == "")
    obj = Ice.ObjectPrx.uncheckedCast(db, "facetABCD")
    test(obj.ice_getFacet() == "facetABCD")
    obj2 = Ice.ObjectPrx.uncheckedCast(obj)
    test(obj2.ice_getFacet() == "facetABCD")
    obj3 = Ice.ObjectPrx.uncheckedCast(obj, "")
    test(obj3.ice_getFacet() == "")
    d = Test.DPrx.uncheckedCast(db)
    test(d.ice_getFacet() == "")
    df = Test.DPrx.uncheckedCast(db, "facetABCD")
    test(df.ice_getFacet() == "facetABCD")
    df2 = Test.DPrx.uncheckedCast(df)
    test(df2.ice_getFacet() == "facetABCD")
    df3 = Test.DPrx.uncheckedCast(df, "")
    test(df3.ice_getFacet() == "")
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    obj = Ice.ObjectPrx.checkedCast(db)
    test(obj.ice_getFacet() == "")
    obj = Ice.ObjectPrx.checkedCast(db, "facetABCD")
    test(obj.ice_getFacet() == "facetABCD")
    obj2 = Ice.ObjectPrx.checkedCast(obj)
    test(obj2.ice_getFacet() == "facetABCD")
    obj3 = Ice.ObjectPrx.checkedCast(obj, "")
    test(obj3.ice_getFacet() == "")
    d = Test.DPrx.checkedCast(db)
    test(d.ice_getFacet() == "")
    df = Test.DPrx.checkedCast(db, "facetABCD")
    test(df.ice_getFacet() == "facetABCD")
    df2 = Test.DPrx.checkedCast(df)
    test(df2.ice_getFacet() == "facetABCD")
    df3 = Test.DPrx.checkedCast(df, "")
    test(df3.ice_getFacet() == "")
    test(Test.DPrx.checkedCast(df, "bogus") == None)
    print("ok")

    sys.stdout.write("testing non-facets A, B, C, and D... ")
    sys.stdout.flush()
    d = Test.DPrx.checkedCast(db)
    test(d)
    test(d == db)
    test(d.callA() == "A")
    test(d.callB() == "B")
    test(d.callC() == "C")
    test(d.callD() == "D")
    print("ok")

    sys.stdout.write("testing facets A, B, C, and D... ")
    sys.stdout.flush()
    df = Test.DPrx.checkedCast(d, "facetABCD")
    test(df)
    test(df.callA() == "A")
    test(df.callB() == "B")
    test(df.callC() == "C")
    test(df.callD() == "D")
    print("ok")

    sys.stdout.write("testing facets E and F... ")
    sys.stdout.flush()
    ff = Test.FPrx.checkedCast(d, "facetEF")
    test(ff)
    test(ff.callE() == "E")
    test(ff.callF() == "F")
    print("ok")

    sys.stdout.write("testing facet G... ")
    sys.stdout.flush()
    gf = Test.GPrx.checkedCast(ff, "facetGH")
    test(gf)
    test(gf.callG() == "G")
    print("ok")

    sys.stdout.write("testing whether casting preserves the facet... ")
    sys.stdout.flush()
    hf = Test.HPrx.checkedCast(gf)
    test(hf)
    test(hf.callG() == "G")
    test(hf.callH() == "H")
    print("ok")

    return gf
