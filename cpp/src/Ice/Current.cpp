// Copyright (c) ZeroC, Inc.

#include "Ice/Current.h"
#include "Ice/LocalExceptions.h"

#include <sstream>

using namespace std;

const Ice::Current Ice::emptyCurrent{};

void
Ice::checkNonIdempotent(const Current& current)
{
    if (current.mode != OperationMode::Normal)
    {
        ostringstream os;
        os << "operation mode mismatch for operation '" << current.operation << "': received = " << current.mode
            << " for non-idempotent operation";
        throw MarshalException{__FILE__, __LINE__, os.str()};
    }
}
