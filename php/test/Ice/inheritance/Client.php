<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function allTests($helper)
{
    $ref = sprintf("initial:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $initial = Test\InitialPrxHelper::createProxy($communicator, $ref);

    echo "getting proxies for interface hierarchy... ";
    flush();
    $ia = $initial->iaop();
    $ib1 = $initial->ib1op();
    $ib2 = $initial->ib2op();
    $ic = $initial->icop();
    test($ia != $ib1);
    test($ia != $ib2);
    test($ia != $ic);
    test($ib1 != $ic);
    test($ib2 != $ic);
    echo "ok\n";

    echo "invoking proxy operations on interface hierarchy... ";
    flush();

    $iao = $ia->iaop($ia);
    test($iao == $ia);
    $iao = $ia->iaop($ib1);
    test($iao == $ib1);
    $iao = $ia->iaop($ib2);
    test($iao == $ib2);
    $iao = $ia->iaop($ic);
    test($iao == $ic);
    $iao = $ib1->iaop($ia);
    test($iao == $ia);
    $iao = $ib1->iaop($ib1);
    test($iao == $ib1);
    $iao = $ib1->iaop($ib2);
    test($iao == $ib2);
    $iao = $ib1->iaop($ic);
    test($iao == $ic);
    $iao = $ib2->iaop($ia);
    test($iao == $ia);
    $iao = $ib2->iaop($ib1);
    test($iao == $ib1);
    $iao = $ib2->iaop($ib2);
    test($iao == $ib2);
    $iao = $ib2->iaop($ic);
    test($iao == $ic);
    $iao = $ic->iaop($ia);
    test($iao == $ia);
    $iao = $ic->iaop($ib1);
    test($iao == $ib1);
    $iao = $ic->iaop($ib2);
    test($iao == $ib2);
    $iao = $ic->iaop($ic);
    test($iao == $ic);

    $iao = $ib1->ib1op($ib1);
    test($iao == $ib1);
    $ib1o = $ib1->ib1op($ib1);
    test($ib1o == $ib1);
    $iao = $ib1->ib1op($ic);
    test($iao == $ic);
    $ib1o = $ib1->ib1op($ic);
    test($ib1o == $ic);
    $iao = $ic->ib1op($ib1);
    test($iao == $ib1);
    $ib1o = $ic->ib1op($ib1);
    test($ib1o == $ib1);
    $iao = $ic->ib1op($ic);
    test($iao == $ic);
    $ib1o = $ic->ib1op($ic);
    test($ib1o == $ic);

    $iao = $ib2->ib2op($ib2);
    test($iao == $ib2);
    $ib2o = $ib2->ib2op($ib2);
    test($ib2o == $ib2);
    $iao = $ib2->ib2op($ic);
    test($iao == $ic);
    $ib2o = $ib2->ib2op($ic);
    test($ib2o == $ic);
    $iao = $ic->ib2op($ib2);
    test($iao == $ib2);
    $ib2o = $ic->ib2op($ib2);
    test($ib2o == $ib2);
    $iao = $ic->ib2op($ic);
    test($iao == $ic);
    $ib2o = $ic->ib2op($ic);
    test($ib2o == $ic);

    $iao = $ic->icop($ic);
    test($iao == $ic);
    $ib1o = $ic->icop($ic);
    test($ib1o == $ic);
    $ib2o = $ic->icop($ic);
    test($ib2o == $ic);
    $ico = $ic->icop($ic);
    test($ico == $ic);

    echo "ok\n";

    return $initial;
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
