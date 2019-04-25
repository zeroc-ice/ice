//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import TestCommon
import PromiseKit

PromiseKit.conf.Q.map = .global()
PromiseKit.conf.Q.return = .global()
PromiseKit.conf.logHandler = { _ in }

var args = CommandLine.arguments
if args.count < 3 {
    print("Usage: \(CommandLine.arguments[0]) <test> <args>")
    exit(1)
}

do {
    var bundleName = args[1].split(separator: ".").map({
        if let c = $0.first {
            return c.uppercased() + $0.dropFirst()
        } else {
            return String($0)
        }
    }).joined(separator: "")

    let exe = args[2]
    if exe == "ServerAMD" {
        bundleName += "AMD"
    }

    let path = "\(Bundle.main.bundlePath)/../\(bundleName).bundle"

    guard let bundle = Bundle(url: URL(fileURLWithPath: path)) else {
        print("Bundle: `\(path)' not found")
        exit(1)
    }

    let className = "\(bundleName).\(exe)"
    guard let helperClass = bundle.classNamed(className) as? TestHelperI.Type else {
        print("test: `\(className)' not found")
        exit(1)
    }

    let testHelper = helperClass.init()
    args.removeFirst(3)
    try testHelper.run(args: args)

} catch {
    for s in Thread.callStackSymbols {
        print(s)
    }
    print("Error \(error)")
    exit(1)
}
