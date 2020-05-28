#include "chat_header.h"

//inits variables with corresponding values
static void init_server_data(fd_set *master, fd_set *read_fds, struct server_data *srv) {
    srv->yes = 1;
    srv->nbytes = 0;
    FD_ZERO(master);                               //clear the master 
    FD_ZERO(read_fds);                             //and temp sets
    memset(&srv->hints, 0, sizeof(srv->hints));       //cleaning struct before using
    srv->hints.ai_family = AF_UNSPEC;                //both IPv4 and IPv6 families
    srv->hints.ai_socktype = SOCK_STREAM;            //type of socket for TCP
    //hints->ai_flags = AI_PASSIVE;                  //this flag says, that function getaddrinfo schould set my it by itself
                                                    //but i set ip to 0.0.0.0 to allow connecting to my computer for my subnet
}


/* *   *   *    *   *   *   *   *   * SERVER PART  *   *   *   **   *   *   *   *   *     */
int main() {
    struct server_data      srv;
    fd_set                  master;                 //master file descriptor list, required to store all connections
    fd_set                  read_fds;               //temp file descriptor list for select(), copy of master, required because select changes fd-list
    int                     i = 0;                  //to check fds for new connections/data 
    int                     j = 0;                  //to send data to all the members of the chat 
    
    init_server_data(&master, &read_fds, &srv); //inits variables with corresponding values

    //returns struct with correct data (internet addres) for future bind() call
    if((srv.rv = getaddrinfo("0.0.0.0", PORT, &srv.hints, &srv.servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(srv.rv));
        exit(EXIT_FAILURE);
    }

    //takes list returned by func getattrinfo and tries to open socket and bind it with port
    bind_socket_with_port(&srv);

    //check if socket got bound with port, try to listen ot socket and 
    //add listening file descriptor to master set
    listen_socket(&srv, &master);

    //main loop 
    while(1) {  
        read_fds = master;                      // copying master
        //waiting untill any socket is ready to be read
        if (select(srv.fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= srv.fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {       //this socket is ready to be read
                if (i == srv.listenfd) {        //if it is our listen socket -> we only have new connection
                    // handle new connections
                    add_new_connection(&srv, &master);
                }
                else {
                    // handle data from a client
                    srv.nbytes = recv(i, srv.request, sizeof(srv.request), 0);  //receive request from client
                    if (srv.nbytes  <= 0) // got error or connection closed by client
                        remove_sock_from_master(&srv, i, &master);
                    else  // we got some data from a client
                        send_mess_to_every_client(&srv, &master, i);
                }// END handle data from client
            }// END got new incoming connection
        }// END looping through file descriptors
    }//END for main loop

    close(srv.listenfd);
    return 0;
}

