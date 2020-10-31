#pragma once

#include <Arduino.h>

#include "Global.h"
#include "Utils/JsonUtils.h"

class LineParsing {
   protected:
    String _key;
    String _file;
    String _page;
    String _descr;
    String _order;
    String _addr;
    String _reg;
    String _pin;
    String _map;
    String _c;
    String _inv;
    String _state;
    String _db;
    String _type;
    String _int;
    String _cnt;
    String _val;

   public:
    LineParsing() :

                    _key{""},
                    _file{""},
                    _page{""},
                    _descr{""},
                    _order{""},
                    _addr{""},
                    _reg{""},
                    _pin{""},
                    _map{""},
                    _c{""},
                    _inv{""},
                    _state{""},
                    _db{""},
                    _type{""},
                    _int{""},
                    _cnt{""},
                    _val{""}

                    {};

    void update() {
        //String order = sCmd.order();
        //SerialPrint("I","module","create '" + order + "'");
        for (int i = 1; i < 12; i++) {
            if (i == 1) _key = sCmd.next();
            if (i == 2) _file = sCmd.next();
            if (i == 3) _page = sCmd.next();
            if (i == 4) _descr = sCmd.next();
            if (i == 5) _order = sCmd.next();
        }

        for (int i = 1; i < 10; i++) {
            String arg = sCmd.next();
            if (arg != "") {
                if (arg.indexOf("pin[") != -1) {
                    _pin = extractInner(arg);
                }
                if (arg.indexOf("inv[") != -1) {
                    _inv = extractInner(arg);
                }
                if (arg.indexOf("st[") != -1) {
                    _state = extractInner(arg);
                }
                if (arg.indexOf("db[") != -1) {
                    _db = extractInner(arg);
                }
                if (arg.indexOf("map[") != -1) {
                    _map = extractInner(arg);
                }
                if (arg.indexOf("c[") != -1) {
                    _c = extractInner(arg);
                }
                if (arg.indexOf("type[") != -1) {
                    _type = extractInner(arg);
                }
                if (arg.indexOf("addr[") != -1) {
                    _addr = extractInner(arg);
                }
                if (arg.indexOf("reg[") != -1) {
                    _reg = extractInner(arg);
                }
                if (arg.indexOf("int[") != -1) {
                    _int = extractInner(arg);
                }
                if (arg.indexOf("cnt[") != -1) {
                    _cnt = extractInner(arg);
                }
                if (arg.indexOf("val[") != -1) {
                    _val = extractInner(arg);
                }
            }
        }

        _page.replace("#", " ");

        _descr.replace("#", " ");

        _descr.replace("%ver%", String(FIRMWARE_VERSION));
        _descr.replace("%name%", jsonReadStr(configSetupJson, F("name")));

        createWidgetClass(_descr, _page, _order, _file, _key);
    }

    //jsonWriteStr(configOptionJson, _key + "_pin", _pin);

    String gkey() {
        return _key;
    }
    String gfile() {
        return _file;
    }
    String gpage() {
        return _page;
    }
    String gdescr() {
        return _descr;
    }
    String gorder() {
        return _order;
    }
    String gpin() {
        return _pin;
    }
    String ginv() {
        return _inv;
    }
    String gstate() {
        return _state;
    }
    String gmap() {
        return _map;
    }
    String gc() {
        return _c;
    }
    String gtype() {
        return _type;
    }
    String gaddr() {
        return _addr;
    }
    String gregaddr() {
        return _reg;
    }
    String gint() {
        return _int;
    }
    String gmaxcnt() {
        return _cnt;
    }
    String gvalue() {
        return _val;
    }

    void clear() {
        _key = "";
        _file = "";
        _page = "";
        _descr = "";
        _order = "";
        _addr = "";
        _reg = "";
        _pin = "";
        _map = "";
        _c = "";
        _inv = "";
        _state = "";
        _db = "";
        _type = "";
        _int = "";
        _cnt = "";
        _val = "";
    }

    String extractInnerDigit(String str) {
        int p1 = str.indexOf("[");
        int p2 = str.indexOf("]");
        return str.substring(p1 + 1, p2);
    }

    void createWidgetClass(String descr, String page, String order, String filename, String topic) {
        if (filename != "na") {
            String buf = "{}";
            if (!loadWidgetClass(filename, buf)) {
                return;
            }

            jsonWriteStr(buf, "page", page);
            jsonWriteStr(buf, "order", order);
            jsonWriteStr(buf, "descr", descr);
            jsonWriteStr(buf, "topic", prex + "/" + topic);

#ifdef LAYOUT_IN_RAM
            all_widgets += widget + "\r\n";
#else
            addFileLn("layout.txt", buf);
#endif
        }
    }

    bool loadWidgetClass(const String& filename, String& buf) {
        buf = readFile(getWidgetFileClass(filename), 2048);
        bool res = !(buf == "Failed" || buf == "Large");
        if (!res) {
            //SerialPrint("[E]","module","on load" + filename);
        }
        return res;
    }

    const String getWidgetFileClass(const String& name) {
        return "/widgets/" + name + ".json";
    }

    //String jsonWriteStr1(String& json, String name, String value) {
    //    DynamicJsonBuffer jsonBuffer;
    //    JsonObject& root = jsonBuffer.parseObject(json);
    //    root[name] = value;
    //    json = "";
    //    root.printTo(json);
    //    return json;
    //}
};

extern LineParsing myLineParsing;
