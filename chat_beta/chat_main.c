#include "./chat_header.h"

int main(int argc, char *argv[]) {
    struct ui_widgets wid;                  //stores all the widgets we need
    pthread_t tid;                          //thread id
    pthread_attr_t attr;                    //thread attributes
    struct thread_data data_for_thread;     //stores data required for executing function in new thread
    struct message_field param;             //stores information for adding new messages

    param.labels = wid.label;               //saves pointer to reserved labels
    param.row = 0;                          //index of first row in grid
    data_for_thread.param = &param;         //we will add new messages in separate thread too, so we need this struct
    data_for_thread.argc = argc;            //save argc
    data_for_thread.argv = &argv;           //save argv, there will be server iddress

    if (pipe(param.fds) == -1) {            //creating pipe to organize communication beetween threads
        perror("pipe: ");
        exit(EXIT_FAILURE);
    }
    
    /*******initialization of GTK, creating builder, connecting signals*********/
    gtk_init(&argc, &argv); // init Gtk; it needs these args because it searches for additional preferences there
    wid.builder = gtk_builder_new_from_file("chat_ui_beta.glade");  //reading xml file | .glade file is a simple xml file with all widgets we need
    g_signal_connect(wid.window_main, "destroy", G_CALLBACK(gtk_main_quit), NULL);  //for correct closing of a window
    gtk_builder_connect_signals(wid.builder, NULL);     //builds table of all signals and callbacks
    /********************************************************************************/

    //READING WIDGETS
	get_widgets_from_xml(&wid);

    /*add grid and message field to the struct because we need to write new
    messages as labels in the message field
    and automaticly scroll down window after receiving messages*/
    param.grid = wid.grid1;
    param.user_message_entry = wid.user_message_entry;
    param.message_scroll_window = wid.message_scroll_window;

    /**CONNETCTING objects to the signals*/
    g_signal_connect(G_OBJECT(wid.button_send), "clicked", G_CALLBACK(on_button_send_clicked), &param);
    g_signal_connect(G_OBJECT(wid.user_message_entry), "changed", G_CALLBACK(on_user_message_entry_changed), &param);

    /*prints history from file (now no, but in the future it will)*/
    display_history(&param);

    /*display window & let gtk control*/
    gtk_widget_show_all(wid.window_main);    //display all widgets of parent widget (window in this case)
	
    /*get default attributes values*/
    pthread_attr_init(&attr);
    /*creating new thread and passing function that will be executed in this thread*/
    pthread_create(&tid, &attr, new_thread_func, &data_for_thread);

    gtk_main();           //entry point to gtk loop
    /*waiting untill second thread ends execution*/
    pthread_join(tid,NULL);
    
    return 0;
}


