module pattern_controller #(
    parameter NUM_LED = 8
)(
    input  wire clk,
    input  wire reset,
    input  wire [1:0] type,  // 0: all leds on, 1: running left-to-right, 2: ping-pong running, 3: all leds flash
    input  wire [2:0] color, // 0: red, 1: green, 2: blue, 3: yellow, 4: cyan, 5: magenta, 6: white, 7: off
    input  wire [2:0] speed, // 0: 200ms, 1: 250ms, 2: 300ms, 3: 350ms, 4: 400ms, 5: 450ms, 6: 500ms, 7: 550ms
    output reg  start,
    output reg  [NUM_LED*24-1:0] rgb_data
);

    // Internal registers
    reg [1:0] type_reg;
    reg [2:0] color_reg;
    reg [2:0] speed_reg;
    
    // Timer for speed control (assuming 100MHz clock)
    reg [26:0] timer;
    reg [26:0] speed_timer;
    reg timer_enable;
    
    // Pattern state
    reg [2:0] pattern_state;
    reg [2:0] led_position;
    reg direction; // 0: forward, 1: backward
    reg flash_state;
    
    // Speed values in clock cycles (100MHz = 100,000,000 Hz)
    // 200ms = 20,000,000 cycles, 250ms = 25,000,000 cycles, etc.
    reg [26:0] speed_values [0:7];
    
    // Color definitions (RGB 24-bit)
    reg [23:0] color_values [0:7];
    
    // State definitions
    localparam IDLE = 0, UPDATE = 1, RUNNING = 2;
    
    // // Initialize speed values
    initial begin
        speed_values[0] = 27'd20_000_000;  // 200ms
        speed_values[1] = 27'd25_000_000;  // 250ms
        speed_values[2] = 27'd30_000_000;  // 300ms
        speed_values[3] = 27'd35_000_000;  // 350ms
        speed_values[4] = 27'd40_000_000;  // 400ms
        speed_values[5] = 27'd45_000_000;  // 450ms
        speed_values[6] = 27'd50_000_000;  // 500ms
        speed_values[7] = 27'd55_000_000;  // 550ms
    end

    // Initialize speed values (for simulation)

