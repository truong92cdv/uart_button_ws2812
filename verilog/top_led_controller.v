module top_led_controller #(
    parameter NUM_LED = 8
)(
    // System signals
    input  wire clk,
    input  wire reset_n,
    
    // Control inputs
    input  wire [1:0] type,  // 0: all leds on, 1: running, 2: 2 direction running, 3: all leds flash
    input  wire [2:0] color, // 0: red, 1: green, 2: blue, 3: yellow, 4: cyan, 5: magenta, 6: white, 7: off
    input  wire [2:0] speed, // 0: 200ms, 1: 250ms, 2: 300ms, 3: 350ms, 4: 400ms, 5: 450ms, 6: 500ms, 7: 550ms
    
    // WS2812 output
    output wire led_data
);

    // Internal signals connecting pattern_controller to ws2812_driver
    wire start;
    wire [NUM_LED*24-1:0] rgb_data;
    wire reset;

    // reset from Zynq is active low, so we need to invert it
    assign reset = ~reset_n;
    
    // Instantiate pattern_controller
    pattern_controller #(
        .NUM_LED(NUM_LED)
    ) u_pattern_controller (
        .clk(clk),
        .reset(reset),
        .type(type),
        .color(color),
        .speed(speed),
        .start(start),
        .rgb_data(rgb_data)
    );
    
    // Instantiate ws2812_driver
    ws2812_driver #(
        .NUM_LED(NUM_LED)
    ) u_ws2812_driver (
        .clk(clk),
        .reset(reset),
        .start(start),
        .rgb_data(rgb_data),
        .led_data(led_data)
    );

endmodule
