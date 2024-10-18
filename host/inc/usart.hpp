#ifndef __WZSERIALPORTPLUS_H__
#define __WZSERIALPORTPLUS_H__

#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include "yaml-cpp/yaml.h"

struct Frame
{
    uint8_t head;
    uint8_t mode;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    uint8_t byte4;
    uint8_t byte5;
    uint8_t byte6;
    uint8_t byte7;
    uint8_t tail;
};

// struct Command
// {
//     uint8_t mode;
//     `uint8_t color;
//     uint8_t xh;
//     uint8_t xl;
//     uint8_t yh;
//     uint8_t yl;
//
// };

using ReceiveCallback = std::function<void (char *, int)>;

class WzSerialportPlus
{
public:
    WzSerialportPlus();

    /**
     * @param name: serialport name , such as: /dev/ttyS1
     * @param baudrate: baud rate , valid: 2400,4800,9600,19200,38400,57600,115200,230400
     * @param stopbit: stop bit , valid: 1,2
     * @param databit: data bit , valid: 7,8
     * @param paritybit: parity bit , valid: 'o'/'O' for odd,'e'/'E' for even,'n'/'N' for none
     */
    WzSerialportPlus(const std::string &name,
                     const int &baudrate,
                     const int &stopbit,
                     const int &databit,
                     const int &paritybit);

    ~WzSerialportPlus();

    bool open();

    /**
     * @brief parameters same as constructor
     */
    bool open(const std::string &name,
              const int &baudrate,
              const int &stopbit,
              const int &databit,
              const int &paritybit);

    void close();

    int send(char *data, int length);

    void setReceiveCalback(ReceiveCallback receiveCallback);

    bool initSerialFromConfig(WzSerialportPlus &serialport, const std::string &configFilePath);

    Frame initSendFrame(const YAML::Node &config);

    Frame initReceiveFrame(const YAML::Node &config);

protected:
    virtual void onReceive(char *data, int length);

private:
    int serialportFd;
    std::string name;
    int baudrate;
    int stopbit;
    int databit;
    int paritybit;
    bool receivable;
    int receiveMaxlength;
    int receiveTimeout; /* unit: ms */
    ReceiveCallback receiveCallback;
};


#endif
