#include "Global.h"
#include "classes/IoTItem.h"
#include <ArduinoJson.h>
#ifdef ESP32
#include "sdkconfig.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "argtable3/argtable3.h"
#include "ping/ping_sock.h"

#endif
#ifdef ESP8266
#include <ESP8266Ping.h>
#endif

#ifdef ESP32
bool pingState = false;
/*
static struct
{
    uint32_t timeout;
    uint32_t interval;
    uint32_t data_size;
    uint32_t count;
    uint32_t tos;
    uint32_t ttl;
    //String host;
    // arg_end *end;
} ping_args;
*/
esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();

static void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args)
{
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    //   Serial.printf("%" PRIu32 " bytes from %s icmp_seq=%" PRIu16 " ttl=%" PRIu16 " time=%" PRIu32 " ms\n",
    //         recv_len, ipaddr_ntoa((ip_addr_t*)&target_addr), seqno, ttl, elapsed_time);
    SerialPrint("i", "Ping", String(recv_len) + " bytes from " + String(ipaddr_ntoa((ip_addr_t *)&target_addr)) + " icmp_seq=" + String(seqno) + " ttl=" + String(ttl) + " time=" + String(elapsed_time) + " ms");
    pingState = true;
}

static void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    // Serial.printf("From %s icmp_seq=%d timeout\n",ipaddr_ntoa((ip_addr_t*)&target_addr), seqno);
    SerialPrint("i", "Ping", "From " + String(ipaddr_ntoa((ip_addr_t *)&target_addr)) + " icmp_seq=" + String(seqno) + " timeout");
    pingState = false;
}

static void cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *args)
{
    ip_addr_t target_addr;
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    uint32_t loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);
    if (IP_IS_V4(&target_addr))
    {
        // Serial.printf("\n--- %s ping statistics ---\n", inet_ntoa(*ip_2_ip4(&target_addr)));
        SerialPrint("i", "Ping", "\n--- " + String(inet_ntoa(*ip_2_ip4(&target_addr))) + " ping statistics ---");
    }
    else
    {
        // Serial.printf("\n--- %s ping statistics ---\n", inet6_ntoa(*ip_2_ip6(&target_addr)));
        SerialPrint("i", "Ping", "\n--- " + String(inet6_ntoa(*ip_2_ip6(&target_addr))) + " ping statistics ---");
    }
    // Serial.printf("%" PRIu32 " packets transmitted, %" PRIu32 " received, %" PRIu32 "%% packet loss, time %" PRIu32 "ms\n",
    //        transmitted, received, loss, total_time_ms);
    SerialPrint("i", "Ping", String(transmitted) + " packets transmitted, " + String(received) + " received, " + String(loss) + "% packet loss, time " + String(total_time_ms) + "ms\n");
    // delete the ping sessions, so that we clean up all resources and can create a new ping session
    // we don't have to call delete function in the callback, instead we can call delete function from other tasks
    esp_ping_delete_session(hdl);
}
#endif
class PingIoTM : public IoTItem
{
private:
    String _ip = "";
    int timeout = 0;
    int interval = 0;
    int data_size = 0;
    int count = 0;
    int tos = 0;
    int ttl = 0;

public:
    PingIoTM(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "ip", _ip);
        jsonRead(parameters, "timeout", timeout);
        jsonRead(parameters, "interval", interval);
        jsonRead(parameters, "data_size", data_size);
        jsonRead(parameters, "count", count);
        jsonRead(parameters, "tos", tos);
        jsonRead(parameters, "ttl", ttl);

#ifdef ESP32
/*
        ping_args.timeout = 10;     // arg_dbl0("W", "timeout", "<t>", "Time to wait for a response, in seconds");
        ping_args.interval = 1;     // arg_dbl0("i", "interval", "<t>", "Wait interval seconds between sending each packet");
        ping_args.data_size = 0;    // arg_int0("s", "size", "<n>", "Specify the number of data bytes to be sent");
        ping_args.count = 0;        // arg_int0("c", "count", "<n>", "Stop after sending count packets");
        ping_args.tos = 0;          // arg_int0("Q", "tos", "<n>", "Set Type of Service related bits in IP datagrams");
        ping_args.ttl = 0;          // arg_int0("T", "ttl", "<n>", "Set Time to Live related bits in IP datagrams");
        // ping_args.end = arg_end(1);
*/
        if (timeout > 0)
            config.timeout_ms = (uint32_t)(timeout * 1000);
        if (interval > 0)
            config.interval_ms = (uint32_t)(interval * 1000);
        if (data_size > 0)
            config.data_size = (uint32_t)(data_size);
        if (count > 0)
            config.count = (uint32_t)(count);
        if (tos > 0)
            config.tos = (uint32_t)(tos);
        if (ttl > 0)
            config.ttl = (uint32_t)(ttl);
#endif
    }

    void doByInterval()
    {
#ifdef ESP8266
        regEvent((float)Ping.ping(_ip.c_str()), "ping");
#endif
    }

    // Основной цикл программы
    void loop()
    {
#ifdef ESP32
        if (value.valD != (float)pingState)
            regEvent((float)pingState, "ping");

#endif
        IoTItem::loop();
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        IoTValue val;
        if (command == "ping")
        {
#ifdef ESP32
            if (param.size())
            {
                // parse IP address
                struct sockaddr_in6 sock_addr6;
                ip_addr_t target_addr;
                memset(&target_addr, 0, sizeof(target_addr));

                if (inet_pton(AF_INET6, _ip.c_str(), &sock_addr6.sin6_addr) == 1)
                {
                    /* convert ip6 string to ip6 address */
                    ipaddr_aton(_ip.c_str(), &target_addr);
                }
                else
                {
                    struct addrinfo hint;
                    struct addrinfo *res = NULL;
                    memset(&hint, 0, sizeof(hint));
                    /* convert ip4 string or hostname to ip4 or ip6 address */
                    if (getaddrinfo(_ip.c_str(), NULL, &hint, &res) != 0)
                    {
                        // Serial.printf("ping: unknown host %s\n", ping_args.host.c_str());
                        SerialPrint("E", "Ping", "ping: unknown host " + _ip);
                        // return 1;
                    }
                    if (res->ai_family == AF_INET)
                    {
                        struct in_addr addr4 = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
                        inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
                    }
                    else
                    {
                        struct in6_addr addr6 = ((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr;
                        inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
                    }
                    freeaddrinfo(res);
                }
                config.target_addr = target_addr;
                /* set callback functions */
                esp_ping_callbacks_t cbs = {
                    .cb_args = NULL,
                    .on_ping_success = cmd_ping_on_ping_success,
                    .on_ping_timeout = cmd_ping_on_ping_timeout,
                    .on_ping_end = cmd_ping_on_ping_end};
                esp_ping_handle_t ping;

                esp_ping_new_session(&config, &cbs, &ping);
                esp_ping_start(ping);
            }
#endif
#ifdef ESP8266
            if (param.size())
            {
                val.valD = Ping.ping(param[0].valS.c_str());
                if (val.valD)
                    SerialPrint("I", "Ping", "Ping  success");
                else
                    SerialPrint("E", "Ping", "Ping  error");
                regEvent(val, "ping");
                return val;
            }
#endif
        }
        return {};
    }

    ~PingIoTM(){};
};

void *getAPI_Ping(String subtype, String param)
{
    if (subtype == F("Ping"))
    {
        return new PingIoTM(param);
    }
    else
    {
        return nullptr;
    }
}
