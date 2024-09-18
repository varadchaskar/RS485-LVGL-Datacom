<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="description" content="README file for RS485 and LVGL Display Interface Code">

</head>
<body>

  <h1>RS485 and LVGL Display Interface - README</h1>

  <h2>1. Overview</h2>
  <p>
    This project demonstrates interfacing an RS485 serial communication with a TFT display using the <a href="https://lvgl.io/">LittlevGL (lvgl)</a> graphics library. The system enables sending and receiving data through RS485 and displays it on the TFT screen. It also features an interactive user interface built with LVGL.
  </p>

  <h2>2. Purpose</h2>
  <p>
    The goal of this code is to create a simple GUI (Graphical User Interface) on a TFT display, allowing data to be sent via RS485 and displayed on-screen. It is designed for embedded systems requiring both data display and user interaction through an intuitive graphical interface.
  </p>

  <h2>3. How It Works</h2>
  <p>
    The main functionalities of this project include:
  </p>
  <ul>
    <li><b>RS485 Communication:</b> The ESP32 uses UART for sending and receiving data through RS485. Data received from RS485 is displayed on the TFT screen.</li>
    <li><b>LVGL GUI:</b> LittlevGL manages the graphical elements on the TFT screen, including displaying text and handling user interactions.</li>
    <li><b>Touchscreen Calibration:</b> The project includes a function to calibrate the touchscreen and store calibration data in SPIFFS.</li>
  </ul>

  <h2>4. Applications</h2>
  <p>
    This code is suitable for various applications, including:
  </p>
  <ul>
    <li><b>Embedded System Interfaces:</b> Ideal for systems requiring a GUI to display and manage data, such as industrial control panels or monitoring systems.</li>
    <li><b>Data Display:</b> Useful for displaying real-time data received from serial interfaces on a graphical screen.</li>
    <li><b>Interactive Projects:</b> Can be used in projects where user input and data visualization are crucial, such as IoT devices or custom control interfaces.</li>
  </ul>

  <h2>5. Key Features</h2>
  <ul>
    <li><b>Real-time Data Display:</b> Shows data received from RS485 on the TFT display in real-time.</li>
    <li><b>Graphical User Interface:</b> Built using LVGL, allowing for a rich and interactive display experience.</li>
    <li><b>Touchscreen Calibration:</b> Ensures accurate touch input by calibrating the touchscreen and saving the calibration data.</li>
    <li><b>Customizable GUI:</b> LVGL provides options to customize the appearance and behavior of the GUI elements.</li>
  </ul>

  <h2>6. Libraries Used</h2>
  <ul>
    <li><b>LittlevGL (lvgl):</b> A comprehensive graphics library for embedded devices that manages GUI elements and interactions.</li>
    <li><b>TFT_eSPI:</b> Library for interfacing with the TFT display, handling drawing and touch inputs.</li>
    <li><b>FS (File System):</b> Used for storing calibration data and handling file operations.</li>
    <li><b>Arduino Core:</b> The code is written using the Arduino framework, making it accessible for a wide range of developers.</li>
    <li><b>ModbusMaster:</b> For handling Modbus RS485 communication, enabling data transfer between devices.</li>
  </ul>

  <h2>7. Code Breakdown</h2>
  <p>
    The code includes the following key sections:
  </p>
  <ul>
    <li><b>Include Libraries:</b> Necessary libraries are included for handling TFT display, LVGL, RS485 communication, and file system operations.</li>
    <li><b>Pin Configuration:</b> Defines the GPIO pins used for various functions such as RS485 communication, touchscreen control, and buzzer.</li>
    <li><b>Initialization:</b> Initializes the TFT display, RS485 interface, LVGL environment, and touch calibration.</li>
    <li><b>Touch Calibration:</b> Function to calibrate the touchscreen and store calibration data.</li>
    <li><b>Data Handling:</b> Functions to receive data from RS485 and display it on the TFT screen, as well as manage GUI elements.</li>
  </ul>

  <h2>8. Potential Future Work</h2>
  <p>
    Future improvements and additional features could include:
  </p>
  <ul>
    <li><b>Dynamic Data Handling:</b> Enhance the system to handle dynamic data or support multiple RS485 devices.</li>
    <li><b>Enhanced GUI Features:</b> Add animations or advanced GUI components to improve user interaction.</li>
    <li><b>Additional Input Methods:</b> Integrate other input methods such as buttons or rotary encoders for more versatile control.</li>
    <li><b>Network Integration:</b> Add functionality to send or receive data over network interfaces like Wi-Fi or Bluetooth.</li>
  </ul>

</body>
</html>
