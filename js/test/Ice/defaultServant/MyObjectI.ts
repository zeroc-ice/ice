// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";

export class MyObjectI extends Test.MyObject {
    ice_ping(current: Ice.Current) {
        const name = current.id.name;

        if (name == "ObjectNotExist") {
            throw new Ice.ObjectNotExistException();
        } else if (name == "FacetNotExist") {
            throw new Ice.FacetNotExistException();
        }
    }

    getName(current: Ice.Current): string {
        const name = current.id.name;

        if (name == "ObjectNotExist") {
            throw new Ice.ObjectNotExistException();
        } else if (name == "FacetNotExist") {
            throw new Ice.FacetNotExistException();
        }
        return name;
    }
}
