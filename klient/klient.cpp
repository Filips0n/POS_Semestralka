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
#include <fstream>
#include <limits>
#include <curses.h>
#include <pthread.h>
#include <algorithm>
#include <ctype.h>

#define SIZE 1024
using namespace std;

typedef struct socketUserData{
    char* server;
    int socketNumber;
    int socket;

    std::string name;
    std::string password;
} DATA;

int logIn(int socket, void *data){
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
int registration(int socket, void *data){
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

        if(logIn(socket, user)==0){return 0;}
        return 1;
    } else if(registered == "n"){
        buffer[0] = 'n';
        n = write(socket, buffer, strlen(buffer));
        if (n < 0){perror("Error writing to socket");return 2;}

        if(registration(socket, user)==0){return 0;}
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
        std::cout << "3 - Poslat subor" << std::endl;
        std::cout << "4 - Prijat subor" << std::endl;
        std::cout << "5 - Vytvorit skupinovu konverzaciu" << std::endl;
        std::cout << "6 - Pridat kontakt" << std::endl;
        std::cout << "7 - Odobrat kontakt" << std::endl;
        std::cout << "8 - Zobrazit kontakty" << std::endl;
        std::cout << "9 - Otvorit Chat" << std::endl;
        std::cout << "10 - Otvorit skupinovy chat" << std::endl;
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
            usleep(100);

            string myname = d->name;
            n = write(d->socket, myname.c_str(), strlen(myname.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            usleep(100);

            std::string name;
            char filename[256];
            char filename2[256];
            FILE *f;
            cout << "Komu chces poslat subor? ";
            cin >> name;
            n = write(d->socket, name.c_str(), strlen(name.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            usleep(100);
            cout << "Zadaj nazov suboru: ";
            std::cin >> filename2;
            bzero(filename, 256);
            strcat(filename, "../");
            strcat(filename, filename2);
            write(d->socket, filename2, 255);
            usleep(100);
            int words = 0;
            char c;
            bzero(buffer, 256);
            f = fopen(filename, "r");
            if (f != NULL) {
                while((c = getc(f)) != EOF){
                    fscanf(f, "%s", buffer);
                    if (isspace(c) || c == '\t' || c == '\n')
                        words++;
                }
                words++;
                write(d->socket, &words, sizeof(int));
                usleep(100);
                rewind(f);
                char ch;
                while (ch != EOF){
                    fscanf(f, "%s", buffer);
                    write(d->socket, buffer, 255);
                    usleep(100);
                    ch = fgetc(f);
                }
                cout << "Subor uspesne poslany na server" << endl;
            } else {
                cout << "Takyto subor neexistuje!" << endl;
            }
        } else if(answer == 4){
            bzero(buffer,256);
            buffer[0] = '4';
            n = write(d->socket, buffer, strlen(buffer));
            cout << "Zadaj nazov suboru: ";
            string filename;
            cin >> filename;
            bzero(buffer, 255);
            strcat(buffer, "../server/files/");
            strcat(buffer, filename.c_str());
            write(d->socket, buffer, 255);
            usleep(100);
            bzero(buffer, 255);
            n = read(d->socket, buffer, 255);
            if (n < 0){perror("Error writing to socket");return nullptr;}
            if (strcmp(buffer, "0") == 0) {
                bzero(buffer, 255);
                strcat(buffer, "../klient/files/");
                strcat(buffer, filename.c_str());
                std::ofstream myfile;
                myfile.open(buffer);
                bzero(buffer, 255);
                int words;
                read(d->socket, &words, 255);
                bzero(buffer, 255);
                int ch = 0;
                while(ch != words){
                    read(d->socket, buffer, 255);
                    //fprintf(fp, "%s", buffer);
                    if (myfile.is_open()) {
                        myfile << buffer << " ";
                    } else std::cout << "Unable to open file";
                    ch++;
                }
                myfile.close();
            } else {
                cout << "Nikto ti neposlal taky subor..." << endl;
            }

        } else if(answer == 5){
            bzero(buffer,256);
            buffer[0] = '5';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            std::string name;
            std::cout << "Zadaj nazov skupiny: ";
            std::cin >> name;
            bzero(buffer,256);
            strcat(buffer, (name).c_str());

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            strcat(buffer, " ");
            strcat(buffer, (d->name).c_str());
            std::string message;
            cout << "Pre ukoncenie pridavania pouzivatelov stlate 'q'" << endl;

            while(message != "q"){
                cout  << "Zadajte meno pouzivatela: ";
                getline(cin, message);
                if(message != "q"){
                    strcat(buffer, " ");
                    strcat(buffer, (message).c_str());
                }
            }
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

        } else if(answer == 6){
            bzero(buffer,256);
            buffer[0] = '6';
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

        } else if(answer == 7){
            bzero(buffer,256);
            buffer[0] = '7';
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
        }  else if(answer == 8){
            bzero(buffer,256);
            buffer[0] = '8';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            sleep(1);
            n = write(d->socket, d->name.c_str(), strlen(d->name.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            char oldContacts[SIZE];
            bzero(oldContacts,SIZE);
            n = read(d->socket, oldContacts, SIZE-1);
            if (n < 0){perror("Error reading from socket");return nullptr;}
            std::cout << "Vase kontakty: " << std::endl;
            std::string str(oldContacts);
            const std::string s = "r ";
            const std::string t = "Ziadost od: ";
            int numberOfRequests = 0;
            std::string::size_type n = 0;
            while ( ( n = str.find( s, n ) ) != std::string::npos )
            {
                str.replace( n, s.size(), t );
                n += t.size();
                numberOfRequests++;
            }
            cout << str << endl;

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            char* end = oldContacts + sizeof(oldContacts) / sizeof(oldContacts[0]);
            char* position = std::find(oldContacts, end, 'r');
            if(position != end){
                std::string message;
                cout << "Prijimanie ziadosti ukoncite stlacenim 'q'" << endl;
                while(message != "q"){
                    if(numberOfRequests > 0){
                        cout  << "Zadajte meno a prijatie(a) alebo neprijatie(n) ziadosti o priatelstvo: ";
                        getline(cin, message);
                    } else { sleep(1);}

                    if(message != "\n"){
                        message = (numberOfRequests <= 0) ? "q" : message;
                        bzero(buffer,256);
                        strcat(buffer, (message).c_str());
                        n = write(d->socket, buffer, strlen(buffer));
                        if (n < 0){perror("Error writing to socket");return nullptr;}
                        numberOfRequests--;
                    }
                }
            }
        } else if(answer == 9 || answer == 10){
            bzero(buffer,256);
            if(answer == 9){
                buffer[0] = '9';
            } else if(answer == 10){
                strcat(buffer, "10");
            }
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            std::string name;
            if(answer == 9){
                std::cout << "S kym chces pisat? ";
            } else if(answer==10){
                std::cout << "Zadaj nazov skupiny: ";
            }
            std::cin >> name;
            if(name == d->name){
                std::cout << "Spravy pisane samemu sebe sa neukladaju!!!" << std::endl;
            }
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
