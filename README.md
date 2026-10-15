# ESP32 Embedded Fault Detection System

Simulated fault detection and load protection system on ESP32 (Wokwi).

## Features
- Monitors voltage, current, temperature
- Detects overvoltage, undervoltage, overcurrent, overheating
- Buzzer + LED alert, relay cuts off the load
- CSV serial logging

## Live Simulation
[Open in Wokwi](PASTE_YOUR_WOKWI_SHARE_LINK)

## Circuit


![circuit](docs/circuit.png)



## Fault Thresholds
| Fault | Limit |
|---|---|
| Overvoltage | > 12 V |
| Undervoltage | < 4.5 V |
| Overcurrent | > 3.5 A |
| Overheat | > 60 °C |

## Output


![serial](docs/serial_output.png)



## Tools
ESP32, Arduino framework (C++), Wokwi
