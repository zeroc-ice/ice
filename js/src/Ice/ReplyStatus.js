// Copyright (c) ZeroC, Inc.

import { defineEnum } from "./EnumBase.js";

export const ReplyStatus = defineEnum([
    ["Ok", 0],
    ["UserException", 1],
    ["ObjectNotExist", 2],
    ["FacetNotExist", 3],
    ["OperationNotExist", 4],
    ["UnknownLocalException", 5],
    ["UnknownUserException", 6],
    ["UnknownException", 7],
]);
