#pragma once

#include <stdlib.h>
#include <queue>
#include <mutex>
#include "../include/shared.h"

class Queue
{
   public:
    Queue();
    virtual ~Queue();
    int enqueue(Request elem);
    Request dequeue();
    int front, rear;

   private:
    static const int queue_size = 1024000;
    Request q[queue_size];
    int volatile _lock;
};