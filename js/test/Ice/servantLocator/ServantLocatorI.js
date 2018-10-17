// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestI = require("TestI").TestI;
    const test = require("TestHelper").TestHelper.test;

    class MyError
    {
    }

    class CookieI
    {
        message()
        {
            return "blahblah";
        }
    }

    class ServantLocatorI
    {
        constructor(category)
        {
            this._category = category;
            this._deactivated = false;
            this._requestId = -1;
        }

        locate(current, cookie)
        {
            test(!this._deactivated);

            test(current.id.category == this._category || this._category.length == 0);

            if(current.id.name == "unknown")
            {
                return null;
            }

            test(current.id.name == "locate" || current.id.name == "finished");
            if(current.id.name == "locate")
            {
                this.exception(current);
            }

            //
            // Ensure locate() is only called once per request.
            //
            test(this._requestId == -1);
            this._requestId = current.requestId;
            cookie.value = new CookieI();
            return new TestI();
        }

        finished(current, servant, cookie)
        {
            test(!this._deactivated);

            //
            // Ensure finished() is only called once per request.
            //
            test(this._requestId == current.requestId);
            this._requestId = -1;

            test(current.id.category == this._category || this._category.length == 0);
            test(current.id.name == "locate" || current.id.name == "finished");

            if(current.id.name == "finished")
            {
                this.exception(current);
            }

            test(cookie.message() == "blahblah");
        }

        deactivate(category)
        {
            test(!this._deactivated);
            this._deactivated = true;
        }

        exception(current)
        {
            if(current.operation == "ice_ids")
            {
                throw new Test.TestIntfUserException();
            }
            else if(current.operation == "requestFailedException")
            {
                throw new Ice.ObjectNotExistException();
            }
            else if(current.operation == "unknownUserException")
            {
                throw new Ice.UnknownUserException("reason");
            }
            else if(current.operation == "unknownLocalException")
            {
                throw new Ice.UnknownLocalException("reason");
            }
            else if(current.operation == "unknownException")
            {
                throw new Ice.UnknownException("reason");
            }
            else if(current.operation == "userException")
            {
                throw new Test.TestIntfUserException();
            }
            else if(current.operation == "localException")
            {
                throw new Ice.SocketException(0);
            }
            else if(current.operation == "jsException")
            {
                throw new MyError();
            }
            else if(current.operation == "unknownExceptionWithServantException")
            {
                throw new Ice.UnknownException("reason");
            }
            else if(current.operation == "impossibleException")
            {
                throw new Test.TestIntfUserException(); // Yes, it really is meant to be TestIntfUserException.
            }
            else if(current.operation == "intfUserException")
            {
                throw new Test.TestImpossibleException(); // Yes, it really is meant to be TestImpossibleException.
            }
            else if(current.operation == "asyncResponse")
            {
                throw new Test.TestImpossibleException();
            }
            else if(current.operation == "asyncException")
            {
                throw new Test.TestImpossibleException();
            }
        }
    }

    exports.ServantLocatorI = ServantLocatorI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
