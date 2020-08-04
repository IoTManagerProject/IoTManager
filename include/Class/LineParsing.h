#pragma once

#include <Arduino.h>

#include "Global.h"

class LineParsing {
   protected:
    String _key;
    String _file;
    String _page;
    String _descr;
    String _order;

    String _addr;
    String _pin;
    String _map;
    String _c;
    String _inv;
    String _state;
    String _db;

   public:
    LineParsing() :

                    _key{""},
                    _file{""},
                    _page{""},
                    _descr{""},
                    _order{""},
                    _addr{""},
                    _pin{""},
                    _map{""},
                    _c{""},
                    _inv{""},
                    _state{""},
                    _db{""}

                    {};

    void update() {
        //String order = sCmd.order();
        //pm.info("create '" + order + "'");
        for (int i = 1; i < 12; i++) {
            if (i == 1) _key = sCmd.next();
            if (i == 2) _file = sCmd.next();
            if (i == 3) _page = sCmd.next();
            if (i == 4) _descr = sCmd.next();
            if (i == 5) _order = sCmd.next();
        }

        for (int i = 1; i < 6; i++) {
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
            }
        }
        _page.replace("#", " ");
        _descr.replace("#", " ");
        _page.replace(".", " ");
        _descr.replace(".", " ");
        createWidgetClass(_descr, _page, _order, _file, _key);
    }

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

    void clear() {
        _key = "";
        _file = "";
        _page = "";
        _descr = "";
        _order = "";
        _addr = "";
        _pin = "";
        _map = "";
        _c = "";
        _inv = "";
        _state = "";
        _db = "";
    }

    String extractInnerDigit(String str) {
        int p1 = str.indexOf("[");
        int p2 = str.indexOf("]");
        return str.substring(p1 + 1, p2);
    }

    void createWidgetClass(String descr, String page, String order, String filename, String topic) {
        String buf = "{}";
        if (!loadWidgetClass(filename, buf)) {
            return;
        }
        descr.replace("#", " ");
        page.replace("#", " ");

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

    bool loadWidgetClass(const String& filename, String& buf) {
        buf = readFile(getWidgetFileClass(filename), 2048);
        bool res = !(buf == "Failed" || buf == "Large");
        if (!res) {
            //pm.error("on load" + filename);
        }
        return res;
    }

    const String getWidgetFileClass(const String& name) {
        return "/widgets/" + name + ".json";
    }
};
