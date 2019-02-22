//
//  main.swift
//  TestDriver
//
//  Created by jose on 2/22/19.
//  Copyright © 2019 ZeroC, Inc. All rights reserved.
//

import Foundation
import stream

var client = stream.Client()
try client.run(args: CommandLine.arguments)
