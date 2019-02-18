// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

//var valueMap = [String:Value.Type]()
//var exceptionMap = [String: UserException.Type]()
//autoreleasepool {
//    var classCount = UInt32(0)
//    let classList = objc_copyClassList(&classCount)!
//
//    for i in 0..<Int(classCount) {
//        let c: AnyClass = classList[i]
//
//        if let valueClass = c as? Value.Type {
//            valueMap[valueClass.ice_staticId()] = valueClass
//        } else if let userExceptionClass = c as? UserException.Type {
//            exceptionMap[userExceptionClass.ice_staticId()] = userExceptionClass
//        }
//    }
//    free(UnsafeMutableRawPointer(classList))
//}

let factoriesInitialized: Bool = {
    return ICEUtil.registerFactories(localException: LocalExceptionFactory.self,
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
        #warning("TODO: we should only install LoggerI if the user as not set their own logger or a logger property (eg. syslog)")
        let logger: ICELoggerProtocol = initData.logger as? LoggerI ?? LoggerWrapper(impl: initData.logger!)
        let handle = try ICEUtil.initialize(args, properties: propsHandle, logger: logger)

        return CommunicatorI(handle: handle, initData: initData)
    }
}

public func createProperties(args: StringSeq? = nil, defaults: Properties? = nil) throws -> (Properties, StringSeq?)  {
    return try autoreleasepool {
        var remArgs: NSArray?
        let propertiesHandle = try ICEUtil.createProperties(args,
                                                            defaults: (defaults as? PropertiesI)?._handle,
                                                            remArgs: &remArgs)

        return (PropertiesI(handle: propertiesHandle), remArgs as? StringSeq)
    }
}

public func currentEncoding() -> EncodingVersion {
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

public func identityToString(identity: Identity) throws -> String {
    return try ICEUtil.identityToString(name: identity.name, category: identity.category) as String
}
