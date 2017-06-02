// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import ice from "ice";
const Ice = ice.Ice;

import { Demo } from "./Demo/Point";
import "./Demo/Circle";
import "./Demo/Square";
import "./Demo/Canvas";

function test(value)
{
    if(!value)
    {
        throw new Error("test failed exception");
    }
}

console.log("testing import/export modules");
let point = new Demo.gx.Point(10, 10);
test(point.x === 10);
test(point.y === 10);

let circle = new Demo.gx.Circle(new Demo.gx.Point(10, 10), 100);
test(circle.center.x == 10);
test(circle.center.y == 10);
test(circle.radius == 100);

let square = new Demo.gx.Square(
    new Demo.gx.Point(0, 0),
    new Demo.gx.Point(10, 0),
    new Demo.gx.Point(10, 10),
    new Demo.gx.Point(0, 10));

test(square.p1.x === 0);
test(square.p1.y === 0);

test(square.p2.x === 10);
test(square.p2.y === 0);

test(square.p3.x === 10);
test(square.p3.y === 10);

test(square.p4.x === 0);
test(square.p4.y === 10);

let communicator = Ice.initialize();
let session = Demo.gx.SessionPrx.uncheckedCast(communicator.stringToProxy("demo:default -h 127.0.0.1"));
test(session !== undefined);

let canvas = Demo.gx.CanvasPrx.uncheckedCast(communicator.stringToProxy("demo:default -h 127.0.0.1"));
test(canvas !== undefined);

communicator.destroy().then(
    function()
    {
        console.log("ok");
    });
