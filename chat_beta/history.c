#include "chat_header.h"

//this is only a simulation of hisory.
//in the future history will be read from the file
//and this function will display it in the message field
void display_history(struct message_field *param) {
    char tmp[128] = {0};

    sprintf(tmp, "someone> %s", "hello!");
    //The following code will populate the grid with non-clicable labels
    //here we should read and display history
    gtk_grid_insert_column(GTK_GRID(param->grid), 1);
	for(int i = 0; i < 10; i++) {
		gtk_grid_insert_row (GTK_GRID(param->grid), param->row);
		param->labels[param->row] = gtk_label_new(tmp);
		gtk_label_set_justify (GTK_LABEL(param->labels[param->row]), GTK_JUSTIFY_LEFT);
		gtk_label_set_xalign(GTK_LABEL(param->labels[param->row]), 0.0);
        if(param->row % 2 == 0)
		    gtk_grid_attach(GTK_GRID(param->grid), param->labels[param->row], 0, param->row, 1, 1);
        else
            gtk_grid_attach (GTK_GRID(param->grid), param->labels[param->row], 1, param->row, 1, 1);
		param->row++;
	}
}
