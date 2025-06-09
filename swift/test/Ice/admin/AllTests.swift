// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import Synchronization
import TestCommon

final class RemoteLoggerI: Ice.RemoteLogger {
    private let _helper: TestHelper
    private let _state = Mutex<State>(State())
    private let _semaphore = DispatchSemaphore(value: 0)

    init(helper: TestHelper) {
        _helper = helper
    }

    func initialize(prefix: String, logMessages: [Ice.LogMessage], current _: Ice.Current) async throws {
        _state.withLock {
            $0.prefix = prefix
            $0.initMessages += logMessages
        }
        _semaphore.signal()
    }

    func log(message: Ice.LogMessage, current _: Ice.Current) async throws {
        _state.withLock {
            $0.logMessages.append(message)
        }
        _semaphore.signal()
    }

    func checkNextInit(prefix: String, type: Ice.LogMessageType, message: String, category: String)
        throws
    {
        try _state.withLock {
            try _helper.test($0.prefix == prefix)
            try _helper.test($0.initMessages.count > 0)
            let logMessage = $0.initMessages.removeFirst()
            try _helper.test(logMessage.type == type)
            try _helper.test(logMessage.message == message)
            try _helper.test(logMessage.traceCategory == category)
        }
    }

    func checkNextLog(type: Ice.LogMessageType, message: String, category: String) throws {
        try _state.withLock {
            try _helper.test($0.logMessages.count > 0)
            let logMessage = $0.logMessages.removeFirst()
            try _helper.test(logMessage.type == type)
            try _helper.test(logMessage.message == message)
            try _helper.test(logMessage.traceCategory == category)
        }
    }

    func wait(calls: Int32) throws {
        for _ in 0..<calls {
            _semaphore.wait()
        }
    }

    private struct State {
        var prefix: String = ""
        var initMessages: [Ice.LogMessage] = []
        var logMessages: [Ice.LogMessage] = []
    }
}

func testFacets(com: Ice.Communicator, builtInFacets: Bool, helper: TestHelper) throws {
    if builtInFacets {
        try helper.test(com.findAdminFacet("Properties") != nil)
        try helper.test(com.findAdminFacet("Process") != nil)
        try helper.test(com.findAdminFacet("Logger") != nil)
        try helper.test(com.findAdminFacet("Metrics") != nil)
    }

    let f1 = TestFacetI()
    let f2 = TestFacetI()
    let f3 = TestFacetI()

    try com.addAdminFacet(servant: f1, facet: "Facet1")
    try com.addAdminFacet(servant: f2, facet: "Facet2")
    try com.addAdminFacet(servant: f3, facet: "Facet3")

    try helper.test(com.findAdminFacet("Facet1") as? TestFacetI === f1)
    try helper.test(com.findAdminFacet("Facet2") as? TestFacetI === f2)
    try helper.test(com.findAdminFacet("Facet3") as? TestFacetI === f3)
    try helper.test(com.findAdminFacet("Bogus") == nil)

    let facetMap = com.findAllAdminFacets()
    if builtInFacets {
        try helper.test(facetMap.count == 7)
        try helper.test(facetMap["Properties"] != nil)
        try helper.test(facetMap["Process"] != nil)
        try helper.test(facetMap["Logger"] != nil)
        try helper.test(facetMap["Metrics"] != nil)
    } else {
        try helper.test(facetMap.count >= 3)
    }

    try helper.test(facetMap["Facet1"] != nil)
    try helper.test(facetMap["Facet2"] != nil)
    try helper.test(facetMap["Facet3"] != nil)

    do {
        try com.addAdminFacet(servant: f1, facet: "Facet1")
        try helper.test(false)
    } catch is Ice.AlreadyRegisteredException {}

    do {
        _ = try com.removeAdminFacet("Bogus")
        try helper.test(false)
    } catch is Ice.NotRegisteredException {}

    _ = try com.removeAdminFacet("Facet1")
    _ = try com.removeAdminFacet("Facet2")
    _ = try com.removeAdminFacet("Facet3")

    do {
        _ = try com.removeAdminFacet("Facet1")
        try helper.test(false)
    } catch is Ice.NotRegisteredException {}
}

