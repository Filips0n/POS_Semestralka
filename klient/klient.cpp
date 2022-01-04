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
#include <pthread.h>

using namespace std;

typedef struct socketData{
    char* server;
    int socketNumber;
    int socket;
} DATA;
typedef struct userData{
    string name;
    string password;
} USER;

int prihlasenie(int socket, void *data){
    USER *user = (USER *)data;
    char buffer[256];
    int n;

    printf("Zadaj meno a heslo: ");
    bzero(buffer,256);
    fgets(buffer, 255, stdin);

    char temp[strlen(buffer)];
    strcpy(temp,buffer);
    user->name = strtok(temp," ");
    user->password = strtok(NULL,"\n");

    n = write(socket, buffer, strlen(buffer));
    if (n < 0){perror("Error writing to socket");return 5;}

    bzero(buffer,256);
    n = read(socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 6;}

    if(strcmp("1", buffer)==0){
        std::cout << "Uspesne prihlaseny" << std::endl;
        return 1;
    } else if(strcmp("2", buffer)==0){
        std::cout << "Zle zadane password" << std::endl;
        return 2;
    } else {
        std::cout << "Zadany pouzivatel neexistuje" << std::endl;
        return 0;
    }
}

int registracia(int socket, void *data){
    USER *user = (USER *)data;
    char buffer[256];
    int n;

    printf("Zadaj meno a heslo: ");
    bzero(buffer,256);
    fgets(buffer, 255, stdin);

    char temp[strlen(buffer)];
    strcpy(temp,buffer);
    user->name = strtok(temp," ");
    user->password = strtok(NULL,"\n");

    n = write(socket, buffer, strlen(buffer));
    if (n < 0){perror("Error writing to socket");return 5;}

    bzero(buffer,256);
    n = read(socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 6;}

    if(strcmp("1", buffer)==0){
        std::cout << "Uspesne registered" << std::endl;
    } else if(strcmp("2", buffer)==0){
        std::cout << "Pouzivatel uz je registered" << std::endl;
    }
    return 0;
}

int connection(void *data){
    DATA *d = (DATA *)data;
    int sockfd;
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

int logInRegisterAcc(int socket, void *data){
    USER *user = (USER *)data;
    int n;
    char buffer[256];
    std::string registered;
    //do {
    std::cout << "Zaregistrovany pouzivatel (a/n): ";
    std::cin >> registered;
    //flush buffer
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    bzero(buffer,256);
    if (registered == "a"){
        buffer[0] = 'a';
        n = write(socket, buffer, strlen(buffer));
        if (n < 0){perror("Error writing to socket");return 1;}

        prihlasenie(socket, user);
        return 1;
    } else if(registered == "n"){
        buffer[0] = 'n';
        n = write(socket, buffer, strlen(buffer));
        if (n < 0){perror("Error writing to socket");return 2;}

        registracia(socket, user);
        return 2;
    } else {
        std::cout << "Zadana nespravna moznost" << std::endl;
        return 0;
    }
    //} while(registered != "n" || registered != "a");
}
void* chatApp(void *data, void *data2){
    DATA *d = (DATA *)data;
    USER *user = (USER *)data2;
    bool logOut = false;
    bool deleteAcc = false;
    int n;
    char buffer[256];
    if (logInRegisterAcc(d->socket, user) == 0){
        return nullptr;
    }

    while(!logOut && !deleteAcc){
        int answer;
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "1 - Zrusenie uctu" << std::endl;
        std::cout << "2 - Odhlasenie" << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        std::cin >> answer;
        if (answer == 1) {
            deleteAcc = true;
            bzero(buffer,256);
            buffer[0] = '1';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            sleep(1);
            bzero(buffer,256);
            string userString = user->name + " " + user->password;
            strcat(buffer, (userString).c_str());
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            cout << "Ucet uspesne zruseny" << endl;
        } else if(answer == 2){
            logOut = true;
            bzero(buffer,256);
            buffer[0] = '2';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            cout << "Odhlasenie uspesne" << endl;
        }
    }
}
int main(int argc, char *argv[])
{
    DATA d = {argv[1], atoi(argv[2]), 0};
    USER user = {"", ""};
    connection(&d);
    chatApp(&d, &user);

    close(d.socket);
    return 0;
}
