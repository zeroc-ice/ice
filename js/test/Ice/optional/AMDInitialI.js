// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    function test(value)
    {
        if(!value)
        {
            throw new Error("test failed");
        }
    }

    class AMDInitialI extends Test.Initial
    {
        async shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        async pingPong(obj, current)
        {
            return obj;
        }

        async opOptionalException(a, b, o, current)
        {
            let ex = new Test.OptionalException();
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
        }

        async opDerivedException(a, b, o, current)
        {
            let ex = new Test.DerivedException();
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
        }

        async opRequiredException(a, b, o, current)
        {
            let ex = new Test.RequiredException();
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
        }

        async opByte(p1, current)
        {
            return [p1, p1];
        }

        async opBool(p1, current)
        {
            return [p1, p1];
        }

        async opShort(p1, current)
        {
            return [p1, p1];
        }

        async opInt(p1, current)
        {
            return [p1, p1];
        }

        async opLong(p1, current)
        {
            return [p1, p1];
        }

        async opFloat(p1, current)
        {
            return [p1, p1];
        }

        async opDouble(p1, current)
        {
            return [p1, p1];
        }

        async opString(p1, current)
        {
            return [p1, p1];
        }

        async opMyEnum(p1, current)
        {
            return [p1, p1];
        }

        async opSmallStruct(p1, current)
        {
            return [p1, p1];
        }

        async opFixedStruct(p1, current)
        {
            return [p1, p1];
        }

        async opVarStruct(p1, current)
        {
            return [p1, p1];
        }

        async opOneOptional(p1, current)
        {
            return [p1, p1];
        }

        async opOneOptionalProxy(p1, current)
        {
            return [p1, p1];
        }

        async opByteSeq(p1, current)
        {
            return [p1, p1];
        }

        async opBoolSeq(p1, current)
        {
            return [p1, p1];
        }

        async opShortSeq(p1, current)
        {
            return [p1, p1];
        }

        async opIntSeq(p1, current)
        {
            return [p1, p1];
        }

        async opLongSeq(p1, current)
        {
            return [p1, p1];
        }

        async opFloatSeq(p1, current)
        {
            return [p1, p1];
        }

        async opDoubleSeq(p1, current)
        {
            return [p1, p1];
        }

        async opStringSeq(p1, current)
        {
            return [p1, p1];
        }

        async opSmallStructSeq(p1, current)
        {
            return [p1, p1];
        }

        async opSmallStructList(p1, current)
        {
            return [p1, p1];
        }

        async opFixedStructSeq(p1, current)
        {
            return [p1, p1];
        }

        async opFixedStructList(p1, current)
        {
            return [p1, p1];
        }

        async opVarStructSeq(p1, current)
        {
            return [p1, p1];
        }

        async opSerializable(p1, current)
        {
            return [p1, p1];
        }

        async opIntIntDict(p1, current)
        {
            return [p1, p1];
        }

        async opStringIntDict(p1, current)
        {
            return [p1, p1];
        }

        async opIntOneOptionalDict(p1, current)
        {
            return [p1, p1];
        }

        async opClassAndUnknownOptional(p, current)
        {
        }

        async sendOptionalClass(req, current)
        {
        }

        async returnOptionalClass(req, current)
        {
            return new Test.OneOptional(53);
        }

        async opG(g, current)
        {
            return g;
        }

        async opVoid(current)
        {
        }

        async opMStruct1(current)
        {
            return new Test.SmallStruct();
        }

        async opMStruct2(p1, current)
        {
            return [p1, p1];
        }

        async opMSeq1(current)
        {
            return [];
        }

        async opMSeq2(p1, current)
        {
            return [p1, p1];
        }

        async opMDict1(current)
        {
            return new Map();
        }

        async opMDict2(p1, current)
        {
            return [p1, p1];
        }

        async opMG1(current)
        {
            return new Test.G();
        }

        async opMG2(p1, current)
        {
            return [p1, p1];
        }

        async supportsRequiredParams(current)
        {
            return false;
        }

        async supportsJavaSerializable(current)
        {
            return false;
        }

        async supportsCsharpSerializable(current)
        {
            return false;
        }

        async supportsCppStringView(current)
        {
            return false;
        }

        async supportsNullOptional(current)
        {
            return true;
        }
    }

    exports.AMDInitialI = AMDInitialI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
