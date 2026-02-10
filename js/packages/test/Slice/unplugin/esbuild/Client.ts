// Copyright (c) ZeroC, Inc.

import { Test } from "./generated/Test.js";

const hello = new Test.Hello();
hello.greeting = "Hello, World!";
export { hello };
