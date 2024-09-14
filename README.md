# GCXL2024
This is the repository for GCXL2024 belonging to the **SAU** team.

## CV Part
### model training
The Train Part is based on [OpenMMLab MMYOLO](https://github.com/open-mmlab/mmyolo).

Train code is in `src/train_model` with full documents.
The training config file is in `src/train_model/yolov5/train_config.py`.Datasets can be downloaded from [Baidu Netdisk](https://pan.baidu.com/s/1NcQSzuD8DhYUe0tgpDfh5Q?pwd=gcxl)
### object detection

### landmark detection

### edge detection

## Chassis Part
### Communication Protocol
![Current Protocols](<docs/pics/Communication Protocols now.png>)
### Progress
Implement the PID control for the speed loop.

Currently, communication with the upper computer is established via UART6, and the upper computer can control the chassis speed by sending commands
## Robotic Arm Part