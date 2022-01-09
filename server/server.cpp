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
#include <vector>

#define SIZE 1024

#define fileUsers "../users.txt"
#define fileContacts "../contacts.txt"
#define fileMessages "../messages.txt"
#define fileGroups "../groups.txt"

using namespace std;

typedef struct socketData{
    int socket;
} DATA;

std::unordered_map<std::string, std::string> users;
std::unordered_map<std::string, int> sockets;
std::unordered_map<std::string, std::string> contacts;
std::unordered_map<std::string, std::string> chatWith;
std::unordered_map<std::string, std::vector<std::string>> groups;

void* continueSocket(int &newsockfd){
    int n;
    char buffer[256];
    bzero(buffer,256);
    strcat(buffer, "continue");
    n = write(newsockfd, buffer, strlen(buffer)-1);
    if (n < 0){perror("Error writing to socket");return nullptr;}
    return nullptr;
}

int checkUserInContacts(std::string myName, std::string name){
    for (auto x : contacts){
        if ((x.first == myName && x.second == name) || (x.first == name && x.second == myName))
            return 1;
    }
    return 0;
}
int checkLoggedInUsers(std::string myName){
    for (auto x : sockets){
        if (x.first == myName)
            return 1;
    }
    return 0;
}
int checkUserInGroup(std::string group, std::string name) {
    for (auto x : groups){
        if (x.first == group)
            for (auto y : x.second){
                if (y == name){
                    return 1;
                }
            }
    }
    return 0;
}
void showUsers() {
    std::cout << "MENO HESLO" << std::endl;
    for (auto x : users)
        std::cout << x.first << " " << x.second << std::endl;
    std::cout << "--------------------" << std::endl;
}

