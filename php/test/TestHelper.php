<?php
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

error_reporting(E_ALL | E_STRICT);

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}

$NS = function_exists("\\Ice\\initialize");

require_once('Ice.php');

class TestHelper
{
    public function __construct()
    {
        $this->_communicator = NULL;
    }

    public function getTestEndpoint()
    {
        global $NS;
        $propertiesCLS = $NS ? "\\Ice\\Properties" : "\\Ice_Properties";
        $properties = NULL;
        $num = 0;
        $protocol = "";
        $args = func_get_args();

        if(func_num_args() > 0)
        {
            if($args[0] instanceof $propertiesCLS)
            {
                $properties = $args[0];
                if(func_num_args() > 1)
                {
                    if(is_numeric($args[1]))
                    {
                        $num = $args[1];
                        if(func_num_args() > 2)
                        {
                            $protocol = $args[2];
                        }
                    }
                    else
                    {
                        $protocol = $args[1];
                    }
                }
            }
            else
            {
                $properties = $this->_communicator->getProperties();
                if(is_numeric($args[0]))
                {
                    $num = $args[0];
                    if(func_num_args() > 1)
                    {
                        $protocol = $args[1];
                    }
                }
                else
                {
                    $protocol = $args[0];
                }
            }
        }
        else
        {
            $properties = $this->_communicator->getProperties();
        }

        if($protocol == "")
        {
            $protocol = $properties->getPropertyWithDefault("Ice.Default.Protocol", "default");
        }

        $port = $properties->getPropertyAsIntWithDefault("Test.BasePort", 12010) + $num;

        return sprintf("%s -p %d", $protocol, $port);
    }

    public function getTestHost($properties=NULL)
    {
        if($properties == NULL)
        {
            $properties = $this->_communicator->getProperties();
        }

        return $properties->getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
    }

    public function getTestProtocol($properties=NULL)
    {
        if($properties == NULL)
        {
            $properties = $this->_communicator->getProperties();
        }

        return $properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
    }

    public function getTestPort()
    {
        global $NS;
        $propertiesCLS = $NS ? "\\Ice\\Properties" : "\\Ice_Properties";
        $properties = NULL;
        $num = 0;
        $args = func_get_args();

        if(func_num_args() > 0)
        {
            if($args[0] instanceof $propertiesCLS)
            {
                $properties = $args[0];
                if(func_num_args() > 1)
                {
                    $num = $args[1];
                }
            }
            else
            {
                $properties = $this->_communicator->getProperties();
                $num = $args[0];
            }
        }
        else
        {
            $properties = $this->_communicator->getProperties();
        }

        return $properties->getPropertyAsIntWithDefault("Test.BasePort", 12010) + $num;
    }

    public function createTestProperties($args = array())
    {
        global $NS;
        $properties = call_user_func_array($NS ? "\\Ice\\createProperties" : "\\Ice_createProperties", array(&$args));
        $properties->parseCommandLineOptions("Test", $args);
        return $properties;
    }

    public function initialize()
    {
        global $NS;
        $initDataCLS = $NS ? "\\Ice\\InitializationData" : "\\Ice_InitializationData";
        $propertiesCLS = $NS ? "\\Ice\\Properties" : "\\Ice_Properties";
        $initData = NULL;
        if(func_num_args() > 0)
        {
            $args = func_get_args();
            if($args[0] instanceof $initDataCLS)
            {
                $initData = $arguments[0];
            }
            else
            {
                $initData = new $initDataCLS;
                if($args[0] instanceof $propertiesCLS)
                {
                    $initData->properties = $args[0];
                }
                else
                {
                    $initData->properties = $this->createTestProperties($args[0]);
                }
            }
        }

        $communicator = call_user_func_array($NS ? "\\Ice\\initialize" : "\\Ice_initialize", array(&$initData));
        if($this->_communicator == NULL)
        {
            $this->_communicator = $communicator;
        }
        return $communicator;
    }

    public function communicator()
    {
        return $this->_communicator;
    }

    public function shutdown()
    {
        if($this->_communicator != NULL)
        {
            $this->_communicator->shutdown();
        }
    }

    public static function runTest($args)
    {
        try
        {
            $filename = $args[1];
            require_once($filename . ".php");
            $classname = pathinfo($filename);
            $helper = new $classname["filename"];
            $helper->run(array_slice($args, 1));
            exit(0);
        }
        catch(Exception $ex)
        {
            echo $ex->getTraceAsString();
            exit(1);
        }
    }
}

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

TestHelper::runTest($argv);
