# Range Sentinel
> Security Beyond the Grid

## Overview

Range Sentinel is a solution aimed at those who live in areas that may not have a
strong connection to the Internet, be it from an ISP or through cellular signals.

It is able to monitor doors, gates, and fences at a long range showing live updates
to help combat nefarious activity.

Range Sentinel beleives in packaging every part of itself in an enclosed system.
Therefore, an Internet connection is _not required_. Instead, a specialized `viewing node`
is used to monitor the system with both a screen and through a locally hosted web server.

> Note: The frequencies emmitted from the wireless LoRa tranceivers may not be 
legal to broadcast in your country! Consult your country's 
[frequency plan](https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country/) if you are unsure. 
Range Sentinel is **NOT LIABLE** for breaking your country's laws.

## Bill of Materials
- Each viewing node consists of:
  - 1 [ESP32 Microsontroller](https://en.wikipedia.org/wiki/ESP32)
  - 1 [XL1276-P01 Wireless LoRa Transceiver Module](https://www.amazon.com/dp/B0BXDNFZ2B)
  - 1 [3.7V 3000mAh Rechargable Battery](https://www.amazon.com/dp/B08T6GT7DV?th=1)
  - 1 [TP4057 1A 3.7V Battery Charging Board](https://www.amazon.com/dp/B0CDWZ9MDC)
  - 1 [6V DC Solar Panel](https://www.amazon.com/dp/B08THXDWS1)
  - 1 [0.96 inch SSD1306 Driver I2C OLED Screen](https://www.amazon.com/DIYmall-Serial-128x64-Display-Arduino/dp/B00O2KDQBE?th=1)

- Each sensor node consists of:
  - 1 [ESP32 Microsontroller](https://en.wikipedia.org/wiki/ESP32)
  - 1 [XL1276-P01 Wireless LoRa Transceiver Module](https://www.amazon.com/dp/B0BXDNFZ2B)
  - 1 [3.7V 3000mAh Rechargable Battery](https://www.amazon.com/dp/B08T6GT7DV?th=1)
  - 1 [TP4057 1A 3.7V Battery Charging Board](https://www.amazon.com/dp/B0CDWZ9MDC)
  - 1 [6V DC Solar Panel](https://www.amazon.com/dp/B08THXDWS1)
  - 1 [Magnetic Reed Switch](https://www.amazon.com/dp/B0BX2ZRZ8T?th=1)
  
> Special boards such as the [LILYGO LoRa32 915Mhz ESP32 Development Board](https://www.amazon.com/dp/B09SHRWVNB?th=1) package most
of the parts in a single unit. This is the preferred way of running Range Sentinel.

## Building

Range Sentinel is built with [Platform IO](https://platformio.org/).
You will need to install at least the core CLI tools to build and flash
the devices.

### Flashing the Viewing Node

Once the viewing node is assembled, it can be flashed using the
`viewing_node` environment.

`platformio run --target upload --environment viewing_node`

Next, the viewing node needs the littlefs file system uploaded to it.

`platformio run --target uploadfs --environment viewing_node`

### Flashing Sensor Nodes

Once assembled, sensor nodes can be flashed with the `sensor_node`
environment.

`platformio run --target upload --environment sensor_node`

## Documentation

Documentation is available in the `docs/` folder. Included is various information about
the layout of structs and webserver information.

Additionally, a `Doxyfile` is included for use with [doxygen](https://www.doxygen.nl/).
