#pragma once

#include <Arduino.h>

#include "Global.h"

class Item {
    
   protected:
    String type;
    String key;
    String file;
    String page;
    String descr;
    String order;

    String addr;
    String pin;
    String map;
    String c;
    String inv;
    String state;

   public:
    Item() : type{""},
             key{""},
             file{""},
             page{""},
             descr{""},
             order{""},
             addr{""},
             pin{""},
             map{""},
             c{""},
             inv{""},
             state{""}

             {};

    void update() {
        //String order = sCmd.order();
        //pm.info("create '" + order + "'");
        for (int i = 1; i < 12; i++) {
            if (i == 1) type = sCmd.next();
            if (i == 2) key = sCmd.next();
            if (i == 3) file = sCmd.next();
            if (i == 4) page = sCmd.next();
            if (i == 5) descr = sCmd.next();
            if (i == 6) order = sCmd.next();
        }

        for (int i = 1; i < 6; i++) {
            String arg = sCmd.next();
            if (arg != "") {
                if (arg.indexOf("pin[") != -1) {
                    pin = extractInner(arg);
                }
                if (arg.indexOf("inv[") != -1) {
                    inv = extractInner(arg);
                }
                if (arg.indexOf("st[") != -1) {
                    state = extractInner(arg);
                }
            }
        }
        createWidgetClass(descr, page, order, file, key);
    }

    String gtype() {
        return type;
    }
    String gkey() {
        return key;
    }
    String gfile() {
        return file;
    }
    String gpage() {
        return page;
    }
    String gdescr() {
        return descr;
    }
    String gorder() {
        return order;
    }
    String gpin() {
        return pin;
    }
    String ginv() {
        return inv;
    }
    String gstate() {
        return state;
    }

    void clear() {
        type = "";
        key = "";
        file = "";
        page = "";
        descr = "";
        order = "";
        addr = "";
        pin = "";
        map = "";
        c = "";
        inv = "";
        state = "";
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
        addFile("layout.txt", buf);
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

extern Item* myItem;