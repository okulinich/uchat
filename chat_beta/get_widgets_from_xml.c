#include "chat_header.h"

void get_widgets_from_xml(struct ui_widgets *wid) {
    /**************READING WIDGETS****************/
    //(meaning of all widgets described in header where structure described)
	wid->window_main = GTK_WIDGET(gtk_builder_get_object(wid->builder, "window_main"));
    wid->fixed1 = GTK_WIDGET(gtk_builder_get_object(wid->builder, "fixed1"));
    wid->stack_main = GTK_WIDGET(gtk_builder_get_object(wid->builder, "stack_main"));
    wid->stack_main_switcher = GTK_WIDGET(gtk_builder_get_object(wid->builder, "stack_main_switcher"));
    wid->page0fixed = GTK_WIDGET(gtk_builder_get_object(wid->builder, "page0fixed"));
    wid->page1fixed = GTK_WIDGET(gtk_builder_get_object(wid->builder, "page1fixed"));
    wid->page2fixed = GTK_WIDGET(gtk_builder_get_object(wid->builder, "page2fixed"));
    wid->page3fixed = GTK_WIDGET(gtk_builder_get_object(wid->builder, "page3fixed"));
    wid->message_scroll_window = GTK_WIDGET(gtk_builder_get_object(wid->builder, "message_scroll_window"));
    wid->viewport1 = GTK_WIDGET(gtk_builder_get_object(wid->builder, "viewport1"));
    wid->grid1 = GTK_WIDGET(gtk_builder_get_object(wid->builder, "grid1"));
    wid->user_message_entry = GTK_WIDGET(gtk_builder_get_object(wid->builder, "user_message_entry"));
    wid->button_send = GTK_WIDGET(gtk_builder_get_object(wid->builder, "button_send"));
    /*********************************************/
}

