//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    class TestI extends Test.TestIntf
    {
        requestFailedException(current)
        {
        }

        unknownUserException(current)
        {
        }

        unknownLocalException(current)
        {
        }

        unknownException(current)
        {
        }

        localException(current)
        {
        }

        userException(current)
        {
        }

        jsException(current)
        {
        }

        unknownExceptionWithServantException(current)
        {
            throw new Ice.ObjectNotExistException();
        }

        impossibleException(throwEx, current)
        {
            if(throwEx)
            {
                throw new Test.TestImpossibleException();
            }

            //
            // Return a value so we can be sure that the stream position
            // is reset correctly if finished() throws.
            //
            return "Hello";
        }

        intfUserException(throwEx, current)
        {
            if(throwEx)
            {
                throw new Test.TestIntfUserException();
            }

            //
            // Return a value so we can be sure that the stream position
            // is reset correctly if finished() throws.
            //
            return "Hello";
        }

        asyncResponse(current)
        {
            throw new Ice.ObjectNotExistException();
        }

        asyncException(current)
        {
            throw new Ice.ObjectNotExistException();
        }

        shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    exports.TestI = TestI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
