<?php
// Copyright (c) ZeroC, Inc.

$NS = function_exists("Ice\initialize");
require_once('Test.php');

function allTests($helper)
{
    $ref = sprintf("test:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $proxy = Test\TestIntfPrxHelper::createProxy($communicator, $ref);

    echo "testing enum values... ";
    flush();

    test(Test\ByteEnum::benum1 == 0);
    test(Test\ByteEnum::benum2 == 1);
    test(Test\ByteEnum::benum3 == Test\ByteConst1);
    test(Test\ByteEnum::benum4 == Test\ByteConst1 + 1);
    test(Test\ByteEnum::benum5 == Test\ShortConst1);
    test(Test\ByteEnum::benum6 == Test\ShortConst1 + 1);
    test(Test\ByteEnum::benum7 == Test\IntConst1);
    test(Test\ByteEnum::benum8 == Test\IntConst1 + 1);
    test(Test\ByteEnum::benum9 == Test\LongConst1);
    test(Test\ByteEnum::benum10 == Test\LongConst1 + 1);
    test(Test\ByteEnum::benum11 == Test\ByteConst2);

    test(Test\ShortEnum::senum1 == 3);
    test(Test\ShortEnum::senum2 == 4);
    test(Test\ShortEnum::senum3 == Test\ByteConst1);
    test(Test\ShortEnum::senum4 == Test\ByteConst1 + 1);
    test(Test\ShortEnum::senum5 == Test\ShortConst1);
    test(Test\ShortEnum::senum6 == Test\ShortConst1 + 1);
    test(Test\ShortEnum::senum7 == Test\IntConst1);
    test(Test\ShortEnum::senum8 == Test\IntConst1 + 1);
    test(Test\ShortEnum::senum9 == Test\LongConst1);
    test(Test\ShortEnum::senum10 == Test\LongConst1 + 1);
    test(Test\ShortEnum::senum11 == Test\ShortConst2);

    test(Test\IntEnum::ienum1 == 0);
    test(Test\IntEnum::ienum2 == 1);
    test(Test\IntEnum::ienum3 == Test\ByteConst1);
    test(Test\IntEnum::ienum4 == Test\ByteConst1 + 1);
    test(Test\IntEnum::ienum5 == Test\ShortConst1);
    test(Test\IntEnum::ienum6 == Test\ShortConst1 + 1);
    test(Test\IntEnum::ienum7 == Test\IntConst1);
    test(Test\IntEnum::ienum8 == Test\IntConst1 + 1);
    test(Test\IntEnum::ienum9 == Test\LongConst1);
    test(Test\IntEnum::ienum10 == Test\LongConst1 + 1);
    test(Test\IntEnum::ienum11 == Test\IntConst2);
    test(Test\IntEnum::ienum12 == Test\LongConst2);

    test(Test\SimpleEnum::red == 0);
    test(Test\SimpleEnum::green == 1);
    test(Test\SimpleEnum::blue == 2);

    echo "ok\n";

    echo "testing enum operations... ";
    flush();

    $byteEnum = 0;
    test($proxy->opByte(Test\ByteEnum::benum1, $byteEnum) == Test\ByteEnum::benum1);
    test($byteEnum == Test\ByteEnum::benum1);
    test($proxy->opByte(Test\ByteEnum::benum11, $byteEnum) == Test\ByteEnum::benum11);
    test($byteEnum == Test\ByteEnum::benum11);

    $shortEnum = 0;
    test($proxy->opShort(Test\ShortEnum::senum1, $shortEnum) == Test\ShortEnum::senum1);
    test($shortEnum == Test\ShortEnum::senum1);
    test($proxy->opShort(Test\ShortEnum::senum11, $shortEnum) == Test\ShortEnum::senum11);
    test($shortEnum == Test\ShortEnum::senum11);

    $intEnum = 0;
    test($proxy->opInt(Test\IntEnum::ienum1, $intEnum) == Test\IntEnum::ienum1);
    test($intEnum == Test\IntEnum::ienum1);
    test($proxy->opInt(Test\IntEnum::ienum11, $intEnum) == Test\IntEnum::ienum11);
    test($intEnum == Test\IntEnum::ienum11);
    test($proxy->opInt(Test\IntEnum::ienum12, $intEnum) == Test\IntEnum::ienum12);
    test($intEnum == Test\IntEnum::ienum12);

    $s = 0;
    test($proxy->opSimple(Test\SimpleEnum::green, $s) == Test\SimpleEnum::green);
    test($s == Test\SimpleEnum::green);

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

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $communicator = $this->initialize($args);
            $proxy= allTests($this);
            $proxy->shutdown();
            $communicator->destroy();
        }
        catch(Exception $ex)
        {
            $communicator->destroy();
            throw $ex;
        }
    }
}
?>
