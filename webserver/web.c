#include "main.h"

// defines
#define PORT 80
#define WEBROOT "/home/goliath/Project/Project_holbertonschool/webserver/web/"
#define BUFFER_SIZE 1024

void fatal(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void handle_connection(int sockfd, struct sockaddr_in *client_addr_ptr)
{
    char buffer[BUFFER_SIZE];
    ssize_t numbytes;
    int fd;

    printf("Handling connection from %s\n", inet_ntoa(client_addr_ptr->sin_addr));

    // Open the index.html file
    fd = open(WEBROOT "index.html", O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return;
    }
        
    // Open the index.php file
    fd = open(WEBROOT "index.php", O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return;
    }

    // Read the file into the buffer
    numbytes = read(fd, buffer, BUFFER_SIZE);
    if (numbytes == -1)
    {
        perror("read");
        return;
    }

    // Send the file to the PHP-FPM server
    int php_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in php_addr;
    php_addr.sin_family = AF_INET;
    php_addr.sin_port = htons(9000);
    php_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    connect(php_sockfd, (struct sockaddr *)&php_addr, sizeof(php_addr));
    send(php_sockfd, buffer, numbytes, 0);

    // Receive the response from the PHP-FPM server
    numbytes = recv(php_sockfd, buffer, BUFFER_SIZE, 0);
    if (numbytes == -1)
    {
        perror("recv");
        return;
    }

    // Send the response to the client
    numbytes = send(sockfd, buffer, numbytes, 0);
    if (numbytes == -1)
    {
        perror("send");
        return;
    }

    // Close the file and the sockets
    close(fd);
    close(php_sockfd);
    close(sockfd);
}

int main(void) {
    int sockfd, new_sockfd, yes = 1;
    struct sockaddr_in host_addr, client_addr;
    socklen_t sin_size;
    printf("Accepting web requests on port %d\n", PORT);
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) // Correction ici
        fatal("in socket");
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        fatal("setting socket option SO_REUSEADDR");
    host_addr.sin_family = AF_INET;         // Ordre des octets de l'hôte.
    host_addr.sin_port = htons(PORT);       // Ordre des octets du réseau (entier court).
    host_addr.sin_addr.s_addr = 0;          // Remplir automatiquement l'IP.
    memset(&(host_addr.sin_zero), '\0', 8); // Reste de la structure à 0.
    if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1)
        fatal("binding to socket");
    if (listen(sockfd, 20) == -1)
        fatal("listenning on socket");
    while (1)
    { // Boucle d'acceptation des connexions entrantes.
        sin_size = sizeof(struct sockaddr_in);
        new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_sockfd == -1)
            fatal("accepting connection");
        handle_connection(new_sockfd, &client_addr);
    }
    return 0;
}