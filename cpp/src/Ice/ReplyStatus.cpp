// Copyright (c) ZeroC, Inc.

#include "Ice/ReplyStatus.h"

using namespace std;

ostream&
Ice::operator<<(ostream& os, ReplyStatus replyStatus)
{
    switch (replyStatus)
    {
        case ReplyStatus::Ok:
            return os << "Ok";
        case ReplyStatus::UserException:
            return os << "UserException";
        case ReplyStatus::ObjectNotExist:
            return os << "ObjectNotExist";
        case ReplyStatus::FacetNotExist:
            return os << "FacetNotExist";
        case ReplyStatus::OperationNotExist:
            return os << "OperationNotExist";
        case ReplyStatus::UnknownLocalException:
            return os << "UnknownLocalException";
        case ReplyStatus::UnknownUserException:
            return os << "UnknownUserException";
        case ReplyStatus::UnknownException:
            return os << "UnknownException";
        case ReplyStatus::Unauthorized:
            return os << "Unauthorized";
        default:
            return os << static_cast<int>(replyStatus);
    }
}
