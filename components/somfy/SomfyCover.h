#include "esphome.h"

#include <EEPROM.h>
#include <EEPROMRollingCodeStorage.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <SomfyRemote.h>

#define EMITTER_GPIO 2

#define CC1101_FREQUENCY 433.42


namespace esphome {
namespace somfy {


class SomfyCover : public Component, public cover::Cover {
private:
  SomfyRemote *remote;
  EEPROMRollingCodeStorage *storage;

public:
  SomfyESPCover(const char *name, const char *key, uint32_t remoteCode)
      : Cover() {
    storage = new EEPROMRollingCodeStorage(name, key);
    remote = new SomfyRemote(EMITTER_GPIO, remoteCode, storage);
  }

  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(true);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);
    traits.set_supports_stop(true);
    return traits;
  }

  void sendCC1101Command(Command command) {
    ELECHOUSE_cc1101.SetTx();
    remote->sendCommand(command);
    ELECHOUSE_cc1101.setSidle();
  }

  void control(const CoverCall &call) override {
    if (call.get_position().has_value()) {
      float pos = *call.get_position();

      if (pos == COVER_OPEN) {
        ESP_LOGI("somfy", "OPEN");
        sendCC1101Command(Command::Up);
      } else if (pos == COVER_CLOSED) {
        ESP_LOGI("somfy", "CLOSE");
        sendCC1101Command(Command::Down);
      } else {
        ESP_LOGI("somfy", "WAT");
      }

      this->position = pos;
      this->publish_state();
    }

    if (call.get_stop()) {
      ESP_LOGI("somfy", "STOP");
      sendCC1101Command(Command::My);
    }
  }

  
};

class SomfyRemote : public Component {
public:
  std::vector<esphome::cover::Cover *> covers;

  void setup() override {
    // need to set GPIO PIN 4 as OUTPUT, otherwise no commands will be sent
    pinMode(EMITTER_GPIO, OUTPUT);
    digitalWrite(EMITTER_GPIO, LOW);

    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setMHZ(CC1101_FREQUENCY);
  }

  void add_cover(const char *name, const char *key, uint32_t remoteCode) {
    auto cover = new SomfyESPCover(name, key, remoteCode);
    covers.push_back(cover);
  }
};

}
}