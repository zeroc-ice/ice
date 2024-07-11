<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice;

class CompressBatch
{
    const Yes = 0;
    const No = 1;
    const BasedOnProxy = 2;
}

class ConnectionClose
{
    const Forcefully = 0;
    const Gracefully = 1;
    const GracefullyWithWait = 2;
}

?>
