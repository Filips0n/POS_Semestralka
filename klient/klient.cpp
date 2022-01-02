#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <limits>

using namespace std;
typedef struct socketData{
    char* server;
    int socketNumber;
    int socket;
} DATA;

int prihlasenie(void *data){
    DATA *d = (DATA *)data;
    char buffer[256];
    int n;
    printf("Please enter a name and password: ");
    bzero(buffer,256);
    fgets(buffer, 255, stdin);
    n = write(d->socket, buffer, strlen(buffer));
    if (n < 0){perror("Error writing to socket");return 5;}

    bzero(buffer,256);
    n = read(d->socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 6;}

    if(strcmp("1", buffer)==0){
        cout << "Uspesne prihlaseny" << endl;
    } else if(strcmp("2", buffer)==0){
        cout << "Zle zadane heslo" << endl;
    } else {
        cout << "Zadany pouzivatel neexistuje" << endl;
    }
}

int registracia(void *data){
    DATA *d = (DATA *)data;
    char buffer[256];
    int n;
    printf("Please enter a name and password: ");
    bzero(buffer,256);
    fgets(buffer, 255, stdin);

    n = write(d->socket, buffer, strlen(buffer));
    if (n < 0){perror("Error writing to socket");return 5;}

    bzero(buffer,256);
    n = read(d->socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 6;}

    if(strcmp("1", buffer)==0){
        cout << "Uspesne zaregistrovany" << endl;
    } else if(strcmp("2", buffer)==0){
        cout << "Pouzivatel uz je zaregistrovany" << endl;
    }
}

int zaregistrujSpojenie(void *data){
    DATA *d = (DATA *)data;
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    server = gethostbyname(d->server);
    if (server == NULL){fprintf(stderr, "Error, no such host\n");return 2;}

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(d->socketNumber);//socket number

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){perror("Error creating socket");return 3;}

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {perror("Error connecting to socket");return 4;}
    d->socket = sockfd;
}

int main(int argc, char *argv[])
{
    DATA d = {argv[1], atoi(argv[2]), NULL};
    zaregistrujSpojenie(&d);
    string zaregistrovany;
    //do {
        cout << "Zaregistrovany pouzivatel (a/n): ";
        cin >> zaregistrovany;
        //flush buffer
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        int n;
        char buffer[256];
        bzero(buffer,256);
        if (zaregistrovany == "a"){
            buffer[0] = 'a';
            n = write(d.socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return 5;}

            prihlasenie(&d);
        } else if(zaregistrovany == "n"){
            buffer[0] = 'n';
            n = write(d.socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return 5;}

            registracia(&d);
        } else {
            cout << "Zadana nespravna moznost" << endl;
        }
    //} while(zaregistrovany != "n" || zaregistrovany != "a");
    int volba;
    cout << "------------------------------------------------------" << endl;
    cout << "1 - Zrusenie uctu" << endl;
    cin >> volba;
    if (volba == 1) {
        bzero(buffer,256);
        buffer[0] = '1';
        n = write(d.socket, buffer, strlen(buffer));
        if (n < 0){perror("Error writing to socket");return 5;}
    }
    close(d.socket);
    return 0;
}
