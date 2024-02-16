//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Timer.h>
#include <IceUtil/Random.h>
#include <TestHelper.h>

#include <vector>

using namespace IceUtil;
using namespace std;

template<typename T>
struct TargetLess
{
    bool operator()(const T& lhs, const T& rhs) const
    {
        if(lhs && rhs)
        {
            return *lhs < *rhs;
        }
        else
        {
            return !lhs && rhs;
        }
    }
};

class TestTask : public IceUtil::TimerTask
{
public:

    TestTask() : _count(0)
    {
    }

    TestTask(const IceUtil::Time& scheduledTime) : _scheduledTime(scheduledTime), _count(0)
    {
    }

    virtual void
    runTimerTask()
    {
        lock_guard lock(_mutex);
        ++_count;
        _run = IceUtil::Time::now(IceUtil::Time::Monotonic);
        _condition.notify_all();
    }

    virtual bool
    operator<(const TestTask& r) const
    {
        return _scheduledTime < r._scheduledTime;
    }

    virtual bool
    hasRun() const
    {
        lock_guard lock(_mutex);
        return _run != IceUtil::Time();
    }

    int
    getCount() const
    {
        lock_guard lock(_mutex);
        return _count;
    }

    virtual IceUtil::Time
    getRunTime() const
    {
        lock_guard lock(_mutex);
        return _run;
    }

    IceUtil::Time
    getScheduledTime() const
    {
        return _scheduledTime;
    }

    virtual void
    waitForRun()
    {
        unique_lock lock(_mutex);
        while(_run == IceUtil::Time())
        {
            if(_condition.wait_for(lock, chrono::seconds(10)) == cv_status::timeout)
            {
                test(false); // Timeout.
            }
        }
    }

    void
    clear()
    {
        _run = IceUtil::Time();
        _count = 0;
    }

private:

    IceUtil::Time _run;
    IceUtil::Time _scheduledTime;
    int _count;
    mutable mutex _mutex;
    condition_variable _condition;
};
using TestTaskPtr = std::shared_ptr<TestTask>;

class DestroyTask : public IceUtil::TimerTask
{
public:

    DestroyTask(const IceUtil::TimerPtr& timer) : _timer(timer), _run(false)
    {
    }

    virtual void
    runTimerTask()
    {
        lock_guard lock(_mutex);
        _timer->destroy();
        _run = true;
        _condition.notify_one();
    }

    virtual void
    waitForRun()
    {
        unique_lock lock(_mutex);
        while(!_run)
        {
            if(_condition.wait_for(lock, chrono::seconds(10)) == cv_status::timeout)
            {
                test(false); // Timeout.
            }
        }
    }

private:

    IceUtil::TimerPtr _timer;
    bool _run;
    mutable mutex _mutex;
    condition_variable _condition;
};
using DestroyTaskPtr = std::shared_ptr<DestroyTask>;

class Client : public Test::TestHelper
{
public:

    void run(int argc, char* argv[]);

};

void
Client::run(int, char*[])
{
    cout << "testing timer... " << flush;
    {
        IceUtil::TimerPtr timer = IceUtil::Timer::create();

        {
            TestTaskPtr task = make_shared<TestTask>();
            timer->schedule(task, IceUtil::Time());
            task->waitForRun();
            task->clear();

            //
            // Verify that the same task cannot be scheduled more than once.
            //
            timer->schedule(task, IceUtil::Time::milliSeconds(100));
            try
            {
                timer->schedule(task, IceUtil::Time());
            }
            catch(const IceUtil::IllegalArgumentException&)
            {
                // Expected.
            }
            task->waitForRun();
            task->clear();
        }

        {
            TestTaskPtr task = make_shared<TestTask>();
            test(!timer->cancel(task));
            timer->schedule(task, IceUtil::Time::seconds(1));
            test(!task->hasRun() && timer->cancel(task) && !task->hasRun());
            test(!timer->cancel(task));
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1100));
            test(!task->hasRun());
        }

        {
            vector<TestTaskPtr> tasks;
            IceUtil::Time start = IceUtil::Time::now(IceUtil::Time::Monotonic) + IceUtil::Time::milliSeconds(500);
            for(int i = 0; i < 20; ++i)
            {
                tasks.push_back(make_shared<TestTask>(IceUtil::Time::milliSeconds(500 + i * 50)));
            }

            IceUtilInternal::shuffle(tasks.begin(), tasks.end());
            vector<TestTaskPtr>::const_iterator p;
            for(p = tasks.begin(); p != tasks.end(); ++p)
            {
                timer->schedule(*p, (*p)->getScheduledTime());
            }

            for(p = tasks.begin(); p != tasks.end(); ++p)
            {
                (*p)->waitForRun();
            }

            test(IceUtil::Time::now(IceUtil::Time::Monotonic) > start);

            sort(tasks.begin(), tasks.end(), TargetLess<shared_ptr<TestTask>>());
            for(p = tasks.begin(); p + 1 != tasks.end(); ++p)
            {
                if((*p)->getRunTime() > (*(p + 1))->getRunTime())
                {
                    test(false);
                }
            }
        }

        {
            TestTaskPtr task = make_shared<TestTask>();
            timer->scheduleRepeated(task, IceUtil::Time::milliSeconds(20));
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
            test(task->hasRun());
            test(task->getCount() > 1);
            test(task->getCount() < 26);
            test(timer->cancel(task));
            int count = task->getCount();
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
            test(count == task->getCount() || count + 1 == task->getCount());
        }

        timer->destroy();
    }
    cout << "ok" << endl;

    cout << "testing timer destroy... " << flush;
    {
        {
            IceUtil::TimerPtr timer = IceUtil::Timer::create();
            DestroyTaskPtr destroyTask = make_shared<DestroyTask>(timer);
            timer->schedule(destroyTask, IceUtil::Time());
            destroyTask->waitForRun();
            try
            {
                timer->schedule(destroyTask, IceUtil::Time());
            }
            catch(const IceUtil::IllegalArgumentException&)
            {
                // Expected;
            }
        }
        {
            IceUtil::TimerPtr timer = IceUtil::Timer::create();
            TestTaskPtr testTask = make_shared<TestTask>();
            timer->schedule(testTask, IceUtil::Time());
            timer->destroy();
            try
            {
                timer->schedule(testTask, IceUtil::Time());
            }
            catch(const IceUtil::IllegalArgumentException&)
            {
                // Expected;
            }
        }
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
