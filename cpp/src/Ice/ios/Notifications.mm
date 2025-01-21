// Copyright (c) ZeroC, Inc.

#import "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    import <Foundation/NSNotification.h>
#    import <Foundation/NSObject.h>
#    import <UIKit/UIApplication.h>

#    include "../ConnectionFactory.h"

#    include <mutex>
#    include <set>

using namespace std;
using namespace IceInternal;

namespace IceInternal
{
    bool registerForBackgroundNotification(const IncomingConnectionFactoryPtr&);
    void unregisterForBackgroundNotification(const IncomingConnectionFactoryPtr&);
}

namespace
{
    class Observer
    {
    public:
        Observer() : _background(false)
        {
            _backgroundObserver =
                [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationDidEnterBackgroundNotification
                                                                  object:nil
                                                                   queue:nil
                                                              usingBlock:^(NSNotification*) {
                                                                didEnterBackground();
                                                              }];

            _foregroundObserver =
                [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationWillEnterForegroundNotification
                                                                  object:nil
                                                                   queue:nil
                                                              usingBlock:^(NSNotification*) {
                                                                willEnterForeground();
                                                              }];
        }

        ~Observer()
        {
            [[NSNotificationCenter defaultCenter] removeObserver:_backgroundObserver];
            [[NSNotificationCenter defaultCenter] removeObserver:_foregroundObserver];
        }

        bool add(const IncomingConnectionFactoryPtr& factory)
        {
            lock_guard lock(_mutex);
            if (_background)
            {
                factory->stopAcceptor();
            }
            else
            {
                factory->startAcceptor(); // Might throw
            }
            _factories.insert(factory);
            return _background;
        }

        void remove(const IncomingConnectionFactoryPtr& factory)
        {
            lock_guard lock(_mutex);
            _factories.erase(factory);
        }

        void didEnterBackground()
        {
            lock_guard lock(_mutex);

            //
            // Notify all the incoming connection factories that we are
            // entering the background mode.
            //
            for (set<IncomingConnectionFactoryPtr>::const_iterator p = _factories.begin(); p != _factories.end(); ++p)
            {
                (*p)->stopAcceptor();
            }
            _background = true;
        }

        void willEnterForeground()
        {
            lock_guard lock(_mutex);

            //
            // Notify all the incoming connection factories that we are
            // entering the foreground mode.
            //
            _background = false;
            for (set<IncomingConnectionFactoryPtr>::const_iterator p = _factories.begin(); p != _factories.end(); ++p)
            {
                (*p)->startAcceptor();
            }
        }

    private:
        mutex _mutex;
        bool _background;
        id _backgroundObserver;
        id _foregroundObserver;
        set<IncomingConnectionFactoryPtr> _factories;
    };
}

static Observer observer;

bool
IceInternal::registerForBackgroundNotification(const IncomingConnectionFactoryPtr& factory)
{
    return observer.add(factory);
}

void
IceInternal::unregisterForBackgroundNotification(const IncomingConnectionFactoryPtr& factory)
{
    observer.remove(factory);
}

#endif
