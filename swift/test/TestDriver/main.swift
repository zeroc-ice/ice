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
    let client = try TestFactory.create(name: args[1])
    args.removeFirst(2)
    try client.run(args: args)
} catch {
    print("Error \(error)")
    exit(1)
}
