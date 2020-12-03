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
atomic<int> toPush = 0;
mutex mut;
int NumThreads[] = { 4,8,16,32 };
int mutexIndex;
atomic<int> atomicIndex;
///////////////////////////////////////
vector<int> consumerNums = { 1, 2, 4 };
vector<int> produserNums = { 1, 2, 4 };
vector<int> sizes = { 1, 4, 16 };
atomic<int> taskNum = 102 * 1024;


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

		vector<thread> threads;

		mutexIndex = 0;
        atomicIndex = 0;     

		for (int i = 0; i < numThreads; i++)
		{
			threads.push_back(thread(func, array, sleep));
		}

		for (int i = 0; i < numThreads; i++)
		{
			threads[i].join();
		}
		auto end = std::chrono::system_clock::now();

		std::chrono::duration<double> time = end - start;

		check(array);

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
    atomic<int> count = 0;

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



void producer(Queue& q)
{
    for (int i = 0; i < taskNum; i++)
    {
        q.push(1);
        toPush.fetch_add(1);
    }
}


void consumer_for_all(Queue& q, atomic<int>& sum, int producerNum ) {

    while (sum.load() < taskNum * producerNum) {
        uint8_t k;
        if (q.pop(k))
        {
        sum += k;
        }
    }

}



void Task(Queue& q) {


    for (auto consumerNum : consumerNums) {
        for (auto producerNum : produserNums) {

            auto start = chrono::high_resolution_clock::now();

            atomic<int> sum = 0;


            vector<thread> threads;

            for (int i = 0; i < producerNum; i++)
                threads.push_back(thread(producer, std::ref(q)));


            for (int i = 0; i < consumerNum; i++)
                threads.push_back(thread(consumer_for_all, std::ref(q), std::ref(sum), producerNum));
            // threads.push_back(thread(consumer,std::ref(b),std::ref(sum),producerNum,consumerNum));

            for (int i = 0; i < consumerNum + producerNum; i++)
                threads[i].join();


            cout << "Производители: " << producerNum << "\tПотребители: " << consumerNum << endl;

            cout << "Сумма: " << sum << "\tОжидаемая сумма: " << taskNum * producerNum << endl;

            auto end = chrono::high_resolution_clock::now();

            chrono::duration<double> time = end - start;

            cout << "Время: " << time.count() << "\n\n";
        }
    }
}

int main() {
    setlocale(LC_ALL, "russian");

    //cout << "Mutex без усыпления: " << endl;
    //Task(mutexCounter);
    //cout << endl << "Mutex с усыплением: " << endl;
    //Task(mutexCounter, true);
    //cout << endl << "Atomic без усыпления: " << endl;
    //Task(atomicCounter);
    //cout << endl << "Atomic с усыплением: " << endl;
    //Task(atomicCounter, true);

    DynamicMutexQueue a;
    Task(a);

   /* vector<int> sizes = { 1,4,16 };
    for (auto size : sizes) {
        StaticMutexQueue b(size);
        cout << "Размер: " << size << endl;
        Task(b);
    }*/


    return 0;
}
