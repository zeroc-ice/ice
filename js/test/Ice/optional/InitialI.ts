//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice } from "ice";
import { Test } from "./Test.js";

export class InitialI extends Test.Initial {
    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }

    pingPong(obj: Ice.Nullable<Ice.Value>, current: Ice.Current): Ice.Nullable<Ice.Value> {
        return obj;
    }

    opOptionalException(a: Ice.Optional<number>, b: Ice.Optional<string>, current: Ice.Current): void {
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

    opDerivedException(a: Ice.Optional<number>, b: Ice.Optional<string>, current: Ice.Current) {
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

    opRequiredException(a: Ice.Optional<number>, b: Ice.Optional<string>, current: Ice.Current) {
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

    opByte(p1: Ice.Optional<number>, current: Ice.Current): [Ice.Optional<number>, Ice.Optional<number>] {
        return [p1, p1];
    }

    opBool(p1: Ice.Optional<boolean>, current: Ice.Current): [Ice.Optional<boolean>, Ice.Optional<boolean>] {
        return [p1, p1];
    }

    opShort(p1: Ice.Optional<number>, current: Ice.Current): [Ice.Optional<number>, Ice.Optional<number>] {
        return [p1, p1];
    }

    opInt(p1: Ice.Optional<number>, current: Ice.Current): [Ice.Optional<number>, Ice.Optional<number>] {
        return [p1, p1];
    }

    opLong(p1: Ice.Optional<Ice.Long>, current: Ice.Current): [Ice.Optional<Ice.Long>, Ice.Optional<Ice.Long>] {
        return [p1, p1];
    }

    opFloat(p1: Ice.Optional<number>, current: Ice.Current): [Ice.Optional<number>, Ice.Optional<number>] {
        return [p1, p1];
    }

    opDouble(p1: Ice.Optional<number>, current: Ice.Current): [Ice.Optional<number>, Ice.Optional<number>] {
        return [p1, p1];
    }

    opString(p1: Ice.Optional<string>, current: Ice.Current): [Ice.Optional<string>, Ice.Optional<string>] {
        return [p1, p1];
    }

    opMyEnum(
        p1: Ice.Optional<Test.MyEnum>,
        current: Ice.Current,
    ): [Ice.Optional<Test.MyEnum>, Ice.Optional<Test.MyEnum>] {
        return [p1, p1];
    }

    opSmallStruct(
        p1: Ice.Optional<Test.SmallStruct>,
        current: Ice.Current,
    ): [Ice.Optional<Test.SmallStruct>, Ice.Optional<Test.SmallStruct>] {
        return [p1, p1];
    }

    opFixedStruct(
        p1: Ice.Optional<Test.FixedStruct>,
        current: Ice.Current,
    ): [Ice.Optional<Test.FixedStruct>, Ice.Optional<Test.FixedStruct>] {
        return [p1, p1];
    }

    opVarStruct(
        p1: Ice.Optional<Test.VarStruct>,
        current: Ice.Current,
    ): [Ice.Optional<Test.VarStruct>, Ice.Optional<Test.VarStruct>] {
        return [p1, p1];
    }

    opOneOptional(
        p1: Ice.Nullable<Test.OneOptional>,
        current: Ice.Current,
    ): [Ice.Nullable<Test.OneOptional>, Ice.Nullable<Test.OneOptional>] {
        return [p1, p1];
    }

    opMyInterfaceProxy(
        p1: Ice.Optional<Ice.Nullable<Test.MyInterfacePrx>>,
        current: Ice.Current,
    ): [Ice.Optional<Ice.Nullable<Test.MyInterfacePrx>>, Ice.Optional<Ice.Nullable<Test.MyInterfacePrx>>] {
        return [p1, p1];
    }

    opByteSeq(
        p1: Ice.Optional<Uint8Array>,
        current: Ice.Current,
    ): [Ice.Optional<Uint8Array>, Ice.Optional<Uint8Array>] {
        return [p1, p1];
    }

    opBoolSeq(p1: Ice.Optional<boolean[]>, current: Ice.Current): [Ice.Optional<boolean[]>, Ice.Optional<boolean[]>] {
        return [p1, p1];
    }

    opShortSeq(p1: Ice.Optional<number[]>, current: Ice.Current): [Ice.Optional<number[]>, Ice.Optional<number[]>] {
        return [p1, p1];
    }

    opIntSeq(p1: Ice.Optional<number[]>, current: Ice.Current): [Ice.Optional<number[]>, Ice.Optional<number[]>] {
        return [p1, p1];
    }

    opLongSeq(
        p1: Ice.Optional<Ice.Long[]>,
        current: Ice.Current,
    ): [Ice.Optional<Ice.Long[]>, Ice.Optional<Ice.Long[]>] {
        return [p1, p1];
    }

    opFloatSeq(p1: Ice.Optional<number[]>, current: Ice.Current): [Ice.Optional<number[]>, Ice.Optional<number[]>] {
        return [p1, p1];
    }

    opDoubleSeq(p1: Ice.Optional<number[]>, current: Ice.Current): [Ice.Optional<number[]>, Ice.Optional<number[]>] {
        return [p1, p1];
    }

    opStringSeq(p1: Ice.Optional<string[]>, current: Ice.Current): [Ice.Optional<string[]>, Ice.Optional<string[]>] {
        return [p1, p1];
    }

    opSmallStructSeq(
        p1: Ice.Optional<Test.SmallStruct[]>,
        current: Ice.Current,
    ): [Ice.Optional<Test.SmallStruct[]>, Ice.Optional<Test.SmallStruct[]>] {
        return [p1, p1];
    }

    opSmallStructList(
        p1: Ice.Optional<Test.SmallStruct[]>,
        current: Ice.Current,
    ): [Ice.Optional<Test.SmallStruct[]>, Ice.Optional<Test.SmallStruct[]>] {
        return [p1, p1];
    }

    opFixedStructSeq(
        p1: Ice.Optional<Test.FixedStruct[]>,
        current: Ice.Current,
    ): [Ice.Optional<Test.FixedStruct[]>, Ice.Optional<Test.FixedStruct[]>] {
        return [p1, p1];
    }

    opFixedStructList(
        p1: Ice.Optional<Test.FixedStruct[]>,
        current: Ice.Current,
    ): [Ice.Optional<Test.FixedStruct[]>, Ice.Optional<Test.FixedStruct[]>] {
        return [p1, p1];
    }

    opVarStructSeq(
        p1: Ice.Optional<Test.VarStruct[]>,
        current: Ice.Current,
    ): [Ice.Optional<Test.VarStruct[]>, Ice.Optional<Test.VarStruct[]>] {
        return [p1, p1];
    }

    opSerializable(
        p1: Ice.Optional<Test.Serializable>,
        current: Ice.Current,
    ): [Ice.Optional<Test.Serializable>, Ice.Optional<Test.Serializable>] {
        return [p1, p1];
    }

    opIntIntDict(
        p1: Ice.Optional<Map<number, number>>,
        current: Ice.Current,
    ): [Ice.Optional<Map<number, number>>, Ice.Optional<Map<number, number>>] {
        return [p1, p1];
    }

    opStringIntDict(
        p1: Ice.Optional<Map<string, number>>,
        current: Ice.Current,
    ): [Ice.Optional<Map<string, number>>, Ice.Optional<Map<string, number>>] {
        return [p1, p1];
    }

    opClassAndUnknownOptional(p: Ice.Nullable<Test.A>, current: Ice.Current) {}

    opG(g: Ice.Nullable<Test.G>, current: Ice.Current): Ice.Nullable<Test.G> {
        return g;
    }

    opVoid(current: Ice.Current): void {}

    opMStruct1(current: Ice.Current): Ice.Optional<Test.SmallStruct> {
        return new Test.SmallStruct();
    }

    opMStruct2(
        p1: Ice.Optional<Test.SmallStruct>,
        current: Ice.Current,
    ): [Ice.Optional<Test.SmallStruct>, Ice.Optional<Test.SmallStruct>] {
        return [p1, p1];
    }

    opMSeq1(current: Ice.Current): Ice.Optional<string[]> {
        return [];
    }

    opMSeq2(p1: Ice.Optional<string[]>, current: Ice.Current): [Ice.Optional<string[]>, Ice.Optional<string[]>] {
        return [p1, p1];
    }

    opMDict1(current: Ice.Current): Ice.Optional<Map<string, number>> {
        return new Map();
    }

    opMDict2(
        p1: Ice.Optional<Map<string, number>>,
        current: Ice.Current,
    ): [Ice.Optional<Map<string, number>>, Ice.Optional<Map<string, number>>] {
        return [p1, p1];
    }

    supportsJavaSerializable(current: Ice.Current) {
        return false;
    }
}
