//
// Created by luckye on 24-10-18.
//
#include "Command.h"

Command::Command()
{
}

Command::~Command()
{
}

Frame Command::initSendFrame(const YAML::Node &config)
{
    Frame frame;
    frame.head = config["frame"]["head"].as<uint8_t>();
    frame.mode = config["frame"]["mode"].as<uint8_t>();
    frame.byte1 = config["frame"]["byte1"].as<uint8_t>();
    frame.byte2 = config["frame"]["byte2"].as<uint8_t>();
    frame.byte3 = config["frame"]["byte3"].as<uint8_t>();
    frame.byte4 = config["frame"]["byte4"].as<uint8_t>();
    frame.byte5 = config["frame"]["byte5"].as<uint8_t>();
    frame.byte6 = config["frame"]["byte6"].as<uint8_t>();
    frame.byte7 = config["frame"]["byte7"].as<uint8_t>();
    frame.tail = config["frame"]["tail"].as<uint8_t>();
    std::clog << "Send frame initialized." << std::endl;
    return frame;
}

Frame Command::initReceiveFrame(const YAML::Node &config)
{
    Frame frame;
    frame.head = config["frame"]["head"].as<uint8_t>();
    frame.mode = config["frame"]["mode"].as<uint8_t>();
    frame.byte1 = config["frame"]["byte1"].as<uint8_t>();
    frame.byte2 = config["frame"]["byte2"].as<uint8_t>();
    frame.byte3 = config["frame"]["byte3"].as<uint8_t>();
    frame.byte4 = config["frame"]["byte4"].as<uint8_t>();
    frame.byte5 = config["frame"]["byte5"].as<uint8_t>();
    frame.byte6 = config["frame"]["byte6"].as<uint8_t>();
    frame.byte7 = config["frame"]["byte7"].as<uint8_t>();
    frame.tail = config["frame"]["tail"].as<uint8_t>();
    std::clog << "Send frame initialized." << std::endl;
    return frame;
}

bool Command::generateQRcodeFrame(Frame &frame, const YAML::Node &config, const std::array<uint8_t, 6> QRCode)
{
    frame.head = config["frame"]["head"].as<uint8_t>();
    frame.mode = config["mode"]["QRmode"].as<uint8_t>();
    frame.byte1 = QRCode[0];
    frame.byte2 = QRCode[1];
    frame.byte3 = QRCode[2];
    frame.byte4 = QRCode[3];
    frame.byte5 = QRCode[4];
    frame.byte6 = QRCode[5];
    frame.byte7 = config["frame"]["byte7"].as<uint8_t>();
    frame.tail = config["frame"]["tail"].as<uint8_t>();
    std::clog << "Send frame initialized." << std::endl;
    return true;
}

bool Command::generateMaterialFrame(Frame &frame, const YAML::Node &config)
{
    return true;
}

bool Command::generateDetectFrame(Frame &frame, const YAML::Node &config)
{
    return true;
}
