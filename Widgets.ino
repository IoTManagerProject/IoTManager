//======================================================================================================================
//===============================================Создание виджетов=======================================================
void createWidget (String widget_name, String  page_name, String page_number, String file, String topic) {

  String widget;
  widget = readFile(file, 1024);

  if (widget == "Failed") return;
  if (widget == "Large") return;

  widget_name.replace("#", " ");
  page_name.replace("#", " ");

  jsonWriteStr(widget, "page", page_name);
  jsonWriteStr(widget, "order", page_number);
  jsonWriteStr(widget, "descr", widget_name);
  jsonWriteStr(widget, "topic", prex + "/" + topic);

#ifdef layout_in_ram
  all_widgets += widget + "\r\n";
#else
  addFile("layout.txt", widget);
#endif
  widget = "";
}

void createWidgetParam (String widget_name, String  page_name, String page_number, String file, String topic, String name1, String param1, String name2, String param2, String name3, String param3) {

  String widget;
  widget = readFile(file, 1024);

  if (widget == "Failed") return;
  if (widget == "Large") return;

  widget_name.replace("#", " ");
  page_name.replace("#", " ");

  jsonWriteStr(widget, "page", page_name);
  jsonWriteStr(widget, "order", page_number);
  jsonWriteStr(widget, "descr", widget_name);
  jsonWriteStr(widget, "topic", prex + "/" + topic);

  if (name1 != "") jsonWriteStr(widget, name1, param1);
  if (name2 != "") jsonWriteStr(widget, name2, param2);
  if (name3 != "") jsonWriteStr(widget, name3, param3);

#ifdef layout_in_ram
  all_widgets += widget + "\r\n";
#else
  addFile("layout.txt", widget);
#endif
  widget = "";
}

void createChart (String widget_name, String  page_name, String page_number, String file, String topic, String maxCount) {

  String widget;
  widget = readFile(file, 1024);

  if (widget == "Failed") return;
  if (widget == "Large") return;

  widget_name.replace("#", " ");
  page_name.replace("#", " ");

  jsonWriteStr(widget, "page", page_name);
  jsonWriteStr(widget, "order", page_number);
  //jsonWriteStr(widget, "descr", widget_name);
  jsonWriteStr(widget, "series", widget_name);
  jsonWriteStr(widget, "maxCount", maxCount);
  jsonWriteStr(widget, "topic", prex + "/" + topic);

#ifdef layout_in_ram
  all_widgets += widget + "\r\n";
#else
  addFile("layout.txt", widget);
#endif
  widget = "";
}

void choose_widget_and_create(String widget_name, String page_name, String page_number, String type, String topik) {

  if (type == "any-data") createWidget (widget_name, page_name, page_number, "widgets/widget.anyData.json", topik);
  if (type == "progress-line") createWidget (widget_name, page_name, page_number, "widgets/widget.progLine.json", topik);
  if (type == "progress-round") createWidget (widget_name, page_name, page_number, "widgets/widget.progRound.json", topik);
  if (type == "fill-gauge") createWidget (widget_name, page_name, page_number, "widgets/widget.fillGauge.json", topik);

}
