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
#include <algorithm>
#define SIZE 1024
using namespace std;

typedef struct socketUserData{
    char* server;
    int socketNumber;
    int socket;

    std::string name;
    std::string password;
} DATA;

int prihlasenie(int socket, void *data){
    DATA *user = (DATA *)data;
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
        std::cout << "Zle zadane heslo" << std::endl;
        return 0;
    } else {
        std::cout << "Zadany pouzivatel neexistuje" << std::endl;
        return 0;
    }
}
int registracia(int socket, void *data){
    DATA *user = (DATA *)data;
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
        std::cout << "Uspesne zaregistrovany" << std::endl;
        return 1;
    } else if(strcmp("2", buffer)==0){
        std::cout << "Pouzivatel uz je zaregistrovany" << std::endl;
        return 0;
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
    DATA *user = (DATA *)data;
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

        if(prihlasenie(socket, user)==0){return 0;}
        return 1;
    } else if(registered == "n"){
        buffer[0] = 'n';
        n = write(socket, buffer, strlen(buffer));
        if (n < 0){perror("Error writing to socket");return 2;}

        if(registracia(socket, user)==0){return 0;}
        return 2;
    } else {
        std::cout << "Zadana nespravna moznost" << std::endl;
        return 0;
    }
    //} while(registered != "n" || registered != "a");
}

void* sendMessage(void *data){
    DATA *d = (DATA *)data;
    char buffer[256];
    int n;
    std::string message;
    while(message != "q"){
        cout << d->name << ": ";
        getline(cin, message);

        bzero(buffer,256);
        strcat(buffer, (message).c_str());
        n = write(d->socket, buffer, strlen(buffer));
        if (n < 0){perror("Error writing to socket");return nullptr;}
    }
    return nullptr;
}

