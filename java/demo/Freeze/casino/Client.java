// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client extends Ice.Application
{
    //
    // Number of bets placed by each player
    // You can (should) increase these values by a factor of 5 or more
    // on a fast system
    //
    static final int betCount1 = 100;
    static final int betCount2 = 20;

    private void
    printBalances(Casino.PlayerPrx[] players)
    {
        for(int i = 0; i < players.length; ++i)
        {
            Casino.PlayerPrx player = players[i];

            if(player != null)
            {
                try
                {
                    System.out.println(player.ice_getIdentity().name + ": " + player.getChips() + " chips");
                }
                catch(Ice.ObjectNotExistException one)
                {
                    //
                    // This player is gone
                    //
                    players[i] = null;
                }
            }
        }
    }

    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        java.util.Random random = new java.util.Random();

        System.out.print("Retrieve bank and players... ");
        System.out.flush();

        Casino.BankPrx bank = Casino.BankPrxHelper.
            uncheckedCast(communicator().propertyToProxy("Bank.Proxy"));

        Casino.PlayerPrx[] players = bank.getPlayers();
        System.out.println("ok");

        System.out.println("Starting balances");
        printBalances(players);

        System.out.println("Current bank earnings: " + bank.getEarnings() + " chips");

        System.out.println("All chips accounted for? " + (bank.checkAllChips() ? "yes" : "no"));

        System.out.print("Each player buys 3,000 chips... ");
        System.out.flush();

        for(int i = 0; i < players.length; ++i)
        {
            Casino.PlayerPrx player = players[i];
            if(player != null)
            {
                if(!bank.buyChips(3000, player))
                {
                    System.out.print("(" + player.ice_getIdentity().name + " is gone) ");
                    players[i] = null;
                }
            }
        }
        System.out.println("ok");

        System.out.println("All chips accounted for? " + (bank.checkAllChips() ? "yes" : "no"));

        System.out.print("Create " + betCount1 + " 10-chips bets... ");
        System.out.flush();

        for(int b = 0; b < betCount1; ++b)
        {
            Casino.BetPrx bet = bank.createBet(10, 200 + random.nextInt(4000));
            for(int i = 0; i < players.length; ++i)
            {
                Casino.PlayerPrx player = players[i];
                if(player != null)
                {
                    try
                    {
                        bet.accept(player);
                    }
                    catch(Ice.ObjectNotExistException ex)
                    {
                        //
                        // Bet already resolved
                        //
                    }
                    catch(Casino.OutOfChipsException ex)
                    {
                        System.out.print("(" + player.ice_getIdentity().name + " is out) ");

                        players[i] = null;
                    }
                }
            }
        }
        System.out.println(" ok");

        System.out.println("Live bets: " + bank.getLiveBetCount());

        int index = random.nextInt(players.length);
        Casino.PlayerPrx gonner = players[index];
        players[index] = null;

        if(gonner != null)
        {
            System.out.print("Destroying " + gonner.ice_getIdentity().name + "... ");
            try
            {
                gonner.destroy();
            }
            catch(Ice.ObjectNotExistException e)
            {
                //
                // Ignored
                //
            }
            System.out.println("ok");
        }

        System.out.println("All chips accounted for? " + (bank.checkAllChips() ? "yes" : "no"));

        System.out.println("Sleep for 2 seconds");

        try
        {
            Thread.sleep(2000);
        }
        catch(InterruptedException ex)
        {
        }
        System.out.println("Live bets: " + bank.getLiveBetCount());

        System.out.print("Create " + betCount2 + " 10-chips bets... ");
        System.out.flush();

        for(int b = 0; b < betCount2; ++b)
        {
            Casino.BetPrx bet = bank.createBet(10, 200 + random.nextInt(4000));
            for(int i = 0; i < players.length; ++i)
            {
                Casino.PlayerPrx player = players[i];
                if(player != null)
                {
                    try
                    {
                        bet.accept(player);
                    }
                    catch(Ice.ObjectNotExistException ex)
                    {
                        //
                        // Bet already resolved
                        //
                    }
                    catch(Casino.OutOfChipsException ex)
                    {
                        System.out.print("(" + player.ice_getIdentity().name + " is out) ");

                        players[i] = null;
                    }
                }
            }
        }
        System.out.println(" ok");

        System.out.println("Live bets: " + bank.getLiveBetCount());
        System.out.println("Sleep for 10 seconds");
        try
        {
            Thread.sleep(10000);
        }
        catch(InterruptedException ex)
        {
        }
        System.out.println("Live bets: " + bank.getLiveBetCount());

        System.out.println("Ending balances");
        printBalances(players);

        System.out.println("Current bank earnings: " + bank.getEarnings() + " chips");

        System.out.println("All chips accounted for? " + (bank.checkAllChips() ? "yes" : "no"));

        return 0;
    }

    static public void
    main(String[] args)
    {
        Client app = new Client();
        app.main("demo.Freeze.casino.Client", args, "config.client");
    }
}
