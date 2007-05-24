#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class EmptyI(Test.Empty):
    pass

def allTests(communicator):
    print "testing facet registration exceptions... ",
    communicator.getProperties().setProperty("FacetExceptionTestAdapter.Endpoints", "default")
    adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter")
    obj = EmptyI()
    adapter.add(obj, communicator.stringToIdentity("d"))
    adapter.addFacet(obj, communicator.stringToIdentity("d"), "facetABCD")
    try:
        adapter.addFacet(obj, communicator.stringToIdentity("d"), "facetABCD")
        test(false)
    except Ice.AlreadyRegisteredException:
        pass
    adapter.removeFacet(communicator.stringToIdentity("d"), "facetABCD")
    try:
        adapter.removeFacet(communicator.stringToIdentity("d"), "facetABCD")
        test(false)
    except Ice.NotRegisteredException:
        pass
    print "ok"

    print "testing removeAllFacets... ",
    obj1 = EmptyI()
    obj2 = EmptyI()
    adapter.addFacet(obj1, communicator.stringToIdentity("id1"), "f1")
    adapter.addFacet(obj2, communicator.stringToIdentity("id1"), "f2")
    obj3 = EmptyI()
    adapter.addFacet(obj1, communicator.stringToIdentity("id2"), "f1")
    adapter.addFacet(obj2, communicator.stringToIdentity("id2"), "f2")
    adapter.addFacet(obj3, communicator.stringToIdentity("id2"), "")
    fm = adapter.removeAllFacets(communicator.stringToIdentity("id1"))
    test(len(fm) == 2)
    test(fm["f1"] == obj1)
    test(fm["f2"] == obj2)
    try:
        adapter.removeAllFacets(communicator.stringToIdentity("id1"))
        test(false)
    except Ice.NotRegisteredException:
        pass
    fm = adapter.removeAllFacets(communicator.stringToIdentity("id2"))
    test(len(fm) == 3)
    test(fm["f1"] == obj1)
    test(fm["f2"] == obj2)
    test(fm[""] == obj3)
    print "ok"

    adapter.deactivate()

    print "testing stringToProxy... ",
    ref = "d:default -p 12010 -t 10000"
    db = communicator.stringToProxy(ref)
    test(db)
    print "ok"

    print "testing unchecked cast... ",
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
    print "ok"

    print "testing checked cast... ",
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
    print "ok"

    print "testing non-facets A, B, C, and D... ",
    d = Test.DPrx.checkedCast(db)
    test(d)
    test(d == db)
    test(d.callA() == "A")
    test(d.callB() == "B")
    test(d.callC() == "C")
    test(d.callD() == "D")
    print "ok"

    print "testing facets A, B, C, and D... ",
    df = Test.DPrx.checkedCast(d, "facetABCD")
    test(df)
    test(df.callA() == "A")
    test(df.callB() == "B")
    test(df.callC() == "C")
    test(df.callD() == "D")
    print "ok"

    print "testing facets E and F... ",
    ff = Test.FPrx.checkedCast(d, "facetEF")
    test(ff)
    test(ff.callE() == "E")
    test(ff.callF() == "F")
    print "ok"

    print "testing facet G... ",
    gf = Test.GPrx.checkedCast(ff, "facetGH")
    test(gf)
    test(gf.callG() == "G")
    print "ok"

    print "testing whether casting preserves the facet... ",
    hf = Test.HPrx.checkedCast(gf)
    test(hf)
    test(hf.callG() == "G")
    test(hf.callH() == "H")
    print "ok"

    return gf
