//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Test = require("Test").Test;

class DI extends Test.D
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

class EmptyI extends Test.Empty
{
}

class FI extends Test.F
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

class HI extends Test.H
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

exports.DI = DI;
exports.EmptyI = EmptyI;
exports.FI = FI;
exports.HI = HI;
