//
// Created by Core on 22/10/2018.
//

#include "tracker.h"


SOCKET listen_sock;
struct sockaddr_in si_other;
int slen = sizeof(si_other) , recv_len;
char buf[PACK_SIZE];



void tracker::init() {

    struct addrinfo *result = nullptr, hints{};
    int status;

    ZeroMemory(&result, sizeof (result));
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_flags = AI_PASSIVE; // to allow binding
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    status = getaddrinfo(inet_ntoa(server_ip), port, &hints, &result);
    if (status != 0) {
        std::cout << "[ERROR]: " << status << " Unable to get address info for Port " << port << ".\n";
    }

    SOCKET serv_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_sock == INVALID_SOCKET) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to create Socket.\n";
        freeaddrinfo(result);
    }

    if (::bind(serv_sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to bind Socket.\n";
        freeaddrinfo(result);
        closesocket(serv_sock);
    }

    print_server((struct sockaddr_in *)result->ai_addr, port, "Tracker server");
    std::cout.flush();

    // We don't need this info any more
    freeaddrinfo(result);
    listen_sock = serv_sock;


}
//keep listening for data

void tracker::listen() {
    while (true) {
        printf("Waiting for data...");
        fflush(stdout);

//clear the buffer by filling null, it might have previously received data
        memset(buf, '\0', PACK_SIZE);

//try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(listen_sock, buf, PACK_SIZE, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR) {
            printf("recvfrom() failed with error code : %d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

//print details of the client/peer and the data received
        printf("Data: %s\n", buf);

        int request = buf[8] - '0';


        std::string message(buf);
        //int pos = message.find("|") + 1;
        //printf("%s\n", message.substr(pos).c_str());


        switch (request) {
            //Download a file from the swarm. FILENAME value must be filled.
            case 1:
                //generateList(message);
                printf("Generating List %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Ask the tracker for the updated list. FILENAME must be filled.
            case 2:
                //generateList(message);
                printf("Generating List %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Inform the tracker that a chunk has been successfully downloaded. FILENAME and CHUNK NO must be filled.
            case 3:
                //addEntry(message);
                //printf("Adding Entry from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Upload a new file. FILENAME, CHUNK NO and IP ADDRESS must be filled.
            case 4:
                //addFile(message);
                printf("Adding Entry from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Exit from swarm. IP ADDRESS must be filled.
            case 5:
                //deleteIP(message);
                printf("Adding Entry from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Query the tracker for a list of files available.
            case 6:
                //query(message);
                printf("Adding Entry from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Query for a specified file. FILENAME must be filled.
            case 7:
                //query(message);
                printf("Adding Entry from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //update IP for STUN server
            case 8:
                //updateIP(message);
                break;
        }


        //Sample reply
        string reply = "Filename.txt||5||127.0.0.1";
        strcpy(buf,reply.c_str());
        // TODO: Stop tracker from crashing when attempting to send
//        if (sendto(listen_sock, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
//        {
//            printf("sendto() failed with error code : %d" , WSAGetLastError());
//        }
    }

    closesocket(listen_sock);
    WSACleanup();

}
string tracker::addEntry(string message){
    return 0;
}
string tracker::addFile(string message){
    return 0;
}
string tracker::query(string message){
    return 0;
}
string tracker::generateList(string message){
    return 0;
}
string tracker::updateIP(string message){
    return 0;
}
string tracker::deleteIP(string message){
    return 0;
}