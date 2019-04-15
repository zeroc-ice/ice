//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice

class RemoteObjectAdapterI: RemoteObjectAdapter {
    init(_ adapter: Ice.ObjectAdapter ) throws {
        _adapter = adapter
        _testIntf = try uncheckedCast(prx: _adapter.add(servant: TestI(), id: Ice.stringToIdentity("test")),
                                      type: TestIntfPrx.self)
        try _adapter.activate()
    }

    func getTestIntf(current: Ice.Current) throws -> TestIntfPrx? {
        return _testIntf
    }

    func deactivate(current: Ice.Current) throws {
        _adapter.destroy()
    }

    var _adapter: Ice.ObjectAdapter
    var _testIntf: TestIntfPrx
}
