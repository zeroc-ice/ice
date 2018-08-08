<?php
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

function allTests($helper)
{
    global $Ice_sliceChecksums;
    $communicator = $helper->communicator();
    $ref = sprintf("test:%s", $helper->getTestEndpoint());
    $base = $communicator->stringToProxy($ref);
    test($base);

    $checksum = $base->ice_checkedCast("::Test::Checksum");
    test($checksum);

    //
    // Verify that no checksums are present for local types.
    //
    echo "testing checksums... ";
    flush();
    test(count($Ice_sliceChecksums) > 0);
    foreach($Ice_sliceChecksums as $i => $value)
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

        test(isset($Ice_sliceChecksums[$i]));

        if($n <= 1)
        {
            test($Ice_sliceChecksums[$i] == $d[$i]);
        }
        else
        {
            test($Ice_sliceChecksums[$i] != $d[$i]);
        }
    }

    echo "ok\n";

    return $checksum;
}

?>
