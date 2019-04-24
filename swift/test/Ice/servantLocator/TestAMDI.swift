//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice
import PromiseKit
import TestCommon

class TestI: TestIntf {
    func requestFailedExceptionAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }
    
    func unknownUserExceptionAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }
    
    func unknownLocalExceptionAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }
    
    func unknownExceptionAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }
    
    func localExceptionAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }
    
    func userExceptionAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }
    
    func csExceptionAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }
    
    func unknownExceptionWithServantExceptionAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(ObjectNotExistException(id: current.id, facet: current.facet, operation: current.operation))
        }
    }
    
    func impossibleExceptionAsync(throw t: Bool, current: Current) -> Promise<String> {
        return Promise<String> { seal in
            if t {
                seal.reject(TestImpossibleException())
            } else {
                //
                // Return a value so we can be sure that the stream position
                // is reset correctly if finished() throws.
                //
                seal.fulfill("Hello")
            }
        }
    }
    
    func intfUserExceptionAsync(throw t: Bool, current: Current) -> Promise<String> {
        return Promise<String> { seal in
            if t {
                seal.reject(TestIntfUserException())
            } else {
                //
                // Return a value so we can be sure that the stream position
                // is reset correctly if finished() throws.
                //
                seal.fulfill("Hello")
            }
        }
    }

    func asyncResponseAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func asyncExceptionAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(TestIntfUserException())
        }
    }

    func shutdownAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            current.adapter!.deactivate()
            seal.fulfill(())
        }
    }
}
