src = chat_main.c callback_send.c get_widgets_from_xml.c history.c new_thread.c rec_send_add_mess_to_ui.c sendall.c get_in_addr.c

src_server = chat_server.c get_in_addr.c sendall.c server_funcs.c

install:
	gcc -Wno-format -o client $(src) -lpthread -Wno-deprecated-declarations -Wno-format-security -lm `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
	gcc $(src_server) -o server -lm `pkg-config --cflags --libs gtk+-3.0`

reinstall:
	rm -rf client
	rm -rf server
	make install
