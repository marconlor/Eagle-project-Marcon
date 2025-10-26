#ifndef RECEIVER_H
#define RECEIVER_H

#include "thread_safe_queue.h"
#include <atomic>

void receiverThread(ThreadSafeQueue* queue, std::atomic<bool>* running);

#endif