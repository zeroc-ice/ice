//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import Foundation

public protocol TextWriter
{
    func write(data:String)
    func writeLine(data:String)
}

public class StdoutWriter : TextWriter
{
    public func write(data:String)
    {
        fputs(data, stdout)
    }

    public func writeLine(data:String)
    {
        print(data)
    }
}

public protocol TestHelper
{
    func run(args:[String]) throws
    func getTestEndpoint(num:Int32, prot:String) -> String
    func getTestEndpoint(properties:Ice.Properties, num:Int32, prot:String) -> String
    func getTestHost() -> String
    func getTestHost(properties:Ice.Properties) -> String
    func getTestProtocol() -> String
    func getTestProtocol(properties:Ice.Properties) -> String
    func getTestPort(num:Int32) -> Int32
    func getTestPort(properties:Ice.Properties, num:Int32) -> Int32
    func createTestProperties(args:[String]) throws -> (Ice.Properties, [String])
    func initialize(args:[String]) throws -> (Ice.Communicator, [String])
    func initialize(properties:Ice.Properties) throws -> Ice.Communicator
    func initialize(initData:Ice.InitializationData) throws -> Ice.Communicator

    func getWriter() -> TextWriter
    func setWriter(writer:TextWriter)
}

open class TestHelperI : TestHelper {

    private var _communicator:Ice.Communicator!
    private var _writer:TextWriter!
    
    public init(){
    }

    open func run(args:[String]) throws {
        print("Subclass has not implemented abstract method `run`!")
        abort()
    }

    public func getTestEndpoint(num:Int32 = 0, prot:String = "") -> String {
        return getTestEndpoint(properties:_communicator!.getProperties(),
                                      num:num,
                                     prot:prot)
    }

    public func getTestEndpoint(properties: Ice.Properties, num: Int32 = 0, prot: String = "") -> String {
        var s = "";
        s += (prot == "") ?  properties.getPropertyWithDefault(key:"Ice.Default.Protocol", value:"default") : prot
        s += " -p "
        let port = properties.getPropertyAsIntWithDefault(key:"Test.BasePort", value:12010) + num
        s += String(port)
        return s
    }

    public func getTestHost() -> String {
        return getTestHost(properties: _communicator!.getProperties())
    }

    public func getTestHost(properties:Ice.Properties) -> String {
        return properties.getPropertyWithDefault(key: "Ice.Default.Host", value: "127.0.0.1")
    }

    public func getTestProtocol() -> String {
        return getTestProtocol(properties: _communicator!.getProperties());
    }

    public func getTestProtocol(properties: Ice.Properties) -> String {
        return properties.getPropertyWithDefault(key: "Ice.Default.Protocol", value: "tcp")
    }

    public func getTestPort(num:Int32) -> Int32 {
        return getTestPort(properties: _communicator.getProperties(), num: num)
    }

    public func getTestPort(properties: Ice.Properties, num: Int32) -> Int32 {
        return properties.getPropertyAsIntWithDefault(key: "Test.BasePort", value: 12010) + num;
    }

    public func createTestProperties(args: [String]) throws -> (Ice.Properties, [String]) {
        var (properties, args) = try Ice.createProperties(args: args)
        args = try properties.parseCommandLineOptions(prefix: "Test", options: args)
        return (properties, args)
    }

    public func initialize(args: [String]) throws -> (Ice.Communicator, [String]) {
        var initData = Ice.InitializationData()
        var (props, args) = try createTestProperties(args: args)
        (initData.properties, args) = (props, args)
        let communicator = try initialize(initData: initData)
        return (communicator, args)
    }

    public func initialize(properties:Ice.Properties) throws -> Ice.Communicator {
        var initData = Ice.InitializationData()
        initData.properties = properties
        return try initialize(initData: initData)
    }

    public func initialize(initData: Ice.InitializationData) throws -> Ice.Communicator {
        let communicator = try Ice.initialize(initData: initData)
        if(_communicator == nil) {
            _communicator = communicator
        }
        return communicator
    }

    public func getWriter() -> TextWriter {
        if(_writer == nil) {
            _writer = StdoutWriter()
        }
        return _writer
    }

    public func setWriter(writer:TextWriter) {
        _writer = writer
    }
}
