// Sample network sockets code - UDP Client.
// Written for COMP3331 by Andrew Bennett, October 2019.

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*

A good reference for C sockets programming (esp. structs and syscalls):
https://beej.us/guide/bgnet/html/multi/index.html

And for information on C Threads:
https://www.gnu.org/software/libc/manual/html_node/ISO-C-Threads.html

One of the main structs used in this program is the "sockaddr_in" struct.
See: https://beej.us/guide/bgnet/html/multi/ipstructsdata.html#structs

struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};

*/

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12000

#define BUF_LEN 2048

// Get the "name" (IP address) of who we're communicating with.
char *get_name(struct sockaddr_in *sa, char *name);

// Populate a sockaddr_in struct with the specified IP / port to connect to.
void fill_sockaddr(struct sockaddr_in *sa, char *ip, int port);

// A wrapper function for fgets, similar to Python's built-in 'input' function.
void get_input(char *buf, char *msg);


int main(int argc, char *argv[]) {

    // Array that we'll use to store the data we're sending / receiving.
    char buf[BUF_LEN] = {0};

    // Temporary array used to store the name of the client when printing.
    char name[BUF_LEN] = {0};

    // Create the client's socket.
    //
    // The first parameter indicates #the address family; in particular,
    // `AF_INET` indicates that the underlying network is using IPv4.
    //
    // The second parameter indicates that the socket is of type
    // SOCK_DGRAM, which means it is a UDP socket (rather than a TCP
    // socket, where we use SOCK_STREAM).
    //
    // This returns a file descriptor, which we'll use with our sendto /
    // recvfrom functions later.
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    // A wrapper function for fgets, similar to Python's built-in
    // 'input' function.
    get_input(buf, "Please type 'Subscribe'\n");

    // Create the sockaddr that the client will use to send data to the
    // server.
    struct sockaddr_in sockaddr_to;
    fill_sockaddr(&sockaddr_to, SERVER_IP, SERVER_PORT);

    // Now that we have a socket and a message, we send the message
    // through the socket to the server.
    //
    // Note the difference between UDP sendto() and TCP send() calls.
    // In TCP we do not need to attach the destination address to the
    // packet, while in UDP we explicilty specify the destination
    // address + port number for each message.
    int numbytes = sendto(sock_fd, buf, strlen(buf), 0,
            (struct sockaddr *) &sockaddr_to, sizeof(sockaddr_to));

    printf("Sent %d bytes to %s:%d\n", numbytes,
            get_name(&sockaddr_to, name),
            ntohs(sockaddr_to.sin_port));


    // Create the sockaddr that the client will use to receive data from
    // the server.
    struct sockaddr_in sockaddr_from;

    // We need to create a variable to store the length of the sockaddr
    // we're using here, which the recvfrom function can update if it
    // stores a different amount of information in the sockaddr.
    socklen_t from_len = sizeof(sockaddr_from);

    // Wait for the reply from the server.
    numbytes = recvfrom(sock_fd, buf, BUF_LEN, 0,
        (struct sockaddr *) &sockaddr_from, &from_len);

    // Null-terminate the result, and remove the newline (if present).
    buf[numbytes] = '\0';
    buf[strcspn(buf, "\n")] = '\0';

    printf("Received %d bytes from %s:%d: %s\n", numbytes,
            get_name(&sockaddr_from, name),
            ntohs(sockaddr_from.sin_port), buf);

    // Remember: strcmp returns 0 if the strings match.
    if (!strcmp(buf, "Subscription successful")) {
        // Receive 10 messages back-to-back from the server.
        for (int i = 0; i < 10; i++) {
            printf("Waiting for message...\n");

            numbytes = recvfrom(sock_fd, buf, BUF_LEN, 0,
                (struct sockaddr *) &sockaddr_from, &from_len);

            buf[numbytes] = '\0';
            buf[strcspn(buf, "\n")] = '\0';

            printf("Received %d bytes from %s: %s\n", numbytes,
                    get_name(&sockaddr_from, name), buf);
        }
    }

    // Prepare to exit -- send the Unsubscribe message to the server.
    strcpy(buf, "Unsubscribe");
    numbytes = sendto(sock_fd, buf, strlen(buf), 0,
            (struct sockaddr *) &sockaddr_to, sizeof(sockaddr_to));

    printf("Sent %d bytes to %s:%d\n", numbytes,
            get_name(&sockaddr_to, name),
            ntohs(sockaddr_to.sin_port));

    close(sock_fd);

    return 0;
}


// Wrapper function for fgets, similar to Python's built-in 'input'
// function.
void get_input(char *buf, char *msg) {
    printf("%s", msg);
    fgets(buf, BUF_LEN, stdin);
    buf[strcspn(buf, "\n")] = '\0'; // Remove the newline
}

// Populate a `sockaddr_in` struct with the IP / port to connect to.
void fill_sockaddr(struct sockaddr_in *sa, char *ip, int port) {

    // Set all of the memory in the sockaddr to 0.
    memset(sa, 0, sizeof(struct sockaddr_in));

    // IPv4.
    sa->sin_family = AF_INET;

    // We need to call `htons` (host to network short) to convert the
    // number from the "host" endianness (most likely little endian) to
    // big endian, also known as "network byte order".
    sa->sin_port = htons(port);
    inet_pton(AF_INET, ip, &(sa->sin_addr));
}

// Get the "name" (IP address) of who we're communicating with.
char *get_name(struct sockaddr_in *sa, char *name) {
    inet_ntop(sa->sin_family, &(sa->sin_addr), name, BUF_LEN);
    return name;
}
