#include "receiver.h"
#include "fake_receiver.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>

using namespace std;

void receiverThread(ThreadSafeQueue* queue, atomic<bool>* running) {
    cout << "[Receiver Thread] Started" << endl;

    while (running->load()) {
        char message[MAX_CAN_MESSAGE_SIZE];
        memset(message, 0, MAX_CAN_MESSAGE_SIZE);

        int bytes_received = can_receive(message);

        if (bytes_received > 0) {
            uint64_t timestamp = getCurrentTimestampMs();
            string msg_str(message);
            queue->push(msg_str, timestamp);
            cout << "[Receiver Thread] Received: " << msg_str
                      << " @ " << timestamp << " (" << bytes_received << " bytes)" << endl;
        } else if (bytes_received < 0) {
            cout << "[Receiver Thread] Error or end of data" << endl;
            break;
        }
    }
    
    cout << "[Receiver Thread] Stopped" << endl;
}