// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

import ice from "ice";

const Ice = ice.Ice;

import {Demo} from "./Demo/Point";
import "./Demo/Canvas";
import "./Demo/Circle";
import "./Demo/Square";

function test(value)
{
    if(!value)
    {
        throw new Error("test failed exception");
    }
}

console.log("testing import/export modules");
const point = new Demo.gx.Point(10, 10);
test(point.x === 10);
test(point.y === 10);

const circle = new Demo.gx.Circle(new Demo.gx.Point(10, 10), 100);
test(circle.center.x == 10);
test(circle.center.y == 10);
test(circle.radius == 100);

const square = new Demo.gx.Square(
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

const communicator = Ice.initialize();
const session = Demo.gx.SessionPrx.uncheckedCast(communicator.stringToProxy("demo:default -h 127.0.0.1"));
test(session !== undefined);

const canvas = Demo.gx.CanvasPrx.uncheckedCast(communicator.stringToProxy("demo:default -h 127.0.0.1"));
test(canvas !== undefined);

communicator.destroy().then(() => console.log("ok"));
