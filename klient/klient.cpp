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

void* waitSocket(int socket) {
    int n;
    char buffer[256];
    bzero(buffer,256);
    n = read(socket, buffer, 255);
    if (n < 0){perror("Error reading from socket");return nullptr;}
    return nullptr;
}

int logIn(int socket, void *data){
    DATA *user = (DATA *)data;
    char buffer[256];
    int n;

    do {
        printf("Zadaj meno a heslo: ");
        bzero(buffer,256);
        fgets(buffer, 255, stdin);

        char temp[strlen(buffer)];
        strcpy(temp,buffer);
        if(strchr(temp, ' '))
        {
            user->name = strtok(temp," ");
            user->password = strtok(NULL,"\n");
            if(user->name.length() < 3 || user->password.length() < 3){
                std::cout << "Zle zadane udaje! Meno aj heslo musia byt dlhsie ako 2 znaky" << std::endl;
            }
        } else {
            std::cout << "Zle zadane udaje! Zadajte v tvare: 'meno' 'heslo'" << std::endl;
        }
    } while(user->name.empty() || user->password.empty() || user->name.length() < 3 || user->password.length() < 3);

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
    } else if(strcmp("3", buffer)==0){
        std::cout << "Zadany pouzivatel neexistuje" << std::endl;
        return 0;
    } else if(strcmp("4", buffer)==0){
        std::cout << "Pouzivatel je uz prihlaseny" << std::endl;
        return 0;
    }
    return 0;
}
int registration(int socket, void *data){
    DATA *user = (DATA *)data;
    char buffer[256];
    int n;

    do {
        printf("Zadaj meno a heslo: ");
        bzero(buffer,256);
        fgets(buffer, 255, stdin);

        char temp[strlen(buffer)];
        strcpy(temp,buffer);
        if(strchr(temp, ' '))
        {
            user->name = strtok(temp," ");
            user->password = strtok(NULL,"\n");
            if(user->name.length() < 3 || user->password.length() < 3){
                std::cout << "Zle zadane udaje! Meno aj heslo musia byt dlhsie ako 2 znaky" << std::endl;
            }
        } else {
            std::cout << "Zle zadane udaje! Zadajte v tvare: 'meno' 'heslo'" << std::endl;
        }
    } while(user->name.empty() || user->password.empty() || user->name.length() < 3 || user->password.length() < 3);

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
}

