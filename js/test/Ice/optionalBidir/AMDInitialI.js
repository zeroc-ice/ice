// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var TestAMD = require("TestAMD").TestAMD;

    var Class = Ice.Class;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var AMDInitialI = Class(TestAMD.Initial, {
        shutdown_async: function(cb, current)
        {
            current.adapter.getCommunicator().shutdown();
            cb.ice_response();
        },
        pingPong_async: function(cb, obj, current)
        {
            cb.ice_response(obj);
        },
        opOptionalException_async: function(cb, a, b, o, current)
        {
            var ex = new TestAMD.OptionalException();
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
            cb.ice_exception(ex);
        },
        opDerivedException_async: function(cb, a, b, o, current)
        {
            var ex = new TestAMD.DerivedException();
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
            cb.ice_exception(ex);
        },
        opRequiredException_async: function(cb, a, b, o, current)
        {
            var ex = new TestAMD.RequiredException();
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
            cb.ice_exception(ex);
        },
        opByte_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opBool_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opShort_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opInt_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opLong_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opFloat_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opDouble_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opString_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opMyEnum_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opSmallStruct_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opFixedStruct_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opVarStruct_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opOneOptional_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opOneOptionalProxy_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opByteSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opBoolSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opShortSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opIntSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opLongSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opFloatSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opDoubleSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opStringSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opSmallStructSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opSmallStructList_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opFixedStructSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opFixedStructList_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opVarStructSeq_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opSerializable_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opIntIntDict_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opStringIntDict_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opIntOneOptionalDict_async: function(cb, p1, current)
        {
            cb.ice_response(p1, p1);
        },
        opClassAndUnknownOptional_async: function(cb, p, current)
        {
            cb.ice_response();
        },
        sendOptionalClass_async: function(cb, req, current)
        {
            cb.ice_response();
        },
        returnOptionalClass_async: function(cb, req, current)
        {
            cb.ice_response(new TestAMD.OneOptional(53));
        },
        opG_async: function(cb, g, current)
        {
            cb.ice_response(g);
        },
        opVoid_async: function(cb, current)
        {
            cb.ice_response();
        },
        supportsRequiredParams_async: function(cb, current)
        {
            cb.ice_response(false);
        },
        supportsJavaSerializable_async: function(cb, current)
        {
            cb.ice_response(false);
        },
        supportsCsharpSerializable_async: function(cb, current)
        {
            cb.ice_response(false);
        }
    });

    exports.AMDInitialI = AMDInitialI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
