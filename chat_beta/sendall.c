#include "chat_header.h"

/*int the future here would be a complicated functions that sends 
    encrypted messages & files to the server                        */

//waits untill the entire message would be sent (because it can be devided into parts)
int sendall(int s, char *buf, int *len) { 
    int total = 0;          // как много байт мы отправляем 
    int bytesleft = *len;   // как много байт осталось отправить 
    int n; 
    
    while(total < *len) { 
        n = send(s, buf+total, bytesleft, 0); 
        if (n == -1) 
            break;
        total += n; 
        bytesleft -= n; 
    } 
    *len = total; // число фактически отосланных байт 
    
    return n == -1 ? -1 : 0; // вернём -1 при ошибке и 0 при успехе 
}