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

unordered_map<string, string> udaje;
void vypis() {
    for (auto x : udaje)
        cout << x.first << " " << x.second << endl;
    cout << "--------------------" << endl;
}
void nacitajUdaje(){
    udaje.clear();
    ifstream infile("../udaje.txt");
    string readLine;
    if (!infile.is_open()) { std::exit(EXIT_FAILURE); }
    while (getline(infile, readLine)) {
        string meno;
        string heslo;
        stringstream line(readLine);
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

    const char * meno = strtok(buffer," ");;
    const char * heslo = strtok(NULL,"\n");

    for (auto x : udaje) {
        if (x.first == meno){
            msg = "2";
            existuje = true;
            break;
        }
    }
    if (!existuje){
        ofstream myfile;
        myfile.open("../udaje.txt",ios::app);
        if (myfile.is_open())
        {
            myfile << meno << " " << heslo << "\n";
            myfile.close();
        } else cout << "Unable to open file";
        nacitajUdaje();
    }

    n = write(*newsockfd, msg, strlen(msg)+1);
    if (n < 0){perror("Error writing to socket");return 5;}
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

}

int main(int argc, char *argv[])
{
    nacitajUdaje();
    /*-------------------------------------------*/
    int sockfd, newsockfd, n;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){perror("Error creating socket");return 1;}

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {perror("Error binding socket address");return 2;}

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0){perror("ERROR on accept");return 3;}
    /*-------------------------------------------*/
    char buffer[256];
    bzero(buffer,256);
    n = read(newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 4;}
    if (strcmp(buffer, "a")==0){
        cout << "Prihlasovanie" << endl;
        skontrolujPrihlasenie(&newsockfd);
    } else if(strcmp(buffer, "n")==0){
        cout << "Registracia" << endl;
        zaregistrujPouzivatela(&newsockfd);
    }
    /*--------------------------------------------*/
    bzero(buffer,256);
    n = read(newsockfd, buffer, 255);
    if (strcmp(buffer, "1")==0){
        zrusUcet();
    }
    close(newsockfd);
    close(sockfd);

    return 0;
}
