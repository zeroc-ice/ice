// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Random.h"
#include "../../src/Ice/Timer.h"
#include "TestHelper.h"

#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace IceInternal;
using namespace std;

template<typename T> struct TargetLess
{
    bool operator()(const T& lhs, const T& rhs) const
    {
        if (lhs && rhs)
        {
            return *lhs < *rhs;
        }
        else
        {
            return !lhs && rhs;
        }
    }
};

class TestTask : public TimerTask
{
public:
    TestTask() : _count(0) {}

    TestTask(const chrono::milliseconds& scheduledTime) : _scheduledTime(scheduledTime), _count(0) {}

    void runTimerTask() override
    {
        lock_guard lock(_mutex);
        ++_count;
        _run = chrono::steady_clock::now();
        _condition.notify_all();
    }

    virtual bool operator<(const TestTask& r) const { return _scheduledTime < r._scheduledTime; }

    [[nodiscard]] virtual bool hasRun() const
    {
        lock_guard lock(_mutex);
        return _run != chrono::steady_clock::time_point();
    }

    [[nodiscard]] int getCount() const
    {
        lock_guard lock(_mutex);
        return _count;
    }

    [[nodiscard]] virtual chrono::steady_clock::time_point getRunTime() const
    {
        lock_guard lock(_mutex);
        return _run;
    }

    [[nodiscard]] chrono::milliseconds getScheduledTime() const { return _scheduledTime; }

    virtual void waitForRun()
    {
        unique_lock lock(_mutex);
        while (_run == chrono::steady_clock::time_point())
        {
            if (_condition.wait_for(lock, chrono::seconds(10)) == cv_status::timeout)
            {
                test(false); // Timeout.
            }
        }
    }

    void clear()
    {
        _run = chrono::steady_clock::time_point();
        _count = 0;
    }

private:
    chrono::steady_clock::time_point _run;
    chrono::milliseconds _scheduledTime;
    int _count;
    mutable mutex _mutex;
    condition_variable _condition;
};
using TestTaskPtr = std::shared_ptr<TestTask>;

class DestroyTask : public TimerTask
{
public:
    DestroyTask(IceInternal::TimerPtr timer) : _timer(std::move(timer)) {}

    void runTimerTask() override
    {
        lock_guard lock(_mutex);
        try
        {
            _timer->destroy();
            test(false);
        }
        catch (const std::runtime_error&)
        {
            // Expected.
        }
        _run = true;
        _condition.notify_one();
    }

    virtual void waitForRun()
    {
        unique_lock lock(_mutex);
        while (!_run)
        {
            if (_condition.wait_for(lock, chrono::seconds(10)) == cv_status::timeout)
            {
                test(false); // Timeout.
            }
        }
    }

private:
    IceInternal::TimerPtr _timer;
    bool _run{false};
    mutable mutex _mutex;
    condition_variable _condition;
};
using DestroyTaskPtr = std::shared_ptr<DestroyTask>;

class Client : public Test::TestHelper
{
public:
    void run(int argc, char* argv[]) override;
};

void
Client::run(int, char*[])
{
    cout << "testing timer... " << flush;
    {
        auto timer = make_shared<IceInternal::Timer>();
        {
            TestTaskPtr task = make_shared<TestTask>();
            timer->schedule(task, chrono::seconds::zero());
            task->waitForRun();
            task->clear();
            //
            // Verify that the same task cannot be scheduled more than once.
            //
            timer->schedule(task, chrono::milliseconds(100));
            try
            {
                timer->schedule(task, chrono::seconds::zero());
            }
            catch (const invalid_argument&)
            {
                // Expected.
            }
            task->waitForRun();
            task->clear();
        }

        {
            TestTaskPtr task = make_shared<TestTask>();
            test(!timer->cancel(task));
            timer->schedule(task, chrono::seconds(1));
            test(!task->hasRun() && timer->cancel(task) && !task->hasRun());
            test(!timer->cancel(task));
            this_thread::sleep_for(chrono::milliseconds(1100));
            test(!task->hasRun());
        }

        {
            vector<TestTaskPtr> tasks;
            tasks.reserve(20);
            auto start = chrono::steady_clock::now() + chrono::milliseconds(500);
            for (int i = 0; i < 20; ++i)
            {
                tasks.push_back(make_shared<TestTask>(chrono::milliseconds(500 + i * 50)));
            }

            IceInternal::shuffle(tasks.begin(), tasks.end());
            vector<TestTaskPtr>::const_iterator p;
            for (p = tasks.begin(); p != tasks.end(); ++p)
            {
                timer->schedule(*p, (*p)->getScheduledTime());
            }

            for (p = tasks.begin(); p != tasks.end(); ++p)
            {
                (*p)->waitForRun();
            }

            test(chrono::steady_clock::now() > start);

            sort(tasks.begin(), tasks.end(), TargetLess<shared_ptr<TestTask>>());
            for (p = tasks.begin(); p + 1 != tasks.end(); ++p)
            {
                if ((*p)->getRunTime() > (*(p + 1))->getRunTime())
                {
                    test(false);
                }
            }
        }

        {
            TestTaskPtr task = make_shared<TestTask>();
            timer->scheduleRepeated(task, chrono::milliseconds(20));
            this_thread::sleep_for(chrono::milliseconds(500));
            test(task->hasRun());
            test(task->getCount() > 1);
            test(task->getCount() < 26);
            test(timer->cancel(task));
            int count = task->getCount();
            this_thread::sleep_for(chrono::milliseconds(100));
            test(count == task->getCount() || count + 1 == task->getCount());
        }

        timer->destroy();
    }
    cout << "ok" << endl;

    cout << "testing timer destroy... " << flush;
    {
        {
            auto timer = make_shared<IceInternal::Timer>();
            DestroyTaskPtr destroyTask = make_shared<DestroyTask>(timer);
            timer->schedule(destroyTask, chrono::seconds::zero());
            destroyTask->waitForRun();
            try
            {
                timer->schedule(destroyTask, chrono::seconds::zero());
            }
            catch (const invalid_argument&)
            {
                // Expected;
            }
            timer->destroy();
        }
        {
            auto timer = make_shared<IceInternal::Timer>();
            TestTaskPtr testTask = make_shared<TestTask>();
            timer->schedule(testTask, chrono::seconds::zero());
            timer->destroy();
            try
            {
                timer->schedule(testTask, chrono::seconds::zero());
            }
            catch (const invalid_argument&)
            {
                // Expected;
            }
        }
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
