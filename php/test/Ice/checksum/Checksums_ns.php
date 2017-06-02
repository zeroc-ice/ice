<?php
// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

function allTests($communicator)
{
    $ref = "test:default -p 12010";
    $base = $communicator->stringToProxy($ref);
    test($base);

    $checksum = $base->ice_checkedCast("::Test::Checksum");
    test($checksum);

    //
    // Verify that no checksums are present for local types.
    //
    echo "testing checksums... ";
    flush();
    $localChecksums = Ice\sliceChecksums();
    test(count($localChecksums) > 0);
    foreach($localChecksums as $i => $value)
    {
        test(!strpos($i, "Local"));
    }

    //
    // Get server's Slice checksums.
    //
    $d = $checksum->getSliceChecksums();

    //
    // Compare the checksums. For a type FooN whose name ends in an integer N,
    // we assume that the server's type does not change for N = 1, and does
    // change for N > 1.
    //
    foreach($d as $i => $value)
    {
        $n = 0;
        preg_match("/\\d+/", $i, $matches);
        if($matches)
        {
            $n = (int)$matches[0];
        }

        test(isset($localChecksums[$i]));

        if($n <= 1)
        {
            test($localChecksums[$i] == $d[$i]);
        }
        else
        {
            test($localChecksums[$i] != $d[$i]);
        }
    }

    echo "ok\n";

    return $checksum;
}

?>