void* sendMessage(void *data){
    DATA *d = (DATA *)data;
    char buffer[256];
    int n;
    std::string message;
    while(message != "q"){
        std::cout << d->name << ": ";
        getline(cin, message);
        if(message == "x") {
            cout << "Pises sifrovanu spravu: ";
            getline(cin, message);
            for(char& c : message) {
                c += 3;
            }
        }
        bzero(buffer,256);
        strcat(buffer, (message).c_str());
        n = write(d->socket, buffer, strlen(buffer));
        if (n < 0){perror("Error writing to socket");return nullptr;}
    }
    return nullptr;
}
void* receiveMessage(void *data){
    DATA *d = (DATA *)data;
    char message[256];
    int n;
    message[0] = '0';
    while(strcmp(message, "q")!=0){
        bzero(message,256);
        n = read(d->socket, message, 255);
        if (n < 0){perror("Error reading from socket");return nullptr;}
        if (strcmp(message, "q")!=0){
            std::cout << strtok(message," ") << ": " << strtok(NULL,"\n") << std::endl;
        }
    }
    return nullptr;
}
void* showRequests(void *data) {
    DATA *d = (DATA *)data;
    char buffer[256];
    int n;
    bzero(buffer,256);
    strcat(buffer, d->name.c_str());
    n = write(d->socket, buffer, 255);
    if (n < 0){perror("Error writing to socket");return nullptr;}

    char requests2[SIZE];
    char requests[SIZE];
    bzero(requests2,SIZE);
    n = read(d->socket, requests2, SIZE-1);
    if (n < 0){perror("Error reading from socket");return nullptr;}

    strcpy(requests, requests2);
    ///continue
    bzero(buffer,256);
    strcat(buffer, "continue");
    n = write(d->socket, buffer, strlen(buffer)-1);
    if (n < 0){perror("Error writing to socket");return nullptr;}
    bzero(buffer,256);
    ////////////
    bzero(requests2,SIZE);
    n = read(d->socket, requests2, SIZE-1);

    if (n < 0){perror("Error reading from socket");return nullptr;}
    if (strcmp(requests2, "ReqDel") == 0) {
        const std::string t = "Ziadost o kontakt od ";
        const std::string f = "Z kontaktov si ta odstanil ";
        const std::string s = "r ";
        const std::string d = "d ";
        std::string req(requests);
        std::string req2(requests);
        std::string::size_type n = 0;
        while ( ( n = req.find( d, n ) ) != std::string::npos )
        {
            req.replace( n, d.size(), f );
            n += f.size();
        }
        n = 0;
        while ( ( n = req.find( s, n ) ) != std::string::npos )
        {
            req.replace( n, s.size(), t );
            n += t.size();
        }
        std::cout << req << std::endl;
        std::cout << "Ak chces prijat/odmietnut ziadosti tak zvol moznost 8 - Zobrazit kontakty..." << std::endl;
    } else if(strcmp(requests2, "Req") == 0){
        const std::string t = "Ziadost o kontakt od ";
        const std::string s = "r ";
        std::string req(requests);
        std::string::size_type n = 0;
        while ( ( n = req.find( s, n ) ) != std::string::npos )
        {
            req.replace( n, s.size(), t );
            n += t.size();
        }
        std::cout << req << std::endl;
        std::cout << "Ak chces prijat/odmietnut ziadosti tak zvol moznost 8 - Zobrazit kontakty..." << std::endl;
    } else if(strcmp(requests2, "Del") == 0){
        const std::string f = "Z kontaktov si ta odstanil ";
        const std::string d = "d ";
        std::string req(requests);
        std::string::size_type n = 0;
        while ( ( n = req.find( d, n ) ) != std::string::npos )
        {
            req.replace( n, d.size(), f );
            n += f.size();
        }
        std::cout << req << std::endl;
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
    showRequests(d);
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
            ///Wait
            waitSocket(d->socket);
            //////
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
            ///Wait
            waitSocket(d->socket);
            //////
            bzero(buffer,256);
            std::string userString = d->name;
            strcat(buffer, (userString).c_str());
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            std::cout << "Odhlasenie uspesne" << std::endl;
        } else if(answer == 3){
            bzero(buffer,256);
            buffer[0] = '3';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            ///Wait
            waitSocket(d->socket);
            //////
            cout << "1 - Poslat subor jednemu pouzivatelovi" << endl;
            cout << "2 - Poslat subor skupine" << endl;
            int answer;
            std::cin >> answer;
            n = write(d->socket, &answer, sizeof(int));

            ///Wait
            waitSocket(d->socket);
            //////

            std::string myname = d->name;
            n = write(d->socket, myname.c_str(), strlen(myname.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            ///Wait
            waitSocket(d->socket);
            //////

            std::string name;
            char filename[256];
            char filename2[256];
            FILE *f;
            if (answer == 1) {
                cout << "Komu chces poslat subor? ";
            } else if (answer == 2) {
                cout << "Akej skupine chces poslat subor? ";
            }

            cin >> name;

            n = write(d->socket, name.c_str(), strlen(name.c_str()));
            if (n < 0){perror("Error writing to socket");return nullptr;}

            //skontrolovanie ci je v kontaktoch
            int ok;
            n = read(d->socket, &ok, 255);
            if(ok==0 && answer == 1){
                cout << "Zadany pouzivatel sa nenachadza v kontaktoch" << endl;
            } else if(ok==0 && answer == 2) {
                cout << "Do tejto skupiny nepatris" << endl;
            } else {
            ///////////////////////////////////
                bzero(filename2, 256);
                std::cout << "Zadaj nazov suboru: ";
                std::cin >> filename2;
                bzero(filename, 256);

                strcat(filename, "../");
                strcat(filename, filename2);
                write(d->socket, filename2, 255);
                ///Wait
                waitSocket(d->socket);
                //////
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
                    ///Wait
                    waitSocket(d->socket);
                    //////
                    rewind(f);
                    char ch = '0';
                    while (ch != EOF){
                        fscanf(f, "%s", buffer);
                        write(d->socket, buffer, 255);
                        ///Wait
                        waitSocket(d->socket);
                        //////
                        ch = fgetc(f);
                    }
                    fclose(f);
                    std::cout << "Subor uspesne poslany na server" << std::endl;
                } else {
                    std::cout << "Takyto subor neexistuje!" << std::endl;
                }
            }
        } else if(answer == 4){
            bzero(buffer,256);
            buffer[0] = '4';
            n = write(d->socket, buffer, strlen(buffer));
            std::cout << "Zadaj nazov suboru: ";
            std::string filename;
            std::cin >> filename;
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
                    if (myfile.is_open()) {
                        myfile << buffer << " ";
                    } else std::cout << "Unable to open file";
                    ch++;
                }
                myfile.close();
            } else {
                std::cout << "Nikto ti neposlal taky subor..." << std::endl;
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
            std::cout << "Pre ukoncenie pridavania pouzivatelov stlate 'q'" << std::endl;

            while(message != "q"){
                std::cout  << "Zadajte meno pouzivatela: ";
                getline(cin, message);
                if(message == d->name){
                    std::cout << "Uz si clenom skupiny" << std::endl;
                } else {
                    if(message != "q"){
                        strcat(buffer, " ");
                        strcat(buffer, (message).c_str());
                    }
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
                std::cout << "Ziadost o pridanie kontaktu uspesne poslana" << std::endl;
            } else {
                std::cout << "Zadany pouzivatel neexistuje" << std::endl;
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
                std::cout << "Odstranenie kontaktu uspesne" << std::endl;
            } else {
                std::cout << "Zadaneho pouzivatela nemate v kontaktoch" << std::endl;
            }
        }  else if(answer == 8){
            bzero(buffer,256);
            buffer[0] = '8';
            n = write(d->socket, buffer, strlen(buffer));
            if (n < 0){perror("Error writing to socket");return nullptr;}
            ///Wait
            waitSocket(d->socket);
            //////
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
            std::cout << str << std::endl;

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            char* end = oldContacts + sizeof(oldContacts) / sizeof(oldContacts[0]);
            char* position = std::find(oldContacts, end, 'r');
            if(position != end){
                std::string message;
                std::cout << "Prijimanie ziadosti ukoncite stlacenim 'q'" << std::endl;
                while(message != "q"){
                    if(numberOfRequests > 0){
                        std::cout  << "Zadajte meno a prijatie(a) alebo neprijatie(n) ziadosti o priatelstvo: ";
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

            //skontrolovanie ci je v kontaktoch
            int ok;
            n = read(d->socket, &ok, 255);
            if(ok==0 && answer == 9){
                std::cout << "Zadany pouzivatel sa nenachadza v kontaktoch" << std::endl;
            } else if(ok==0 && answer == 10) {
                std::cout << "Do tejto skupiny nepatris" << std::endl;
            } else {
                ///continue
                bzero(buffer,256);
                strcat(buffer, "continue");
                n = write(d->socket, buffer, strlen(buffer)-1);
                if (n < 0){perror("Error writing to socket");return nullptr;}
                bzero(buffer,256);
                ////////////
                std::cout << "Pises si s " << name << ", ak chces konverzaciu ukoncit stlac 'q'" << std::endl;
                std::cout << "Ak chce poslat sifrovanu spravu stlac klavesu 'x'" << std::endl;
                std::cout << "Vase spravy:" <<  std::endl;

                char messages[1024];
                bzero(messages,1024);
                n = read(d->socket, messages, 1023);
                if (n < 0){perror("Error reading from socket");return nullptr;}
                //Vsetky predchadzajuce spravy
                std::cout << messages << std::endl;
                bzero(messages,1024);
                pthread_t sendThread, receiveThread;
                pthread_create(&sendThread, NULL, &sendMessage, d);
                pthread_create(&receiveThread, NULL, &receiveMessage, d);

                pthread_join(sendThread, NULL);
                pthread_join(receiveThread, NULL);
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
