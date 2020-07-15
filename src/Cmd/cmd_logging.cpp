#include "Cmd.h"

#include "Collection/Logger.h"
#include "Collection/Widgets.h"

// logging {name:"temp1",interval:"5s",store:"2h",descr:"Температура",page:"Датчики",order:2}
void cmd_logging() {
    KeyValueStore* params = new KeyValueStore(sCmd.next());
    String name = params->read("name");
    String interval = params->read("interval");
    String store = params->read("store");
    String descr = params->read("descr");
    String page = params->read("page");
    String order = params->read("order");
    delete params;

    unsigned long intervalTime = parsePeriod(interval);
    unsigned long storeTime = parsePeriod(store);
    size_t maxCount = storeTime / intervalTime;

    Logger::add(name.c_str(), intervalTime, storeTime);
 
    Widgets::createChart(descr, page, order, "chart", name + "_ch", maxCount);
}