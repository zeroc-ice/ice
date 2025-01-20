// Copyright (c) ZeroC, Inc.

import { Glacier2 as Glacier2_PermissionsVerifier } from "./PermissionsVerifier.js";
import { Glacier2 as Glacier2_Router } from "./Router.js";
import { Glacier2 as Glacier2_SSLInfo } from "./SSLInfo.js";
import { Glacier2 as Glacier2_Session } from "./Session.js";

export const Glacier2 = {
    ...Glacier2_PermissionsVerifier,
    ...Glacier2_Router,
    ...Glacier2_SSLInfo,
    ...Glacier2_Session,
};
