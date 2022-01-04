#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <pthread.h>

using namespace std;
int aktualnyPocet = 0;
typedef struct socketData{
    int socketNumber;
    int socket;
} DATA;

std::unordered_map<std::string, std::string> users;
void vypis() {
    for (auto x : users)
        std::cout << x.first << " " << x.second << std::endl;
    std::cout << "--------------------" << std::endl;
}
void loadUsers(){
    users.clear();
    std::ifstream infile("../users.txt");
    std::string readLine;
    if (!infile.is_open()) { std::exit(EXIT_FAILURE); }
    while (getline(infile, readLine)) {
        std::string meno;
        std::string heslo;
        std::stringstream line(readLine);
        getline(line, meno, ' ');
        getline(line, heslo, ' ');
        users[meno]=heslo;
    }
    infile.close();
    vypis();
}

int registerAcc(int *newsockfd){
    int n;
    char buffer[256];
    bzero(buffer,256);
    const char* msg = "1";
    bool existuje = false;

    n = read(*newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 4;}

    const char * meno = strtok(buffer," ");
    const char * heslo = strtok(NULL,"\n");

    for (auto x : users) {
        if (x.first == meno){
            msg = "2";
            existuje = true;
            break;
        }
    }

    if (!existuje){
        std::ofstream myfile;
        myfile.open("../users.txt",std::ios::app);
        if (myfile.is_open()) {
            myfile << meno << " " << heslo << "\n";
            myfile.close();
        } else std::cout << "Unable to open file";
        loadUsers();
    }

    n = write(*newsockfd, msg, strlen(msg)+1);
    if (n < 0){perror("Error writing to socket");return 5;}
    return 0;
}

int logIn(int *newsockfd){
    int n;
    char buffer[256];
    bzero(buffer,256);
    const char* msg = "3";

    n = read(*newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 4;}

    const char * token = strtok(buffer," ");;
    const char * token2 = strtok(NULL,"\n");

    auto item = users.find(token);
    if(item != users.end()){
        msg = (strcmp((item->second).c_str(), token2)==0) ? "1" : "2";
    } else msg = "3";

    n = write(*newsockfd, msg, strlen(msg)+1);
    if (n < 0){perror("Error writing to socket");return 5;}
    return 0;
}

void zrusUcet(char* pDeleteLine){
    std::ifstream infile("../users.txt");
    std::ofstream temp;
    temp.open("../temp.txt");
    std::string deleteLine = std::string(pDeleteLine);
    string line;
    while(getline(infile, line)){
        if (line != deleteLine)
            temp << line << std::endl;
    }
    infile.close();
    temp.close();
    remove("../users.txt");
    rename("../temp.txt", "../users.txt");
    loadUsers();
}

void* chatApp(void *data){
    DATA *d = (DATA *)data;
    int sockfd, newsockfd, n;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(d->socketNumber);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){perror("Error creating socket");return nullptr;}

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {perror("Error binding socket address");return nullptr;}

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0){perror("ERROR on accept");return nullptr;}
    d->socket = newsockfd;
    std::cout << "Uspesne pripojeny socket na porte: " << d->socketNumber << std::endl;
    //------------------------------------------------------------------------------//

    char buffer[256];
    bzero(buffer,256);
    n = read(d->socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return nullptr;}

    if (strcmp(buffer, "a")==0){
        logIn(&d->socket);
    } else if(strcmp(buffer, "n")==0){
        registerAcc(&d->socket);
    }
    cout << "Koniec prihlasovania" << d->socketNumber << endl;
    /*--------------------------------------------*/
    bool logOut = false;
    bool deleteAcc = false;
    while(!logOut && !deleteAcc){
        bzero(buffer,256);
        n = read(d->socket, buffer, 255);

        if(strcmp(buffer, "1")==0){
            deleteAcc = true;
            bzero(buffer,256);
            n = read(d->socket, buffer, 255);
            zrusUcet(buffer);
        } else if(strcmp(buffer, "2")==0) {
            logOut = true;
        }
    }


    //----------------------------------------------------------------------//
    close(d->socket);
}

int main(int argc, char *argv[])
{
    pthread_t threadsApp[users.size()+1];
    int threadsSize = sizeof threadsApp / sizeof threadsApp[0] + 1;
    loadUsers();
    /*-------------------------------------------*/
    struct socketData ports[threadsSize];
    for (int i = 0; i < threadsSize; ++i) {
        ports[i].socketNumber = atoi(argv[1]) + i;
        ports[i].socket = 0;
    }
    for (int i = 0; i < threadsSize; ++i) {
        pthread_create(&threadsApp[i], NULL, &chatApp, &ports[i]);
    }

    for (int i = 0; i < threadsSize; ++i) {
        pthread_join(threadsApp[i], NULL);
    }
    return 0;
}
