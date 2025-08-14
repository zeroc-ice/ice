// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

final class EmptyI: Empty {}

func allTests(_ helper: TestHelper) async throws -> GPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    output.write("testing Ice.Admin.Facets property... ")
    try test(communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets").count == 0)
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "foobar")
    var facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets")
    try test(facetFilter == ["foobar"])
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "foo\\'bar")
    facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets")
    try test(facetFilter == ["foo'bar"])
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "'foo bar' toto 'titi'")
    facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets")
    try test(facetFilter == ["foo bar", "toto", "titi"])
    communicator.getProperties().setProperty(
        key: "Ice.Admin.Facets", value: "'foo bar\\' toto' 'titi'")
    facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets")
    try test(facetFilter == ["foo bar' toto", "titi"])
    // communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
    // facetFilter = try communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
    // test(facetFilter.Length == 0);
    communicator.getProperties().setProperty(key: "Ice.Admin.Facets", value: "")
    output.writeLine("ok")

    do {
        output.write("testing add facet with uuid... ")
        let testAdapter = try communicator.createObjectAdapterWithEndpoints(
            name: "TestAdapter2",
            endpoints: "default")
        try test(testAdapter.addFacetWithUUID(servant: EmptyI(), facet: "facetABCD").ice_getFacet() == "facetABCD")
        testAdapter.destroy()
        output.writeLine("ok")
    }

    output.write("testing facet registration exceptions... ")
    communicator.getProperties().setProperty(
        key: "FacetExceptionTestAdapter.Endpoints", value: "tcp -h *")
    let adapter = try communicator.createObjectAdapter("FacetExceptionTestAdapter")
    let obj = EmptyI()
    _ = try adapter.add(servant: obj, id: Ice.stringToIdentity("d"))
    _ = try adapter.addFacet(
        servant: obj, id: Ice.stringToIdentity("d"), facet: "facetABCD")
    do {
        _ = try adapter.addFacet(
            servant: obj, id: Ice.stringToIdentity("d"), facet: "facetABCD")
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
    _ = try adapter.addFacet(servant: obj1, id: Ice.stringToIdentity("id1"), facet: "f1")
    _ = try adapter.addFacet(servant: obj2, id: Ice.stringToIdentity("id1"), facet: "f2")
    let obj3 = EmptyI()
    _ = try adapter.addFacet(servant: obj1, id: Ice.stringToIdentity("id2"), facet: "f1")
    _ = try adapter.addFacet(servant: obj2, id: Ice.stringToIdentity("id2"), facet: "f2")
    _ = try adapter.addFacet(servant: obj3, id: Ice.stringToIdentity("id2"), facet: "")
    var fm = try adapter.removeAllFacets(Ice.stringToIdentity("id1"))
    try test(fm.count == 2)
    try test(fm["f1"] as? EmptyI === obj1)
    try test(fm["f2"] as? EmptyI === obj2)
    do {
        _ = try adapter.removeAllFacets(Ice.stringToIdentity("id1"))
        try test(false)
    } catch is Ice.NotRegisteredException {}
    fm = try adapter.removeAllFacets(Ice.stringToIdentity("id2"))
    try test(fm.count == 3)
    try test(fm["f1"] as? EmptyI === obj1)
    try test(fm["f2"] as? EmptyI === obj2)
    try test(fm[""] as? EmptyI === obj3)
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
    prx = try await checkedCast(prx: db, type: ObjectPrx.self)!
    try test(prx.ice_getFacet() == "")
    prx = try await checkedCast(prx: db, type: ObjectPrx.self, facet: "facetABCD")!
    try test(prx.ice_getFacet() == "facetABCD")
    prx2 = try await checkedCast(prx: prx, type: ObjectPrx.self)!
    try test(prx2.ice_getFacet() == "facetABCD")
    prx3 = try await checkedCast(prx: prx, type: ObjectPrx.self, facet: "")!
    try test(prx3.ice_getFacet() == "")
    d = try await checkedCast(prx: db, type: DPrx.self)!
    try test(d.ice_getFacet() == "")
    df = try await checkedCast(prx: db, type: DPrx.self, facet: "facetABCD")!
    try test(df.ice_getFacet() == "facetABCD")
    df2 = try await checkedCast(prx: df, type: DPrx.self)!
    try test(df2.ice_getFacet() == "facetABCD")
    df3 = try await checkedCast(prx: df, type: DPrx.self, facet: "")!
    try test(df3.ice_getFacet() == "")
    output.writeLine("ok")

    output.write("testing non-facets A, B, C, and D... ")
    d = try await checkedCast(prx: db, type: DPrx.self)!
    try test(d == db)
    try await test(d.callA() == "A")
    try await test(d.callB() == "B")
    try await test(d.callC() == "C")
    try await test(d.callD() == "D")
    output.writeLine("ok")

    output.write("testing facets A, B, C, and D... ")
    df = try await checkedCast(prx: d, type: DPrx.self, facet: "facetABCD")!
    try await test(df.callA() == "A")
    try await test(df.callB() == "B")
    try await test(df.callC() == "C")
    try await test(df.callD() == "D")
    output.writeLine("ok")

    output.write("testing facets E and F... ")
    let ff = try await checkedCast(prx: d, type: FPrx.self, facet: "facetEF")!
    try await test(ff.callE() == "E")
    try await test(ff.callF() == "F")
    output.writeLine("ok")

    output.write("testing facet G... ")
    let gf = try await checkedCast(prx: ff, type: GPrx.self, facet: "facetGH")!
    try await test(gf.callG() == "G")
    output.writeLine("ok")

    output.write("testing whether casting preserves the facet... ")
    let hf = try await checkedCast(prx: gf, type: HPrx.self)!
    try await test(hf.callG() == "G")
    try await test(hf.callH() == "H")
    output.writeLine("ok")
    return gf
}
