// Copyright (c) ZeroC, Inc.

#pragma once

["cs:namespace:Ice.timeout"]
module Test
{
    sequence<byte> ByteSeq;
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    interface Timeout
    {
        void op();
        void sendData(ByteSeq seq);
        void sleep(int to);
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    interface Controller
    {
        void holdAdapter(int to);
        void resumeAdapter();
        void shutdown();
    }
}
