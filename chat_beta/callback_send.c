#include "./chat_header.h"

//callback function for send button
void on_button_send_clicked(GtkButton *b, gpointer user_data) {
    struct message_field *param = user_data;    //here parameters for adding new messages to the message field stored

    if(strlen(param->tmp) > 0) {                                //if user typed something to the entry field
        printf("\nuser typed message > %s\n", param->tmp);      //denug info
        write(param->fds[1], param->tmp, strlen(param->tmp));   //writes message from entry field to [1]file desriptor from pipe
        /*following function adds new label with text from message to the messages field*/
        add_new_mess_to_ui(param->tmp, 1, param);
    }
}

//callback function for reading text from entry field where user types his messages
void on_user_message_entry_changed(GtkEntry *e, gpointer user_data) {
    struct message_field *param = user_data;
    sprintf(param->tmp, "%s", gtk_entry_get_text(e));   //save message to struct to send in to the server next
}
