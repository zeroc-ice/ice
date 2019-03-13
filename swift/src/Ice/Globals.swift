// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

// Factories are registed once, when this file is loaded
let factoriesInitialized: Bool = {
    ICEUtil.registerFactories(localException: LocalExceptionFactory.self,
                              connectionInfo: ConnectionInfoFactory.self,
                              endpointInfo: EndpointInfoFactory.self)
}()

public func initialize(args: StringSeq = [],
                       initData userInitData: InitializationData? = nil,
                       configFile: String? = nil) throws -> Communicator {
    // Ensure factores are initialized
    precondition(factoriesInitialized)

    return try autoreleasepool {
        var initData = userInitData ?? InitializationData()

        if initData.logger == nil {
            initData.logger = LoggerI()
        }

        if initData.properties == nil {
            let (props, _) = try Ice.createProperties()
            initData.properties = props
        }

        if configFile != nil {
            try initData.properties!.load(file: configFile!)
        }

        let propsHandle = (initData.properties as? PropertiesI)?._handle
        // TODO: we should only install LoggerI if the user as not set their own
        // logger or a logger property (eg. syslog)
        let logger: ICELoggerProtocol = initData.logger as? LoggerI ?? LoggerWrapper(impl: initData.logger!)
        let handle = try ICEUtil.initialize(args, properties: propsHandle, logger: logger)
        //
        // Update initData.properties reference to point to the properties object
        // created by Ice::initialize.
        //
        initData.properties = PropertiesI(handle: handle.getProperties())
        return CommunicatorI(handle: handle, initData: initData)
    }
}

public func createProperties(args: StringSeq? = nil, defaults: Properties? = nil) throws -> (Properties, StringSeq) {
    return try autoreleasepool {
        var remArgs: NSArray?
        let propertiesHandle = try ICEUtil.createProperties(args,
                                                            defaults: (defaults as? PropertiesI)?._handle,
                                                            remArgs: &remArgs)

        // swiftlint:disable force_cast
        return (PropertiesI(handle: propertiesHandle), remArgs as! StringSeq)
    }
}

public var currentEncoding: EncodingVersion {
    var encoding = EncodingVersion()
    ICEUtil.currentEncoding(major: &encoding.major, minor: &encoding.minor)
    return encoding
}

public func stringToIdentity(string: String) throws -> Identity {
    var name = NSString()
    var category = NSString()
    try ICEUtil.stringToIdentity(str: string, name: &name, category: &category)
    return Identity(name: name as String, category: category as String)
}

public func identityToString(identity: Identity, mode: ToStringMode = ToStringMode.Unicode) throws -> String {
    return try ICEUtil.identityToString(name: identity.name,
                                        category: identity.category,
                                        mode: mode.rawValue) as String
}

public let Encoding_1_0 = Protocol.Encoding_1_0
public let Encoding_1_1 = Protocol.Encoding_1_1
