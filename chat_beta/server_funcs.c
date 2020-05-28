#include "chat_header.h"

void bind_socket_with_port(struct server_data *srv) {
    for(srv->p = srv->servinfo; srv->p != NULL; srv->p = srv->p->ai_next) {
        //creating socket 
        if((srv->listenfd = socket(srv->p->ai_family, srv->p->ai_socktype, srv->p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        //setting socket options on socket level (SOL_SOCKET): SO_REUSEADDR - bind socket only if no sockets listen on this port
        if((setsockopt(srv->listenfd, SOL_SOCKET, SO_REUSEADDR, &srv->yes, sizeof(int))) == -1) {
            perror("setsockopt");
            exit(1);
        }

        //binding socket with port
        if (bind(srv->listenfd, srv->p->ai_addr, srv->p->ai_addrlen) == -1) { 
            close(srv->listenfd); 
            perror("server: bind"); 
            continue; 
        }

        break;
    }
}

//check if socket got bound with port, try to listen ot socket and 
//add listening file descriptor to master set
void listen_socket(struct server_data *srv, fd_set *master) {
    //if we got here, it means we didn't get bound
    if (srv->p == NULL) { 
        fprintf(stderr, "server: failed to bind\n"); 
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(srv->servinfo);     //always free pointer filled by getaddrinfo

    //listening to socket
    if (listen(srv->listenfd, BACKLOG) == -1) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }

    //if we got here, it means we got connection
    // add the listener to the master set
    FD_SET(srv->listenfd, master);
    // keep track of the biggest file descriptor
    srv->fdmax = srv->listenfd; // so far, it's this one

    printf("server: waiting for connections...\n"); 
}

//accept connection with new client, add accept file descriptor to the master set
void add_new_connection(struct server_data *srv, fd_set *master) {
    srv->sin_size = sizeof(srv->remoteaddr);
    srv->acceptfd = accept(srv->listenfd, (struct sockaddr *)&srv->remoteaddr, &srv->sin_size);
    if (srv->acceptfd == -1)
        perror("accept"); 
    else {
        FD_SET(srv->acceptfd, master);      // add this new connection to master set
        if (srv->acceptfd > srv->fdmax)     // keep track of the max
            srv->fdmax = srv->acceptfd;
        printf("selectserver: new connection from %s on socket %d\n",
        inet_ntop(srv->remoteaddr.ss_family, get_in_addr((struct sockaddr*)&srv->remoteaddr),srv->remoteIP, INET6_ADDRSTRLEN), srv->acceptfd);
    }
}

////if socket is closed -> deletes it from master set, else -> prints error that corresponds to receive error
void remove_sock_from_master(struct server_data *srv, int bad_socket, fd_set *master) {
    if (srv->nbytes == 0) {  // connection closed
        printf("selectserver: socket %d hung up\n", bad_socket);
    }
    else
        perror("recv");
    close(bad_socket);               // closing connection on our side
    FD_CLR(bad_socket, master);     // removing socket from master set
}

//re-sends received message to every chat member except author of the message
void send_mess_to_every_client(struct server_data *srv, fd_set *master, int mess_author) {
    for(int j = 0; j <= srv->fdmax; j++) {
        // send to everyone!
        if (FD_ISSET(j, master)) {
            // except the listener and ourselves
            if (j != srv->listenfd && j != mess_author) {
                if (sendall(j, srv->request, &srv->nbytes) == -1)
                    fprintf(stderr, "sendall error\n");
            }
        }
    }
}
