// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./Test";

export class AMDInitialI extends Test.Initial
{
    shutdown(current:Ice.Current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    pingPong(obj:Ice.Value, current:Ice.Current):Ice.Value
    {
        return obj;
    }

    opOptionalException(a:number, b:string, o:Test.OneOptional, current:Ice.Current):void
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

    opDerivedException(a:number, b:string, o:Test.OneOptional, current:Ice.Current):void
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

    opRequiredException(a:number, b:string, o:Test.OneOptional, current:Ice.Current):void
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

    opByte(p1:number, current:Ice.Current):[number, number]
    {
        return [p1, p1];
    }

    opBool(p1:boolean, current:Ice.Current):[boolean, boolean]
    {
        return [p1, p1];
    }

    opShort(p1:number, current:Ice.Current):[number, number]
    {
        return [p1, p1];
    }

    opInt(p1:number, current:Ice.Current):[number, number]
    {
        return [p1, p1];
    }

    opLong(p1:Ice.Long, current:Ice.Current):[Ice.Long, Ice.Long]
    {
        return [p1, p1];
    }

    opFloat(p1:number, current:Ice.Current):[number, number]
    {
        return [p1, p1];
    }

    opDouble(p1:number, current:Ice.Current):[number, number]
    {
        return [p1, p1];
    }

    opString(p1:string, current:Ice.Current):[string, string]
    {
        return [p1, p1];
    }

    opMyEnum(p1:Test.MyEnum, current:Ice.Current):[Test.MyEnum, Test.MyEnum]
    {
        return [p1, p1];
    }

    opSmallStruct(p1:Test.SmallStruct, current:Ice.Current):[Test.SmallStruct, Test.SmallStruct]
    {
        return [p1, p1];
    }

    opFixedStruct(p1:Test.FixedStruct, current:Ice.Current):[Test.FixedStruct, Test.FixedStruct]
    {
        return [p1, p1];
    }

    opVarStruct(p1:Test.VarStruct, current:Ice.Current):[Test.VarStruct, Test.VarStruct]
    {
        return [p1, p1];
    }

    opOneOptional(p1:Test.OneOptional, current:Ice.Current):[Test.OneOptional, Test.OneOptional]
    {
        return [p1, p1];
    }

    opOneOptionalProxy(p1:Ice.ObjectPrx, current:Ice.Current):[Ice.ObjectPrx, Ice.ObjectPrx]
    {
        return [p1, p1];
    }

    opByteSeq(p1:Test.ByteSeq, current:Ice.Current):[Test.ByteSeq, Test.ByteSeq]
    {
        return [p1, p1];
    }

    opBoolSeq(p1:Test.BoolSeq, current:Ice.Current):[Test.BoolSeq, Test.BoolSeq]
    {
        return [p1, p1];
    }

    opShortSeq(p1:Test.ShortSeq, current:Ice.Current):[Test.ShortSeq, Test.ShortSeq]
    {
        return [p1, p1];
    }

    opIntSeq(p1:Test.IntSeq, current:Ice.Current):[Test.IntSeq, Test.IntSeq]
    {
        return [p1, p1];
    }

    opLongSeq(p1:Test.LongSeq, current:Ice.Current):[Test.LongSeq, Test.LongSeq]
    {
        return [p1, p1];
    }

    opFloatSeq(p1:Test.FloatSeq, current:Ice.Current):[Test.FloatSeq, Test.FloatSeq]
    {
        return [p1, p1];
    }

    opDoubleSeq(p1:Test.DoubleSeq, current:Ice.Current):[Test.DoubleSeq, Test.DoubleSeq]
    {
        return [p1, p1];
    }

    opStringSeq(p1:Test.StringSeq, current:Ice.Current):[Test.StringSeq, Test.StringSeq]
    {
        return [p1, p1];
    }

    opSmallStructSeq(p1:Test.SmallStructSeq, current:Ice.Current):[Test.SmallStructSeq, Test.SmallStructSeq]
    {
        return [p1, p1];
    }

    opSmallStructList(p1:Test.SmallStructList, current:Ice.Current):[Test.SmallStructList, Test.SmallStructList]
    {
        return [p1, p1];
    }

    opFixedStructSeq(p1:Test.FixedStructSeq, current:Ice.Current):[Test.FixedStructSeq, Test.FixedStructSeq]
    {
        return [p1, p1];
    }

    opFixedStructList(p1:Test.FixedStructList, current:Ice.Current):[Test.FixedStructList, Test.FixedStructList]
    {
        return [p1, p1];
    }

    opVarStructSeq(p1:Test.VarStructSeq, current:Ice.Current):[Test.VarStructSeq, Test.VarStructSeq]
    {
        return [p1, p1];
    }

    opSerializable(p1:Test.Serializable, current:Ice.Current):[Test.Serializable, Test.Serializable]
    {
        return [p1, p1];
    }

    opIntIntDict(p1:Test.IntIntDict, current:Ice.Current):[Test.IntIntDict, Test.IntIntDict]
    {
        return [p1, p1];
    }

    opStringIntDict(p1:Test.StringIntDict, current:Ice.Current):[Test.StringIntDict, Test.StringIntDict]
    {
        return [p1, p1];
    }

    opIntOneOptionalDict(p1:Test.IntOneOptionalDict, current:Ice.Current):[Test.IntOneOptionalDict, Test.IntOneOptionalDict]
    {
        return [p1, p1];
    }

    opClassAndUnknownOptional(p:Test.A, current:Ice.Current):void
    {
    }

    sendOptionalClass(req:boolean, one:Test.OneOptional, current:Ice.Current):void
    {
    }

    returnOptionalClass(req:boolean, current:Ice.Current):Test.OneOptional
    {
        return new Test.OneOptional(53);
    }

    opG(g:Test.G, current:Ice.Current):Test.G
    {
        return g;
    }

    opVoid(current:Ice.Current):void
    {
    }

    opMStruct1(current:Ice.Current):Test.SmallStruct
    {
        return new Test.SmallStruct();
    }

    opMStruct2(p1:Test.SmallStruct, current:Ice.Current):[Test.SmallStruct, Test.SmallStruct]
    {
        return [p1, p1];
    }

    opMSeq1(current:Ice.Current):Test.StringSeq
    {
        return [];
    }

    opMSeq2(p1:Test.StringSeq, current:Ice.Current):[Test.StringSeq, Test.StringSeq]
    {
        return [p1, p1];
    }

    opMDict1(current:Ice.Current):Test.StringIntDict
    {
        return new Map();
    }

    opMDict2(p1:Test.StringIntDict, current:Ice.Current):[Test.StringIntDict, Test.StringIntDict]
    {
        return [p1, p1];
    }

    opMG1(current:Ice.Current):Test.G
    {
        return new Test.G();
    }

    opMG2(p1:Test.G, current:Ice.Current):[Test.G, Test.G]
    {
        return [p1, p1];
    }

    supportsRequiredParams(current:Ice.Current):boolean
    {
        return false;
    }

    supportsJavaSerializable(current:Ice.Current):boolean
    {
        return false;
    }

    supportsCsharpSerializable(current:Ice.Current):boolean
    {
        return false;
    }

    supportsCppStringView(current:Ice.Current):boolean
    {
        return false;
    }

    supportsNullOptional(current:Ice.Current):boolean
    {
        return true;
    }
}
