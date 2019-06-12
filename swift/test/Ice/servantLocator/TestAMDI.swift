//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice
import PromiseKit
import TestCommon

class TestI: TestIntf {
    func requestFailedExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func unknownUserExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func unknownLocalExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func unknownExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func localExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func userExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func csExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func unknownExceptionWithServantExceptionAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(ObjectNotExistException(id: current.id, facet: current.facet, operation: current.operation))
        }
    }

    func impossibleExceptionAsync(throw t: Bool, current _: Current) -> Promise<String> {
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

    func intfUserExceptionAsync(throw t: Bool, current _: Current) -> Promise<String> {
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

    func asyncResponseAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func asyncExceptionAsync(current _: Current) -> Promise<Void> {
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

class TestActivationI: TestActivation {
    var _helper: TestHelper

    init(_ helper: TestHelper) {
        _helper = helper
    }

    func activateServantLocatorAsync(activate: Bool, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            if activate {
                try current.adapter!.addServantLocator(locator: ServantLocatorI("", _helper), category: "")
                try current.adapter!.addServantLocator(locator: ServantLocatorI("category", _helper),
                                                       category: "category")
            } else {
                var locator = try current.adapter!.removeServantLocator("")
                locator.deactivate("")
                locator = try current.adapter!.removeServantLocator("category")
                locator.deactivate("category")
            }
            seal.fulfill(())
        }
    }
}
