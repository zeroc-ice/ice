// Copyright (c) ZeroC, Inc.

import { IceGrid as IceGrid_Admin } from "./Admin.js";
import { IceGrid as IceGrid_Descriptor } from "./Descriptor.js";
import { IceGrid as IceGrid_Exception } from "./Exception.js";
import { IceGrid as IceGrid_FileParser } from "./FileParser.js";
import { IceGrid as IceGrid_Registry } from "./Registry.js";
import { IceGrid as IceGrid_Session } from "./Session.js";
import { IceGrid as IceGrid_UserAccountMapper } from "./UserAccountMapper.js";

export const IceGrid = {
    ...IceGrid_Admin,
    ...IceGrid_Descriptor,
    ...IceGrid_Exception,
    ...IceGrid_FileParser,
    ...IceGrid_Registry,
    ...IceGrid_Session,
    ...IceGrid_UserAccountMapper,
};
