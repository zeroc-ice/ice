// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <IceUtil/Cond.h>
#include <IceUtil/Options.h>
#include <iostream>

using namespace std;
using namespace IceUtil;
using namespace IceUtilInternal;

enum State
{
    StateStart,
    StatePlayerA,
    StatePlayerB,
    StateGameOver
};

struct Game
{
    IceUtil::Mutex mutex;
    IceUtil::Cond cond;
    State state;
    int hits;
    bool broadcast;
    bool noise;
};

class Player : public Thread
{
public:

    Player(Game* game, State which) :
        _game(game), _which(which)
    {
    }

    virtual void
    run()
    {
        Mutex::Lock sync(_game->mutex);
        while(_game->state < StateGameOver)
        {
            ++_game->hits;
            _game->state = _which;
            if(_game->broadcast)
            {
                _game->cond.broadcast();
            }
            else
            {
                _game->cond.signal();
            }
            do
            {
                _game->cond.wait(sync);
                if(_game->state == _which && !_game->noise)
                {
                    cout << endl << "PLAYER-" << ((_which == StatePlayerA) ? "B" : "A")
                         << ": Supurious wakeup" << endl;
                }
            }
            while(_game->state == _which);
        }
    }

private:

    Game* _game;
    State _which;
};
typedef Handle<Player> PlayerPtr;

int
main(int argc, char** argv)
{
    Options opts;
    opts.addOpt("t", "timeout", Options::NeedArg, "5");
    opts.addOpt("b", "broadcast");
    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        return EXIT_FAILURE;
    }

    Game game;
    game.hits = 0;
    game.state = StateStart;
    game.broadcast = opts.isSet("b");
    game.noise = false;

    PlayerPtr playerA = new Player(&game, StatePlayerA);
    playerA->start();
    PlayerPtr playerB = new Player(&game, StatePlayerB);
    playerB->start();

    cout << "running spurious wake test (" << ((game.broadcast) ? "broadcast" : "signal") << ")... " << flush;

    ThreadControl::sleep(Time::seconds(atoi(opts.optArg("t").c_str())));

    if(game.broadcast)
    {
        {
            Mutex::Lock sync(game.mutex);
            game.noise = true;
        }
        for(int i =0 ; i < 100000; ++i)
        {
            game.cond.broadcast();
        }
        {
            Mutex::Lock sync(game.mutex);
            game.noise = false;
        }
    }

    {
        Mutex::Lock sync(game.mutex);
        game.state = StateGameOver;
        game.cond.broadcast();
    }

    playerA->getThreadControl().join();
    playerB->getThreadControl().join();

    cout << game.hits << " ok" << endl;

    return 0;
}