//    initial begin
//        speed_values[0] = 27'd20_00;  // 200ms
//        speed_values[1] = 27'd25_00;  // 250ms
//        speed_values[2] = 27'd30_00;  // 300ms
//        speed_values[3] = 27'd35_00;  // 350ms
//        speed_values[4] = 27'd40_00;  // 400ms
//        speed_values[5] = 27'd45_00;  // 450ms
//        speed_values[6] = 27'd50_00;  // 500ms
//        speed_values[7] = 27'd55_00;  // 550ms
//    end
    
    // Initialize color values (WS2812 uses GRB order, not RGB)
    initial begin
        color_values[0] = 24'h00FF00;  // Red (G=0, R=FF, B=0)
        color_values[1] = 24'hFF0000;  // Green (G=FF, R=0, B=0)
        color_values[2] = 24'h0000FF;  // Blue (G=0, R=0, B=FF)
        color_values[3] = 24'hFFFF00;  // Yellow (G=FF, R=FF, B=0)
        color_values[4] = 24'hFF00FF;  // Cyan (G=FF, R=0, B=FF)
        color_values[5] = 24'h00FFFF;  // Magenta (G=00, R=FF, B=FF)
        color_values[6] = 24'hFFFFFF;  // White (G=FF, R=FF, B=FF)
        color_values[7] = 24'h000000;  // Off (G=0, R=0, B=0)
    end
    
    // Main state machine
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            type_reg <= 2'b00;
            color_reg <= 3'b000;
            speed_reg <= 3'b000;
            rgb_data <= 0;
            start <= 0;
            timer <= 0;
            speed_timer <= 0;
            timer_enable <= 0;
            pattern_state <= IDLE;
            led_position <= 0;
            direction <= 0;
            flash_state <= 0;
        end else begin
            // Default values
            start <= 0;
            
            // Check for parameter changes
            if ((type != type_reg) || (color != color_reg) || (speed != speed_reg)) begin
                type_reg <= type;
                color_reg <= color;
                speed_reg <= speed;
                pattern_state <= UPDATE;
                timer_enable <= 0;
                speed_timer <= 0;
            end
            
            case (pattern_state)
                IDLE: begin
                    timer_enable <= 0;
                    speed_timer <= 0;
                end
                
                UPDATE: begin
                    // Generate new pattern based on current parameters
                    generate_pattern();
                    start <= 1;
                    pattern_state <= RUNNING;
                    timer_enable <= 1;
                    speed_timer <= speed_values[speed_reg];
                end
                
                RUNNING: begin
                    if (timer_enable) begin
                        if (speed_timer > 0) begin
                            speed_timer <= speed_timer - 1;
                        end else begin
                            // Timer expired, update pattern
                            update_pattern();
                            start <= 1;
                            speed_timer <= speed_values[speed_reg];
                        end
                    end
                end
            endcase
        end
    end
    
    // Generate initial pattern based on type and color
    task generate_pattern;
        integer i;
        begin
            case (type_reg)
                2'b00: begin // All LEDs on
                    for (i = 0; i < NUM_LED; i = i + 1) begin
                        rgb_data[i*24 +: 24] <= color_values[color_reg];
                    end
                    timer_enable <= 0; // Static pattern
                end
                
                2'b01: begin // Running left-to-right pattern (single LED moving)
                    for (i = 0; i < NUM_LED; i = i + 1) begin
                        if (i == 0) begin
                            rgb_data[i*24 +: 24] <= color_values[color_reg];
                        end else begin
                            rgb_data[i*24 +: 24] <= 24'h000000;
                        end
                    end
                    led_position <= 0;
                    direction <= 0;
                end
                
                2'b10: begin // ping-pong running
                    for (i = 0; i < NUM_LED; i = i + 1) begin
                        if (i == 0) begin
                            rgb_data[i*24 +: 24] <= color_values[color_reg];
                        end else begin
                            rgb_data[i*24 +: 24] <= 24'h000000;
                        end
                    end
                    led_position <= 0;
                    direction <= 0;
                end
                
                2'b11: begin // All LEDs flash
                    for (i = 0; i < NUM_LED; i = i + 1) begin
                        rgb_data[i*24 +: 24] <= color_values[color_reg];
                    end
                    flash_state <= 1;
                end
            endcase
        end
    endtask
    
    // Update pattern for animation
    task update_pattern;
        integer i;
        begin
            case (type_reg)
                2'b01: begin // Running pattern (single LED moving left-to-right, then restart)
                    // Turn off all LEDs first
                    for (i = 0; i < NUM_LED; i = i + 1) begin
                        rgb_data[i*24 +: 24] <= 24'h000000;
                    end
                    
                    // Move to next position (simple left-to-right)
                    led_position <= led_position + 1;
                    if (led_position >= NUM_LED) begin
                        led_position <= 0;
                    end
                    
                    // Turn on only the current LED
                    rgb_data[led_position*24 +: 24] <= color_values[color_reg];
                end
                
                2'b10: begin // Ping-pong running (single LED moving back and forth)
                    // Turn off all LEDs first
                    for (i = 0; i < NUM_LED; i = i + 1) begin
                        rgb_data[i*24 +: 24] <= 24'h000000;
                    end
                    
                    // Move to next position (ping-pong)
                    if (direction == 0) begin
                        led_position <= led_position + 1;
                        if (led_position >= NUM_LED - 2) begin
                            direction <= 1;
                        end
                    end else begin
                        led_position <= led_position - 1;
                        if (led_position <= 1) begin
                            direction <= 0;
                        end
                    end
                    
                    // Turn on only the current LED
                    rgb_data[led_position*24 +: 24] <= color_values[color_reg];
                end
                
                2'b11: begin // All LEDs flash
                    flash_state <= ~flash_state;
                    for (i = 0; i < NUM_LED; i = i + 1) begin
                        rgb_data[i*24 +: 24] <= flash_state ? color_values[color_reg] : 24'h000000;
                    end
                end
            endcase
        end
    endtask
    
endmodule
