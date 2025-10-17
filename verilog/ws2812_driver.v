module ws2812_driver #(
    parameter NUM_LED = 8
)(
    input  wire clk,
    input  wire reset,
    input  wire start,
    input  wire [NUM_LED*24-1:0] rgb_data,
    output reg  led_data
);
    localparam T0H = 35, T0L = 80;      // T0H=0.35us, T0L=0.8us @100MHz
    localparam T1H = 70, T1L = 60;      // T1H=0.7us, T1L=0.6us @100MHz
    localparam RES = 6000;              // Reset pulse > 50us @100MHz

    reg [12:0] timer;
    reg [5:0] bit_cnt;
    reg [5:0] led_idx;
    reg [23:0] shift_reg;
    reg state;
    reg bit_state;

    localparam IDLE = 0, SEND = 1;
    localparam BIT_HIGH = 0, BIT_LOW = 1;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            timer <= 0;
            bit_cnt <= 0;
            led_idx <= 0;
            shift_reg <= 0;
            led_data <= 0;
            state <= IDLE;
            bit_state <= BIT_HIGH;
        end else begin
            case (state)
                IDLE: begin
                    if (start) begin
                        state <= SEND;
                        shift_reg <= rgb_data[23:0];
                        led_idx <= 0;
                        bit_cnt <= 0;
                        timer <= 0;
                        bit_state <= BIT_HIGH;
                    end
                end

                SEND: begin
                    if (bit_cnt < 24) begin
                        case (bit_state)
                            BIT_HIGH: begin
                                if (timer == 0) begin
                                    led_data <= 1;
                                    timer <= shift_reg[23] ? T1H : T0H;
                                end else if (timer == 1) begin
                                    led_data <= 0;
                                    timer <= shift_reg[23] ? T1L : T0L;
                                    bit_state <= BIT_LOW;
                                end else begin
                                    timer <= timer - 1;
                                end
                            end
                            
                            BIT_LOW: begin
                                if (timer == 1) begin
                                    shift_reg <= {shift_reg[22:0], 1'b0};
                                    bit_cnt <= bit_cnt + 1;
                                    bit_state <= BIT_HIGH;
                                    timer <= 0;
                                end else begin
                                    timer <= timer - 1;
                                end
                            end
                        endcase
                    end else begin
                        if (led_idx < NUM_LED - 1) begin
                            led_idx <= led_idx + 1;
                            shift_reg <= rgb_data[(led_idx + 1)*24 +: 24];
                            bit_cnt <= 0;
                            timer <= 0;
                            bit_state <= BIT_HIGH;
                        end else begin
                            if (timer < RES) begin
                                led_data <= 0;
                                timer <= timer + 1;
                            end else begin
                                state <= IDLE;
                            end
                        end
                    end
                end
            endcase
        end
    end
endmodule
