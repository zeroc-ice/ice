// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

(function(module, require, exports)
{
    const Test = require("Test").Test;

    class AMDInitialI extends Test.Initial
    {
        shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        pingPong(obj, current)
        {
            return obj;
        }

        opOptionalException(a, b, o, current)
        {
            const ex = new Test.OptionalException();
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

        opDerivedException(a, b, o, current)
        {
            const ex = new Test.DerivedException();
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

        opRequiredException(a, b, o, current)
        {
            const ex = new Test.RequiredException();
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

        opByte(p1, current)
        {
            return [p1, p1];
        }

        opBool(p1, current)
        {
            return [p1, p1];
        }

        opShort(p1, current)
        {
            return [p1, p1];
        }

        opInt(p1, current)
        {
            return [p1, p1];
        }

        opLong(p1, current)
        {
            return [p1, p1];
        }

        opFloat(p1, current)
        {
            return [p1, p1];
        }

        opDouble(p1, current)
        {
            return [p1, p1];
        }

        opString(p1, current)
        {
            return [p1, p1];
        }

        opMyEnum(p1, current)
        {
            return [p1, p1];
        }

        opSmallStruct(p1, current)
        {
            return [p1, p1];
        }

        opFixedStruct(p1, current)
        {
            return [p1, p1];
        }

        opVarStruct(p1, current)
        {
            return [p1, p1];
        }

        opOneOptional(p1, current)
        {
            return [p1, p1];
        }

        opOneOptionalProxy(p1, current)
        {
            return [p1, p1];
        }

        opByteSeq(p1, current)
        {
            return [p1, p1];
        }

        opBoolSeq(p1, current)
        {
            return [p1, p1];
        }

        opShortSeq(p1, current)
        {
            return [p1, p1];
        }

        opIntSeq(p1, current)
        {
            return [p1, p1];
        }

        opLongSeq(p1, current)
        {
            return [p1, p1];
        }

        opFloatSeq(p1, current)
        {
            return [p1, p1];
        }

        opDoubleSeq(p1, current)
        {
            return [p1, p1];
        }

        opStringSeq(p1, current)
        {
            return [p1, p1];
        }

        opSmallStructSeq(p1, current)
        {
            return [p1, p1];
        }

        opSmallStructList(p1, current)
        {
            return [p1, p1];
        }

        opFixedStructSeq(p1, current)
        {
            return [p1, p1];
        }

        opFixedStructList(p1, current)
        {
            return [p1, p1];
        }

        opVarStructSeq(p1, current)
        {
            return [p1, p1];
        }

        opSerializable(p1, current)
        {
            return [p1, p1];
        }

        opIntIntDict(p1, current)
        {
            return [p1, p1];
        }

        opStringIntDict(p1, current)
        {
            return [p1, p1];
        }

        opIntOneOptionalDict(p1, current)
        {
            return [p1, p1];
        }

        opClassAndUnknownOptional(p, current)
        {
        }

        sendOptionalClass(req, current)
        {
        }

        returnOptionalClass(req, current)
        {
            return new Test.OneOptional(53);
        }

        opG(g, current)
        {
            return g;
        }

        opVoid(current)
        {
        }

        opMStruct1(current)
        {
            return new Test.SmallStruct();
        }

        opMStruct2(p1, current)
        {
            return [p1, p1];
        }

        opMSeq1(current)
        {
            return [];
        }

        opMSeq2(p1, current)
        {
            return [p1, p1];
        }

        opMDict1(current)
        {
            return new Map();
        }

        opMDict2(p1, current)
        {
            return [p1, p1];
        }

        opMG1(current)
        {
            return new Test.G();
        }

        opMG2(p1, current)
        {
            return [p1, p1];
        }

        supportsRequiredParams(current)
        {
            return false;
        }

        supportsJavaSerializable(current)
        {
            return false;
        }

        supportsCsharpSerializable(current)
        {
            return false;
        }

        supportsCppStringView(current)
        {
            return false;
        }

        supportsNullOptional(current)
        {
            return true;
        }
    }

    exports.AMDInitialI = AMDInitialI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
