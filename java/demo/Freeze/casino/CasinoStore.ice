// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Casino.ice>
#include <Ice/Identity.ice>

//
// Provide the persistent classes used to implement Casino
//
module CasinoStore
{
    //
    // Potential bet winner
    //
    interface Winner
    {
        //
        // Win a bet
        //
        ["freeze:write:mandatory"] void win(int count);
    };
    ["java:type:java.util.Vector<WinnerPrx>:java.util.Vector<WinnerPrx>"] sequence<Winner*> WinnerPrxSeq;


    class PersistentPlayer implements Casino::Player, Winner
    {
        //
        // A bet withdraws a number of chips from this player
        //
        ["freeze:write:mandatory"] void withdraw(int count) throws Casino::OutOfChipsException;

        int chips;
    };

    class PersistentBet implements Casino::Bet
    {
        //
        // The total number of chips in play, i.e. amount * potentialWinners.size()
        //
        int getChipsInPlay();

        //
        // Milliseconds since the epoch
        //
        long getCloseTime();

        //
        // Decide who wins and self-destroys
        //
        ["freeze:write"] void complete(int random);

        //
        // The first potential winner is the Bank
        //
        WinnerPrxSeq potentialWinners;

        //
        // Each potential winner puts amount in play
        //
        int amount;

        //
        // The time when the bet completes (ms since the epoch)
        //
        long closeTime;
    }; 
    
    class PersistentBank implements Casino::Bank, Winner
    {
        //
        // Player p returns all its chips
        //
        ["freeze:write:mandatory"] void returnAllChips(PersistentPlayer* p);

        //
        // Reload the existing bets into the Bet resolver
        //
        void reloadBets();

        int chipsSold;
        int outstandingChips;
    };
};
