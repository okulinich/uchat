#include "chat_header.h"

//if user typed something UI wrote that text to the writing end of pipe
//this function reads message from read end of pipe
//and tries to send it to the server
void send_mess_from_ui_to_server(struct thread_data *data1, struct client_data *cld) {
    read(data1->param->fds[0], cld->buff_ui, cld->buff_ui_len);         //read message from read end of pipe
    cld->bytes_sent = strlen(cld->buff_ui);
    if (sendall(cld->sockfd, cld->buff_ui, &cld->bytes_sent) == -1) {   //send to server
        fprintf(stderr, "sendall error\n");
        close(cld->sockfd);
        exit(EXIT_FAILURE);
    }
}

//adds new message to the message field in UI
//side - 0 or 1, 0 - left (corresponds to the message sent by someone to you) 1 - rigth (your message)
//param - struct that stores all required data to add new label to the UI
void add_new_mess_to_ui(char *message, int side, struct message_field *param) {
    gtk_grid_insert_row(GTK_GRID(param->grid), param->row);
    param->labels[param->row] = gtk_label_new(message);
    gtk_label_set_justify (GTK_LABEL(param->labels[param->row]), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign (GTK_LABEL(param->labels[param->row]), 0.0);
    gtk_grid_attach (GTK_GRID(param->grid), param->labels[param->row], side, param->row, 1, 1);
    gtk_widget_show(param->labels[param->row]);
    gtk_entry_set_text(GTK_ENTRY(param->user_message_entry), (gchar *)"");  //replace user input with empty string
    usleep(30000);   //required because of unknown reason (scroll doesn't work without it)
    scroll_window_to_last_message(param);
    param->row++;
}

//if user on another terminel/computer sends message -> server wants to send it through the socket
//this function receives server's message and adds new message as a label to the UI
void receive_from_serv_add_to_ui(struct thread_data *data1, struct client_data *cld) {
    if ((cld->bytes_recv = recv(cld->sockfd, cld->buf_serv, MAXDATASIZE - 1, 0)) == -1) { 
        perror("recv");
        close(cld->sockfd);
        exit(EXIT_FAILURE); 
    }
    cld->buf_serv[cld->bytes_recv] = '\0'; 
    printf(">(someone)> %s", cld->buf_serv);    //debug info (doesn't work -> why?)
    /*ADD message from server to the message field in UI*/
    add_new_mess_to_ui(cld->buf_serv, 0, data1->param);
}

//scroll down message field to the last message sent
void scroll_window_to_last_message(struct message_field *param) {
    GtkAdjustment* adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(param->message_scroll_window));
    gtk_adjustment_set_value(adjustment, gtk_adjustment_get_upper(adjustment));
}
