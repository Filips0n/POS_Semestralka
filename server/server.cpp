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
#define fileUsers "../users.txt"
#define fileContacts "../contacts.txt"
#define fileMessages "../messages.txt"
using namespace std;

typedef struct socketData{
    int socket;
} DATA;

std::unordered_map<std::string, std::string> users;
std::unordered_map<std::string, int> sockets;
std::unordered_map<std::string, std::string> contacts;

void showUsers() {
    std::cout << "MENO HESLO" << std::endl;
    for (auto x : users)
        std::cout << x.first << " " << x.second << std::endl;
    std::cout << "--------------------" << std::endl;
}
void* loadData(std::string filename, std::unordered_map<std::string, std::string> &pStructure) {
    pStructure.clear();
    std::ifstream infile(filename);
    std::string readLine;
    if (!infile.is_open()) { std::exit(EXIT_FAILURE); }
    while (getline(infile, readLine)) {
        std::string data1;
        std::string data2;
        std::stringstream line(readLine);
        getline(line, data1, ' ');
        getline(line, data2, ' ');
        pStructure[data1]=data2;
    }
    infile.close();
    if(pStructure == users){
        showUsers();
    }
    return nullptr;
}

void* saveData(std::string filename, std::unordered_map<std::string, std::string> &pStructure, std::string data1, std::string data2){
    std::ofstream myfile;
    myfile.open(filename,std::ios::app);
    if (myfile.is_open()) {
        myfile << data1 << " " << data2 << "\n";
        myfile.close();
    } else std::cout << "Unable to open file";
    loadData(filename, pStructure);
    return nullptr;
}
void saveMessage(char recieverSender[256], char message[256]) {
    std::string recievSend(recieverSender);
    std::string mes(message);
    std::ofstream myfile;
    myfile.open(fileMessages,std::ios::app);
    if (myfile.is_open()) {
        myfile << recievSend << " " << mes << "\n";
        myfile.close();
    } else std::cout << "Unable to open file";
}

