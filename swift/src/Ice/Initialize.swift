//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

// Factories are registed once, when this file is loaded
private let initialized: Bool = {
    ICEUtil.registerFactories(exception: ExceptionFactory.self,
                              connectionInfo: ConnectionInfoFactory.self,
                              endpointInfo: EndpointInfoFactory.self,
                              adminFacet: AdminFacetFactory.self)
    return true
}()

public func initialize(_ args: [String], initData: InitializationData? = nil) throws -> Communicator {
    return try initializeImpl(args: args, initData: initData ?? InitializationData(), withConfigFile: true).0
}

public func initialize(_ args: inout [String], initData: InitializationData? = nil) throws -> Communicator {
    let result = try initializeImpl(args: args, initData: initData ?? InitializationData(), withConfigFile: true)
    args = result.1
    return result.0
}

public func initialize(args: [String], configFile: String) throws -> Communicator {
    var initData = InitializationData()
    let properties = createProperties()
    try properties.load(configFile)
    initData.properties = properties
    return try initialize(args, initData: initData)
}

public func initialize(args: inout [String], configFile: String) throws -> Communicator {
    var initData = InitializationData()
    let properties = createProperties()
    try properties.load(configFile)
    initData.properties = properties
    return try initialize(&args, initData: initData)
}

public func initialize(_ initData: InitializationData? = nil) throws -> Communicator {
    // This is the no-configFile flavor: we never load config from ICE_CONFIG
    return try initializeImpl(args: [], initData: initData ?? InitializationData(), withConfigFile: false).0
}

public func initialize(_ configFile: String) throws -> Communicator {
    return try initialize(args: [], configFile: configFile)
}

private func initializeImpl(args: [String],
                            initData userInitData: InitializationData,
                            withConfigFile: Bool) throws -> (Communicator, [String]) {
    // Ensure factories are initialized
    guard initialized else {
        fatalError("Unable to initialie Ice")
    }

    var initData = userInitData
    if initData.properties == nil {
        initData.properties = createProperties()
    }
    //
    // Logger
    //
    // precedence:
    // - initData.logger
    // - logger property
    // - C++ plugin loggers
    // - Swift logger
    //
    // If no user logger or property has been specified then use the Swift logger.
    // This logger may be overwritten by a logger plug-in during initialization
    if initData.logger == nil,
        initData.properties!.getProperty("Ice.LogFile").isEmpty,
        initData.properties!.getProperty("Ice.UseSyslog").isEmpty {
        initData.logger = LoggerI()
    }

    var loggerP: ICELoggerProtocol?
    if let l = initData.logger {
        loggerP = l as? LoggerI ?? LoggerWrapper(handle: l)
    }

    if let l = initData.logger {
        loggerP = l as? LoggerI ?? LoggerWrapper(handle: l)
    }

    let propsHandle = (initData.properties as! PropertiesI)._handle

    return try autoreleasepool {
        var remArgs: NSArray?
        let handle = try ICEUtil.initialize(args,
                                            properties: propsHandle,
                                            withConfigFile: withConfigFile,
                                            logger: loggerP,
                                            remArgs: &remArgs)

        //
        // Update initData.properties reference to point to the properties object
        // created by Ice::initialize, in case it changed
        //
        let newPropsHandle = handle.getProperties()
        initData.properties = newPropsHandle.fromLocalObject(to: PropertiesI.self) {
            PropertiesI(handle: newPropsHandle)
        }

        //
        // Update initData.logger referecnce in case we are using a C++ logger (defined though a property) or
        //  a C++ logger plug-in installed a new logger
        //
        if let objcLogger = handle.getLogger() as? ICELogger {
            initData.logger = objcLogger.fromLocalObject(to: ObjcLoggerWrapper.self) { ObjcLoggerWrapper(handle: objcLogger) }
        }

        precondition(initData.logger != nil && initData.properties != nil)

        let communicator = CommunicatorI(handle: handle, initData: initData)
        if remArgs == nil {
            return (communicator, [])
        } else {
            // swiftlint:disable force_cast
            return (communicator, remArgs as! [String])
        }
    }
}

public func createProperties() -> Properties {
    return PropertiesI(handle: ICEUtil.createProperties())
}

public func createProperties(_ args: [String], defaults: Properties? = nil) throws -> Properties {
    return try autoreleasepool {
        let propertiesHandle = try ICEUtil.createProperties(args,
                                                            defaults: (defaults as? PropertiesI)?._handle,
                                                            remArgs: nil)
        return PropertiesI(handle: propertiesHandle)
    }
}

public func createProperties(_ args: inout [String], defaults: Properties? = nil) throws -> Properties {
    return try autoreleasepool {
        var remArgs: NSArray?
        let propertiesHandle = try ICEUtil.createProperties(args,
                                                            defaults: (defaults as? PropertiesI)?._handle,
                                                            remArgs: &remArgs)

        // swiftlint:disable force_cast
        args = remArgs as! [String]
        return PropertiesI(handle: propertiesHandle)
    }
}

public let intVersion: Int = 30702
public let stringVersion: String = "3.7.2"

public var currentEncoding: EncodingVersion {
    var encoding = EncodingVersion()
    ICEUtil.currentEncoding(major: &encoding.major, minor: &encoding.minor)
    return encoding
}

public func stringToIdentity(_ string: String) throws -> Identity {
    return try autoreleasepool {
        var name = NSString()
        var category = NSString()
        try ICEUtil.stringToIdentity(str: string, name: &name, category: &category)
        return Identity(name: name as String, category: category as String)
    }
}

public func identityToString(id: Identity, mode: ToStringMode = ToStringMode.Unicode) -> String {
    return ICEUtil.identityToString(name: id.name, category: id.category, mode: mode.rawValue)
}

public func encodingVersionToString(_ encoding: EncodingVersion) -> String {
    return ICEUtil.encodingVersionToString(major: encoding.major, minor: encoding.minor)
}

public let Encoding_1_0 = EncodingVersion(major: 1, minor: 0)
public let Encoding_1_1 = EncodingVersion(major: 1, minor: 1)
