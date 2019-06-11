//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

class EmptyI: Empty {}

func allTests(_ helper: TestHelper) throws -> GPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    output.write("testing Ice.Admin.Facets property... ")
    try test(communicator.getProperties().getPropertyAsList("Ice.Admin.Facets").count == 0)
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "foobar")
    var facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets")
    try test(facetFilter == ["foobar"])
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "foo\\'bar")
    facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets")
    try test(facetFilter == ["foo'bar"])
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "'foo bar' toto 'titi'")
    facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets")
    try test(facetFilter == ["foo bar", "toto", "titi"])
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "'foo bar\\' toto' 'titi'")
    facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets")
    try test(facetFilter == ["foo bar' toto", "titi"])
    // communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
    // facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
    // test(facetFilter.Length == 0);
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "")
    output.writeLine("ok")

    output.write("testing facet registration exceptions... ")
    communicator.getProperties().setProperty(key: "FacetExceptionTestAdapter.Endpoints", value: "tcp -h *")
    let adapter = try communicator.createObjectAdapter("FacetExceptionTestAdapter")
    let obj = EmptyI()
    _ = try adapter.add(servant: EmptyDisp(obj), id: Ice.stringToIdentity("d"))
    _ = try adapter.addFacet(servant: EmptyDisp(obj), id: Ice.stringToIdentity("d"), facet: "facetABCD")
    do {
        _ = try adapter.addFacet(servant: EmptyDisp(obj), id: Ice.stringToIdentity("d"), facet: "facetABCD")
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}

    _ = try adapter.removeFacet(id: Ice.stringToIdentity("d"), facet: "facetABCD")
    do {
        _ = try adapter.removeFacet(id: Ice.stringToIdentity("d"), facet: "facetABCD")
        try test(false)
    } catch is Ice.NotRegisteredException {}
    output.writeLine("ok")

    output.write("testing removeAllFacets... ")
    let obj1 = EmptyI()
    let obj2 = EmptyI()
    _ = try adapter.addFacet(servant: EmptyDisp(obj1), id: Ice.stringToIdentity("id1"), facet: "f1")
    _ = try adapter.addFacet(servant: EmptyDisp(obj2), id: Ice.stringToIdentity("id1"), facet: "f2")
    let obj3 = EmptyI()
    _ = try adapter.addFacet(servant: EmptyDisp(obj1), id: Ice.stringToIdentity("id2"), facet: "f1")
    _ = try adapter.addFacet(servant: EmptyDisp(obj2), id: Ice.stringToIdentity("id2"), facet: "f2")
    _ = try adapter.addFacet(servant: EmptyDisp(obj3), id: Ice.stringToIdentity("id2"), facet: "")
    var fm = try adapter.removeAllFacets(Ice.stringToIdentity("id1"))
    try test(fm.count == 2)
    try test((fm["f1"] as! EmptyDisp).servant as? EmptyI === obj1)
    try test((fm["f2"] as! EmptyDisp).servant as? EmptyI === obj2)
    do {
        _ = try adapter.removeAllFacets(Ice.stringToIdentity("id1"))
        try test(false)
    } catch is Ice.NotRegisteredException {}
    fm = try adapter.removeAllFacets(Ice.stringToIdentity("id2"))
    try test(fm.count == 3)
    try test((fm["f1"] as! EmptyDisp).servant as? EmptyI === obj1)
    try test((fm["f2"] as! EmptyDisp).servant as? EmptyI === obj2)
    try test((fm[""] as! EmptyDisp).servant as? EmptyI === obj3)
    output.writeLine("ok")

    adapter.deactivate()

    output.write("testing stringToProxy... ")

    let db = try communicator.stringToProxy("d:\(helper.getTestEndpoint(num: 0))")!
    output.writeLine("ok")

    output.write("testing unchecked cast... ")
    var prx = uncheckedCast(prx: db, type: ObjectPrx.self)
    try test(prx.ice_getFacet() == "")
    prx = uncheckedCast(prx: db, type: ObjectPrx.self, facet: "facetABCD")
    try test(prx.ice_getFacet() == "facetABCD")
    var prx2 = uncheckedCast(prx: prx, type: ObjectPrx.self)
    try test(prx2.ice_getFacet() == "facetABCD")
    var prx3 = uncheckedCast(prx: prx, type: ObjectPrx.self, facet: "")
    try test(prx3.ice_getFacet() == "")
    var d = uncheckedCast(prx: db, type: DPrx.self)
    try test(d.ice_getFacet() == "")
    var df = uncheckedCast(prx: db, type: DPrx.self, facet: "facetABCD")
    try test(df.ice_getFacet() == "facetABCD")
    var df2 = uncheckedCast(prx: df, type: DPrx.self)
    try test(df2.ice_getFacet() == "facetABCD")
    var df3 = uncheckedCast(prx: df, type: DPrx.self, facet: "")
    try test(df3.ice_getFacet() == "")
    output.writeLine("ok")

    output.write("testing checked cast... ")
    prx = try checkedCast(prx: db, type: ObjectPrx.self)!
    try test(prx.ice_getFacet() == "")
    prx = try checkedCast(prx: db, type: ObjectPrx.self, facet: "facetABCD")!
    try test(prx.ice_getFacet() == "facetABCD")
    prx2 = try checkedCast(prx: prx, type: ObjectPrx.self)!
    try test(prx2.ice_getFacet() == "facetABCD")
    prx3 = try checkedCast(prx: prx, type: ObjectPrx.self, facet: "")!
    try test(prx3.ice_getFacet() == "")
    d = try checkedCast(prx: db, type: DPrx.self)!
    try test(d.ice_getFacet() == "")
    df = try checkedCast(prx: db, type: DPrx.self, facet: "facetABCD")!
    try test(df.ice_getFacet() == "facetABCD")
    df2 = try checkedCast(prx: df, type: DPrx.self)!
    try test(df2.ice_getFacet() == "facetABCD")
    df3 = try checkedCast(prx: df, type: DPrx.self, facet: "")!
    try test(df3.ice_getFacet() == "")
    output.writeLine("ok")

    output.write("testing non-facets A, B, C, and D... ")
    d = try checkedCast(prx: db, type: DPrx.self)!
    try test(d == db)
    try test(d.callA() == "A")
    try test(d.callB() == "B")
    try test(d.callC() == "C")
    try test(d.callD() == "D")
    output.writeLine("ok")

    output.write("testing facets A, B, C, and D... ")
    df = try checkedCast(prx: d, type: DPrx.self, facet: "facetABCD")!
    try test(df.callA() == "A")
    try test(df.callB() == "B")
    try test(df.callC() == "C")
    try test(df.callD() == "D")
    output.writeLine("ok")

    output.write("testing facets E and F... ")
    let ff = try checkedCast(prx: d, type: FPrx.self, facet: "facetEF")!
    try test(ff.callE() == "E")
    try test(ff.callF() == "F")
    output.writeLine("ok")

    output.write("testing facet G... ")
    let gf = try checkedCast(prx: ff, type: GPrx.self, facet: "facetGH")!
    try test(gf.callG() == "G")
    output.writeLine("ok")

    output.write("testing whether casting preserves the facet... ")
    let hf = try checkedCast(prx: gf, type: HPrx.self)!
    try test(hf.callG() == "G")
    try test(hf.callH() == "H")
    output.writeLine("ok")
    return gf
}
