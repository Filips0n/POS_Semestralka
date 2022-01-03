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
#include <curses.h>

using namespace std;
typedef struct socketData{
    char* server;
    int socketNumber;
    int socket;
} DATA;

typedef struct udaje{
    string meno;
    string heslo;
} UDAJE;

int prihlasenie(void *data, void *data2){
    DATA *d = (DATA *)data;
    UDAJE *udaje = (UDAJE *)data2;
    char buffer[256];
    int n;
    printf("Please enter a name and password: ");
    bzero(buffer,256);
    fgets(buffer, 255, stdin);

    char temp[strlen(buffer)];
    strcpy(temp,buffer);
    udaje->meno = strtok(temp," ");
    udaje->heslo = strtok(NULL,"\n");

    n = write(d->socket, buffer, strlen(buffer));
    if (n < 0){perror("Error writing to socket");return 5;}

    bzero(buffer,256);
    n = read(d->socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 6;}

    if(strcmp("1", buffer)==0){
        std::cout << "Uspesne prihlaseny" << std::endl;
    } else if(strcmp("2", buffer)==0){
        std::cout << "Zle zadane heslo" << std::endl;
    } else {
        std::cout << "Zadany pouzivatel neexistuje" << std::endl;
    }
    return 0;
}

int registracia(void *data, void *data2){
    DATA *d = (DATA *)data;
    UDAJE *udaje = (UDAJE *)data2;
    char buffer[256];
    int n;
    printf("Please enter a name and password: ");
    bzero(buffer,256);
    fgets(buffer, 255, stdin);

    char temp[strlen(buffer)];
    strcpy(temp,buffer);
    udaje->meno = strtok(buffer," ");
    udaje->heslo = strtok(NULL,"\n");

    n = write(d->socket, buffer, strlen(buffer));
    if (n < 0){perror("Error writing to socket");return 5;}

    bzero(buffer,256);
    n = read(d->socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 6;}

    if(strcmp("1", buffer)==0){
        std::cout << "Uspesne zaregistrovany" << std::endl;
    } else if(strcmp("2", buffer)==0){
        std::cout << "Pouzivatel uz je zaregistrovany" << std::endl;
    }
    return 0;
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
    return 0;
}

int main(int argc, char *argv[])
{
    DATA d = {argv[1], atoi(argv[2]), 0};
    UDAJE udaje = {"", ""};
    zaregistrujSpojenie(&d);
    std::string zaregistrovany;
    //do {
        std::cout << "Zaregistrovany pouzivatel (a/n): ";
        std::cin >> zaregistrovany;
        //flush buffer
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        int n;
        char buffer[256];
        bzero(buffer,256);
        if (zaregistrovany == "a"){
            buffer[0] = 'a';
            n = write(d.socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return 5;}

            prihlasenie(&d, &udaje);
        } else if(zaregistrovany == "n"){
            buffer[0] = 'n';
            n = write(d.socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return 5;}

            registracia(&d, &udaje);
        } else {
            std::cout << "Zadana nespravna moznost" << std::endl;
        }
    //} while(zaregistrovany != "n" || zaregistrovany != "a");
//    initscr();
//    curs_set(FALSE);
//    clear();
    int volba;
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "1 - Zrusenie uctu" << std::endl;
    std::cin >> volba;
    if (volba == 1) {
        bzero(buffer,256);
        buffer[0] = '1';
        string udajeString = udaje.meno + " " + udaje.heslo;
        strcat(buffer, " ");
        strcat(buffer, (udajeString).c_str());
        cout << buffer << endl;
        n = write(d.socket, buffer, strlen(buffer));
        if (n < 0){perror("Error writing to socket");return 5;}
    }
    close(d.socket);
    return 0;
}
