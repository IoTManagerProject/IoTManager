#pragma once

extern void udpInit();
extern bool isUdpEnabled();
extern void loopUdp();
extern void handleUdp_esp32();
extern void do_udp_data_parse();
extern void add_dev_in_list(String filename, String id, String dev_name, String ip);
extern void send_mqtt_to_udp();