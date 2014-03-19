// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Casino.h>
#include <stdlib.h>

//
// Number of bets placed by each player
// You can (should) increase these values by a factor of 5 or more
// on a fast system
//
const int betCount1 = 100;
const int betCount2 = 20;

using namespace std;

class CasinoClient : public Ice::Application
{
public:
    virtual int run(int, char*[]);

private:
    void printBalances(Casino::PlayerPrxSeq&) const;
};

int
main(int argc, char* argv[])
{
    CasinoClient app;
    return app.main(argc, argv, "config.client");
}

int
CasinoClient::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    //
    // Initialize pseudo-random number generator
    //
    srand((unsigned int)IceUtil::Time::now().toMicroSeconds());

    cout << "Retrieve bank and players... " << flush;

    Casino::BankPrx bank = Casino::BankPrx::uncheckedCast(communicator()->propertyToProxy("Bank.Proxy"));

    Casino::PlayerPrxSeq players = bank->getPlayers();
    cout << "ok" << endl;

    cout << "Starting balances" << endl;
    printBalances(players);

    cout << "Current bank earnings: " << bank->getEarnings() << " chips" << endl;

    cout << "All chips accounted for? " << (bank->checkAllChips() ? "yes" : "no") << endl;

    cout << "Each player buys 3,000 chips... " << flush;

    for(size_t i = 0; i < players.size(); ++i)
    {
        Casino::PlayerPrx player = players[i];
        if(player != 0)
        {
            if(!bank->buyChips(3000, player))
            {
                cout << "(" << player->ice_getIdentity().name << " is gone) " << flush;
                players[i] = 0;
            }
        }
    }
    cout << "ok" << endl;

    cout << "All chips accounted for? " << (bank->checkAllChips() ? "yes" : "no") << endl;

    cout << "Create " << betCount1 <<  " 10-chips bets... " << flush;

    int b;
    for(b = 0; b < betCount1; ++b)
    {
        Casino::BetPrx bet = bank->createBet(10, 200 + rand() % 4000);
        for(size_t i = 0; i < players.size(); ++i)
        {
            Casino::PlayerPrx player = players[i];
            if(player != 0)
            {
                try
                {
                    bet->accept(player);
                }
                catch(const Ice::ObjectNotExistException&)
                {
                    //
                    // Bet already resolved
                    //
                }
                catch(const Casino::OutOfChipsException&)
                {
                    cout << "(" << player->ice_getIdentity().name << " is out) " << flush;

                    players[i] = 0;
                }
            }
        }
    }
    cout << " ok" << endl;

    cout << "Live bets: " << bank->getLiveBetCount() << endl;

    int index = static_cast<int>(rand() % players.size());
    Casino::PlayerPrx gonner = players[index];
    players[index] = 0;

    if(gonner != 0)
    {
        cout << "Destroying " << gonner->ice_getIdentity().name + "... " << flush;
        try
        {
            gonner->destroy();
        }
        catch(const Ice::ObjectNotExistException&)
        {
            //
            // Ignored
            //
        }
        cout << "ok" << endl;
    }

    cout << "All chips accounted for? " << (bank->checkAllChips() ? "yes" : "no") << endl;

    cout << "Sleep for 2 seconds" << endl;
    IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(2));

    cout << "Live bets: " << bank->getLiveBetCount() << endl;

    cout << "Create " << betCount2 << " 10-chips bets... " << flush;

    for(b = 0; b < betCount2; ++b)
    {
        Casino::BetPrx bet = bank->createBet(10, 200 + rand() % 4000);
        for(size_t i = 0; i < players.size(); ++i)
        {
            Casino::PlayerPrx player = players[i];
            if(player != 0)
            {
                try
                {
                    bet->accept(player);
                }
                catch(const Ice::ObjectNotExistException&)
                {
                    //
                    // Bet already resolved
                    //
                }
                catch(const Casino::OutOfChipsException&)
                {
                    cout << "(" << player->ice_getIdentity().name << " is out) " << flush;

                    players[i] = 0;
                }
            }
        }
    }
    cout << " ok" << endl;

    cout << "Live bets: " << bank->getLiveBetCount() << endl;
    cout << "Sleep for 10 seconds" << endl;
    IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(10));
    cout << "Live bets: " << bank->getLiveBetCount() << endl;

    cout << "Ending balances" << endl;
    printBalances(players);

    cout << "Current bank earnings: " << bank->getEarnings() << " chips" << endl;

    cout << "All chips accounted for? " << (bank->checkAllChips() ? "yes" : "no") << endl;

    return 0;
}

void
CasinoClient::printBalances(Casino::PlayerPrxSeq& players) const
{
    for(size_t i = 0; i < players.size(); ++i)
    {
        Casino::PlayerPrx player = players[i];

        if(player != 0)
        {
            try
            {
                int chips = player->getChips();
                cout << player->ice_getIdentity().name << ": " << chips << " chips" << endl;
            }
            catch(const Ice::ObjectNotExistException&)
            {
                //
                // This player is gone
                //
                players[i] = 0;
            }
        }
    }
}
