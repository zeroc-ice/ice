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
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var Class = Ice.Class;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var InitialI = Class(Test.Initial, {
        shutdown: function(current)
        {
            current.adapter.getCommunicator().shutdown();
        },
        pingPong: function(obj, current)
        {
            return obj;
        },
        opOptionalException: function(a, b, o, current)
        {
            var ex = new Test.OptionalException();
            if(a !== undefined)
            {
                ex.a = a;
            }
            else
            {
                ex.a = undefined; // The member "a" has a default value.
            }
            if(b !== undefined)
            {
                ex.b = b;
            }
            if(o !== undefined)
            {
                ex.o = o;
            }
            throw ex;
        },
        opDerivedException: function(a, b, o, current)
        {
            var ex = new Test.DerivedException();
            if(a !== undefined)
            {
                ex.a = a;
            }
            else
            {
                ex.a = undefined; // The member "a" has a default value.
            }
            if(b !== undefined)
            {
                ex.b = b;
                ex.ss = b;
            }
            else
            {
                ex.ss = undefined; // The member "ss" has a default value.
            }
            if(o !== undefined)
            {
                ex.o = o;
                ex.o2 = o;
            }
            throw ex;
        },
        opRequiredException: function(a, b, o, current)
        {
            var ex = new Test.RequiredException();
            if(a !== undefined)
            {
                ex.a = a;
            }
            else
            {
                ex.a = undefined; // The member "a" has a default value.
            }
            if(b !== undefined)
            {
                ex.b = b;
                ex.ss = b;
            }
            if(o !== undefined)
            {
                ex.o = o;
                ex.o2 = o;
            }
            throw ex;
        },
        opByte: function(p1, current)
        {
            return [p1, p1];
        },
        opByteReq: function(p1, current)
        {
            return [p1, p1];
        },
        opBool: function(p1, current)
        {
            return [p1, p1];
        },
        opBoolReq: function(p1, current)
        {
            return [p1, p1];
        },
        opShort: function(p1, current)
        {
            return [p1, p1];
        },
        opShortReq: function(p1, current)
        {
            return [p1, p1];
        },
        opInt: function(p1, current)
        {
            return [p1, p1];
        },
        opIntReq: function(p1, current)
        {
            return [p1, p1];
        },
        opLong: function(p1, current)
        {
            return [p1, p1];
        },
        opLongReq: function(p1, current)
        {
            return [p1, p1];
        },
        opFloat: function(p1, current)
        {
            return [p1, p1];
        },
        opFloatReq: function(p1, current)
        {
            return [p1, p1];
        },
        opDouble: function(p1, current)
        {
            return [p1, p1];
        },
        opDoubleReq: function(p1, current)
        {
            return [p1, p1];
        },
        opString: function(p1, current)
        {
            return [p1, p1];
        },
        opStringReq: function(p1, current)
        {
            return [p1, p1];
        },
        opMyEnum: function(p1, current)
        {
            return [p1, p1];
        },
        opMyEnumReq: function(p1, current)
        {
            return [p1, p1];
        },
        opSmallStruct: function(p1, current)
        {
            return [p1, p1];
        },
        opSmallStructReq: function(p1, current)
        {
            return [p1, p1];
        },
        opFixedStruct: function(p1, current)
        {
            return [p1, p1];
        },
        opFixedStructReq: function(p1, current)
        {
            return [p1, p1];
        },
        opVarStruct: function(p1, current)
        {
            return [p1, p1];
        },
        opVarStructReq: function(p1, current)
        {
            return [p1, p1];
        },
        opOneOptional: function(p1, current)
        {
            return [p1, p1];
        },
        opOneOptionalReq: function(p1, current)
        {
            return [p1, p1];
        },
        opOneOptionalProxy: function(p1, current)
        {
            return [p1, p1];
        },
        opOneOptionalProxyReq: function(p1, current)
        {
            return [p1, p1];
        },
        opByteSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opByteSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opBoolSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opBoolSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opShortSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opShortSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opIntSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opIntSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opLongSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opLongSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opFloatSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opFloatSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opDoubleSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opDoubleSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opStringSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opStringSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opSmallStructSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opSmallStructSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opSmallStructList: function(p1, current)
        {
            return [p1, p1];
        },
        opSmallStructListReq: function(p1, current)
        {
            return [p1, p1];
        },
        opFixedStructSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opFixedStructSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opFixedStructList: function(p1, current)
        {
            return [p1, p1];
        },
        opFixedStructListReq: function(p1, current)
        {
            return [p1, p1];
        },
        opVarStructSeq: function(p1, current)
        {
            return [p1, p1];
        },
        opVarStructSeqReq: function(p1, current)
        {
            return [p1, p1];
        },
        opSerializable: function(p1, current)
        {
            return [p1, p1];
        },
        opSerializableReq: function(p1, current)
        {
            return [p1, p1];
        },
        opIntIntDict: function(p1, current)
        {
            return [p1, p1];
        },
        opIntIntDictReq: function(p1, current)
        {
            return [p1, p1];
        },
        opStringIntDict: function(p1, current)
        {
            return [p1, p1];
        },
        opStringIntDictReq: function(p1, current)
        {
            return [p1, p1];
        },
        opClassAndUnknownOptional: function(p, current)
        {
        },
        sendOptionalClass: function(req, current)
        {
        },
        opG: function(g, current)
        {
            return g;
        },
        opVoid: function()
        {
        },
        returnOptionalClass: function(req, current)
        {
            return new Test.OneOptional(53);
        },
        supportsRequiredParams: function(current)
        {
            return false;
        },
        supportsJavaSerializable: function(current)
        {
            return false;
        },
        supportsCsharpSerializable: function(current)
        {
            return false;
        }
    });

    exports.InitialI = InitialI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
