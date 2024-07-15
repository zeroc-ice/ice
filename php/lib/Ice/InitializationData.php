<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice;

class InitializationData
{
    public function __construct($properties=null, $logger=null)
    {
        $this->properties = $properties;
        $this->logger = $logger;
    }

    public $properties;
    public $logger;
}

?>
