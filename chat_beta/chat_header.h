#pragma once

/*
Чат складається з двох частин: клієнтської і серверної, відповідно це дві окремі програми.
Весь сервер міститься в файлі chat_server.с, його основні функції в файлі server_funcs. Але деякі
функції , як sendall використовуються і сервером і клієнтом, тому вони винесені в окремі файли. 
В сервері мало зрозумілого) але нічого слкадного, все з цього туторіалу:
(ангійською)
https://beej.us/guide/bgnet/html/#close-and-shutdownget-outta-my-face
(російською)
https://masandilov.ru/network/guide_to_network_programming
Половина клієнта теж взята звідти.

Із клієнтською частиною веселіше, все ускладнює GUI. Раджу глянути кілька відосів цього чувака
https://www.youtube.com/watch?v=g-KDOH_uqPk&list=PLmMgHNtOIstZEvqYJncYUx52n8_OV0uWy&index=2&t=0s
після того як спробуєш кілька його прикладів, стає конкретно ясніше.
В чому основна фішка: як можна побачити в файлі chat_main.c в мейні там тільки різні функції графічної бібліотеки.
Все тому, що вся клієнтська частина, яка комунікує з нашим червером винесена в окремий потік.
Чому так? - клієнт це по-суті вічний цикл, який відловлює повідомлення від сервера і якийсь інпут від користувача.
Цим займається функція SELECT, вона слухає ті файлові дескриптори, які ти їй передаєш і повідомляє коли щось сталося.
Якби не було графічного інтерфейсу, а весь чат був в терміналі, тоді ми просто могли б передати у функцію SELECT 
дескриптор сокета (куди повідомлення присилає сервер) і STDIN (куди пише свої повідомлення користувач). Тоді якщо 
повідомлення прийшло від сервера - воно зчитується із сокета і відображається в терміналі, а коли користувач набрав
якесь повідомлення в терміналі - воно зчитується з терміналу і відправляється на сервер.
АЛЕ у нас є ще графічний інтерфейс. UI це теж по-суті вічний цикл, який відловлює події: натискання клавіш, кліки мишки,
і т.д. ТОБТО в нас має одночасно виконуватись два вічних цикли - клієнт і графічний ітерфейс.
Розмістити їх в двох процесах неможливо, тому що процеси виконуються послідовно, тому вони розміщені в двох
потоках. Отримуємо ситуацію коли в нас одночасно виконуються два цикли - один відловлює повідомлення сервера і 
добавляє їх в графічний інтерфейс (клієнт), інший відловлює повідомлення, набрані у відповідному полі користувачем
і відправляє їх на сервер. (А сервер уже розсилає всім іншим користувачам чату (клієнтам)) 
Головна складність: організувати комунікацію між потоками. Іншими словами, якщо без графічного інтерфейсу ми добавляли
в функцію SELECT девкриптор STDIN щоб сервер читав введені користувачем повідомлення, то який дескриптор добавити в
цю функцію якщо користувач вводить свої повідомлення не в терміналі, а у вікні графічного інтерфейсу.
РІШЕННЯ: пайпи (pipe, конвеєр). За допомогою функції pipe ( man pipe ) я створюю два файлові дескриптори які зберігаються
в масиві fds[2]. Все, що було записано в дескриптор fds[1] (він відповідає за ввід - stdout) далі може бути зчитано 
з дескриптора fds[0] (аналог stdin). Тому функція, яка обробляє натискання кнопки send> із графіного інтерфейсу
зчитує повідомлення користувача з відповідного вікна і записує його в fds[1]. Клієнт же слухає функцією SELECT іншу сторону
пайпа fds[0], зчитує звідти повідомлення, яке записала функція графічного інтерфейсу і відправляє це повідомлення серверу.
*/


#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"                     //default port for our app
#define MAXDATASIZE 1024                //max message length
#define BUFSIZE 1024
#define LABELS_NUM 1024                 //number of labels that are reserved to display messages
#define BACKLOG 10                      //number of connections that can wait in 

//srtuct that stores information for adding new messages 
//to the gtk window where messages appear
struct message_field {
    int fds[2];                     //file descriptors for pipe
    char tmp[MAXDATASIZE];          //temporary string for reading/writing messages
    int row;                        //current row in grid (number of labels(messages) displayed in the message field)
    GtkWidget *grid;                //table in message field
    GtkWidget **labels;             //labels reserved to display messages
    GtkWidget *user_message_entry;  //entry field where user types his messages
};

struct ui_widgets {
    GtkBuilder *builder;                 //class that can costruct app from xml file
    GtkWidget  *window_main;             //main window widget
    GtkWidget  *fixed1;                  //fixed container for main window
    GtkWidget  *stack_main;              //main steck with home|chat|settings|else
    GtkWidget  *stack_main_switcher;     //buttons to choose chat|home...
    GtkWidget  *page0fixed;              //conteiner for stack page
    GtkWidget  *page1fixed;
    GtkWidget  *page2fixed;
    GtkWidget  *page3fixed;              //conteiner for stack page
    GtkWidget  *message_scroll_window;   //scroll window in chat for messages
    GtkWidget  *viewport1;               //viewport for scroll window in chat
    GtkWidget  *grid1;                   //grid for messages in chat
    GtkWidget  *label[LABELS_NUM];       //reserving 1000 labels for messages
    GtkWidget  *user_message_entry;      //field for user text messages
    GtkWidget  *button_send;             //send message button
};

