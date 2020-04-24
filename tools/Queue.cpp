 #include "../enclave/Queue.h"
 #include "../include/sync_utils.hpp"

Queue::Queue()
{
    front = rear = 0;
    for (int i = 0; i < queue_size; i++)
    {
        q[i] = NULL;
    }
}

Queue::~Queue() {}

int Queue::enqueue(Request elem)
{
    spin_lock(&_lock);

    if (rear - front == queue_size)
    {
        spin_unlock(&_lock);
        abort();
        return -1;
    }

    q[rear % queue_size] = elem;
    ++rear;

    spin_unlock(&_lock);
    return 0;
}

Request Queue::dequeue()
{
    spin_lock(&_lock);

    if (front == rear)
    {
        spin_unlock(&_lock);
        return NULL;
    }

    Request result = q[front % queue_size];
    ++front;
    spin_unlock(&_lock);

    return result;
}

// Queue::Queue(){};

// void Queue::enqueue(Request x)
// {
//     std::unique_lock<std::mutex> locker(gaurd);
//     arr.push(x);
//     cond.notify_all();
// }

// Request Queue::dequeue()
// {
//     std::unique_lock<std::mutex> locker(gaurd);
//     while(arr.size() == 0)
//         cond.wait(locker);
//     Request ans = arr.front();
//     arr.pop();
//     cond.notify_all();
//     return ans;
// }

// int Queue::size()
// {
//     std::unique_lock<std::mutex> locker(gaurd);
//     return arr.size();
// }
