//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import TestBundle

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

    let testHelper = TestBundle.getTestHelper(name: testName)
    try await testHelper.run(args: args)

} catch {
    for s in Thread.callStackSymbols {
        print(s)
    }
    print("Error \(error)")
    exit(1)
}
