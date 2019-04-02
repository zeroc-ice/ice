//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import TestCommon

var args = CommandLine.arguments
if args.count < 2 {
    print("Usage: \(CommandLine.arguments[0]) <test> <args>")
    exit(1)
}

do {
    let test: String = args[1]
    let bundleName = test.split(separator: ".").map({
        if let c = $0.first {
            return c.uppercased() + $0.dropFirst()
        } else {
            return String($0)
        }
    }).joined(separator: "")

    let path = "\(Bundle.main.bundlePath)/../\(bundleName).bundle"

    guard let bundle = Bundle(url: URL(fileURLWithPath: path)) else {
        print("Bundle: `\(bundleName)' not found")
        exit(1)
    }

    guard let factory = bundle.classNamed("\(bundleName).TestFactoryI") as? TestFactory.Type else {
        print("test factory: `\(bundleName).TestFactoryI' not found")
        exit(1)
    }

    let client = factory.create()
    args.removeFirst(2)
    try client.run(args: args)

} catch {
    for s in Thread.callStackSymbols {
        print(s)
    }
    print("Error \(error)")
    exit(1)
}
