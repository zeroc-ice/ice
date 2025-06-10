// Copyright (c) ZeroC, Inc.

import Ice

final class AI: A {
    func callA(current _: Ice.Current) -> String {
        return "A"
    }
}

final class BI: B {
    func callA(current _: Ice.Current) -> String {
        return "A"
    }

    func callB(current _: Ice.Current) -> String {
        return "B"
    }
}

final class CI: C {
    func callA(current _: Ice.Current) -> String {
        return "A"
    }

    func callC(current _: Ice.Current) -> String {
        return "C"
    }
}

final class DI: D {
    func callA(current _: Ice.Current) -> String {
        return "A"
    }

    func callB(current _: Ice.Current) -> String {
        return "B"
    }

    func callC(current _: Ice.Current) -> String {
        return "C"
    }

    func callD(current _: Ice.Current) -> String {
        return "D"
    }
}

final class EI: E {
    func callE(current _: Ice.Current) -> String {
        return "E"
    }
}

final class FI: F {
    func callE(current _: Ice.Current) -> String {
        return "E"
    }

    func callF(current _: Ice.Current) -> String {
        return "F"
    }
}

final class GI: G {
    private let _communicator: Ice.Communicator

    public init(communicator: Ice.Communicator) {
        _communicator = communicator
    }

    func callG(current _: Ice.Current) -> String {
        return "G"
    }

    func shutdown(current _: Ice.Current) {
        _communicator.shutdown()
    }
}

final class HI: H {
    private let _communicator: Ice.Communicator

    init(communicator: Ice.Communicator) {
        _communicator = communicator
    }

    func callG(current _: Ice.Current) -> String {
        return "G"
    }

    func callH(current _: Ice.Current) -> String {
        return "H"
    }

    func shutdown(current _: Ice.Current) {
        _communicator.shutdown()
    }
}
