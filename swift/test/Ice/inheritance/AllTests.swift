//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

func allTests(_ helper: TestHelper) throws -> InitialPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    let base = try communicator.stringToProxy("initial:\(helper.getTestEndpoint(num: 0))")!

    output.write("testing checked cast... ")
    let initial = try checkedCast(prx: base, type: InitialPrx.self)!
    try test(initial == base)
    output.writeLine("ok")

    output.write("getting proxies for interface hierarchy... ")
    let ia = try initial.iaop()!
    let ib1 = try initial.ib1op()!
    let ib2 = try initial.ib2op()!
    let ic = try initial.icop()!
    try test(ia !== ib1)
    try test(ia !== ib2)
    try test(ia !== ic)
    try test(ib1 !== ic)
    try test(ib2 !== ic)
    output.writeLine("ok")

    output.write("invoking proxy operations on interface hierarchy... ")

    var iao = try ia.iaop(ia)!
    try test(iao == ia)
    iao = try ia.iaop(ib1)!
    try test(iao == ib1)
    iao = try ia.iaop(ib2)!
    try test(iao == ib2)
    iao = try ia.iaop(ic)!
    try test(iao == ic)
    iao = try ib1.iaop(ia)!
    try test(iao == ia)
    iao = try ib1.iaop(ib1)!
    try test(iao == ib1)
    iao = try ib1.iaop(ib2)!
    try test(iao == ib2)
    iao = try ib1.iaop(ic)!
    try test(iao == ic)
    iao = try ib2.iaop(ia)!
    try test(iao == ia)
    iao = try ib2.iaop(ib1)!
    try test(iao == ib1)
    iao = try ib2.iaop(ib2)!
    try test(iao == ib2)
    iao = try ib2.iaop(ic)!
    try test(iao == ic)
    iao = try ic.iaop(ia)!
    try test(iao == ia)
    iao = try ic.iaop(ib1)!
    try test(iao == ib1)
    iao = try ic.iaop(ib2)!
    try test(iao == ib2)
    iao = try ic.iaop(ic)!
    try test(iao == ic)

    iao = try ib1.ib1op(ib1)!
    try test(iao == ib1)
    var ib1o = try ib1.ib1op(ib1)!
    try test(ib1o == ib1)
    iao = try ib1.ib1op(ic)!
    try test(iao == ic)
    ib1o = try ib1.ib1op(ic)!
    try test(ib1o == ic)
    iao = try ic.ib1op(ib1)!
    try test(iao == ib1)
    ib1o = try ic.ib1op(ib1)!
    try test(ib1o == ib1)
    iao = try ic.ib1op(ic)!
    try test(iao == ic)
    ib1o = try ic.ib1op(ic)!
    try test(ib1o == ic)

    iao = try ib2.ib2op(ib2)!
    try test(iao == ib2)
    var ib2o = try ib2.ib2op(ib2)!
    try test(ib2o == ib2)
    iao = try ib2.ib2op(ic)!
    try test(iao == ic)
    ib2o = try ib2.ib2op(ic)!
    try test(ib2o == ic)
    iao = try ic.ib2op(ib2)!
    try test(iao == ib2)
    ib2o = try ic.ib2op(ib2)!
    try test(ib2o == ib2)
    iao = try ic.ib2op(ic)!
    try test(iao == ic)
    ib2o = try ic.ib2op(ic)!
    try test(ib2o == ic)

    iao = try ic.icop(ic)!
    try test(iao == ic)
    ib1o = try ic.icop(ic)!
    try test(ib1o == ic)
    ib2o = try ic.icop(ic)!
    try test(ib2o == ic)
    let ico = try ic.icop(ic)!
    try test(ico == ic)
    output.writeLine("ok")

    return initial
}
