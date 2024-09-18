#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ModbusMaster.h>

#define TOUCH_CS 21
#define BUTTON_PIN_1 25 // ESP32 GPIO25 pin connected to button's pin
#define BUZZER_PIN 13   // ESP32 GPIO13 pin connected to Buzzer's pin

#define RS485_RXD 26   // RS485 receive pin
#define RS485_TXD 12   // RS485 transmit pin

#define CALIBRATION_FILE "/TouchCalData3"
#define REPEAT_CAL true   // Set to true to force calibration
#define LVGL_REFRESH_TIME 5u // 5 ms

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
ModbusMaster node;         // ModbusMaster object

/* Screen resolution */
static const uint32_t screenWidth = 320;
static const uint32_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

static lv_style_t style_default, style_pressed;

lv_obj_t *label;           // Label to show received Modbus data
lv_obj_t *keyboard = NULL; // On-screen keyboard object
lv_obj_t *textarea = NULL; // Text area object for keyboard input

String receivedData = "No data received yet."; // To hold Modbus received data

/* Touch calibration */
void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (!REPEAT_CAL) {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    tft.setTouch(calData);
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void lvgl_port_tp_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// Function to send data over Modbus
void sendModbusData(const char *data) {
  // Convert the text into a number and send it via Modbus
  int value = atoi(data);  // Example: convert the string to an integer
  node.writeSingleRegister(0x02, value);  // Write the value to Modbus register (address 0x02)
}

/* Function to handle text input from the keyboard */
static void kb_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *kb = lv_event_get_target(e);

  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    const char *text = lv_textarea_get_text(textarea); // Get text from the textarea

    // Send data via Modbus RS-485
    sendModbusData(text);

    // Remove keyboard and textarea
    lv_obj_del(textarea);
    textarea = NULL;
    lv_obj_del(kb);
    keyboard = NULL;
  }
}

/* Event handler for Button 2 to show keyboard */
static void event_handler_btn2(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    if (keyboard == NULL) {
      textarea = lv_textarea_create(lv_scr_act()); // Create the textarea for text input
      lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 60); // Position the textarea
      lv_textarea_set_one_line(textarea, true); // Make it a single line

      keyboard = lv_keyboard_create(lv_scr_act());
      lv_keyboard_set_textarea(keyboard, textarea); // Attach keyboard to the textarea
      lv_obj_set_size(keyboard, screenWidth, screenHeight / 2); // Adjust size of the keyboard
      lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER); // Set keyboard mode
      lv_obj_add_event_cb(keyboard, kb_event_handler, LV_EVENT_ALL, NULL); // Add event handler for keyboard
    }
  }
}

/* Create Buttons and add them to the screen */
void lv_example_buttons(void) {
  /* Create Label to show received data */
  label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, receivedData.c_str());
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10); // Position label to show received data

  /* Button parameters */
  const int button_width = 120;
  const int button_height = 60;

  /* Create Button 2 */
  lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn2, button_width, button_height); // Set size of the button
  lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL); // Add event for keyboard
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, -40); // Position Button 2

  lv_obj_t *btn2_label = lv_label_create(btn2);
  lv_label_set_text(btn2_label, "Enter Text");
  lv_obj_center(btn2_label);
}

void setup() {
  Serial.begin(115200);

  // Initialize RS-485 Modbus communication
  Serial1.begin(115200, SERIAL_8N1, RS485_RXD, RS485_TXD); 
  node.begin(1, Serial1);  // Initialize Modbus Master with Slave ID 1

  SPIFFS.begin();
  tft.begin();        /* TFT init */
  tft.setRotation(1); /* Landscape orientation */

  uint16_t calData[5] = {275, 3574, 351, 3485, 7};
  tft.setTouch(calData);

  lv_init();

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /* Initialize the display */
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_port_tp_read;
  lv_indev_drv_register(&indev_drv);

  /* Styles initialization */
  lv_style_init(&style_default);
  lv_style_set_bg_color(&style_default, lv_color_hex(0x0000FF)); // Default blue

  lv_style_init(&style_pressed);
  lv_style_set_bg_color(&style_pressed, lv_color_hex(0xFF0000)); // Pressed red

  /* Create buttons */
  lv_example_buttons();
}

// Function to read data from Modbus
void readModbusData() {
  uint8_t result = node.readHoldingRegisters(0x02, 1);
  
  if (result == node.ku8MBSuccess) {
    uint16_t data = node.getResponseBuffer(0);
    receivedData = String(data);  // Update the received data string
    lv_label_set_text(label, receivedData.c_str());
    Serial.println("Received from Modbus: " + receivedData);
  } else {
    Serial.println("Modbus read error: " + String(result));
  }
}

void loop() {
  lv_timer_handler(); // Let the GUI do its work
  delay(LVGL_REFRESH_TIME);

  // Step 1: Check for Modbus updates
  readModbusData();
}
