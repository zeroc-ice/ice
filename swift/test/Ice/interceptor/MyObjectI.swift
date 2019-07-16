//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class MyObjectI: MyObject {
    func add(x: Int32, y: Int32, current _: Current) throws -> Int32 {
        return x + y
    }

    func badAdd(x _: Int32, y _: Int32, current _: Current) throws -> Int32 {
        throw InvalidInputException()
    }

    func notExistAdd(x _: Int32, y _: Int32, current _: Current) throws -> Int32 {
        throw ObjectNotExistException()
    }

    func amdAddAsync(x: Int32, y: Int32, current _: Current) -> Promise<Int32> {
        return Promise<Int32> { seal in
            DispatchQueue.global(qos: .background).asyncAfter(deadline: .now() + .milliseconds(1000)) {
                seal.fulfill(x + y)
            }
        }
    }

    func amdBadAddAsync(x _: Int32, y _: Int32, current _: Current) -> Promise<Int32> {
        return Promise<Int32> { seal in
            DispatchQueue.global(qos: .background).asyncAfter(deadline: .now() + .milliseconds(1000)) {
                seal.reject(InvalidInputException())
            }
        }
    }

    func amdNotExistAddAsync(x _: Int32, y _: Int32, current _: Current) -> Promise<Int32> {
        return Promise<Int32> { seal in
            DispatchQueue.global(qos: .background).asyncAfter(deadline: .now() + .milliseconds(1000)) {
                seal.reject(ObjectNotExistException())
            }
        }
    }
}
