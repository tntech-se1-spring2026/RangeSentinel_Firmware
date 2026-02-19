#ifndef PINS_H
#define PINS_H

/* * --- LORA RADIO PINS (SX1276) ---
 * These pins connect the ESP32 to the LoRa transceiver.
 */
#define RFM95_CS    18  // Chip Select: The "Attention" line. When LOW, the ESP32 is talking to the LoRa radio.
#define RFM95_INT   26  // Interrupt (DIO0): The radio pulls this HIGH to tell the ESP32 "I just received a packet" or "I'm done sending."
#define RFM95_RST   23  // Reset: Used to hard-reboot the radio chip. GPIO 23 is specific to the T3 V1.6.1 board layout.
#define RFM95_BUSY  32  // Busy: Mainly used by newer LoRa chips (like SX1262), but wired on some T3 versions for compatibility.

/* * --- SPI BUS (Serial Peripheral Interface) ---
 * This is the high-speed "Data Highway" shared by the LoRa radio and the SD Card.
 */
#define SPI_SCK     5   // Serial Clock: The metronome that keeps data bits in sync between chips.
#define SPI_MISO    19  // Master In Slave Out: The "Listener" line—data flowing FROM the LoRa/SD TO the ESP32.
#define SPI_MOSI    27  // Master Out Slave In: The "Speaker" line—data flowing FROM the ESP32 TO the LoRa/SD.
#define SPI_SS      18  // Slave Select: Same as RFM95_CS. Defined here to satisfy standard SPI library defaults.

/* * --- OLED DISPLAY (I2C Bus) ---
 * The built-in 0.96" screen uses a 2-wire "I2C" protocol.
 */
#define OLED_SDA    21  // Serial Data: The line where display data travels.
#define OLED_SCL    22  // Serial Clock: The timing signal for the I2C data.
#define OLED_RESET  -1  // Reset: -1 means "None." The screen resets automatically when the ESP32 power cycles.

/* * --- SD CARD (SPI Device #2) ---
 * The SD slot shares the SPI highway but has its own "Attention" line.
 */
#define SD_CS       13  // SD Chip Select: Pull this LOW to talk to the SD card. Keep HIGH when talking to LoRa.

/* * --- DOOR SENSOR/REED SWITCH ---
 * The pins relating to the door reed switch
 */
#define RS_PIN      4   // GPIO pin that the switch is attached to; 4 is for the devboard

/* * --- ESP32-CAM CONTROLLER CONNECTIONS (UART) ---
 * 
 */

#endif