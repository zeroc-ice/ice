// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { Test } from "./Test.js";

export class AMDInitialI extends Test.Initial {
    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }

    pingPong(obj: Ice.Value | null, current: Ice.Current): Ice.Value | null {
        return obj;
    }

    opOptionalException(a: number | undefined, b: string | undefined, current: Ice.Current): void {
        const ex = new Test.OptionalException();
        if (a !== undefined) {
            ex.a = a;
        } else {
            ex.a = undefined; // The member "a" has a default value.
        }
        if (b !== undefined) {
            ex.b = b;
        }
        throw ex;
    }

    opDerivedException(a: number | undefined, b: string | undefined, current: Ice.Current) {
        const ex = new Test.DerivedException();
        if (a !== undefined) {
            ex.a = a;
        } else {
            ex.a = undefined; // The member "a" has a default value.
        }
        if (b !== undefined) {
            ex.b = b;
            ex.ss = b;
        } else {
            ex.ss = undefined; // The member "ss" has a default value.
        }
        ex.d1 = "d1";
        ex.d2 = "d2";
        throw ex;
    }

    opRequiredException(a: number | undefined, b: string | undefined, current: Ice.Current) {
        const ex = new Test.RequiredException();
        if (a !== undefined) {
            ex.a = a;
        } else {
            ex.a = undefined; // The member "a" has a default value.
        }
        if (b !== undefined) {
            ex.b = b;
            ex.ss = b;
        }
        throw ex;
    }

    opByte(p1: number | undefined, current: Ice.Current): [number | undefined, number | undefined] {
        return [p1, p1];
    }

    opBool(p1: boolean | undefined, current: Ice.Current): [boolean | undefined, boolean | undefined] {
        return [p1, p1];
    }

    opShort(p1: number | undefined, current: Ice.Current): [number | undefined, number | undefined] {
        return [p1, p1];
    }

    opInt(p1: number | undefined, current: Ice.Current): [number | undefined, number | undefined] {
        return [p1, p1];
    }

    opLong(p1: Ice.Long | number | undefined, current: Ice.Current): [Ice.Long | undefined, Ice.Long | undefined] {
        return [typeof p1 == "number" ? new Ice.Long(p1) : p1, typeof p1 == "number" ? new Ice.Long(p1) : p1];
    }

    opFloat(p1: number | undefined, current: Ice.Current): [number | undefined, number | undefined] {
        return [p1, p1];
    }

    opDouble(p1: number | undefined, current: Ice.Current): [number | undefined, number | undefined] {
        return [p1, p1];
    }

    opString(p1: string | undefined, current: Ice.Current): [string | undefined, string | undefined] {
        return [p1, p1];
    }

    opMyEnum(p1: Test.MyEnum | undefined, current: Ice.Current): [Test.MyEnum | undefined, Test.MyEnum | undefined] {
        return [p1, p1];
    }

    opSmallStruct(
        p1: Test.SmallStruct | undefined,
        current: Ice.Current,
    ): [Test.SmallStruct | undefined, Test.SmallStruct | undefined] {
        return [p1, p1];
    }

    opFixedStruct(
        p1: Test.FixedStruct | undefined,
        current: Ice.Current,
    ): [Test.FixedStruct | undefined, Test.FixedStruct | undefined] {
        return [p1, p1];
    }

    opVarStruct(
        p1: Test.VarStruct | undefined,
        current: Ice.Current,
    ): [Test.VarStruct | undefined, Test.VarStruct | undefined] {
        return [p1, p1];
    }

    opOneOptional(
        p1: Test.OneOptional | null,
        current: Ice.Current,
    ): [Test.OneOptional | null, Test.OneOptional | null] {
        return [p1, p1];
    }

    opMyInterfaceProxy(
        p1: Test.MyInterfacePrx | null | undefined,
        current: Ice.Current,
    ): [Test.MyInterfacePrx | null | undefined, Test.MyInterfacePrx | null | undefined] {
        return [p1, p1];
    }

    opByteSeq(p1: Uint8Array | undefined, current: Ice.Current): [Uint8Array | undefined, Uint8Array | undefined] {
        return [p1, p1];
    }

    opBoolSeq(p1: boolean[] | undefined, current: Ice.Current): [boolean[] | undefined, boolean[] | undefined] {
        return [p1, p1];
    }

    opShortSeq(p1: number[] | undefined, current: Ice.Current): [number[] | undefined, number[] | undefined] {
        return [p1, p1];
    }

    opIntSeq(p1: number[] | undefined, current: Ice.Current): [number[] | undefined, number[] | undefined] {
        return [p1, p1];
    }

    opLongSeq(p1: Ice.Long[] | undefined, current: Ice.Current): [Ice.Long[] | undefined, Ice.Long[] | undefined] {
        return [p1, p1];
    }

    opFloatSeq(p1: number[] | undefined, current: Ice.Current): [number[] | undefined, number[] | undefined] {
        return [p1, p1];
    }

    opDoubleSeq(p1: number[] | undefined, current: Ice.Current): [number[] | undefined, number[] | undefined] {
        return [p1, p1];
    }

    opStringSeq(p1: string[] | undefined, current: Ice.Current): [string[] | undefined, string[] | undefined] {
        return [p1, p1];
    }

    opSmallStructSeq(
        p1: Test.SmallStruct[] | undefined,
        current: Ice.Current,
    ): [Test.SmallStruct[] | undefined, Test.SmallStruct[] | undefined] {
        return [p1, p1];
    }

    opSmallStructList(
        p1: Test.SmallStruct[] | undefined,
        current: Ice.Current,
    ): [Test.SmallStruct[] | undefined, Test.SmallStruct[] | undefined] {
        return [p1, p1];
    }

    opFixedStructSeq(
        p1: Test.FixedStruct[] | undefined,
        current: Ice.Current,
    ): [Test.FixedStruct[] | undefined, Test.FixedStruct[] | undefined] {
        return [p1, p1];
    }

    opFixedStructList(
        p1: Test.FixedStruct[] | undefined,
        current: Ice.Current,
    ): [Test.FixedStruct[] | undefined, Test.FixedStruct[] | undefined] {
        return [p1, p1];
    }

    opVarStructSeq(
        p1: Test.VarStruct[] | undefined,
        current: Ice.Current,
    ): [Test.VarStruct[] | undefined, Test.VarStruct[] | undefined] {
        return [p1, p1];
    }

    opSerializable(
        p1: Test.Serializable | undefined,
        current: Ice.Current,
    ): [Test.Serializable | undefined, Test.Serializable | undefined] {
        return [p1, p1];
    }

    opIntIntDict(
        p1: Map<number, number> | undefined,
        current: Ice.Current,
    ): [Map<number, number> | undefined, Map<number, number> | undefined] {
        return [p1, p1];
    }

    opStringIntDict(
        p1: Map<string, number> | undefined,
        current: Ice.Current,
    ): [Map<string, number> | undefined, Map<string, number> | undefined] {
        return [p1, p1];
    }

    opClassAndUnknownOptional(p: Test.A | null, current: Ice.Current) {}

    opG(g: Test.G | null, current: Ice.Current): Test.G | null {
        return g;
    }

    opVoid(current: Ice.Current): void {}

    opMStruct1(current: Ice.Current): Test.SmallStruct | undefined {
        return new Test.SmallStruct();
    }

    opMStruct2(
        p1: Test.SmallStruct | undefined,
        current: Ice.Current,
    ): [Test.SmallStruct | undefined, Test.SmallStruct | undefined] {
        return [p1, p1];
    }

    opMSeq1(current: Ice.Current): string[] | undefined {
        return [];
    }

    opMSeq2(p1: string[] | undefined, current: Ice.Current): [string[] | undefined, string[] | undefined] {
        return [p1, p1];
    }

    opMDict1(current: Ice.Current): Map<string, number> | undefined {
        return new Map();
    }

    opMDict2(
        p1: Map<string, number> | undefined,
        current: Ice.Current,
    ): [Map<string, number> | undefined, Map<string, number> | undefined] {
        return [p1, p1];
    }

    supportsJavaSerializable(current: Ice.Current) {
        return false;
    }
}
