#include "esphome.h"

using namespace esphome;

class MCP4921 : public Component, public FloatOutput {
 public:

    int CS_PIN = 15;

    void setup() override {
        SPI.begin();
        digitalWrite(CS_PIN, HIGH); // Disable the slave SPI device for now
        pinMode(CS_PIN, OUTPUT);
        set_mA(4.0);
    }

    uint16_t mA2bitval(float mA) {
            return (uint16_t)round((mA - 4.02) / (19.99 - 4.02) *
                             (3980 - 800) +
                         800);
    }

     void write_state(float state) override { 
        byte bitval_HI;
        byte bitval_LO;

        // The standard Arduino SPI library handles data of 8 bits long. The value
        // decoding the DAC output is 12 bits, hence transfer in two steps.
        bitval_ = mA2bitval(state);
        bitval_HI = (bitval_ >> 8) & 0x0F; // 0x0F = b00001111
        bitval_HI |= 0x30;                 // 0x30 = b00110000
        bitval_LO = bitval_;

        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(CS_PIN, LOW);  // Enable slave device
        SPI.transfer(bitval_HI);     // Transfer high byte
        SPI.transfer(bitval_LO);     // Transfer low byte
        digitalWrite(CS_PIN, HIGH); // Disable slave device
        SPI.endTransaction();
    }




}