int registerAcc(int *newsockfd){
    int n;
    char buffer[256];
    bzero(buffer,256);
    const char* msg = "1";
    bool existuje = false;

    n = read(*newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 4;}

    const char * name = strtok(buffer," ");
    const char * password = strtok(NULL,"\n");
    sockets[name] = *newsockfd;
    for (auto x : users) {
        if (x.first == name){
            msg = "2";
            existuje = true;
            break;
        }
    }

    if (!existuje){
        saveData(fileUsers, users, name, password);
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

int deleteData(std::string filename, std::unordered_map<std::string, std::string> &pStructure, char* pDeleteLine){
    int deleteOk = 0;
    std::ifstream infile(filename);
    std::ofstream temp;
    temp.open("../temp.txt");
    std::string deleteLine = std::string(pDeleteLine);
    string line;
    while(getline(infile, line)){
        if (line != deleteLine) {
            temp << line << std::endl;
        } else {
            deleteOk = 1;
        }

    }
    infile.close();
    temp.close();
    remove(filename.c_str());
    rename("../temp.txt", filename.c_str());
    loadData(filename, pStructure);
    return deleteOk;
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

void* chatWithUser(int &newsockfd){
    char recieverSender[256];
    char recieverSender2[256];
    char message[256];
    char oldMessages[1024];
    int n;

    bzero(oldMessages,1024);
    bzero(recieverSender,256);

    n = read(newsockfd, recieverSender, 255);
    if (n < 0){perror("Error writing to socket");return nullptr;}

    strcpy(recieverSender2, recieverSender);
    std::string reciever = std::string((char *)strtok(recieverSender2," "));
    std::string sender = std::string((char *)strtok(NULL,"\n"));

    std::ifstream infile(fileMessages);
    std::string s;
    int numberOfMessages = 0;
    while(getline(infile, s)){
        std::stringstream stringLine(s);
        getline( stringLine, s, ' ' );
        std::string fileReciever = s;
        if (s == reciever || s == sender){
            getline( stringLine, s, ' ' );
            if((s != fileReciever) && (s == reciever || s == sender)){
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
    return nullptr;
}

void* showContacts(int &newsockfd){
    char oldContacts[SIZE];
    char requestContacts[SIZE];
    char buffer[256];
    int n;

    bzero(oldContacts,SIZE);
    bzero(requestContacts,SIZE);
    bzero(buffer,256);

    n = read(newsockfd, buffer, 255);
    std::ifstream infile(fileContacts);
    std::string s;
    int numberOfContacts = 0;
    bool requests = false;
    while(getline(infile, s)){
        std::stringstream stringLine(s);
        getline( stringLine, s, ' ' );
        if (s.compare("r") != 0){
            if (strcmp(buffer, s.c_str())==0){
                getline( stringLine, s, ' ' );
                strcat(oldContacts, s.c_str());
                strcat(oldContacts, "\n");
                numberOfContacts++;
            } else {
                std::string name = s;
                getline( stringLine, s, ' ' );
                if (strcmp(buffer, s.c_str())==0){
                    strcat(oldContacts, name.c_str());
                    strcat(oldContacts, "\n");
                    numberOfContacts++;
                }
            }
        } else {
            getline( stringLine, s, ' ' );
            if (strcmp(buffer, s.c_str())==0){
                strcat(requestContacts, "r ");
                getline( stringLine, s, ' ' );
                strcat(requestContacts, s.c_str());
                strcat(requestContacts, "\n");
                numberOfContacts++;
                requests = true;
            }
        }
    }
    infile.close();
    strcat(oldContacts, requestContacts);

    if(numberOfContacts <= 0){
        strcat(oldContacts, "Ziadne kontakty\n");
    }

    n = write(newsockfd, oldContacts, strlen(oldContacts));
    if (n < 0){perror("Error writing to socket");return nullptr;}

    if(requests == true){
        char message[256];
        while(strcmp(message, "q")!=0){
            bzero(message,256);
            n = read(newsockfd, message, 255);
            if((strcmp(message, "q")!=0)){
                std::string name = std::string((char *)strtok(message," "));
                std::string accept = std::string((char *)strtok(NULL,"\n"));
                string myName(buffer);
                char deleteLine[256];
                //odstranim kontakt, ak je a ako ano, znovu pridam kontakt uz bez 'r'
                bzero(deleteLine, 256);
                strcat(deleteLine, ("r " + myName + " " + name).c_str());

                deleteData(fileContacts, contacts, deleteLine);
                if(accept.compare("a")==0){
                    saveData(fileContacts, contacts, myName, name);
                }
            }
        }
    }
    return nullptr;
}

void* deleteContact(int &newsockfd){
    std::string success = "0";
    char message[256];
    int n;
    bzero(message,256);
    n = read(newsockfd, message, 255);

    std::string myName = std::string((char *)strtok(message," "));
    std::string delName = std::string((char *)strtok(NULL,"\n"));

    char deleteLine[256];
    bzero(deleteLine, 256);
    strcat(deleteLine, (myName + " " + delName).c_str());
    success = std::to_string(deleteData(fileContacts, contacts, deleteLine));
    if(success == "0"){
        bzero(deleteLine, 256);
        strcat(deleteLine, (delName + " " + myName).c_str());
        success = std::to_string(deleteData(fileContacts, contacts, deleteLine));
    }

    n = write(newsockfd, success.c_str(), strlen(success.c_str()));
    if (n < 0){perror("Error writing to socket");return nullptr;}
    return nullptr;
}

void* chatApp(void *data){
    DATA *d = (DATA *)data;
    int newsockfd, n;
    char buffer[256];

    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    cli_len = sizeof(cli_addr);

    newsockfd = accept(d->socket, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0){perror("ERROR on accept");return nullptr;}
    //------------------------------------------------------------------------------//
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
        if (n < 0){perror("Error reading from socket");return nullptr;}

        if(strcmp(buffer, "1")==0){
            deleteAcc = true;
            bzero(buffer,256);
            n = read(newsockfd, buffer, 255);
            if (n < 0){perror("Error reading from socket");return nullptr;}

            deleteData(fileUsers, users, buffer);
        } else if(strcmp(buffer, "2")==0) {
            logOut = true;
        } else if(strcmp(buffer, "3")==0) {
            FILE *fp;
            char *filename = (char*)"../recv.txt";
            char buffer[SIZE];

            bzero(buffer,256);
            n = read(newsockfd, buffer, 255);
            if (n < 0){perror("Error reading from socket");return nullptr;}

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
                    saveData(fileContacts, contacts, "r " + reciever, sender);
                    break;
                }
            }
            n = write(newsockfd, success.c_str(), strlen(success.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}

        } else if(strcmp(buffer, "6")==0) {
            deleteContact(newsockfd);
        } else if(strcmp(buffer, "7")==0) {
            showContacts(newsockfd);
        }  else if(strcmp(buffer, "8")==0) {
            chatWithUser(newsockfd);
        }
    }
    //----------------------------------------------------------------------//
    close(newsockfd);
    return nullptr;
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
    return 1;
}

int main(int argc, char *argv[])
{
    loadData(fileUsers, users);
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
