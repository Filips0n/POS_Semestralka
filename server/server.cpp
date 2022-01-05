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
#define SIZE 1024
using namespace std;

typedef struct socketData{
    int socket;
} DATA;

std::unordered_map<std::string, std::string> users;
std::unordered_map<std::string, int> sockets;
std::unordered_map<std::string, std::string> contacts;

void saveMessage(char recieverSender[256], char message[256]);

void* loadContacts(){
    contacts.clear();
    std::ifstream infile("../contacts.txt");
    std::string readLine;
    if (!infile.is_open()) { std::exit(EXIT_FAILURE); }
    while (getline(infile, readLine)) {
        std::string name;
        std::string contact;
        std::stringstream line(readLine);
        getline(line, name, ' ');
        getline(line, contact, ' ');
        users[name]=contact;
    }
    infile.close();
}
void* saveContact(std::string name, std::string contact){
    std::ofstream myfile;
    myfile.open("../contacts.txt",std::ios::app);
    if (myfile.is_open()) {
        myfile << name << " " << contact << "\n";
        myfile.close();
    } else std::cout << "Unable to open file";
    loadContacts();
}

void showUsers() {
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
    showUsers();
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
    sockets[meno] = *newsockfd;
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
    sockets[token] = *newsockfd;
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
int findRecieverSocket(std::string name){
    int socket = 0;
    for (auto x : sockets){
        if (x.first == name){
            socket = x.second;
            break;
        }
    }
    return socket;
}
void* chatApp(void *data){
    DATA *d = (DATA *)data;
    int newsockfd, n;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    cli_len = sizeof(cli_addr);

    newsockfd = accept(d->socket, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0){perror("ERROR on accept");return nullptr;}
    //------------------------------------------------------------------------------//

    char buffer[256];
    bzero(buffer,256);
    n = read(newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return nullptr;}

    if (strcmp(buffer, "a")==0){
        logIn(&newsockfd);
    } else if(strcmp(buffer, "n")==0){
        registerAcc(&newsockfd);
    }
    /*--------------------------------------------*/
    bool logOut = false;
    bool deleteAcc = false;
    while(!logOut && !deleteAcc){
        bzero(buffer,256);
        n = read(newsockfd, buffer, 255);

        if(strcmp(buffer, "1")==0){
            deleteAcc = true;
            bzero(buffer,256);
            n = read(newsockfd, buffer, 255);
            zrusUcet(buffer);
        } else if(strcmp(buffer, "2")==0) {
            logOut = true;
        } else if(strcmp(buffer, "3")==0) {
            FILE *fp;
            char *filename = "../recv.txt";
            char buffer[SIZE];

            bzero(buffer,256);
            n = read(newsockfd, buffer, 255);

            fp = fopen(filename, "w");
            while (1) {
                n = recv(findRecieverSocket(buffer), buffer, SIZE, 0);
                if (n <= 0){break;}
                fprintf(fp, "%s", buffer);
                bzero(buffer, SIZE);
            }
            printf("[+]Data written in the file successfully.\n");
        } else if(strcmp(buffer, "4")==0) {

        } else if(strcmp(buffer, "5")==0) {
            std::string success = "0";

            bzero(buffer,256);
            n = read(newsockfd, buffer, 255);

            std::string sender = std::string((char *)strtok(buffer," "));
            std::string reciever = std::string((char *)strtok(NULL,"\n"));

            for (auto x: users) {
                if (reciever == x.first){
                    success = "1";
                    saveContact("r " + reciever, sender);
                    break;
                }
            }
            n = write(newsockfd, success.c_str(), strlen(success.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}
        } else if(strcmp(buffer, "7")==0) {
            char contacts[SIZE];
            char requestContacts[SIZE];
            bzero(contacts,SIZE);
            bzero(requestContacts,SIZE);
            bzero(buffer,256);

            n = read(newsockfd, buffer, 255);

            std::ifstream infile("../contacts.txt");
            std::string s;
            int numberOfContacts = 0;
            while(getline(infile, s)){
                std::stringstream stringLine(s);
                getline( stringLine, s, ' ' );
                if (s.compare("r") != 0){
                    if (strcmp(buffer, s.c_str())==0){
                        getline( stringLine, s, ' ' );
                        strcat(contacts, s.c_str());
                        strcat(contacts, "\n");
                        numberOfContacts++;
                    }
                } else {
                    strcat(requestContacts, s.c_str());
                    strcat(requestContacts, " ");
                    getline( stringLine, s, ' ' );
                    if (strcmp(buffer, s.c_str())==0){
                        getline( stringLine, s, ' ' );
                        strcat(requestContacts, s.c_str());
                        strcat(requestContacts, "\n");
                        numberOfContacts++;
                    }
                }
            }
            infile.close();
            strcat(contacts, requestContacts);

            if(numberOfContacts <= 0){
                strcat(contacts, "Prazdna historia\n");
            }

            n = write(newsockfd, contacts, strlen(contacts));
            if (n < 0){perror("Error writing to socket");return nullptr;}
        }  else if(strcmp(buffer, "8")==0) {
            char recieverSender[256];
            char recieverSender2[256];
            char message[256];
            char oldMessages[1024];

            bzero(oldMessages,1024);
            bzero(recieverSender,256);

            n = read(newsockfd, recieverSender, 255);
            if (n < 0){perror("Error writing to socket");return nullptr;}

            strcpy(recieverSender2, recieverSender);
            std::string reciever = std::string((char *)strtok(recieverSender2," "));
            std::string sender = std::string((char *)strtok(NULL,"\n"));

            std::ifstream infile("../messages.txt");
            std::string s;
            int numberOfMessages = 0;
            while(getline(infile, s)){
                std::stringstream stringLine(s);
                getline( stringLine, s, ' ' );
                if (s == reciever || s == sender){
                    getline( stringLine, s, ' ' );
                    if(s == reciever || s == sender){
                        strcat(oldMessages, s.c_str());
                        strcat(oldMessages, ": ");
                        getline( stringLine, s);
                        strcat(oldMessages, s.c_str());
                        strcat(oldMessages, "\n");
                        numberOfMessages++;
                    }
                }
            }
            infile.close();
            if(numberOfMessages <= 0){
                strcat(oldMessages, "Prazdna historia\n");
            }

            n = write(newsockfd, oldMessages, strlen(oldMessages));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            while(strcmp(message, "q")!=0){
                bzero(message,256);
                n = read(newsockfd, message, 255);
                if(strcmp(message, "q")!=0){
                    saveMessage(recieverSender, message);
                } else {
                    n = write(newsockfd, message, strlen(message));
                    if (n < 0){perror("Error writing to socket");return nullptr;}
                }
                char message2[256];
                strcpy(message2, sender.c_str());
                strcat(message2, " ");
                strcat(message2, message);
                int recieveSocket = findRecieverSocket(reciever);
                if(recieveSocket != 0){
                    n = write(recieveSocket, message2, strlen(message2));
                    if (n < 0){perror("Error writing to socket");return nullptr;}
                }
            }
        }
    }
    //----------------------------------------------------------------------//
    close(newsockfd);
}

void saveMessage(char recieverSender[256], char message[256]) {
    std::string recievSend(recieverSender);
    std::string mes(message);
    std::ofstream myfile;
    myfile.open("../messages.txt",std::ios::app);
    if (myfile.is_open()) {
        myfile << recievSend << " " << mes << "\n";
        myfile.close();
    } else std::cout << "Unable to open file";
}

int connection(void *data, int server){
    DATA *d = (DATA *)data;
    int sockfd, newsockfd, n;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(server);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){perror("Error creating socket");return 22;}

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {perror("Error binding socket address");return 23;}

    listen(sockfd, 5);

    d->socket = sockfd;
}

int main(int argc, char *argv[])
{
    loadUsers();
    //------------------------------------------//
    pthread_t threadsApp[users.size()+1];
    int threadsSize = sizeof threadsApp / sizeof threadsApp[0];
    cout << threadsSize << endl;
    /*-------------------------------------------*/
    struct socketData port;
    connection(&port, atoi(argv[1]));

    for (int i = 0; i < threadsSize; ++i) {
        pthread_create(&threadsApp[i], NULL, &chatApp, &port);
    }

    for (int i = 0; i < threadsSize; ++i) {
        pthread_join(threadsApp[i], NULL);
    }
    return 0;
}
