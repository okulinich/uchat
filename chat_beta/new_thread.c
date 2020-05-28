#include "chat_header.h"

//inits variables with corresponding values
static void init_client_data(fd_set *master, fd_set *read_fds, struct client_data *cld) {
    FD_ZERO(master);                            //clear the master 
    FD_ZERO(read_fds);                          //and temp sets
    memset(&cld->hints, 0, sizeof(cld->hints)); //cleaning struct before using
    cld->hints.ai_family = AF_UNSPEC;           //both ipv4 and ipv6
    cld->hints.ai_socktype = SOCK_STREAM;       //socktype for TCP
    cld->buff_ui_len = MAXDATASIZE;             //length of buf where messages from ui stored
}

//add file descriptors to the master set and print debug info
static void prepare_to_listening(struct thread_data *data1, struct client_data *cld, fd_set *master) {
    //convert network address struct from specific family to string
    inet_ntop(cld->p->ai_family, get_in_addr((struct sockaddr *)cld->p->ai_addr), cld->s, sizeof(cld->s)); 
    printf("client: connecting to %s\n", cld->s);
    freeaddrinfo(cld->servinfo);
    //if we got here, it means we got connection
    // add the listener to the master set
    FD_SET(cld->sockfd, master);
    //add input file descriptor from pipe
    FD_SET(data1->param->fds[0], master);
}



/****************************************WHOLE CLIENT PART IS HERE**************************************/
void *new_thread_func(void *some_data) {
    struct thread_data *data1 = (struct thread_data *)some_data;    //stores data requider for the server
    struct client_data cld;                     //stores all the data required for conection with a server
    fd_set  master;                             //master file descriptor list, required to store all connections
    fd_set  read_fds;                           //temp file descriptor list for select(), copy of master, required because select changes fd-list
    
   
    if (data1->argc != 2) {                     //check if server ip not specified
        fprintf(stderr,"usage: ./client hostname\n");
        exit(EXIT_FAILURE);
    }

    init_client_data(&master, &read_fds, &cld); //inits variables with corresponding values

    //returns list of structs with correct data (internet addres) for future bind() call
    if ((cld.rv = getaddrinfo((*data1->argv)[1], PORT, &cld.hints, &cld.servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(cld.rv)); 
        exit(EXIT_FAILURE); 
    }

    connect_to_the_server(&cld);    //try to create socket and connect to the server using address mentioned by the user

    //if we got here - we didin't get connected
    if (cld.p == NULL) { 
        fprintf(stderr, "client: failed to connect"); 
        close(cld.sockfd);
        exit(EXIT_FAILURE); 
    }

    prepare_to_listening(data1, &cld, &master); //add file descriptors to the master set and print debug info

    //main loop 
    while(1) {  
        read_fds = master;                      // copying master
        //waiting untill any socket is ready to be read
        if (select(cld.sockfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        //check if user typed message in UI (user interface)
        //if he typed -> corresponding function wrote this text to the writing end of pipe
        //and it will be displayed in the reading end of pipe
        if(FD_ISSET(data1->param->fds[0], &read_fds)) {
            //user wants to send message in chat
            send_mess_from_ui_to_server(data1, &cld);
        }
        //check if server wants to send us some messages through the coresponding socket
        if(FD_ISSET(cld.sockfd, &read_fds)) {
            //server want`s to display message from other user
            //reading message from socket
            receive_from_serv_add_to_ui(data1, &cld);
        }
    }

    close(cld.sockfd);              //close socket
    close(data1->param->fds[0]);    //close read end of pipe
    close(data1->param->fds[1]);    //close write end of pipe
    pthread_exit(0);
}


void connect_to_the_server(struct client_data *cld) {
    //checking whole list returned on the previous step and searching for correct variant of data
    for(cld->p = cld->servinfo; cld->p != NULL; cld->p = cld->p->ai_next) { 
        //creating socket
        if ((cld->sockfd = socket(cld->p->ai_family, cld->p->ai_socktype, cld->p->ai_protocol)) == -1) { 
            perror("client: socket"); 
            continue; 
        }
        //connecting to socket
        if (connect(cld->sockfd, cld->p->ai_addr, cld->p->ai_addrlen) == -1) { 
            close(cld->sockfd); 
            perror("client: connect"); 
            continue; 
        } 
        //if created and connected successfully -> break loop and go on
        break; 
    }
}
