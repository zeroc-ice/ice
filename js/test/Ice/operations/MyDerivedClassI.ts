// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class MyDerivedClassI extends Test.MyDerivedClass {
    _opByteSOnewayCount: number;
    _endpoints: Ice.Endpoint[];
    constructor(endpoints: Ice.Endpoint[]) {
        super();
        this._opByteSOnewayCount = 0;
        this._endpoints = endpoints;
    }

    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }

    supportsCompress(current: Ice.Current) {
        return false;
    }

    opVoid(current: Ice.Current) {
        test(current.mode === Ice.OperationMode.Normal);
    }

    opBool(p1: boolean, p2: boolean, current: Ice.Current): [boolean, boolean] {
        return [p2, p1];
    }

    opBoolS(p1: boolean[], p2: boolean[], current: Ice.Current): [boolean[], boolean[]] {
        const p3 = p1.concat(p2);
        return [p1.reverse(), p3];
    }

    opBoolSS(p1: boolean[][], p2: boolean[][], current: Ice.Current): [boolean[][], boolean[][]] {
        const p3 = p1.concat(p2);
        return [p1.reverse(), p3];
    }

    opByte(p1: number, p2: number, current: Ice.Current): [number, number] {
        return [p1, (p1 ^ p2) & 0xff];
    }

    opByteBoolD(
        p1: Map<number, boolean>,
        p2: Map<number, boolean>,
        current: Ice.Current,
    ): [Map<number, boolean>, Map<number, boolean>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opByteS(p1: Uint8Array, p2: Uint8Array, current: Ice.Current): [Uint8Array, Uint8Array] {
        const p3 = new Uint8Array(p1.length);
        for (let i = 0; i < p1.length; i++) {
            p3[i] = p1[p1.length - (i + 1)];
        }

        const r = new Uint8Array(p1.length + p2.length);
        for (let i = 0; i < p1.length; ++i) {
            r[i] = p1[i];
        }
        for (let i = 0; i < p2.length; ++i) {
            r[i + p1.length] = p2[i];
        }
        return [r, p3];
    }

    opByteSS(p1: Uint8Array[], p2: Uint8Array[], current: Ice.Current): [Uint8Array[], Uint8Array[]] {
        const r = p1.concat(p2);
        return [r, p1.reverse()];
    }

    opFloatDouble(p1: number, p2: number, current: Ice.Current): [number, number, number] {
        return [p2, p1, p2];
    }

    opFloatDoubleS(p1: number[], p2: number[], current: Ice.Current): [number[], number[], number[]] {
        const r = p2.concat(p1);
        const p4 = p2.reverse();
        return [r, p1, p4];
    }

    opFloatDoubleSS(p1: number[][], p2: number[][], current: Ice.Current): [number[][], number[][], number[][]] {
        const r = p2.concat(p2);
        const p4 = p2.reverse();
        return [r, p1, p4];
    }

    opLongFloatD(
        p1: Ice.HashMap<BigInt, number>,
        p2: Ice.HashMap<BigInt, number>,
        current: Ice.Current,
    ): [Ice.HashMap<BigInt, number>, Ice.HashMap<BigInt, number>] {
        const r = new Ice.HashMap(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opMyClass(p1: Test.MyClassPrx, current: Ice.Current): [Test.MyClassPrx, Test.MyClassPrx, Test.MyClassPrx] {
        const p2 = p1;
        const p3 = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(Ice.stringToIdentity("noSuchIdentity")));
        const r = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id));
        return [r.ice_endpoints(this._endpoints), p2, p3.ice_endpoints(this._endpoints)];
    }

    opMyEnum(p1: Test.MyEnum, current: Ice.Current): [Test.MyEnum, Test.MyEnum] {
        return [Test.MyEnum.enum3, p1];
    }

    opShortIntD(
        p1: Map<number, number>,
        p2: Map<number, number>,
        current: Ice.Current,
    ): [Map<number, number>, Map<number, number>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opShortIntLong(p1: number, p2: number, p3: BigInt, current: Ice.Current): [BigInt, number, number, BigInt] {
        return [p3, p1, p2, p3];
    }

    opShortIntLongS(
        p1: number[],
        p2: number[],
        p3: BigInt[],
        current: Ice.Current,
    ): [BigInt[], number[], number[], BigInt[]] {
        return [p3, p1, p2.reverse(), p3.concat(p3)];
    }

    opShortIntLongSS(
        p1: number[][],
        p2: number[][],
        p3: BigInt[][],
        current: Ice.Current,
    ): [BigInt[][], number[][], number[][], BigInt[][]] {
        return [p3, p1, p2.reverse(), p3.concat(p3)];
    }

    opString(p1: string, p2: string, current: Ice.Current): [string, string] {
        return [p1 + " " + p2, p2 + " " + p1];
    }

    opStringMyEnumD(
        p1: Map<string, Test.MyEnum>,
        p2: Map<string, Test.MyEnum>,
        current: Ice.Current,
    ): [Map<string, Test.MyEnum>, Map<string, Test.MyEnum>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opMyEnumStringD(
        p1: Map<Test.MyEnum, string>,
        p2: Map<Test.MyEnum, string>,
        current: Ice.Current,
    ): [Map<Test.MyEnum, string>, Map<Test.MyEnum, string>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opMyStructMyEnumD(
        p1: Ice.HashMap<Test.MyStruct, Test.MyEnum>,
        p2: Ice.HashMap<Test.MyStruct, Test.MyEnum>,
        current: Ice.Current,
    ): [Ice.HashMap<Test.MyStruct, Test.MyEnum>, Ice.HashMap<Test.MyStruct, Test.MyEnum>] {
        const r = new Ice.HashMap(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opByteBoolDS(
        p1: Map<number, boolean>[],
        p2: Map<number, boolean>[],
        current: Ice.Current,
    ): [Map<number, boolean>[], Map<number, boolean>[]] {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opShortIntDS(
        p1: Map<number, number>[],
        p2: Map<number, number>[],
        current: Ice.Current,
    ): [Map<number, number>[], Map<number, number>[]] {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opLongFloatDS(
        p1: Ice.HashMap<BigInt, number>[],
        p2: Ice.HashMap<BigInt, number>[],
        current: Ice.Current,
    ): [Ice.HashMap<BigInt, number>[], Ice.HashMap<BigInt, number>[]] {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opStringStringDS(
        p1: Map<string, string>[],
        p2: Map<string, string>[],
        current: Ice.Current,
    ): [Map<string, string>[], Map<string, string>[]] {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opStringMyEnumDS(
        p1: Map<string, Test.MyEnum>[],
        p2: Map<string, Test.MyEnum>[],
        current: Ice.Current,
    ): [Map<string, Test.MyEnum>[], Map<string, Test.MyEnum>[]] {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opMyEnumStringDS(
        p1: Map<Test.MyEnum, string>[],
        p2: Map<Test.MyEnum, string>[],
        current: Ice.Current,
    ): [Map<Test.MyEnum, string>[], Map<Test.MyEnum, string>[]] {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opMyStructMyEnumDS(
        p1: Ice.HashMap<Test.MyStruct, Test.MyEnum>[],
        p2: Ice.HashMap<Test.MyStruct, Test.MyEnum>[],
        current: Ice.Current,
    ): [Ice.HashMap<Test.MyStruct, Test.MyEnum>[], Ice.HashMap<Test.MyStruct, Test.MyEnum>[]] {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opByteByteSD(
        p1: Map<number, Uint8Array>,
        p2: Map<number, Uint8Array>,
        current: Ice.Current,
    ): [Map<number, Uint8Array>, Map<number, Uint8Array>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opBoolBoolSD(
        p1: Map<boolean, boolean[]>,
        p2: Map<boolean, boolean[]>,
        current: Ice.Current,
    ): [Map<boolean, boolean[]>, Map<boolean, boolean[]>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opShortShortSD(
        p1: Map<number, number[]>,
        p2: Map<number, number[]>,
        current: Ice.Current,
    ): [Map<number, number[]>, Map<number, number[]>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opIntIntSD(
        p1: Map<number, number[]>,
        p2: Map<number, number[]>,
        current: Ice.Current,
    ): [Map<number, number[]>, Map<number, number[]>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opLongLongSD(
        p1: Ice.HashMap<BigInt, BigInt[]>,
        p2: Ice.HashMap<BigInt, BigInt[]>,
        current: Ice.Current,
    ): [Ice.HashMap<BigInt, BigInt[]>, Ice.HashMap<BigInt, BigInt[]>] {
        const r = new Ice.HashMap(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Ice.HashMap(p2);
        return [r, p3];
    }

    opStringFloatSD(
        p1: Map<string, number[]>,
        p2: Map<string, number[]>,
        current: Ice.Current,
    ): [Map<string, number[]>, Map<string, number[]>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opStringDoubleSD(
        p1: Map<string, number[]>,
        p2: Map<string, number[]>,
        current: Ice.Current,
    ): [Map<string, number[]>, Map<string, number[]>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opStringStringSD(
        p1: Map<string, string[]>,
        p2: Map<string, string[]>,
        current: Ice.Current,
    ): [Map<string, string[]>, Map<string, string[]>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opMyEnumMyEnumSD(
        p1: Map<Test.MyEnum, Test.MyEnum[]>,
        p2: Map<Test.MyEnum, Test.MyEnum[]>,
        current: Ice.Current,
    ): [Map<Test.MyEnum, Test.MyEnum[]>, Map<Test.MyEnum, Test.MyEnum[]>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opIntS(s: number[], current: Ice.Current): number[] {
        return s.map((v) => -v);
    }

    opByteSOneway(s: Uint8Array, current: Ice.Current): void {
        this._opByteSOnewayCount += 1;
    }

    opByteSOnewayCallCount(current: Ice.Current): number {
        const count = this._opByteSOnewayCount;
        this._opByteSOnewayCount = 0;
        return count;
    }

    opContext(current: Ice.Current): Ice.Context {
        return current.ctx;
    }

    opDoubleMarshaling(p1: number, p2: number[], current: Ice.Current) {
        const d = 1278312346.0 / 13.0;
        test(p1 === d);
        for (let i = 0; i < p2.length; ++i) {
            test(p2[i] === d);
        }
    }

    opStringS(p1: string[], p2: string[], current: Ice.Current): [string[], string[]] {
        const p3 = p1.concat(p2);
        const r = p1.reverse();
        return [r, p3];
    }

    opStringSS(p1: string[][], p2: string[][], current: Ice.Current): [string[][], string[][]] {
        const p3 = p1.concat(p2);
        const r = p2.reverse();
        return [r, p3];
    }

    opStringSSS(p1: string[][][], p2: string[][][], current: Ice.Current): [string[][][], string[][][]] {
        const p3 = p1.concat(p2);
        const r = p2.reverse();
        return [r, p3];
    }

    opStringStringD(
        p1: Map<string, string>,
        p2: Map<string, string>,
        current: Ice.Current,
    ): [Map<string, string>, Map<string, string>] {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opStruct(p1: Test.Structure, p2: Test.Structure, current: Ice.Current): [Test.Structure, Test.Structure] {
        p1.s.s = "a new string";
        return [p2, p1];
    }

    opIdempotent(current: Ice.Current) {
        test(current.mode === Ice.OperationMode.Idempotent);
    }

    opDerived(current: Ice.Current) {}

    opByte1(value: number, current: Ice.Current): number {
        return value;
    }

    opShort1(value: number, current: Ice.Current): number {
        return value;
    }

    opInt1(value: number, current: Ice.Current): number {
        return value;
    }

    opLong1(value: BigInt, current: Ice.Current): BigInt {
        return value;
    }

    opFloat1(value: number, current: Ice.Current): number {
        return value;
    }

    opDouble1(value: number, current: Ice.Current): number {
        return value;
    }

    opString1(value: string, current: Ice.Current): string {
        return value;
    }

    opStringS1(value: string[], current: Ice.Current): string[] {
        return value;
    }

    opByteBoolD1(value: Map<number, boolean>, current: Ice.Current): Map<number, boolean> {
        return value;
    }

    opStringS2(value: string[], current: Ice.Current): string[] {
        return value;
    }

    opByteBoolD2(value: Map<number, boolean>, current: Ice.Current): Map<number, boolean> {
        return value;
    }

    opMyClass1(value: Test.MyClass1, current: Ice.Current): Test.MyClass1 {
        return value;
    }

    opMyStruct1(value: Test.MyStruct1, current: Ice.Current): Test.MyStruct1 {
        return value;
    }

    opStringLiterals(current: Ice.Current): string[] {
        return [
            Test.s0,
            Test.s1,
            Test.s2,
            Test.s3,
            Test.s4,
            Test.s5,
            Test.s6,
            Test.s7,
            Test.s8,
            Test.s9,
            Test.s10,
            Test.sw0,
            Test.sw1,
            Test.sw2,
            Test.sw3,
            Test.sw4,
            Test.sw5,
            Test.sw6,
            Test.sw7,
            Test.sw8,
            Test.sw9,
            Test.sw10,
            Test.ss0,
            Test.ss1,
            Test.ss2,
            Test.ss3,
            Test.ss4,
            Test.ss5,
            Test.su0,
            Test.su1,
            Test.su2,
        ];
    }

    opWStringLiterals(current: Ice.Current): string[] {
        return this.opStringLiterals(current);
    }

    opMStruct1(current: Ice.Current): Test.Structure {
        return new Test.Structure();
    }

    opMStruct2(p1: Test.Structure, current: Ice.Current): [Test.Structure, Test.Structure] {
        return [p1, p1];
    }

    opMSeq1(current: Ice.Current): string[] {
        return [];
    }

    opMSeq2(p1: string[], current: Ice.Current): [string[], string[]] {
        return [p1, p1];
    }

    opMDict1(current: Ice.Current): Map<string, string> {
        return new Map();
    }

    opMDict2(p1: Map<string, string>, current: Ice.Current): [Map<string, string>, Map<string, string>] {
        return [p1, p1];
    }
}
