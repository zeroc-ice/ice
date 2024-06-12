//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import PromiseKit
import TestBundle

PromiseKit.conf.Q.map = .global()
PromiseKit.conf.Q.return = .global()
PromiseKit.conf.logHandler = { _ in }

var args = CommandLine.arguments
if args.count < 3 {
    print("Usage: \(CommandLine.arguments[0]) <test> <args>")
    exit(1)
}

do {
    var testPath = args[1]
    let exe = args[2]
    if exe == "ServerAMD" {
        testPath += "AMD"
    }

    let testName = "\(testPath).\(exe)"

    try TestBundle.runTest(name: testName, args: args)

} catch {
    for s in Thread.callStackSymbols {
        print(s)
    }
    print("Error \(error)")
    exit(1)
}
