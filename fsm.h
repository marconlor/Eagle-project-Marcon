#ifndef FSM_H
#define FSM_H

#include "can_parser.h"
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

enum class State {
    IDLE,
    RUN
};

class FSM {
private:
    State current_state_;
    ofstream log_file_;
    int session_counter_;
    string current_log_filename_;

    map<uint16_t, vector<uint64_t>> message_times_;

public:
    FSM() : current_state_(State::IDLE), session_counter_(0) {
        cout << "[FSM] Initialized in IDLE state" << endl;
    }

    ~FSM() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    State getCurrentState() const {
        return current_state_;
    }

    string getStateString() const {
        return (current_state_ == State::IDLE) ? "IDLE" : "RUN";
    }

    void transitionToRun() {
        if (current_state_ == State::IDLE) {
            current_state_ = State::RUN;
            openNewLogFile();
            cout << "[FSM] Transition: IDLE -> RUN (Session "
                      << session_counter_ << ")" << endl;
        }
    }

    void transitionToIdle() {
        if (current_state_ == State::RUN) {
            current_state_ = State::IDLE;
            closeLogFile();
            saveStatistics();
            clearStatistics();
            cout << "[FSM] Transition: RUN -> IDLE" << endl;
        }
    }

    void processMessage(const CANMessage& msg, const string& raw_message) {
        message_times_[msg.id].push_back(msg.timestamp);

        if (current_state_ == State::RUN) {
            logMessage(msg.timestamp, raw_message);
        }
    }

    void handleMessage(const string& raw_message, uint64_t timestamp) {
        try {
            CANMessage msg = parseCANMessage(raw_message, timestamp);
            MessageType type = detectMessageType(msg);

            cout << "[FSM] State: " << getStateString()
                      << ", Message: " << raw_message
                      << " (" << messageTypeToString(type) << ")" << endl;

            if (type == MessageType::START && current_state_ == State::IDLE) {
                transitionToRun();
            } else if (type == MessageType::STOP && current_state_ == State::RUN) {
                transitionToIdle();
            }

            processMessage(msg, raw_message);

        } catch (const exception& e) {
            cerr << "[FSM] Error processing message: " << e.what() << endl;
        }
    }

private:
    void openNewLogFile() {
        current_log_filename_ = "session_" + to_string(session_counter_) + ".log";
        log_file_.open(current_log_filename_);

        if (!log_file_.is_open()) {
            cerr << "[FSM] Error: Could not open log file: "
                      << current_log_filename_ << endl;
        } else {
            cout << "[FSM] Opened log file: " << current_log_filename_ << endl;
        }
    }

    void closeLogFile() {
        if (log_file_.is_open()) {
            log_file_.flush();
            log_file_.close();
            cout << "[FSM] Closed log file: " << current_log_filename_ << endl;
        }
    }

    void logMessage(uint64_t timestamp, const string& raw_message) {
        if (log_file_.is_open()) {
            log_file_ << "(" << timestamp << ") " << raw_message << "\n";
        }
    }

    void saveStatistics() {
        string stats_filename = "statistics_" + to_string(session_counter_) + ".csv";
        ofstream stats_file(stats_filename);

        if (!stats_file.is_open()) {
            cerr << "[FSM] Error: Could not open statistics file: "
                      << stats_filename << endl;
            return;
        }

        cout << "[FSM] Saving statistics to: " << stats_filename << endl;

        stats_file << "ID,number_of_messages,mean_time\n";

        for (const auto& pair : message_times_) {
            uint16_t id = pair.first;
            const vector<uint64_t>& timestamps = pair.second;

            size_t num_messages = timestamps.size();
            double mean_time = 0.0;

            if (num_messages >= 2) {
                uint64_t total_diff = 0;
                for (size_t i = 1; i < timestamps.size(); ++i) {
                    total_diff += (timestamps[i] - timestamps[i-1]);
                }
                mean_time = static_cast<double>(total_diff) / (num_messages - 1);
            }

            stats_file << dec << id << ","
                      << num_messages << ","
                      << fixed << setprecision(2) << mean_time << "\n";
        }

        stats_file.close();
        cout << "[FSM] Statistics saved successfully" << endl;

        session_counter_++;
    }

    void clearStatistics() {
        message_times_.clear();
    }
};

#endif