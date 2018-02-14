<?
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

$NS = function_exists("Ice\\initialize");
require_once ($NS ? 'Ice_ns.php' : 'Ice.php');
require_once 'Test.php';

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

function allTests($communicator)
{
    $ref = "test:default -p 12010";
    $obj = $communicator->stringToProxy($ref);
    test($obj != null);
    $proxy = $obj->ice_checkedCast("::Test::TestIntf");
    test($proxy != null);

    global $NS;

    $ByteConst1 = $NS ? constant("Test\\ByteConst1") : constant("Test_ByteConst1");
    $ShortConst1 = $NS ? constant("Test\\ShortConst1") : constant("Test_ShortConst1");
    $IntConst1 = $NS ? constant("Test\\IntConst1") : constant("Test_IntConst1");
    $LongConst1 = $NS ? constant("Test\\LongConst1") : constant("Test_LongConst1");

    $ByteConst2 = $NS ? constant("Test\\ByteConst2") : constant("Test_ByteConst2");
    $ShortConst2 = $NS ? constant("Test\\ShortConst2") : constant("Test_ShortConst2");
    $IntConst2 = $NS ? constant("Test\\IntConst2") : constant("Test_IntConst2");
    $LongConst2 = $NS ? constant("Test\\LongConst2") : constant("Test_LongConst2");

    $benum1 = $NS ? constant("Test\\ByteEnum::benum1") : constant("Test_ByteEnum::benum1");
    $benum2 = $NS ? constant("Test\\ByteEnum::benum2") : constant("Test_ByteEnum::benum2");
    $benum3 = $NS ? constant("Test\\ByteEnum::benum3") : constant("Test_ByteEnum::benum3");
    $benum4 = $NS ? constant("Test\\ByteEnum::benum4") : constant("Test_ByteEnum::benum4");
    $benum5 = $NS ? constant("Test\\ByteEnum::benum5") : constant("Test_ByteEnum::benum5");
    $benum6 = $NS ? constant("Test\\ByteEnum::benum6") : constant("Test_ByteEnum::benum6");
    $benum7 = $NS ? constant("Test\\ByteEnum::benum7") : constant("Test_ByteEnum::benum7");
    $benum8 = $NS ? constant("Test\\ByteEnum::benum8") : constant("Test_ByteEnum::benum8");
    $benum9 = $NS ? constant("Test\\ByteEnum::benum9") : constant("Test_ByteEnum::benum9");
    $benum10 = $NS ? constant("Test\\ByteEnum::benum10") : constant("Test_ByteEnum::benum10");
    $benum11 = $NS ? constant("Test\\ByteEnum::benum11") : constant("Test_ByteEnum::benum11");

    $senum1 = $NS ? constant("Test\\ShortEnum::senum1") : constant("Test_ShortEnum::senum1");
    $senum2 = $NS ? constant("Test\\ShortEnum::senum2") : constant("Test_ShortEnum::senum2");
    $senum3 = $NS ? constant("Test\\ShortEnum::senum3") : constant("Test_ShortEnum::senum3");
    $senum4 = $NS ? constant("Test\\ShortEnum::senum4") : constant("Test_ShortEnum::senum4");
    $senum5 = $NS ? constant("Test\\ShortEnum::senum5") : constant("Test_ShortEnum::senum5");
    $senum6 = $NS ? constant("Test\\ShortEnum::senum6") : constant("Test_ShortEnum::senum6");
    $senum7 = $NS ? constant("Test\\ShortEnum::senum7") : constant("Test_ShortEnum::senum7");
    $senum8 = $NS ? constant("Test\\ShortEnum::senum8") : constant("Test_ShortEnum::senum8");
    $senum9 = $NS ? constant("Test\\ShortEnum::senum9") : constant("Test_ShortEnum::senum9");
    $senum10 = $NS ? constant("Test\\ShortEnum::senum10") : constant("Test_ShortEnum::senum10");
    $senum11 = $NS ? constant("Test\\ShortEnum::senum11") : constant("Test_ShortEnum::senum11");

    $ienum1 = $NS ? constant("Test\\IntEnum::ienum1") : constant("Test_IntEnum::ienum1");
    $ienum2 = $NS ? constant("Test\\IntEnum::ienum2") : constant("Test_IntEnum::ienum2");
    $ienum3 = $NS ? constant("Test\\IntEnum::ienum3") : constant("Test_IntEnum::ienum3");
    $ienum4 = $NS ? constant("Test\\IntEnum::ienum4") : constant("Test_IntEnum::ienum4");
    $ienum5 = $NS ? constant("Test\\IntEnum::ienum5") : constant("Test_IntEnum::ienum5");
    $ienum6 = $NS ? constant("Test\\IntEnum::ienum6") : constant("Test_IntEnum::ienum6");
    $ienum7 = $NS ? constant("Test\\IntEnum::ienum7") : constant("Test_IntEnum::ienum7");
    $ienum8 = $NS ? constant("Test\\IntEnum::ienum8") : constant("Test_IntEnum::ienum8");
    $ienum9 = $NS ? constant("Test\\IntEnum::ienum9") : constant("Test_IntEnum::ienum9");
    $ienum10 = $NS ? constant("Test\\IntEnum::ienum10") : constant("Test_IntEnum::ienum10");
    $ienum11 = $NS ? constant("Test\\IntEnum::ienum11") : constant("Test_IntEnum::ienum11");
    $ienum12 = $NS ? constant("Test\\IntEnum::ienum12") : constant("Test_IntEnum::ienum12");

    $red = $NS ? constant("Test\\SimpleEnum::red") : constant("Test_SimpleEnum::red");
    $green = $NS ? constant("Test\\SimpleEnum::green") : constant("Test_SimpleEnum::green");
    $blue = $NS ? constant("Test\\SimpleEnum::blue") : constant("Test_SimpleEnum::blue");

    echo "testing enum values... ";
    flush();

    test($benum1 == 0);
    test($benum2 == 1);
    test($benum3 == $ByteConst1);
    test($benum4 == $ByteConst1 + 1);
    test($benum5 == $ShortConst1);
    test($benum6 == $ShortConst1 + 1);
    test($benum7 == $IntConst1);
    test($benum8 == $IntConst1 + 1);
    test($benum9 == $LongConst1);
    test($benum10 == $LongConst1 + 1);
    test($benum11 == $ByteConst2);

    test($senum1 == 3);
    test($senum2 == 4);
    test($senum3 == $ByteConst1);
    test($senum4 == $ByteConst1 + 1);
    test($senum5 == $ShortConst1);
    test($senum6 == $ShortConst1 + 1);
    test($senum7 == $IntConst1);
    test($senum8 == $IntConst1 + 1);
    test($senum9 == $LongConst1);
    test($senum10 == $LongConst1 + 1);
    test($senum11 == $ShortConst2);

    test($ienum1 == 0);
    test($ienum2 == 1);
    test($ienum3 == $ByteConst1);
    test($ienum4 == $ByteConst1 + 1);
    test($ienum5 == $ShortConst1);
    test($ienum6 == $ShortConst1 + 1);
    test($ienum7 == $IntConst1);
    test($ienum8 == $IntConst1 + 1);
    test($ienum9 == $LongConst1);
    test($ienum10 == $LongConst1 + 1);
    test($ienum11 == $IntConst2);
    test($ienum12 == $LongConst2);

    test($red == 0);
    test($green == 1);
    test($blue == 2);

    echo "ok\n";

    echo "testing enum operations... ";
    flush();

    $byteEnum = 0;
    test($proxy->opByte($benum1, $byteEnum) == $benum1);
    test($byteEnum == $benum1);
    test($proxy->opByte($benum11, $byteEnum) == $benum11);
    test($byteEnum == $benum11);

    $shortEnum = 0;
    test($proxy->opShort($senum1, $shortEnum) == $senum1);
    test($shortEnum == $senum1);
    test($proxy->opShort($senum11, $shortEnum) == $senum11);
    test($shortEnum == $senum11);

    $intEnum = 0;
    test($proxy->opInt($ienum1, $intEnum) == $ienum1);
    test($intEnum == $ienum1);
    test($proxy->opInt($ienum11, $intEnum) == $ienum11);
    test($intEnum == $ienum11);
    test($proxy->opInt($ienum12, $intEnum) == $ienum12);
    test($intEnum == $ienum12);

    $s = 0;
    test($proxy->opSimple($green, $s) == $green);
    test($s == $green);

    echo "ok\n";

    echo "testing enum exceptions... ";
    flush();

    try
    {
        $proxy->opByte(-1, $byteEnum); // Negative enumerators are not supported
        test(false);
    }
    catch(InvalidArgumentException $ex)
    {
    }

    try
    {
        $proxy->opByte(2, $byteEnum); // Invalid enumerator
        test(false);
    }
    catch(InvalidArgumentException $ex)
    {
    }

    try
    {
        $proxy->opByte(128, $byteEnum); // Invalid enumerator
        test(false);
    }
    catch(InvalidArgumentException $ex)
    {
    }

    try
    {
        $proxy->opShort(-1, $shortEnum); // Negative enumerators are not supported
        test(false);
    }
    catch(InvalidArgumentException $ex)
    {
    }

    try
    {
        $proxy->opShort(0, $shortEnum); // Invalid enumerator
        test(false);
    }
    catch(InvalidArgumentException $ex)
    {
    }

    try
    {
        $proxy->opShort(128, $shortEnum); // Invalid enumerator
        test(false);
    }
    catch(InvalidArgumentException $ex)
    {
    }

    try
    {
        $proxy->opInt(-1, $intEnum); // Negative enumerators are not supported
        test(false);
    }
    catch(InvalidArgumentException $ex)
    {
    }

    try
    {
        $proxy->opInt(2, $intEnum); // Invalid enumerator
        test(false);
    }
    catch(InvalidArgumentException $ex)
    {
    }

    echo "ok\n";

    return $proxy;
}

$communicator = Ice_initialize($argv);
$t = allTests($communicator);
$t->shutdown();
$communicator->destroy();
exit();
?>
