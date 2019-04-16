import Ice

class ServantLocatorI: Ice.ServantLocator {

    func locate(_ curr: Current) throws -> (returnValue: Object?, cookie: AnyObject?) {
        return (nil, nil)
    }

    func finished(curr: Current, servant: Object, cookie: AnyObject) throws {
    }

    func deactivate(_ category: String) {
    }
}
