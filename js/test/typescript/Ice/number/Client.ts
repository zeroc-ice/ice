// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {TestHelper} from "../../../Common/TestHelper"
const test = TestHelper.test;

export class Client extends TestHelper
{
    run(args:string[])
    {
        const out = this.getWriter();
        out.write("Testing Ice.Long... ");
        //
        // Test positive numbers
        //
        test(new Ice.Long(0x00000000, 0x00000000).toNumber() === 0); // 0
        test(new Ice.Long(0x00000000, 0x00000001).toNumber() === 1); // 1
        test(new Ice.Long(0x00000000, 0x00000400).toNumber() === 1024); // 1024
        test(new Ice.Long(0x00000000, 0xFFFFFFFF).toNumber() === Math.pow(2, 32) - 1); // 2^32 - 1
        test(new Ice.Long(0x00000001, 0x00000000).toNumber() === Math.pow(2, 32)); // 2^32
        test(new Ice.Long(0x00000001, 0xFFFFFFFF).toNumber() === Math.pow(2, 33) - 1); // 2^33 - 1
        test(new Ice.Long(0x001FFFFF, 0xFFFFFFFF).toNumber() === Math.pow(2, 53) - 1); // 2^53 - 1
        test(new Ice.Long(0x00200000, 0x00000000).toNumber() === Number.POSITIVE_INFINITY); // 2^53

        //
        // Test negative numbers
        //
        test(new Ice.Long(0xFFFFFFFF, 0xFFFFFFFF).toNumber() === -1);
        test(new Ice.Long(0xFFFFFFFF, 0xFFFFFFFE).toNumber() === -2);
        test(new Ice.Long(0xFFFFFFFF, 0xFFFFFF9C).toNumber() === -100);

        test(new Ice.Long(0xFFFFFFFF, 0x00000000).toNumber() === -Math.pow(2, 32)); // -(2^32)
        test(new Ice.Long(0xFFFFFFFE, 0x00000000).toNumber() === -Math.pow(2, 33)); // -(2^33)
        test(new Ice.Long(0xFFFFFFFE, 0x00000001).toNumber() === -(Math.pow(2, 33) - 1)); // -(2^33 - 1)
        test(new Ice.Long(0xFFF00000, 0x00000000).toNumber() === -Math.pow(2, 52)); // -(2^52)
        test(new Ice.Long(0xFFF00000, 0x00000001).toNumber() === -(Math.pow(2, 52) - 1)); // -(2^52 - 1)
        test(new Ice.Long(0xFFE00000, 0x00000001).toNumber() === -(Math.pow(2, 53) - 1)); // -(2^53 - 1)
        test(new Ice.Long(0xFFE00000, 0x00000000).toNumber() === Number.NEGATIVE_INFINITY); // -(2^53)

        //
        // Test conversion from positive number
        //
        test(new Ice.Long(0).toNumber() === 0); // 0
        test(new Ice.Long(1).toNumber() === 1); // 1
        test(new Ice.Long(1024).toNumber() === 1024); // 1024
        test(new Ice.Long(Math.pow(2, 32) - 1).toNumber() === Math.pow(2, 32) - 1); // 2^32 - 1
        test(new Ice.Long(Math.pow(2, 32)).toNumber() === Math.pow(2, 32)); // 2^32
        test(new Ice.Long(Math.pow(2, 33) - 1).toNumber() === Math.pow(2, 33) - 1); // 2^33 - 1
        test(new Ice.Long(Math.pow(2, 53) - 1).toNumber() === Math.pow(2, 53) - 1); // 2^53 - 1

        test(Ice.LongHelper.validate(new Ice.Long(0))); // 0
        test(Ice.LongHelper.validate(new Ice.Long(1))); // 1
        test(Ice.LongHelper.validate(new Ice.Long(1024))); // 1024
        test(Ice.LongHelper.validate(new Ice.Long(Math.pow(2, 32) - 1))); // 2^32 - 1
        test(Ice.LongHelper.validate(new Ice.Long(Math.pow(2, 32)))); // 2^32
        test(Ice.LongHelper.validate(new Ice.Long(Math.pow(2, 33) - 1))); // 2^33 - 1
        test(Ice.LongHelper.validate(new Ice.Long(Math.pow(2, 53) - 1))); // 2^53 - 1

        //
        // Test conversion from negative number
        //
        test(new Ice.Long(-1).toNumber() === -1);
        test(new Ice.Long(-2).toNumber() === -2);
        test(new Ice.Long(-100).toNumber() === -100);

        test(new Ice.Long(-Math.pow(2, 32)).toNumber() === -Math.pow(2, 32)); // -(2^32)
        test(new Ice.Long(-Math.pow(2, 33)).toNumber() === -Math.pow(2, 33)); // -(2^33)
        test(new Ice.Long(-Math.pow(2, 33) - 1).toNumber() === -Math.pow(2, 33) - 1); // -(2^33 - 1)
        test(new Ice.Long(-Math.pow(2, 52)).toNumber() === -Math.pow(2, 52)); // -(2^52)
        test(new Ice.Long(-Math.pow(2, 52) - 1).toNumber() === -Math.pow(2, 52) - 1); // -(2^52 - 1)

        test(Ice.LongHelper.validate(new Ice.Long(-1)));
        test(Ice.LongHelper.validate(new Ice.Long(-2)));
        test(Ice.LongHelper.validate(new Ice.Long(-100)));
        test(Ice.LongHelper.validate(new Ice.Long(-Math.pow(2, 32)))); // -(2^32)
        test(Ice.LongHelper.validate(new Ice.Long(-Math.pow(2, 33)))); // -(2^33)
        test(Ice.LongHelper.validate(new Ice.Long(-Math.pow(2, 33) - 1))); // -(2^33 - 1)
        test(Ice.LongHelper.validate(new Ice.Long(-Math.pow(2, 52)))); // -(2^52)
        test(Ice.LongHelper.validate(new Ice.Long(-Math.pow(2, 52) - 1))); // -(2^52 - 1)

        out.writeLine("ok");
    }
}
