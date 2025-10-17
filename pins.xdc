# Buttons
set_property PACKAGE_PIN F7 [get_ports {btn_in[0]}]
set_property PACKAGE_PIN F8 [get_ports {btn_in[1]}]
set_property PACKAGE_PIN D6 [get_ports {btn_in[2]}]
set_property PACKAGE_PIN D7 [get_ports {btn_in[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {btn_in[*]}]
set_property PULLUP true [get_ports {btn_in[*]}]

# WS2812 data output
set_property PACKAGE_PIN E5 [get_ports {led_data}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_data}]
