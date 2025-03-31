#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <json-c/json.h>

#define SERVADDR "34.246.184.49"
#define SERVPORT 8080

int get_status(char response[], char version[]) {
    response += strlen(version);
    int ret = 0;
    ret = 100 * (response[0] - '0');
    ret += 10 * (response[1] - '0');
    ret += (response[2] - '0');
    return ret;
}

int verif_number(char num[]) {
    int len = strlen(num);
    int number = 0;
    for (int i = 0; i < len; ++i)
        if (num[i] < '0' || num[i] > '9')
            return -1;
        else
            number = number * 10 + (num[i] - '0');
    return number;
}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char comanda[100], username[100], password[100], id[100], cookie[300];
    char title[100], author[100], genre[100], publisher[100], page_count[100], aux[300];
    int logat = 0;
    entered_in_library = 0;
    memset(access_library, 0, 300);
    while (1) {
        memset(comanda, 0, 100);
        memset(username, 0, 100);
        memset(password, 0, 100);
        memset(id, 0, 100);
        memset(title, 0, 100);
        memset(author, 0, 100);
        memset(genre, 0, 100);
        memset(publisher, 0, 100);
        memset(page_count, 0, 100);
        scanf("%s", comanda);
        if (!strcmp(comanda, "register")) {
            gets();
            printf("username=");
            gets(username);
            printf("password=");
            gets(password);
            if (strchr(username, ' ')) {
                printf("EROARE: user invalid!\n");
                continue;
            }
            sockfd = open_connection(SERVADDR, SERVPORT, AF_INET, SOCK_STREAM, 0);
            json_object *payload_json = json_object_new_object();
            json_object_object_add(payload_json, "username", json_object_new_string(username));
            json_object_object_add(payload_json, "password", json_object_new_string(password));
            char *payload_string = json_object_to_json_string(payload_json);
            int len = strlen(payload_string);
            message = compute_post_request(SERVADDR, "/api/v1/tema/auth/register", "application/json", payload_string, len, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int status = get_status(response, "HTTP/1.1 ");
            memset(message, 0, strlen(message));
            memset(response, 0, strlen(response));
            close_connection(sockfd);
            if (status == 201)
                printf("Utilizatorul a fost înregistrat cu SUCCES!\n");
            else
                printf("EROARE la inregistrare!\n");
        } else if (!strcmp(comanda, "login")) {
            if (logat) {
                printf("EROARE: Exista deja client logat!\n");
                continue;
            }
            gets();
            printf("username=");
            gets(username);
            printf("password=");
            gets(password);
            if (strchr(username, ' ')) {
                printf("EROARE: user invalid!\n");
                continue;
            }
            sockfd = open_connection(SERVADDR, SERVPORT, AF_INET, SOCK_STREAM, 0);
            json_object *payload_json = json_object_new_object();
            json_object_object_add(payload_json, "username", json_object_new_string(username));
            json_object_object_add(payload_json, "password", json_object_new_string(password));
            char *payload_string = json_object_to_json_string(payload_json);
            int len = strlen(payload_string);
            message = compute_post_request(SERVADDR, "/api/v1/tema/auth/login", "application/json", payload_string, len, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int status = get_status(response, "HTTP/1.1 ");
            close_connection(sockfd);
            if (status == 200) {
                memset(cookie, 0, 300);
                memset(message, 0, strlen(message));
                message = strstr(response, "Set-Cookie");
                message += strlen("Set-Cookie: ");
                int cnt = 0;
                while (message[cnt] != ';') {
                    cookie[cnt] = message[cnt];
                    ++cnt;
                }
                printf("Utilizatorul a fost logat cu SUCCES!\n");
                logat = 1;
            } else {
                printf("EROARE la logare!\n");
            }
            memset(message, 0, strlen(message));
            memset(response, 0, strlen(response));
        } else if (!strcmp(comanda, "enter_library")) {
            if (!logat) {
                printf("EROARE: Nu esti logat ca utilizator!\n");
                continue;
            }
            sockfd = open_connection(SERVADDR, SERVPORT, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(SERVADDR, "/api/v1/tema/library/access", NULL, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int status = get_status(response, "HTTP/1.1 ");
            close_connection(sockfd);
            if (status == 200) {
                printf("SUCCES: acces biblioteca!\n");
                entered_in_library = 1;
                memset(access_library, 0, 300);
                memset(message, 0, strlen(message));
                message = strstr(response, "token");
                message += 8;
                int cnt = 0;
                while (message[cnt] != '"') {
                    access_library[cnt] = message[cnt];
                    ++cnt;
                }
            } else {
                printf("EROARE la intrarea in biblioteca!\n");
            }
            memset(message, 0, strlen(message));
            memset(response, 0, strlen(response));
        } else if (!strcmp(comanda, "get_books")) {
            if (!logat) {
                printf("Trebuie intai sa te loghezi!\n");
                continue;
            }
            if (!entered_in_library) {
                printf("Trebuie sa ai acces la biblioteca intai!\n");
                continue;
            }
            sockfd = open_connection(SERVADDR, SERVPORT, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(SERVADDR, "/api/v1/tema/library/books", NULL, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int status = get_status(response, "HTTP/1.1 ");
            close_connection(sockfd);
            if (status == 200)
                printf("%s\n", strstr(response, "["));
            else
                printf("EROARE la get_books!\n");
            memset(message, 0, strlen(message));
            memset(response, 0, strlen(response));
        } else if (!strcmp(comanda, "get_book")) {
            if (!logat) {
                printf("Trebuie intai sa te loghezi!\n");
                continue;
            }
            if (!entered_in_library) {
                printf("Trebuie sa ai acces la biblioteca intai!\n");
                continue;
            }
            gets();
            printf("id=");
            gets(id);
            sockfd = open_connection(SERVADDR, SERVPORT, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(SERVADDR, "/api/v1/tema/library/books", id, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int status = get_status(response, "HTTP/1.1 ");
            close_connection(sockfd);
            if (status == 200)
                printf("%s\n", strstr(response, "{"));
            else
                printf("EROARE la get_book!\n");
            memset(message, 0, strlen(message));
            memset(response, 0, strlen(response));
        } else if (!strcmp(comanda, "add_book")) {
            if (!logat) {
                printf("Trebuie intai sa te loghezi!\n");
                continue;
            }
            if (!entered_in_library) {
                printf("Trebuie sa ai acces la biblioteca intai!\n");
                continue;
            }
            gets();
            printf("title=");
            gets(title);
            printf("author=");
            gets(author);
            printf("genre=");
            gets(genre);
            printf("publisher=");
            gets(publisher);
            printf("page_count=");
            gets(page_count);
            int page_cnt = verif_number(page_count);
            if (page_cnt == -1) {
                printf("Tip de date incorect pentru numarul de pagini: %s!\n", page_count);
                continue;
            }
            sockfd = open_connection(SERVADDR, SERVPORT, AF_INET, SOCK_STREAM, 0);
            json_object *payload_json = json_object_new_object();
            json_object_object_add(payload_json, "title", json_object_new_string(title));
            json_object_object_add(payload_json, "author", json_object_new_string(author));
            json_object_object_add(payload_json, "genre", json_object_new_string(genre));
            json_object_object_add(payload_json, "publisher", json_object_new_string(publisher));
            json_object_object_add(payload_json, "page_count", json_object_new_int(page_cnt));
            char *payload_string = json_object_to_json_string(payload_json);
            int len = strlen(payload_string);
            message = compute_post_request(SERVADDR, "/api/v1/tema/library/books", "application/json", payload_string, len, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int status = get_status(response, "HTTP/1.1 ");
            close_connection(sockfd);
            if (status == 200)
                printf("Carte adugata cu SUCCES!\n");
            else
                printf("EROARE la add_book!\n");
            memset(message, 0, strlen(message));
            memset(response, 0, strlen(response));
        } else if (!strcmp(comanda, "delete_book")) {
            if (!logat) {
                printf("Trebuie intai sa te loghezi!\n");
                continue;
            }
            if (!entered_in_library) {
                printf("Trebuie sa ai acces la biblioteca intai!\n");
                continue;
            }
            gets();
            printf("id=");
            gets(id);
            sockfd = open_connection(SERVADDR, SERVPORT, AF_INET, SOCK_STREAM, 0);
            message = compute_delete_request(SERVADDR, "/api/v1/tema/library/books", id, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int status = get_status(response, "HTTP/1.1 ");
            close_connection(sockfd);
            if (status == 200)
                printf("Cartea cu id %s a fost stearsa cu succes!\n", id);
            else
                printf("EROARE la delete_book!\n");
            memset(message, 0, strlen(message));
            memset(response, 0, strlen(response));
        } else if (!strcmp(comanda, "logout")) {
            if (!logat) {
                printf("EROARE: Momentan niciun user nu este logat!\n");
                continue;
            }
            sockfd = open_connection(SERVADDR, SERVPORT, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(SERVADDR, "/api/v1/tema/auth", "logout", &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            int status = get_status(response, "HTTP/1.1 ");
            close_connection(sockfd);
            if (status == 200) {
                memset(access_library, 0, strlen(access_library));
                logat = 0;
                entered_in_library = 0;
                printf("SUCCES: utilizator delogat!\n");
            } else {
                printf("EROARE la logare!\n");
            }
            memset(message, 0, strlen(message));
            memset(response, 0, strlen(response));
        } else if (!strcmp(comanda, "exit")) {
            break;
        }
    }
    return 0;
}
