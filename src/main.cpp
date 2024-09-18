/*
 * Author: Varad Chaskar
 * Date: 18th September 2024
 * Time: 11:40 AM
 * 
 * Description: 
 * This code integrates Modbus RS485 communication with an ESP32 using the ModbusMaster library. 
 * It enables sending and receiving data over RS485 to/from a Modbus slave. 
 * The code uses LVGL to create an on-screen interface with buttons, while the TFT_eSPI library drives the display. 
 * The Modbus communication allows you to send text data and receive sensor data.
 * 
 * Libraries Used: 
 * - ModbusMaster: For handling Modbus RS485 communication.
 * - TFT_eSPI: For controlling the TFT display.
 * - LVGL: For creating GUI elements like buttons and labels.
 * - SPIFFS: For file system handling, used for touch calibration.
 * - SPI: For SPI communication with the display.
 */

#include <Arduino.h>    // Base library for ESP32 development
#include <FS.h>         // File system library for handling SPIFFS
#include <SPI.h>        // SPI library for communication with the display
#include <lvgl.h>       // LVGL library for GUI elements
#include <TFT_eSPI.h>   // Library for controlling the TFT display
#include <ModbusMaster.h>  // ModbusMaster library for handling Modbus RS485 communication

#define TOUCH_CS 21        // Chip select pin for the touch interface
#define BUTTON_PIN_1 25    // GPIO pin 25 for Button 1
#define BUZZER_PIN 13      // GPIO pin 13 for the buzzer

#define RXD_PIN 26         // RS-485 receive pin connected to ESP32
#define TXD_PIN 12         // RS-485 transmit pin connected to ESP32

#define CALIBRATION_FILE "/TouchCalData3"   // File to store touch calibration data
#define REPEAT_CAL true    // If true, forces a touch calibration each time
#define LVGL_REFRESH_TIME 5u // Refresh rate for the LVGL library in milliseconds

TFT_eSPI tft = TFT_eSPI();   // Initialize TFT display object
ModbusMaster node;           // Create a ModbusMaster object for handling Modbus communication

/* Screen resolution */
static const uint32_t screenWidth = 320;   // Define screen width (in pixels)
static const uint32_t screenHeight = 240;  // Define screen height (in pixels)

static lv_disp_draw_buf_t draw_buf;        // Buffer for LVGL drawing
static lv_color_t buf[screenWidth * 10];   // Buffer size (number of pixels in width * 10)

/* GUI styles */
static lv_style_t style_default, style_pressed;

lv_obj_t *label;           // Label to display received Modbus data
lv_obj_t *keyboard = NULL; // Object for the on-screen keyboard
lv_obj_t *textarea = NULL; // Text area for keyboard input

String receivedData = "No data received yet."; // String to store Modbus received data

/* Touch calibration function */
void touch_calibrate() {
  uint16_t calData[5];   // Array to store calibration data
  uint8_t calDataOK = 0; // Flag to check if calibration data exists

  // Initialize SPIFFS and check if calibration data exists
  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // If calibration data exists and repeat calibration is false, load calibration data
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

  // If calibration data is valid, use it; otherwise, calibrate touch and save the data
  if (calDataOK && !REPEAT_CAL) {
    tft.setTouch(calData);
  } else {
    tft.fillScreen(TFT_BLACK);     // Clear the screen
    tft.setCursor(20, 0);          // Set cursor position
    tft.setTextFont(2);            // Set text font size
    tft.setTextSize(1);            // Set text size
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color

    tft.println("Touch corners as indicated");  // Display instructions for touch calibration

    // Calibrate touch and save the data to file
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

/* Function to read touch inputs and pass to LVGL */
void lvgl_port_tp_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
  uint16_t touchX, touchY;         // Variables to hold touch coordinates
  bool touched = tft.getTouch(&touchX, &touchY); // Get touch status and coordinates

  // If touched, update LVGL with coordinates, otherwise mark as released
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

/* Display flushing for LVGL */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);  // Calculate width of the drawing area
  uint32_t h = (area->y2 - area->y1 + 1);  // Calculate height of the drawing area

  tft.startWrite();                       // Start writing to the display
  tft.setAddrWindow(area->x1, area->y1, w, h); // Set the address window for the drawing area
  tft.pushColors((uint16_t *)&color_p->full, w * h, true); // Push pixel data to the display
  tft.endWrite();                         // End writing

  lv_disp_flush_ready(disp);              // Notify LVGL that flushing is complete
}