void* recieveMessage(void *data){
    DATA *d = (DATA *)data;
    char message[256];
    int n;
    while(strcmp(message, "q")!=0){
        bzero(message,256);
        //cout << "cakam na spravu" << endl;
        n = read(d->socket, message, 255);
        if (n < 0){perror("Error reading from socket");return nullptr;}
        if (strcmp(message, "q")!=0){
            cout << strtok(message," ") << ": " << strtok(NULL,"\n") << endl;
        }
    }
    return nullptr;
}
void* chatApp(void *data){
    DATA *d = (DATA *)data;
    bool logOut = false;
    bool deleteAcc = false;
    int n;
    char buffer[256];
    if (logInRegisterAcc(d->socket, d) == 0){
        return nullptr;
    }

    while(!logOut && !deleteAcc){
        int answer;
        bzero(buffer,256);
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "1 - Zrusenie uctu" << std::endl;
        std::cout << "2 - Odhlasenie" << std::endl;
        std::cout << "3 - Posat subor" << std::endl;
        std::cout << "4 - Vytvorit skupinovu konverzaciu" << std::endl;
        std::cout << "5 - Pridat kontakt" << std::endl;
        std::cout << "6 - Odobrat kontakt" << std::endl;
        std::cout << "7 - Zobrazit kontakty" << std::endl;
        std::cout << "8 - Otvorit Chat" << std::endl;
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
            std::string userString = d->name + " " + d->password;
            strcat(buffer, (userString).c_str());
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            std::cout << "Ucet uspesne zruseny" << std::endl;
        } else if(answer == 2){
            logOut = true;
            bzero(buffer,256);
            buffer[0] = '2';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            std::cout << "Odhlasenie uspesne" << std::endl;
        } else if(answer == 3){
            bzero(buffer,256);
            buffer[0] = '3';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            std::string name;
            char filename[256];
            char filename2[256];
            FILE *fp;
            cout << "Komu chces poslat subor? ";
            cin >> name;
            cout << endl;
            n = write(d->socket, name.c_str(), strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            cout << "Zadaj nazov suboru: ";
            std::cin >> filename;
            fp = fopen(filename, "r");
            if (fp == NULL) {
                perror("[-]Error in reading file.");
                exit(1);
            }
            char data[SIZE] = {0};

            while(fgets(data, SIZE, fp) != NULL) {
                if (send(d->socket, data, sizeof(data), 0) == -1) {
                    perror("[-]Error in sending file.");
                    exit(1);
                }
                bzero(data, SIZE);
            }
            printf("[+]File data sent successfully.\n");
        } else if(answer == 4){

        } else if(answer == 5){
            bzero(buffer,256);
            buffer[0] = '5';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            std::string name;
            std::cout << "Komu chces poslat ziadost? ";
            std::cin >> name;
            name = d->name + " " + name;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            n = write(d->socket, name.c_str(), strlen(name.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            bzero(buffer,256);
            n = read(d->socket, buffer, 255);
            if (n < 0){perror("Error reading from socket");return nullptr;}
            if(strcmp(buffer,"1")==0){
                cout << "Ziadost o pridanie kontaktu uspesne poslana" << endl;
            } else {
                cout << "Zadany pouzivatel neexistuje" << endl;
            }

        } else if(answer == 6){
            bzero(buffer,256);
            buffer[0] = '6';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            std::string name;
            std::cout << "Koho si ziadas odstranit z kontaktov? ";
            std::cin >> name;
            name = d->name + " " + name;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            n = write(d->socket, name.c_str(), strlen(name.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            bzero(buffer,256);
            n = read(d->socket, buffer, 255);
            if (n < 0){perror("Error reading from socket");return nullptr;}
            if(strcmp(buffer,"1")==0){
                cout << "Odstranenie kontaktu uspesne" << endl;
            } else {
                cout << "Zadaneho pouzivatela nemate v kontaktoch" << endl;
            }
        } else if(answer == 7){
            bzero(buffer,256);
            buffer[0] = '7';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            sleep(1);
            n = write(d->socket, d->name.c_str(), strlen(d->name.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            char contacts[SIZE];
            bzero(contacts,SIZE);
            n = read(d->socket, contacts, SIZE-1);
            if (n < 0){perror("Error reading from socket");return nullptr;}
            std::cout << "Vase kontakty: " << std::endl;
            std::cout << contacts << std::endl;

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            char* end = contacts + sizeof(contacts) / sizeof(contacts[0]);
            char* position = std::find(contacts, end, 'r');
            if(position != end){
                std::string message;
                cout << "Prijimanie ziadosti ukoncite stlacenim 'q'" << endl;
                while(message != "q"){
                    cout  << "Zadajte meno a prijatie(a) alebo neprijatie(n) ziadosti o priatelstvo: ";
                    getline(cin, message);

                    bzero(buffer,256);
                    strcat(buffer, (message).c_str());
                    n = write(d->socket, buffer, strlen(buffer));
                    if (n < 0){perror("Error writing to socket");return nullptr;}
                }
            }
            cin.get();

        } else if(answer == 8){
            bzero(buffer,256);
            buffer[0] = '8';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            sleep(1);

            std::string name;
            std::cout << "S kym chces pisat? ";
            std::cin >> name;
            if(name == d->name){
                std::cout << "Nemozes si pisat sam so sebou" << std::endl;
            } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            bzero(buffer,256);
            strcat(buffer, (name).c_str());
            strcat(buffer, " ");
            strcat(buffer, (d->name).c_str());
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            std::cout << "Pises si s " << name << ", ak chces konverzaciu ukoncit stlac 'q'" << std::endl;
            std::cout << "Vase spravy:" <<  std::endl;

            char messages[1024];
            bzero(messages,1024);
            n = read(d->socket, messages, 1023);
            if (n < 0){perror("Error reading from socket");return nullptr;}
            //Vsetky predchadzajuce spravy
            std::cout << messages << std::endl;

            pthread_t sendThread, recieveThread;
            pthread_create(&sendThread, NULL, &sendMessage, d);
            pthread_create(&recieveThread, NULL, &recieveMessage, d);

            pthread_join(sendThread, NULL);
            pthread_join(recieveThread, NULL);
            }
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return nullptr;
}

int main(int argc, char *argv[])
{
    DATA d = {argv[1], atoi(argv[2]), 0,"", ""};
    connection(&d);
    chatApp(&d);

    close(d.socket);
    return 0;
}
