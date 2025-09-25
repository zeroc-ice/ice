<?php
// Copyright (c) ZeroC, Inc.

error_reporting(E_ALL);

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}

require_once('Ice.php');

class TestHelper
{
    private $_communicator;

    public function __construct()
    {
        $this->_communicator = NULL;
    }

    public function getTestEndpoint()
    {
        $properties = NULL;
        $num = 0;
        $protocol = "";
        $args = func_get_args();

        if(func_num_args() > 0)
        {
            if($args[0] instanceof Ice\Properties)
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
            $protocol = $properties->getIceProperty("Ice.Default.Protocol");
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

        return $properties->getIceProperty("Ice.Default.Protocol");
    }

    public function getTestPort()
    {
        $properties = NULL;
        $num = 0;
        $args = func_get_args();

        if(func_num_args() > 0)
        {
            if($args[0] instanceof Ice\Properties)
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
        $properties = Ice\createProperties($args);
        $properties->parseCommandLineOptions("Test", $args);
        return $properties;
    }

    public function initialize()
    {
        $initData = NULL;
        if(func_num_args() > 0)
        {
            $args = func_get_args();
            if($args[0] instanceof Ice\InitializationData)
            {
                $initData = $args[0];
            }
            else
            {
                $initData = new Ice\InitializationData;
                if($args[0] instanceof Ice\Properties)
                {
                    $initData->properties = $args[0];
                }
                else
                {
                    $initData->properties = $this->createTestProperties($args[0]);
                }
            }
        }

        $communicator = Ice\initialize($initData);
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
            echo $ex->getMessage();
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
