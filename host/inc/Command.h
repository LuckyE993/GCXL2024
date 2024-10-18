//
// Created by luckye on 24-10-18.
//

#ifndef COMMAND_H
#define COMMAND_H
#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include "yaml-cpp/yaml.h"
#include "usart.hpp"

class Command
{
public:
    Command();

    ~Command();

    Frame initSendFrame(const YAML::Node &config);

    Frame initReceiveFrame(const YAML::Node &config);

    bool generateQRcodeFrame(Frame &frame, const YAML::Node &config, std::array<uint8_t, 6> QRCode);

    bool generateMaterialFrame(Frame &frame, const YAML::Node &config);

    bool generateDetectFrame(Frame &frame, const YAML::Node &config);

    int mode;
};


#endif //COMMAND_H