func allTests(_ helper: TestHelper) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()

    output.write("testing communicator operations... ")
    do {
        //
        // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
        //
        let properties = Ice.createProperties()
        properties.setProperty(key: "Ice.Admin.Endpoints", value: "tcp -h 127.0.0.1")
        properties.setProperty(key: "Ice.Admin.InstanceName", value: "Test")

        var initData = Ice.InitializationData()
        initData.properties = properties
        let com = try Ice.initialize(initData)
        try testFacets(com: com, builtInFacets: true, helper: helper)
        com.destroy()
    }

    do {
        //
        // Test: Verify that the operations work correctly in the presence of facet filters.
        //
        let properties = Ice.createProperties()
        properties.setProperty(key: "Ice.Admin.Endpoints", value: "tcp -h 127.0.0.1")
        properties.setProperty(key: "Ice.Admin.InstanceName", value: "Test")
        properties.setProperty(key: "Ice.Admin.Facets", value: "Properties")
        var initData = Ice.InitializationData()
        initData.properties = properties
        let com = try Ice.initialize(initData)
        try testFacets(com: com, builtInFacets: false, helper: helper)
        com.destroy()
    }

    do {
        //
        // Test: Verify that the operations work correctly with the Admin object disabled.
        //
        let com = try Ice.initialize()
        try testFacets(com: com, builtInFacets: false, helper: helper)
        com.destroy()
    }

    do {
        //
        // Test: Verify that the operations work correctly with Ice.Admin.Enabled=1
        //
        let properties = Ice.createProperties()
        properties.setProperty(key: "Ice.Admin.Enabled", value: "1")
        var initData = Ice.InitializationData()
        initData.properties = properties
        let com = try Ice.initialize(initData)
        try test(com.getAdmin() == nil)
        let id = try Ice.stringToIdentity("test-admin")
        do {
            _ = try com.createAdmin(adminAdapter: nil, adminId: id)
            try test(false)
        } catch is Ice.InitializationException {}

        let adapter = try com.createObjectAdapter("")
        _ = try com.createAdmin(adminAdapter: adapter, adminId: id)
        try test(com.getAdmin() != nil)

        try testFacets(com: com, builtInFacets: true, helper: helper)
        com.destroy()
    }

    do {
        //
        // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
        //
        let properties = Ice.createProperties()
        properties.setProperty(key: "Ice.Admin.Endpoints", value: "tcp -h 127.0.0.1")
        properties.setProperty(key: "Ice.Admin.InstanceName", value: "Test")
        properties.setProperty(key: "Ice.Admin.DelayCreation", value: "1")
        var initData = Ice.InitializationData()
        initData.properties = properties
        let com = try Ice.initialize(initData)
        try testFacets(com: com, builtInFacets: true, helper: helper)
        _ = try com.getAdmin()
        try testFacets(com: com, builtInFacets: true, helper: helper)
        com.destroy()
    }
    output.writeLine("ok")

    let str = "factory:\(helper.getTestEndpoint(num: 0)) -t 10000"
    let factory = try uncheckedCast(
        prx: communicator.stringToProxy(str)!, type: RemoteCommunicatorFactoryPrx.self)

    output.write("testing process facet... ")
    do {
        //
        // Test: Verify that Process::shutdown() operation shuts down the communicator.
        //
        let props = [
            "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
            "Ice.Admin.InstanceName": "Test",
        ]
        let com = try await factory.createCommunicator(props)!
        let obj = try await com.getAdmin()!
        let proc = try await checkedCast(prx: obj, type: Ice.ProcessPrx.self, facet: "Process")!
        try await proc.shutdown()
        try await com.waitForShutdown()
        try await com.destroy()
    }
    output.writeLine("ok")

    output.write("testing properties facet... ")
    do {
        let props = [
            "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
            "Ice.Admin.InstanceName": "Test",
            "Prop1": "1",
            "Prop2": "2",
            "Prop3": "3",
        ]

        let com = try await factory.createCommunicator(props)!
        let obj = try await com.getAdmin()!
        let pa = try await checkedCast(
            prx: obj, type: Ice.PropertiesAdminPrx.self, facet: "Properties")!

        //
        // Test: PropertiesAdmin::getProperty()
        //
        try await test(pa.getProperty("Prop2") == "2")
        try await test(pa.getProperty("Bogus") == "")

        //
        // Test: PropertiesAdmin::getProperties()
        //
        let pd = try await pa.getPropertiesForPrefix("")
        try test(pd.count == 5)
        try test(pd["Ice.Admin.Endpoints"] == "tcp -h 127.0.0.1")
        try test(pd["Ice.Admin.InstanceName"] == "Test")
        try test(pd["Prop1"] == "1")
        try test(pd["Prop2"] == "2")
        try test(pd["Prop3"] == "3")

        //
        // Test: PropertiesAdmin::setProperties()
        //
        let setProps = [
            "Prop1": "10",  // Changed
            "Prop2": "20",  // Changed
            "Prop3": "",  // Removed
            "Prop4": "4",  // Added
            "Prop5": "5",  // Added
        ]
        try await pa.setProperties(setProps)
        try await test(pa.getProperty("Prop1") == "10")
        try await test(pa.getProperty("Prop2") == "20")
        try await test(pa.getProperty("Prop3") == "")
        try await test(pa.getProperty("Prop4") == "4")
        try await test(pa.getProperty("Prop5") == "5")
        var changes = try await com.getChanges()
        try test(changes.count == 5)
        try test(changes["Prop1"] == "10")
        try test(changes["Prop2"] == "20")
        try test(changes["Prop3"] == "")
        try test(changes["Prop4"] == "4")
        try test(changes["Prop5"] == "5")
        try await pa.setProperties(setProps)
        changes = try await com.getChanges()
        try test(changes.count == 0)

        try await com.destroy()
    }
    output.writeLine("ok")

    // TODO: some of these tests are failing as they relied on on a blocked thread. We'll fix this by introduce a
    // MaxDispatch setting on the OA.
    //
    // output.write("testing logger facet... ")
    // do {
    //     let props = [
    //         "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
    //         "Ice.Admin.InstanceName": "Test",
    //         "NullLogger": "1",
    //     ]

    //     let com = try factory.createCommunicator(props)!

    //     try com.trace(category: "testCat", message: "trace")
    //     try com.warning("warning")
    //     try com.error("error")
    //     try com.print("print")

    //     let obj = try com.getAdmin()!
    //     let logger = try await checkedCast(prx: obj, type: Ice.LoggerAdminPrx.self, facet: "Logger")!

    //     //
    //     // Get all
    //     //
    //     var (logMessages, prefix) = try logger.getLog(
    //         messageTypes: [], traceCategories: [], messageMax: -1)

    //     try test(logMessages.count == 4)
    //     try test(prefix == "NullLogger")
    //     try test(logMessages[0].traceCategory == "testCat" && logMessages[0].message == "trace")
    //     try test(logMessages[1].message == "warning")
    //     try test(logMessages[2].message == "error")
    //     try test(logMessages[3].message == "print")

    //     //
    //     // Get only errors and warnings
    //     //
    //     try com.error("error2")
    //     try com.print("print2")
    //     try com.trace(category: "testCat", message: "trace2")
    //     try com.warning("warning2")

    //     (logMessages, prefix) = try logger.getLog(
    //         messageTypes: [
    //             Ice.LogMessageType.ErrorMessage,
    //             Ice.LogMessageType.WarningMessage,
    //         ],
    //         traceCategories: [],
    //         messageMax: -1)

    //     try test(logMessages.count == 4)
    //     try test(prefix == "NullLogger")

    //     for msg in logMessages {
    //         try test(
    //             msg.type == Ice.LogMessageType.ErrorMessage || msg.type == Ice.LogMessageType.WarningMessage
    //         )
    //     }

    //     //
    //     // Get only errors and traces with Cat = "testCat"
    //     //
    //     try com.trace(category: "testCat2", message: "A")
    //     try com.trace(category: "testCat", message: "trace3")
    //     try com.trace(category: "testCat2", message: "B")

    //     (logMessages, prefix) = try logger.getLog(
    //         messageTypes: [
    //             Ice.LogMessageType.ErrorMessage,
    //             Ice.LogMessageType.TraceMessage,
    //         ],
    //         traceCategories: ["testCat"],
    //         messageMax: -1)
    //     try test(logMessages.count == 5)
    //     try test(prefix == "NullLogger")

    //     for msg in logMessages {
    //         try test(
    //             msg.type == Ice.LogMessageType.ErrorMessage
    //                 || (msg.type == Ice.LogMessageType.TraceMessage && msg.traceCategory == "testCat"))
    //     }

    //     //
    //     // Same, but limited to last 2 messages(trace3 + error3)
    //     //
    //     try com.error("error3")

    //     (logMessages, prefix) = try logger.getLog(
    //         messageTypes: [
    //             Ice.LogMessageType.ErrorMessage,
    //             Ice.LogMessageType.TraceMessage,
    //         ],
    //         traceCategories: ["testCat"],
    //         messageMax: 2)
    //     try test(logMessages.count == 2)
    //     try test(prefix == "NullLogger")

    //     try test(logMessages[0].message == "trace3")
    //     try test(logMessages[1].message == "error3")

    //     //
    //     // Now, test RemoteLogger
    //     //
    //     let adapter = try communicator.createObjectAdapterWithEndpoints(
    //         name: "RemoteLoggerAdapter",
    //         endpoints: "tcp -h localhost")

    //     let remoteLogger = RemoteLoggerI(helper: helper)

    //     let myProxy = try uncheckedCast(
    //         prx: adapter.addWithUUID(remoteLogger),
    //         type: Ice.RemoteLoggerPrx.self)

    //     try adapter.activate()

    //     //
    //     // No filtering
    //     //
    //     (logMessages, prefix) = try logger.getLog(messageTypes: [], traceCategories: [], messageMax: -1)

    //     try logger.attachRemoteLogger(
    //         prx: myProxy, messageTypes: [], traceCategories: [], messageMax: -1)
    //     try remoteLogger.wait(calls: 1)

    //     for m in logMessages {
    //         try remoteLogger.checkNextInit(
    //             prefix: prefix, type: m.type, message: m.message, category: m.traceCategory)
    //     }

    //     try com.trace(category: "testCat", message: "rtrace")
    //     try com.warning("rwarning")
    //     try com.error("rerror")
    //     try com.print("rprint")

    //     try remoteLogger.wait(calls: 4)

    //     try remoteLogger.checkNextLog(
    //         type: Ice.LogMessageType.TraceMessage,
    //         message: "rtrace",
    //         category: "testCat")

    //     try remoteLogger.checkNextLog(
    //         type: Ice.LogMessageType.WarningMessage,
    //         message: "rwarning",
    //         category: "")

    //     try remoteLogger.checkNextLog(
    //         type: Ice.LogMessageType.ErrorMessage,
    //         message: "rerror",
    //         category: "")

    //     try remoteLogger.checkNextLog(
    //         type: Ice.LogMessageType.PrintMessage,
    //         message: "rprint",
    //         category: "")

    //     try test(logger.detachRemoteLogger(myProxy))
    //     try test(!logger.detachRemoteLogger(myProxy))

    //     //
    //     // Use Error + Trace with "traceCat" filter with 4 limit
    //     //
    //     (logMessages, prefix) = try logger.getLog(
    //         messageTypes: [
    //             Ice.LogMessageType.ErrorMessage,
    //             Ice.LogMessageType.TraceMessage,
    //         ],
    //         traceCategories: ["testCat"],
    //         messageMax: 4)
    //     try test(logMessages.count == 4)

    //     try logger.attachRemoteLogger(
    //         prx: myProxy,
    //         messageTypes: [Ice.LogMessageType.ErrorMessage, Ice.LogMessageType.TraceMessage],
    //         traceCategories: ["testCat"],
    //         messageMax: 4)
    //     try remoteLogger.wait(calls: 1)

    //     for m in logMessages {
    //         try remoteLogger.checkNextInit(
    //             prefix: prefix, type: m.type, message: m.message, category: m.traceCategory)
    //     }

    //     try com.warning("rwarning2")
    //     try com.trace(category: "testCat", message: "rtrace2")
    //     try com.warning("rwarning3")
    //     try com.error("rerror2")
    //     try com.print("rprint2")

    //     try remoteLogger.wait(calls: 2)

    //     try remoteLogger.checkNextLog(
    //         type: Ice.LogMessageType.TraceMessage, message: "rtrace2", category: "testCat")
    //     try remoteLogger.checkNextLog(
    //         type: Ice.LogMessageType.ErrorMessage, message: "rerror2", category: "")

    //     //
    //     // Attempt reconnection with slightly different proxy
    //     //
    //     do {
    //         try logger.attachRemoteLogger(
    //             prx: uncheckedCast(prx: myProxy.ice_oneway(), type: Ice.RemoteLoggerPrx.self),
    //             messageTypes: [
    //                 Ice.LogMessageType.ErrorMessage,
    //                 Ice.LogMessageType.TraceMessage,
    //             ],
    //             traceCategories: ["testCat"],
    //             messageMax: 4)
    //         try test(false)
    //     } catch is Ice.RemoteLoggerAlreadyAttachedException {}

    //     try com.destroy()
    // }
    // output.writeLine("ok")

    output.write("testing custom facet... ")
    do {
        //
        // Test: Verify that the custom facet is present.
        //
        let props = [
            "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
            "Ice.Admin.InstanceName": "Test",
        ]
        let com = try await factory.createCommunicator(props)!
        let obj = try await com.getAdmin()!
        let tf = try await checkedCast(prx: obj, type: TestFacetPrx.self, facet: "TestFacet")!
        try await tf.op()
        try await com.destroy()
    }
    output.writeLine("ok")

    output.write("testing facet filtering... ")
    do {
        //
        // Test: Set Ice.Admin.Facets to expose only the Properties facet,
        // meaning no other facet is available.
        //
        let props = [
            "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
            "Ice.Admin.InstanceName": "Test",
            "Ice.Admin.Facets": "Properties",
        ]

        let com = try await factory.createCommunicator(props)!
        let obj = try await com.getAdmin()!
        do {
            _ = try await checkedCast(prx: obj, type: Ice.ProcessPrx.self, facet: "Process")
            try test(false)
        } catch is Ice.FacetNotExistException {}
        do {
            _ = try await checkedCast(prx: obj, type: TestFacetPrx.self, facet: "TestFacet")
            try test(false)
        } catch is Ice.FacetNotExistException {}
        try await com.destroy()
    }

    do {
        //
        // Test: Set Ice.Admin.Facets to expose only the Process facet,
        // meaning no other facet is available.
        //
        let props = [
            "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
            "Ice.Admin.InstanceName": "Test",
            "Ice.Admin.Facets": "Process",
        ]
        let com = try await factory.createCommunicator(props)!
        let obj = try await com.getAdmin()!
        do {
            _ = try await checkedCast(prx: obj, type: Ice.PropertiesAdminPrx.self, facet: "Properties")
            try test(false)
        } catch is Ice.FacetNotExistException {}
        do {
            _ = try await checkedCast(prx: obj, type: TestFacetPrx.self, facet: "TestFacet")
            try test(false)
        } catch is Ice.FacetNotExistException {}
        try await com.destroy()
    }

    do {
        //
        // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
        // meaning no other facet is available.
        //
        let props = [
            "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
            "Ice.Admin.InstanceName": "Test",
            "Ice.Admin.Facets": "TestFacet",
        ]

        let com = try await factory.createCommunicator(props)!
        let obj = try await com.getAdmin()!
        do {
            _ = try await checkedCast(prx: obj, type: Ice.PropertiesAdminPrx.self, facet: "Properties")
            try test(false)
        } catch is Ice.FacetNotExistException {}
        do {
            _ = try await checkedCast(prx: obj, type: Ice.ProcessPrx.self, facet: "Process")
            try test(false)
        } catch is Ice.FacetNotExistException {}
        try await com.destroy()
    }

    do {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
        // facet names.
        //
        let props = [
            "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
            "Ice.Admin.InstanceName": "Test",
            "Ice.Admin.Facets": "Properties TestFacet",
        ]

        let com = try await factory.createCommunicator(props)!
        let obj = try await com.getAdmin()!
        let pa = try await checkedCast(
            prx: obj, type: Ice.PropertiesAdminPrx.self, facet: "Properties")!
        try await test(pa.getProperty("Ice.Admin.InstanceName") == "Test")
        let tf = try await checkedCast(prx: obj, type: TestFacetPrx.self, facet: "TestFacet")!
        try await tf.op()
        do {
            _ = try await checkedCast(prx: obj, type: Ice.ProcessPrx.self, facet: "Process")
            try test(false)
        } catch is Ice.FacetNotExistException {}
        try await com.destroy()
    }

    do {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
        // facet names.
        //
        let props = [
            "Ice.Admin.Endpoints": "tcp -h 127.0.0.1",
            "Ice.Admin.InstanceName": "Test",
            "Ice.Admin.Facets": "TestFacet, Process",
        ]
        let com = try await factory.createCommunicator(props)!
        let obj = try await com.getAdmin()!
        do {
            _ = try await checkedCast(prx: obj, type: Ice.PropertiesAdminPrx.self, facet: "Properties")
            try test(false)
        } catch is Ice.FacetNotExistException {}
        let tf = try await checkedCast(prx: obj, type: TestFacetPrx.self, facet: "TestFacet")!
        try await tf.op()
        let proc = try await checkedCast(prx: obj, type: Ice.ProcessPrx.self, facet: "Process")!
        try await proc.shutdown()
        try await com.waitForShutdown()
        try await com.destroy()
    }
    output.writeLine("ok")

    try await factory.shutdown()
}
