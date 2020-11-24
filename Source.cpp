#include <cassert>
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <queue>
#include <string>
#include <chrono>

#define NumTasks (1024 * 1024)

using namespace std;


mutex mut;
int NumThreads[] = { 4,8,16,32 };
int mutexIndex;
atomic<int> atomicIndex;
///////////////////////////////////////
vector<int> consumerNums = { 1, 2, 4 };
vector<int> produserNums = { 1, 2, 4 };
int taskNum = 1024 * 10;


////Task 1
int* arrayInit()
{
	int* array = new int[NumTasks];
	for (int i = 0; i < NumTasks; i++)
	{
		array[i] = 0;
	}

	return array;
}


void check(int* array)
{
	for (int i = 0; i < NumTasks; i++)
	{
		assert(array[i] == 1);
	}
}


void Task(void(*func) (int* array, bool flag), bool sleep = false)
{
	for (int numThreads : NumThreads)
	{
        auto start = std::chrono::system_clock::now();

		int* array = arrayInit();
        int* arr = new int[NumTasks];
        for (int i = 0; i < NumTasks; i++)
            arr[i] = 0;
		vector<thread> threads;

		mutexIndex = 0;
        atomicIndex = 0;     

		for (int i = 0; i < numThreads; i++)
		{
			threads.push_back(thread(func, arr, sleep));
		}

		for (int i = 0; i < numThreads; i++)
		{
			threads[i].join();
		}
		auto end = std::chrono::system_clock::now();

		std::chrono::duration<double> time = end - start;

		check(arr);

		cout << "Количество потоков: " << numThreads << endl;
		cout << "Время: " << time.count() << " c" << endl << endl;;
	}
}

void mutexCounter(int* array, bool sleep)
{
    int counter = 0;

    do
    {
        mut.lock();
        counter = mutexIndex++;
        mut.unlock();
        array[counter]++;
        if (sleep)
            this_thread::sleep_for(chrono::nanoseconds(10));
    } while (counter < NumTasks);


}

void atomicCounter(int* array, bool sleep)
{

	int counter = 0;
	do
	{
		counter = atomicIndex.fetch_add(1);
		array[counter]++;
		if (sleep)
			this_thread::sleep_for(chrono::nanoseconds(10));

	} while (counter < NumTasks);

}

////Task 2.1
class Queue {
public:
    virtual void push(uint8_t val) = 0;
    virtual bool pop(uint8_t& val) = 0;

};

class DynamicMutexQueue :public Queue {
private:

    queue<uint8_t> q;
    mutex m;

public:
    void push(uint8_t val) override {

        m.lock();
        q.push(val);
        m.unlock();

    }

    bool pop(uint8_t& val) override {

        m.lock();
        if (q.empty()) {
            m.unlock();
            this_thread::sleep_for(chrono::milliseconds(1));
            m.lock();
            if (q.empty())
            {
                m.unlock();
                return false;
            }

        }
        val = q.front();
        q.pop();
        m.unlock();
        return true;

    }
};

////Task 2.2
class StaticMutexQueue : public Queue
{

private:
    queue<uint8_t> q;
    int queue_size;
    mutex m;
    condition_variable push_condition;
    condition_variable pop_condition;

public:
    StaticMutexQueue(int size)
    {
        this->queue_size = size;
    }

    void push(uint8_t val) override
    {
        unique_lock<mutex> ul(m);
        push_condition.wait(ul, [&]() {return q.size() < queue_size; });
        q.push(val);
        pop_condition.notify_one();
    }

    bool pop(uint8_t& val) override
    {
        unique_lock<mutex> ul(m);
        if (q.empty())
        {
            pop_condition.wait_for(ul, std::chrono::milliseconds(1));
            if (q.empty())
            {
                ul.unlock();
                return false;
            }
        }
        val = q.front();
        q.pop();
        push_condition.notify_one();
        return true;

    }



};

void Task(Queue& q) {


    for (auto consumerNum : consumerNums) {
        for (auto producerNum : produserNums) {

            auto start = chrono::high_resolution_clock::now();

            atomic<int> sum = 0;

            auto producer = [&]() {
                for (int i = 0; i < taskNum; i++) {
                    q.push(1);
                }
            };

            auto consumer = [&]() {

                for (int i = 0; i < taskNum * producerNum / consumerNum; i++) {
                    uint8_t poppedValue = 0;
                    while (!q.pop(poppedValue));
                    sum += poppedValue;
                }
            };


            vector<thread> threads;
            for (int i = 0; i < producerNum; i++)
                threads.push_back(thread(producer));

            for (int i = 0; i < consumerNum; i++)
                threads.push_back(thread(consumer));

            for (int i = 0; i < consumerNum + producerNum; i++)
                threads[i].join();


            cout << "Производители: " << consumerNum << "\tПотребители: " << producerNum << endl;

            cout << "Сумма: " << sum << "\tОжидаемая сумма: " << taskNum * producerNum << endl;

            auto end = chrono::high_resolution_clock::now();

            chrono::duration<double> time = end - start;

            cout << "Время: " << time.count() << "\n\n";

            assert(sum == taskNum * producerNum);
        }
    }
}


int main() {
    setlocale(LC_ALL, "russian");

    cout << "Mutex без усыпления: " << endl;
    Task(mutexCounter);
    //cout << endl << "Mutex с усыплением: " << endl;
    //Task(mutexCounter, true);
    //cout << endl << "Atomic без усыпления: " << endl;
    //Task(atomicCounter);
    //cout << endl << "Atomic с усыплением: " << endl;
    //Task(atomicCounter, true);

    //DynamicMutexQueue a;
    //Task(a);

    //vector<int> sizes = { 1,4,16 };
    //for (auto size : sizes) {
    //    StaticMutexQueue b(size);
    //    cout << "Размер: " << size << endl;
    //    Task(b);
    //}


    return 0;
}