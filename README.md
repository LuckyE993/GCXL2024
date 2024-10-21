# CV Part Upper
## Program

## Autostart
In N305 

``` 
Start-Key -> Del(Tap) -> Advanced -> OEM Configuration -> AC Power Loss -> switch to Power ON
```


``` bash
sudo gedit ~/.config/autostart/startup_terminal.desktop
```

startup_terminal.desktop file content
```bash
[Desktop Entry]
Type=Application
Terminal=true
Exec=/home/car/GCXL2024/host/script.sh
Name=Startup Terminal
Comment=Automatically
```

``` bash
cd GCXL2024
sudo gedit host/script.sh
```
script.sh file content

```bash
#!/bin/bash

sudo chmod 777 /dev/ttyUSB*
sudo chmod 777 /dev/video*

sleep 2

cd /home/car/GCXL2024/host/build/

./host
```

sudo without type password
``` bash
sudo visudo
```

modify(add) the last line
``` bash
username ALL=(ALL) NOPASSWD: /bin/chmod
```