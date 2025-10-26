#ifndef CAN_PARSER_H
#define CAN_PARSER_H

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <iomanip>

using namespace std;

struct CANMessage {
    uint16_t id;
    vector<uint8_t> payload;
    uint64_t timestamp;
};

enum class MessageType {
    START,
    STOP,
    REGULAR
};

CANMessage parseCANMessage(const string& raw_message, uint64_t timestamp) {
    CANMessage msg;
    msg.timestamp = timestamp;

    size_t separator_pos = raw_message.find('#');
    if (separator_pos == string::npos) {
        throw invalid_argument("Invalid CAN message: missing '#' separator");
    }

    string id_str = raw_message.substr(0, separator_pos);
    if (id_str.empty()) {
        throw invalid_argument("Invalid CAN message: empty ID");
    }

    try {
        unsigned long id_value = stoul(id_str, nullptr, 16);
        if (id_value > 0xFFF) {
            throw invalid_argument("Invalid CAN message: ID exceeds 12 bits");
        }
        msg.id = static_cast<uint16_t>(id_value);
    } catch (const exception& e) {
        throw invalid_argument("Invalid CAN message: cannot parse ID");
    }

    string payload_str = raw_message.substr(separator_pos + 1);

    if (payload_str.length() % 2 != 0) {
        throw invalid_argument("Invalid CAN message: payload has odd number of characters");
    }

    if (payload_str.length() > 16) {
        throw std::invalid_argument("Invalid CAN message: payload exceeds 8 bytes");
    }

    for (size_t i = 0; i < payload_str.length(); i += 2) {
        std::string byte_str = payload_str.substr(i, 2);
        try {
            uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
            msg.payload.push_back(byte);
        } catch (const std::exception& e) {
            throw std::invalid_argument("Invalid CAN message: cannot parse payload byte");
        }
    }

    return msg;
}

MessageType detectMessageType(const CANMessage& msg) {
    if (msg.id == 0xA0 && msg.payload.size() == 2) {
        if ((msg.payload[0] == 0x66 && msg.payload[1] == 0x01) ||
            (msg.payload[0] == 0xFF && msg.payload[1] == 0x01)) {
            return MessageType::START;
        }

        if (msg.payload[0] == 0x66 && msg.payload[1] == 0xFF) {
            return MessageType::STOP;
        }
    }

    return MessageType::REGULAR;
}

string messageTypeToString(MessageType type) {
    switch (type) {
        case MessageType::START: return "START";
        case MessageType::STOP: return "STOP";
        case MessageType::REGULAR: return "REGULAR";
    }
    return "UNKNOWN";
}

std::string canMessageToString(const CANMessage& msg) {
    std::stringstream ss;
    ss << "ID: 0x" << std::hex << std::setfill('0') << std::setw(3) << msg.id;
    ss << " (" << std::dec << msg.id << ")";
    ss << ", Payload: [";
    for (size_t i = 0; i < msg.payload.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "0x" << std::hex << std::setfill('0') << std::setw(2)
           << static_cast<int>(msg.payload[i]);
    }
    ss << "], Timestamp: " << std::dec << msg.timestamp;
    return ss.str();
}

#endif