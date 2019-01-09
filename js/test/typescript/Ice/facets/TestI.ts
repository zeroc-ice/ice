// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";

const test = TestHelper.test;

export class DI extends Test.D
{
    callA(current:Ice.Current):string
    {
        return "A";
    }

    callB(current:Ice.Current):string
    {
        return "B";
    }

    callC(current:Ice.Current):string
    {
        return "C";
    }

    callD(current:Ice.Current):string
    {
        return "D";
    }
}

export class EmptyI extends Test.Empty
{
}

export class FI extends Test.F
{
    callE(current:Ice.Current):string
    {
        return "E";
    }

    callF(current:Ice.Current):string
    {
        return "F";
    }
}

export class HI extends Test.H
{
    callG(current:Ice.Current):string
    {
        return "G";
    }

    callH(current:Ice.Current):string
    {
        return "H";
    }

    shutdown(current:Ice.Current):void
    {
        current.adapter.getCommunicator().shutdown();
    }
}
