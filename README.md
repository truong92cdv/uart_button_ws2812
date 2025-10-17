# uart_button_ws2812

Điều khiển module NeoPixel 8 RGB Led WS2812 bằng Button và UART qua Console (Vitis App)

Đề bài: [Assignment MIO-EIO](Assigment_MIO-EIO.pdf) - Thầy Nguyễn Văn Hải - FPT Jetking

## 0. Demo
 
[Video Demo - Youtube](https://youtu.be/NjMW7L0Sqqs)

Chuẩn bị Board
![Demo](images/lab_2.jpg)

Console:
![Console](images/console.png)

## 1. RTL Design
Code verilog gồm 3 module: 
- **ws2812_driver.v** tạo tín hiệu xuất ra chân led của NeoPixel 8 RGB Led WS2812.
- **pattern_controller** nhận các tín hiệu **type** (kiểu chớp đèn), **color** (màu đèn), **speed** (tốc độ chớp) từ Processor, và xuất hiện tín hiệu điều khiển đến **ws2812_driver.v**.
- **top_led_controller.v**: module top kết nối 2 module con phía trên.