//stores data required for executing function in new thread
struct thread_data {
    struct message_field *param;    //stores information for adding new messages 
    int argc;                       //num of cmd args
    char ***argv;                   //cmd arguments
};

//stores all the data required for conection with a server
struct client_data {
    struct  addrinfo hints;                     //for local preferences
    struct  addrinfo *servinfo;                 //for getting info from gettaddrinfo
    struct  addrinfo *p;                        //for treveling in list returned by gettaddrinfo
    int sockfd;                                 //socket file descriptor for connnection with server
    int     rv;                                 //result of getaddrinfo
    char    s[INET6_ADDRSTRLEN];                //string representation of address struct
    char buff_ui[MAXDATASIZE];                  //stores messages from UI
    size_t buff_ui_len;                         //stores length of buff_uier and will store length of message received from server
    char buf_serv[MAXDATASIZE];                 //buff_uier for recieving data from server
    int bytes_sent;                             //bytes sent to the server
    int bytes_recv;                             //bytes received from the server
};

//stores all the data requider to run server
struct server_data {
    struct addrinfo hints;//hints for local preferences, servinfo for getting info from gettaddrinfo, p for treveling in list
    struct addrinfo *servinfo;
    struct addrinfo *p;
    struct sockaddr_storage remoteaddr; //information about client's address | can be replaced with sockaddr_in and sockaddr_in6
    int listenfd;   //descriptor for listening new connections
    int acceptfd;   //for connection with clients
    int                     yes;                //set flags (int setsockopt)
    char                    remoteIP[INET6_ADDRSTRLEN];    //!rename to remoteIP * for getting internet address struct as a string - inet_ntop
    int                     rv;                     //addrinfo res 
    socklen_t               sin_size;               //address strunct size for accept
    char                    request[BUFSIZE];     //input message from client
    int                     fdmax;                  //maximum file descriptor number - first arg to select
    int                     nbytes;             //amount of bytes received from client
};


//waits untill the entire message would be sent (because it can be devided into parts)
int sendall(int s, char *buf, int *len);
///*prints history from file (now no, but in the future it will)*/
void display_history(struct message_field *param);
//function that will be executed in separate thread
void *new_thread_func(void *some_data);
//callback-function for send button
void on_button_send_clicked(GtkButton *b, gpointer user_data);
//callback-function for reading user's messages from entry to c-string
void on_user_message_entry_changed(GtkEntry *e, gpointer user_data);
//reads all the widgets we need from xml-file that corresponds to our app
void get_widgets_from_xml(struct ui_widgets *wid);
//returns struct that stores host address in IPv4 or IPv6 format
void *get_in_addr(struct sockaddr *sa);
//adds new message to the message field in UI
//side - 0 or 1, 0 - left (corresponds to the message sent by someone to you) 1 - rigth (your message)
//param - struct that stores all required data to add new label to the UI
void add_new_mess_to_ui(char *message, int side, struct message_field *param);
//if user on another terminel/computer sends message -> server wants to send it through the socket
//this function receives server's message and adds new message as a label to the UI
void receive_from_serv_add_to_ui(struct thread_data *data1, struct client_data *cld);
//adds new message to the message field in UI
//side - 0 or 1, 0 - left (corresponds to the message sent by someone to you) 1 - rigth (your message)
//param - struct that stores all required data to add new label to the UI
void add_new_mess_to_ui(char *message, int side, struct message_field *param);
//if user typed something UI wrote that text to the writing end of pipe
//this function reads message from read end of pipe
//and tries to send it to the server
void send_mess_from_ui_to_server(struct thread_data *data1, struct client_data *cld);
//checks whole list returned on the previous step and searching for correct variant of data
void connect_to_the_server(struct client_data *cld);



/*  *   *   *   *   *   *   *   *   S E R V E R    P A R T  *   *   *   *   *   *   *    */
//takes list returned by func getattrinfo and tries to open socket and bind it with port
void bind_socket_with_port(struct server_data *srv);
//check if socket got bound with port, try to listen ot socket and 
//add listening file descriptor to master set
void listen_socket(struct server_data *srv, fd_set *master);
//accept connection with new client, add accept file descriptor to the master set
void add_new_connection(struct server_data *srv, fd_set *master);
//if socket is closed -> deletes it from master set, else -> prints error that corresponds to receive error
void remove_sock_from_master(struct server_data *srv, int bad_socket, fd_set *master);
//re-sends received message to every chat member except author of the message
void send_mess_to_every_client(struct server_data *srv, fd_set *master, int mess_author);
