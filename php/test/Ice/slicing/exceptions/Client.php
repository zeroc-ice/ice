<?
error_reporting(E_ALL | E_STRICT);

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}

$NS = function_exists("Ice\\initialize");
require_once ($NS ? 'Ice_ns.php' : 'Ice.php');
require_once 'Test.php';

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        die("\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n");
    }
}

function allTests($communicator)
{
    global $NS;

    $obj = $communicator->stringToProxy("Test:default -p 12010");
    $test = $obj->ice_checkedCast("::Test::TestIntf");

    echo "testing throwing a base exception... ";
    flush();
    {
        try
        {
            $test->baseAsBase();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(!($b instanceof $excls))
            {
                throw $ex;
            }
            test($b->b == "Base.b");
            test(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"));
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown derived exception... ";
    flush();
    {
        try
        {
            $test->unknownDerivedAsBase();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(!($b instanceof $excls))
            {
                throw $ex;
            }
            test($b->b == "UnknownDerived.b");
            test(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"));
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known derived exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->knownDerivedAsBase();
            test(false);
        }
        catch(Exception $k)
        {
            $excls = $NS ? "Test\\KnownDerived" : "Test_KnownDerived";
            if(!($k instanceof $excls))
            {
                throw $ex;
            }
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == ($NS ? "Test\\KnownDerived" : "Test_KnownDerived"));
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known derived exception thrown as derived exception... ";
    flush();
    {
        try
        {
            $test->knownDerivedAsKnownDerived();
            test(false);
        }
        catch(Exception $k)
        {
            $excls = $NS ? "Test\\KnownDerived" : "Test_KnownDerived";
            if(!($k instanceof $excls))
            {
                throw $ex;
            }
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == ($NS ? "Test\\KnownDerived" : "Test_KnownDerived"));
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown intermediate exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->unknownIntermediateAsBase();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(!($b instanceof $excls))
            {
                throw $ex;
            }
            test($b->b == "UnknownIntermediate.b");
            test(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"));
        }
    }
    echo "ok\n";

    echo "testing slicing of known intermediate exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->knownIntermediateAsBase();
            test(false);
        }
        catch(Exception $ki)
        {
            $excls = $NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate";
            if(!($ki instanceof $excls))
            {
                throw $ex;
            }
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == ($NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate"));
        }
    }
    echo "ok\n";

    echo "testing slicing of known most derived exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsBase();
            test(false);
        }
        catch(Exception $kmd)
        {
            $excls = $NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived";
            if(!($kmd instanceof $excls))
            {
                throw $ex;
            }
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == ($NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived"));
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known intermediate exception thrown as intermediate exception... ";
    flush();
    {
        try
        {
            $test->knownIntermediateAsKnownIntermediate();
            test(false);
        }
        catch(Exception $ki)
        {
            $excls = $NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate";
            if(!($ki instanceof $excls))
            {
                throw $ex;
            }
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == ($NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate"));
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known most derived thrown as intermediate exception... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsKnownIntermediate();
            test(false);
        }
        catch(Exception $kmd)
        {
            $excls = $NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived";
            if(!($kmd instanceof $excls))
            {
                throw $ex;
            }
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == ($NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived"));
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known most derived thrown as most derived exception... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsKnownMostDerived();
            test(false);
        }
        catch(Exception $kmd)
        {
            $excls = $NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived";
            if(!($kmd instanceof $excls))
            {
                throw $ex;
            }
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == ($NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived"));
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with known intermediate thrown as base exception... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived1AsBase();
            test(false);
        }
        catch(Exception $ki)
        {
            $excls = $NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate";
            if(!($ki instanceof $excls))
            {
                throw $ex;
            }
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == ($NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate"));
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with known intermediate thrown as intermediate exception... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived1AsKnownIntermediate();
            test(false);
        }
        catch(Exception $ki)
        {
            $excls = $NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate";
            if(!($ki instanceof $excls))
            {
                throw $ex;
            }
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == ($NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate"));
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with unknown intermediate thrown as base exception... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived2AsBase();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(!($b instanceof $excls))
            {
                throw $ex;
            }
            test($b->b == "UnknownMostDerived2.b");
            test(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"));
        }
    }
    echo "ok\n";

    return $test;
}

$communicator = Ice_initialize($argv);
$test = allTests($communicator);
$test->shutdown();
$communicator->destroy();
exit();
?>
