//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import Foundation

protocol TextWriter
{
    func write(data:String)
    func writeLine(data:String)
}

class StdoutWriter : TextWriter
{
    func write(data:String)
    {
        fputs(data, stdout)
    }

    func writeLine(data:String)
    {
        print(data)
    }
}

protocol TestHelper
{
    func run(args:[String]?) -> Void
    func getTestEndpoint(num:Int, prot:String) -> String
    func getTestEndpoint(properties:Ice.Properties, num:Int, prot:String) -> String
    func getTestHost() -> String
    func getTestHost(properties:Ice.Properties) -> String
    func getTestProtocol() -> String
    func getTestProtocol(properties:Ice.Properties) -> String
    func getTestPort(num:Int) -> Int
    func getTestPort(properties:Ice.Properties, num:Int) -> Int
    func createTestProperties(args:[String]?) -> (Ice.Properties, [String]?)
    func initialize(args:[String]?) -> (Ice.Communicator, [String]?)
    func initialize(properties:Ice.Properties) -> Ice.Communicator
    func initialize(initData:Ice.InitializationData) -> Ice.Communicator

    func getWriter() -> TextWriter
    func setWriter(writer:TextWriter) -> Void
}

extension TestHelper
{
    private var _communicator:Ice.Communicator
    private var _writer:StdoutWriter

    func getTestEndpoint(num:Int = 0, prot:String = "") -> String
    {
        return getTestEndpoint(_communicator.getProperties(), num, prot)
    }

    func getTestEndpoint(properties:Ice.Properties, num:Int = 0, prot:String = "") -> String
    {
        var s = "";
        s += (prot == "") ?  properties.getPropertyWithDefault("Ice.Default.Protocol", "default") : prot
        s += " -p "
        s += properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num
        return s
    }

    func getTestHost() -> String
    {
        return getTestHost(_communicator.getProperties())
    }

    func getTestHost(properties:Ice.Properties) -> String
    {
        return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1")
    }

    func getTestProtocol() -> String
    {
        return getTestProtocol(_communicator.getProperties());
    }

    func getTestProtocol(properties:Ice.Properties) -> String
    {
        return properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp")
    }

    func getTestPort(num:Int) -> Int
    {
        return getTestPort(_communicator.getProperties(), num)
    }

    func getTestPort(properties:Ice.Properties, num:Int) -> Int
    {
        return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;
    }

    func createTestProperties(args:[String]?) -> (Ice.Properties, [String]?)
    {
        var (properties, args) = createProperties(args)
        args = properties.parseCommandLineOptions("Test", args)
        return (properties, args)
    }

    func initialize(args:[String]?) -> (Ice.Communicator, [String]?)
    {
        var initData = Ice.InitializationData()
        initData.properties = createTestProperties(args)
        return initialize(initData)
    }

    func initialize(properties:Ice.Properties) -> Ice.Communicator
    {
        var initData = Ice.InitializationData()
        initData.properties = properties
        return initialize(initData)
    }

    func initialize(initData:Ice.InitializationData) -> Ice.Communicator
    {
        var communicator = Ice.initialize(initData)
        if(_communicator == nil)
        {
            _communicator = communicator
        }
        return communicator
    }

    func getWriter() -> TextWriter
    {
        if(_writer == nil)
        {
            _writer = StdoutWriter()
        }
        return _writer
    }

    func setWriter(writer:TextWriter)
    {
        _writer = writer
    }
}
