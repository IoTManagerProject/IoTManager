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
  #ifdef layout_in_rom
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
  #ifdef layout_in_rom
  all_widgets += widget + "\r\n";
  #else
  addFile("layout.txt", widget);
  #endif
  widget = "";
}