/* Send data via Modbus */
void sendModbusData(const char *data) {
  node.writeSingleRegister(0x40001, atoi(data)); // Send the data to Modbus register 40001
}

/* Event handler for keyboard input */
static void kb_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e); // Get the event code
  lv_obj_t *kb = lv_event_get_target(e);       // Get the target object (keyboard)

  // If the event is "ready" or "cancel", send data via Modbus
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    const char *text = lv_textarea_get_text(textarea); // Get the text from the textarea

    sendModbusData(text);          // Send the text via Modbus
    Serial.println("Sent to Modbus: " + String(text)); // Debug output

    // Remove the textarea and keyboard from the screen
    lv_obj_del(textarea);
    textarea = NULL;
    lv_obj_del(kb);
    keyboard = NULL;
  }
}

/* Event handler for Button 2 (shows the keyboard) */
static void event_handler_btn2(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e); // Get event code

  if (code == LV_EVENT_CLICKED) {  // If button is clicked, show the keyboard
    if (keyboard == NULL) {
      textarea = lv_textarea_create(lv_scr_act());   // Create textarea
      lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 60); // Position it on the screen
      lv_textarea_set_one_line(textarea, true);     // Make it single-line input

      keyboard = lv_keyboard_create(lv_scr_act());  // Create keyboard
      lv_keyboard_set_textarea(keyboard, textarea); // Attach keyboard to textarea
      lv_obj_set_size(keyboard, screenWidth, screenHeight / 2); // Set size of the keyboard
      lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER); // Lowercase input mode
      lv_obj_add_event_cb(keyboard, kb_event_handler, LV_EVENT_ALL, NULL); // Attach event handler
    }
  }
}

/* Create buttons for the screen */
void lv_example_buttons(void) {
  label = lv_label_create(lv_scr_act());          // Create label to show received data
  lv_label_set_text(label, receivedData.c_str()); // Initialize label text
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10); // Position label

  const int button_width = 120;   // Button width in pixels
  const int button_height = 60;   // Button height in pixels

  lv_obj_t *btn2 = lv_btn_create(lv_scr_act());   // Create Button 2
  lv_obj_set_size(btn2, button_width, button_height); // Set button size
  lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL); // Add event handler
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, -40); // Center the button

  lv_obj_t *btn2_label = lv_label_create(btn2);  // Add label to Button 2
  lv_label_set_text(btn2_label, "Option 2");     // Set label text
}

/* Setup function */
void setup() {
  Serial.begin(115200);           // Initialize serial communication at 115200 baud
  node.begin(1, Serial2);         // Begin Modbus communication on Serial2 with slave ID 1
  Serial2.begin(9600, SERIAL_8N1, RXD_PIN, TXD_PIN); // Set up RS485 serial communication

  tft.begin();                    // Initialize the TFT display
  tft.setRotation(1);             // Set display rotation
  lv_init();                      // Initialize the LVGL library

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10); // Initialize the drawing buffer

  // Initialize the display driver for LVGL
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize the touch input driver for LVGL
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_port_tp_read;
  lv_indev_drv_register(&indev_drv);

  touch_calibrate();    // Calibrate the touch screen
  lv_example_buttons(); // Create on-screen buttons
}

/* Main loop */
void loop() {
  lv_timer_handler();   // Call LVGL handler to update GUI
  delay(LVGL_REFRESH_TIME); // Add delay to control GUI refresh rate
}
