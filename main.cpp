#include "receiver.h"
#include "fake_receiver.h"
#include "fsm.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

using namespace std;

atomic<bool> g_running(true);

void signalHandler(int signal) {
    cout << "\n[Main] Received interrupt signal, shutting down..." << endl;
    g_running.store(false);
}

int main(int argc, char* argv[]) {
    cout << "=== CAN Telemetry System ===" << endl;
    cout << "Press Ctrl+C to stop\n" << endl;

    signal(SIGINT, signalHandler);

    const char* filepath = (argc > 1) ? argv[1] : "can_messages.txt";

    cout << "[Main] Opening CAN interface: " << filepath << endl;
    if (open_can(filepath) != 0) {
        cerr << "[Main] Error: Could not open CAN interface file: " << filepath << endl;
        cerr << "[Main] Usage: " << argv[0] << " <path_to_can_messages.txt>" << endl;
        return 1;
    }
    cout << "[Main] CAN interface opened successfully\n" << endl;

    ThreadSafeQueue queue;

    FSM fsm;

    atomic<bool> receiver_running(true);
    thread receiver_thread(receiverThread, &queue, &receiver_running);
    cout << "[Main] Receiver thread started\n" << endl;

    int messages_processed = 0;
    int empty_checks = 0;
    const int MAX_EMPTY_CHECKS = 100;

    while (g_running.load() && empty_checks < MAX_EMPTY_CHECKS) {
        RawCANData data;

        if (queue.tryPop(data)) {
            messages_processed++;

            fsm.handleMessage(data.message, data.timestamp);

            empty_checks = 0;
        } else {
            this_thread::sleep_for(chrono::milliseconds(100));
            empty_checks++;
        }
    }

    cout << "\n[Main] Starting shutdown sequence..." << endl;

    receiver_running.store(false);
    receiver_thread.join();
    cout << "[Main] Receiver thread stopped" << endl;

    cout << "[Main] Processing remaining messages in queue..." << endl;
    RawCANData data;
    while (queue.tryPop(data)) {
        messages_processed++;
        fsm.handleMessage(data.message, data.timestamp);
    }

    if (fsm.getCurrentState() == State::RUN) {
        cout << "[Main] FSM still in RUN state, forcing transition to IDLE..." << endl;
        fsm.handleMessage("0A0#66FF", getCurrentTimestampMs());
    }

    close_can();
    cout << "[Main] CAN interface closed" << endl;

    cout << "\n=== Session Summary ===" << endl;
    cout << "Total messages processed: " << messages_processed << endl;
    cout << "Final FSM state: " << fsm.getStateString() << endl;
    cout << "\nGenerated files:" << endl;
    cout << "  - session_*.log (log files for each RUN session)" << endl;
    cout << "  - statistics_*.csv (statistics for each session)" << endl;

    cout << "\n=== Telemetry System Shutdown Complete ===" << endl;

    return 0;
}
