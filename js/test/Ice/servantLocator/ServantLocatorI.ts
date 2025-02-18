// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";
import { TestI } from "./TestI.js";

const test = TestHelper.test;

class MyError {}

class Cookie {
    message() {
        return "blahblah";
    }
}

export class ServantLocatorI implements Ice.ServantLocator {
    _category: string;
    _deactivated: boolean;
    _requestId: number;
    constructor(category: string) {
        this._category = category;
        this._deactivated = false;
        this._requestId = -1;
    }

    locate(current: Ice.Current, cookie: Ice.Holder<Object>): Ice.Object | null {
        test(!this._deactivated);

        test(current.id.category == this._category || this._category.length == 0);

        if (current.id.name == "unknown") {
            return null;
        }

        if (current.id.name == "invalidReturnValue" || current.id.name == "invalidReturnType") {
            return null;
        }

        test(current.id.name == "locate" || current.id.name == "finished");
        if (current.id.name == "locate") {
            this.exception(current);
        }

        //
        // Ensure locate() is only called once per request.
        //
        test(this._requestId == -1);
        this._requestId = current.requestId;
        cookie.value = new Cookie();
        return new TestI();
    }

    finished(current: Ice.Current, servant: Ice.Object, cookie: Object) {
        test(!this._deactivated);

        //
        // Ensure finished() is only called once per request.
        //
        test(this._requestId == current.requestId);
        this._requestId = -1;

        test(current.id.category == this._category || this._category.length == 0);
        test(current.id.name == "locate" || current.id.name == "finished");

        if (current.id.name == "finished") {
            this.exception(current);
        }

        test((cookie as Cookie).message() == "blahblah");
    }

    deactivate(category: string) {
        test(!this._deactivated);
        this._deactivated = true;
    }

    exception(current: Ice.Current) {
        if (current.operation == "ice_ids") {
            throw new Test.TestIntfUserException();
        } else if (current.operation == "requestFailedException") {
            throw new Ice.ObjectNotExistException();
        } else if (current.operation == "unknownUserException") {
            throw new Ice.UnknownUserException("reason");
        } else if (current.operation == "unknownLocalException") {
            throw new Ice.UnknownLocalException("reason");
        } else if (current.operation == "unknownException") {
            throw new Ice.UnknownException("reason");
        } else if (current.operation == "userException") {
            throw new Test.TestIntfUserException();
        } else if (current.operation == "localException") {
            throw new Ice.SocketException();
        } else if (current.operation == "jsException") {
            throw new MyError();
        } else if (current.operation == "unknownExceptionWithServantException") {
            throw new Ice.UnknownException("reason");
        } else if (current.operation == "impossibleException") {
            throw new Test.TestIntfUserException(); // Yes, it really is meant to be TestIntfUserException.
        } else if (current.operation == "intfUserException") {
            throw new Test.TestImpossibleException(); // Yes, it really is meant to be TestImpossibleException.
        } else if (current.operation == "asyncResponse") {
            throw new Test.TestImpossibleException();
        } else if (current.operation == "asyncException") {
            throw new Test.TestImpossibleException();
        }
    }
}