void* loadGroupChats(){
    groups.clear();
    std::ifstream infile(fileGroups);
    std::string readLine;
    if (!infile.is_open()) { std::exit(EXIT_FAILURE); }
    while (getline(infile, readLine)) {
        std::string data1;
        std::string data2;
        std::stringstream line(readLine);
        int Num = 0;
        char prev = ' ';

        for(unsigned int i = 0; i < readLine.size(); i++) {
            if(readLine[i] != ' ' && prev == ' ') Num++;
            prev = readLine[i];
        }
        getline(line, data1, ' ');
        for (int i = 0; i < Num-1; ++i) {
            getline(line, data2, ' ');
            groups[data1].push_back(data2);
        }
    }
    infile.close();
    return nullptr;
}
void* saveGroupChat(int &newsockfd){
    char buffer[256];
    char countBuffer[256];
    int n;
    char *point;

    bzero(buffer,256);
    n = read(newsockfd, buffer, 255);
    strcpy(countBuffer, buffer);
    point = strtok (countBuffer," ");
    int wordCount = 0;
    while (point != NULL)
    {
        wordCount++;
        point = strtok (NULL, " ");
    }
    std::string name = std::string((char *)strtok(buffer," "));

    std::ofstream myfile;
    myfile.open(fileGroups,std::ios::app);
    if (myfile.is_open()) {
        myfile << name;
        for (int i = 0; i < wordCount-1; ++i) {
            name = std::string((char *)strtok(NULL," "));
            myfile << " " << name;
        }
    } else std::cout << "Unable to open file";
    myfile << "\n";
    myfile.close();
    loadGroupChats();
    return nullptr;
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
int deleteData(std::string filename, std::unordered_map<std::string, std::string> &pStructure, char* pDeleteLine){
    int deleteOk = 0;
    std::ifstream infile(filename);
    std::ofstream temp;
    temp.open("../temp.txt");
    std::string deleteLine = std::string(pDeleteLine);
    std::string line;
    while(getline(infile, line)){
        if ((line != deleteLine) && (line.find(deleteLine) == std::string::npos)) {
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

    for (auto x : users) {
        if (x.first == name){
            msg = "2";
            existuje = true;
            break;
        }
    }

    if (!existuje){
        sockets[name] = *newsockfd;
        saveData(fileUsers, users, name, password);
    }

    n = write(*newsockfd, msg, strlen(msg)+1);
    if (n < 0){perror("Error writing to socket");return 5;}
    if(msg == "1"){return 1;}
    return 0;
}
int logIn(int *newsockfd){
    int n;
    char buffer[256];
    bzero(buffer,256);
    const char* msg = "3";

    n = read(*newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return 4;}

    const char * token = strtok(buffer," ");
    const char * token2 = strtok(NULL,"\n");
    if(checkLoggedInUsers(token)==1){
        msg = "4";
        n = write(*newsockfd, msg, strlen(msg)+1);
        if (n < 0){perror("Error writing to socket");return 5;}
    }


    auto item = users.find(token);
    if(item != users.end()){
        msg = (strcmp((item->second).c_str(), token2)==0) ? "1" : "2";
        if(msg=="1"){
            sockets[token] = *newsockfd;
        }
    } else msg = "3";

    n = write(*newsockfd, msg, strlen(msg)+1);
    if (n < 0){perror("Error writing to socket");return 5;}
    if(msg == "1"){return 1;}
    return 0;
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

void* showContacts(int &newsockfd){
    char oldContacts[SIZE];
    char requestContacts[SIZE];
    char buffer[256];
    int n;

    bzero(oldContacts,SIZE);
    bzero(requestContacts,SIZE);
    bzero(buffer,256);
    ////Continue
    continueSocket(newsockfd);
    //////////////
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
                std::string myName(buffer);
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
void* showRequests(int &newsockfd) {
    vector<std::string> deleteLines;
    int n;
    char requests[SIZE];
    char buffer[256];
    bzero(buffer,256);
    bzero(requests,SIZE);
    n = read(newsockfd, buffer, 255);
    std::ifstream infile(fileContacts);
    std::string s;
    std::string rd;
    std::string deleteLine;
    int numberOfRequests = 0;
    int numberOfDeletions = 0;
    while(getline(infile, s)){
        std::stringstream stringLine(s);
        getline( stringLine, s, ' ' );
        if(s.compare("d") == 0){deleteLine="d ";};
        if (s.compare("r") == 0 || s.compare("d") == 0){
            rd = s;
            getline( stringLine, s, ' ' );
            if(rd.compare("d") == 0){deleteLine+=s; deleteLine+=" ";};
            if (strcmp(buffer, s.c_str())==0){
                if(rd.compare("r") == 0){
                    numberOfRequests++;
                    strcat(requests, "r ");
                } else {
                    numberOfDeletions++;
                    strcat(requests, "d ");
                }
                getline( stringLine, s, ' ' );
                if(rd.compare("d") == 0){deleteLine+=s; deleteLines.push_back(deleteLine);};
                strcat(requests, s.c_str());
                strcat(requests, "\n");
            }
        }
    }
    infile.close();

    for (auto i = deleteLines.begin(); i != deleteLines.end(); ++i){
        deleteLine = *i;
        deleteData(fileContacts, contacts, const_cast<char*>(deleteLine.c_str()));
    }

    usleep(100);
    if(strlen(requests) == 0) {strcat(requests, "0");}
    n = write(newsockfd, requests, strlen(requests));
    if (n < 0){perror("Error writing to socket");return nullptr;}
    bzero(requests,SIZE);
    ///Wait
    bzero(buffer,256);
    n = read(newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return nullptr;}
    ///////////
    if(numberOfRequests == 0 && numberOfDeletions == 0) {
        strcat(requests, "0");
    } else if(numberOfRequests > 0 && numberOfDeletions > 0){
        strcat(requests, "ReqDel");
    } else if(numberOfRequests > 0 && numberOfDeletions <= 0){
        strcat(requests, "Req");
    } else if(numberOfRequests <= 0 && numberOfDeletions > 0){
        strcat(requests, "Del");
    }
    if(strlen(requests) == 0) {strcat(requests, "0");}
    n = write(newsockfd, requests, strlen(requests));
    if (n < 0){perror("Error writing to socket");return nullptr;}
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
    saveData(fileContacts, contacts, "d " + delName, myName);
    n = write(newsockfd, success.c_str(), strlen(success.c_str()));
    if (n < 0){perror("Error writing to socket");return nullptr;}
    return nullptr;
}

void* chat(int &newsockfd, bool user){
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
    ////////////////////////
    int ok = 1;
    if(user == true){
        ok = checkUserInContacts(sender, reciever);
    } else {
        ok = checkUserInGroup(reciever, sender);
    }
    n = write(newsockfd, &ok, sizeof(int));
    if (n < 0){perror("Error writing to socket");return nullptr;}
    if(ok == 0) {return nullptr;}
    //////////////////////////
    chatWith[sender] = reciever;

    std::ifstream infile(fileMessages);
    std::string s;
    int numberOfMessages = 0;
    while(getline(infile, s)){
        std::stringstream stringLine(s);
        getline( stringLine, s, ' ' );
        if(user == false){
            if (s == reciever){
                getline( stringLine, s, ' ' );
                strcat(oldMessages, s.c_str());
                strcat(oldMessages, ": ");
                getline( stringLine, s);
                strcat(oldMessages, s.c_str());
                strcat(oldMessages, "\n");
                numberOfMessages++;
            }
        } else {
            std::string recieverFromFile = s;
            if (s == reciever || s == sender){
                getline( stringLine, s, ' ' );
                if((s != recieverFromFile) && (s == reciever || s == sender)){
                    strcat(oldMessages, s.c_str());
                    strcat(oldMessages, ": ");
                    getline( stringLine, s);
                    strcat(oldMessages, s.c_str());
                    strcat(oldMessages, "\n");
                    numberOfMessages++;
                }
            }
        }
    }
    infile.close();
    if(numberOfMessages <= 0){
        strcat(oldMessages, "Prazdna historia\n");
    }
    ///Wait
    char buffer[256];
    bzero(buffer,256);
    n = read(newsockfd, buffer, 255);
    if (n < 0){perror("Error reading from socket");return nullptr;}
    ///////////
    n = write(newsockfd, oldMessages, strlen(oldMessages));
    if (n < 0){perror("Error writing to socket");return nullptr;}

    bzero(message,256);
    while(strcmp(message, "q")!=0){
        bzero(message,256);
        n = read(newsockfd, message, 255);
        if(strcmp(message, "q")!=0){
            saveMessage(recieverSender, message);
            char message2[256];
            strcpy(message2, sender.c_str());
            strcat(message2, " ");
            strcat(message2, message);
            if(user == false){
                for (auto y : groups[reciever]) {
                    int recieveSocket = findRecieverSocket(y);
                    if((chatWith[y] == reciever) && (recieveSocket != 0) && (newsockfd != recieveSocket)){
                        n = write(recieveSocket, message2, strlen(message2));
                        if (n < 0){perror("Error writing to socket");return nullptr;}
                    }
                }
            } else {
                int recieveSocket = findRecieverSocket(reciever);
                if((chatWith[reciever] == sender) && (recieveSocket != 0) && (newsockfd != recieveSocket)){
                    n = write(recieveSocket, message2, strlen(message2));
                    if (n < 0){perror("Error writing to socket");return nullptr;}
                }
            }
        } else {
            //ukoncenie cakania na read v recieveMessage
            n = write(newsockfd, message, strlen(message));
            if (n < 0){perror("Error writing to socket");return nullptr;}
        }
    }
    chatWith[sender] = "";
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
        if(logIn(&newsockfd)==0){close(newsockfd);return nullptr;};
    } else if(strcmp(buffer, "n")==0){
        if(registerAcc(&newsockfd)==0){close(newsockfd);return nullptr;};
    }
    /*--------------------------------------------*/
    bool logOut = false;
    bool deleteAcc = false;
    showRequests(newsockfd);
    while(!logOut && !deleteAcc){
        bzero(buffer,256);
        n = read(newsockfd, buffer, 255);
        if (n < 0){perror("Error reading from socket");return nullptr;}

        if(strcmp(buffer, "1")==0){
            ////Continue
            continueSocket(newsockfd);
            //////////////
            deleteAcc = true;
            bzero(buffer,256);
            n = read(newsockfd, buffer, 255);
            if (n < 0){perror("Error reading from socket");return nullptr;}

            deleteData(fileUsers, users, buffer);

            std::string myName = std::string((char *)strtok(buffer," "));
            char deleteLine[256];
            bzero(deleteLine, 256);
            strcat(deleteLine, (myName).c_str());
            deleteData(fileContacts, contacts, deleteLine);
            sockets.erase(myName);

        } else if(strcmp(buffer, "2")==0) {
            ////Continue
            continueSocket(newsockfd);
            //////////////
            logOut = true;
            bzero(buffer, 255);
            n = read(newsockfd, buffer, 255);
            if (n < 0){perror("Error writing to socket");return nullptr;}
            std::string myName = std::string(buffer);
            sockets.erase(myName);

        } else if(strcmp(buffer, "3")==0) {
            ////Continue
            continueSocket(newsockfd);
            //////////////
            bool skupina = false;
            int answer;
            n = read(newsockfd, &answer, 255);
            if (n < 0){perror("Error writing to socket");return nullptr;}
            if(answer == 2)
                skupina = true;
            ////Continue
            continueSocket(newsockfd);
            //////////////
            int ch = 0;
            string sender;
            bzero(buffer, 255);
            n = read(newsockfd, buffer, 255);
            if (n < 0){perror("Error writing to socket");return nullptr;}
            sender = std::string(buffer);
            string reciever;
            ////Continue
            continueSocket(newsockfd);
            //////////////
            bzero(buffer, 255);
            n = read(newsockfd, buffer, 255);
            if (n < 0){perror("Error writing to socket");return nullptr;}
            reciever = std::string(buffer);
            ////////////////////////
            int ok = 1;
            if(skupina) {
                ok = checkUserInGroup(reciever, sender);
            } else {
                ok = checkUserInContacts(sender, reciever);
            }
            n = write(newsockfd, &ok, sizeof(int));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            if(ok == 1) {
            //////////////////////////
                string filename;
                bzero(buffer, 255);
                n = read(newsockfd, buffer, 255);
                if (n < 0){perror("Error writing to socket");return nullptr;}
                ////Continue
                continueSocket(newsockfd);
                //////////////
                filename = std::string(buffer);
                char filename2[256];
                strcat(filename2, "../server/files/");
                strcat(filename2, filename.c_str());
                std::ofstream myfile;
                myfile.open(filename2);
                bzero(buffer, 255);
                int words;
                read(newsockfd, &words, 255);
                ////Continue
                continueSocket(newsockfd);
                //////////////
                bzero(buffer, 255);
                while(ch != words){
                    n = read(newsockfd, buffer, 255);
                    ////Continue
                    continueSocket(newsockfd);
                    //////////////

                    if (n < 0){perror("Error writing to socket");return nullptr;}
                    if (myfile.is_open()) {
                        myfile << buffer << " ";
                    } else std::cout << "Unable to open file";
                    ch++;
                }
                myfile.close();
                //printf("[+]Data written in the file successfully.\n");

                char message[256];
                bzero(message,256);
                strcat(message, "Poslal som subor ");
                strcat(message, filename.c_str());
                char recieverSender[256];
                bzero(recieverSender,256);

                strcat(recieverSender, reciever.c_str());
                strcat(recieverSender, " ");
                strcat(recieverSender, sender.c_str());

                saveMessage(recieverSender, message);
                int recieveSocket = findRecieverSocket(reciever);
                if(recieveSocket != 0) {
                    n = write(recieveSocket, message, strlen(message));
                    if (n < 0) {
                        perror("Error writing to socket");
                        return nullptr;
                    }
                }
            }
        } else if(strcmp(buffer, "4")==0) {
            bzero(buffer,256);
            n = read(newsockfd, buffer, 255);
            if (n < 0){perror("Error writing to socket");return nullptr;}
            int words = 0;
            char c;
            FILE *f;
            f = fopen(buffer, "r");
            bzero(buffer,256);
            if (f == NULL) {
                buffer[0] = '1';
            } else {
                buffer[0] = '0';
            }
            write(newsockfd, buffer, 255);
            while((c = getc(f)) != EOF){
                fscanf(f, "%s", buffer);
                if (isspace(c) || c == '\t' || c == '\n')
                    words++;
            }

            char chr;
            write(newsockfd, &words, sizeof(int));
            usleep(100);
            bzero(buffer, 255);
            rewind(f);
            while (chr != EOF){
                fscanf(f, "%s", buffer);
                write(newsockfd, buffer, 255);
                usleep(100);
                chr = fgetc(f);
            }
            fclose(f);
        } else if(strcmp(buffer, "5")==0) {
            saveGroupChat(newsockfd);
        } else if(strcmp(buffer, "6")==0) {
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

        } else if(strcmp(buffer, "7")==0) {
            deleteContact(newsockfd);
        } else if(strcmp(buffer, "8")==0) {
            showContacts(newsockfd);
        } else if(strcmp(buffer, "9")==0) {
            chat(newsockfd, true);
        } else if(strcmp(buffer, "10")==0) {
            chat(newsockfd, false);
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
    loadData(fileContacts, contacts);
    loadGroupChats();
    //------------------------------------------//
    pthread_t threadsApp[users.size()+3];
    int threadsSize = sizeof threadsApp / sizeof threadsApp[0];
    std::cout << "Maximalny pocet klientov: " << threadsSize << std::endl;
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
