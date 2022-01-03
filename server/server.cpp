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
using namespace std;
typedef struct socketData{
    int socketNumber;
    int socket;
    int socket2;
} DATA;
std::unordered_map<std::string, std::string> udaje;
void vypis() {
    for (auto x : udaje)
        std::cout << x.first << " " << x.second << std::endl;
    std::cout << "--------------------" << std::endl;
}
void nacitajUdaje(){
    udaje.clear();
    std::ifstream infile("../udaje.txt");
    std::string readLine;
    if (!infile.is_open()) { std::exit(EXIT_FAILURE); }
    while (getline(infile, readLine)) {
        std::string meno;
        std::string heslo;
        std::stringstream line(readLine);
        getline(line, meno, ' ');
        getline(line, heslo, ' ');
        udaje[meno]=heslo;
    }
    infile.close();
    vypis();
}

int zaregistrujPouzivatela(int *newsockfd){
    int n;
    char buffer[256];
    bzero(buffer,256);
    const char* msg = "1";
    bool existuje = false;
    n = read(*newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 4;}

    const char * meno = strtok(buffer," ");
    const char * heslo = strtok(NULL,"\n");

    for (auto x : udaje) {
        if (x.first == meno){
            msg = "2";
            existuje = true;
            break;
        }
    }
    if (!existuje){
        std::ofstream myfile;
        myfile.open("../udaje.txt",std::ios::app);
        if (myfile.is_open())
        {
            myfile << meno << " " << heslo << "\n";
            myfile.close();
        } else std::cout << "Unable to open file";
        nacitajUdaje();
    }

    n = write(*newsockfd, msg, strlen(msg)+1);
    if (n < 0){perror("Error writing to socket");return 5;}
    return 0;
}

int skontrolujPrihlasenie(int *newsockfd){
    int n;
    char buffer[256];
    bzero(buffer,256);
    const char* msg = "3";
    n = read(*newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 4;}

    const char * token = strtok(buffer," ");;
    const char * token2 = strtok(NULL,"\n");

    auto item = udaje.find(token);
    if(item != udaje.end()){
        if(strcmp((item->second).c_str(), token2)==0){
            msg = "1";
        } else {
            msg = "2";
        }
    } else {
        msg = "3";
    }
    n = write(*newsockfd, msg, strlen(msg)+1);
    if (n < 0){perror("Error writing to socket");return 5;}
    return 0;
}

void zrusUcet(char* pDeleteLine){
    std::ifstream infile("../udaje.txt");
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
    remove("../udaje.txt");
    rename("../temp.txt", "../udaje.txt");
    nacitajUdaje();
}

int zaregistrujSpojenie(void *data){
    DATA *d = (DATA *)data;
    int sockfd, newsockfd, n;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(d->socketNumber);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){perror("Error creating socket");return 1;}

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {perror("Error binding socket address");return 2;}

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0){perror("ERROR on accept");return 3;}
    d->socket = newsockfd;
    d->socket2 = sockfd;
    return 0;
}
int main(int argc, char *argv[])
{
    nacitajUdaje();
    /*-------------------------------------------*/
    DATA d = {atoi(argv[1]), 0,0};
    zaregistrujSpojenie(&d);
    /*-------------------------------------------*/
    char buffer[256];
    bzero(buffer,256);
    int n;
    n = read(d.socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 4;}
    if (strcmp(buffer, "a")==0){
        std::cout << "Prihlasovanie" << std::endl;
        skontrolujPrihlasenie(&d.socket);
    } else if(strcmp(buffer, "n")==0){
        std::cout << "Registracia" << std::endl;
        zaregistrujPouzivatela(&d.socket);
    }
    /*--------------------------------------------*/
    bzero(buffer,256);
    n = read(d.socket, buffer, 255);
    char * volba = strtok(buffer," ");
    char * line = strtok(NULL,"\n");
    if (strcmp(volba, "1")==0){
        zrusUcet(line);
    }
    close(d.socket);
    close(d.socket2);

    return 0;
}
