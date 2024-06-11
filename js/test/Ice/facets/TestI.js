//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Test } from "./Test.js";

export class DI extends Test.D
{
    callA(current)
    {
        return "A";
    }

    callB(current)
    {
        return "B";
    }

    callC(current)
    {
        return "C";
    }

    callD(current)
    {
        return "D";
    }
}

export class EmptyI extends Test.Empty
{
}

export class FI extends Test.F
{
    callE(current)
    {
        return "E";
    }

    callF(current)
    {
        return "F";
    }
}

export class HI extends Test.H
{
    callG(current)
    {
        return "G";
    }

    callH(current)
    {
        return "H";
    }

    shutdown(current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
