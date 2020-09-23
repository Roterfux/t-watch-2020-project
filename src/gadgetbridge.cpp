#include "config.h"
#include <Arduino.h>
#include "ArduinoJson.h"
#include "gui.h"
#include "main.h"

StaticJsonDocument<512> json;
static MBox *mbox = nullptr;
unsigned long notify_id = 0;

void process_gadgetbridge_notify() {
    notify_id = json["id"];
    char format[1024];
    const char* src = json["sender"]; // Debug sends "sender"
    const char* title = json["subject"]; // Debug sends "subject"
    const char* body = json["body"];
    Serial.printf("%s: %s\n\n%s", src, title, body);

    TTGOClass *ttgo = TTGOClass::getWatch();


    snprintf(format, sizeof(format), "%s:\n%s\n\n%s", src, title, body);
    delete mbox;
    mbox = new MBox;
    mbox->create(format, [](lv_obj_t *obj, lv_event_t event) {
    //mbox->create("%s: %s\n\n%s", src, title, body, [](lv_obj_t *obj, lv_event_t event) {
        if (event == LV_EVENT_VALUE_CHANGED) {
            delete mbox;
            mbox = nullptr;
            notify_id = 0;
        }
    });

    // Turn on display if off
    if (!ttgo->bl->isOn()) {
        xEventGroupSetBits(*get_isr_group(), WATCH_FLAG_SLEEP_EXIT);
    }

    // Trigger vibration
    ttgo->motor->adjust(255);
    ttgo->motor->onec();
}




void process_gadgetbridge_json(const char* json_string) {
    deserializeJson(json, json_string);
    
    const char* t = json["t"];
    Serial.printf("Received: %s\n", t);
    if (!strcmp(t, "notify")) {
        process_gadgetbridge_notify();
    } else {
        Serial.printf("Unhandled GB type: %s\n", t);
    }
}
