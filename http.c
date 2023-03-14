#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define HEADER_FMT "HTTP/1.1 %d %s\nContent-Length: %ld\nContent-Type: %s\n\n"
#define BASE_STATIC_PATH "/static";
#define BUF_SIZE 4096
#define KEEP_ALIVE "keep-alive"

void handle_error(int sock, int err_code);
void fill_header(char *header, int status, long len, char *type);
void find_type(char *ct_type, char *uri);

int handle_http(int sock)
{
    char buffer[BUF_SIZE];
    char header[BUF_SIZE];

    int str_len;

    if ((read(sock, buffer, BUF_SIZE)) < 0)
    {
        perror("[error] Failed to read request. \n");
    }

    char *method = strtok(buffer, " ");
    char *uri = strtok(NULL, " ");

    strtok(NULL, "\n");
    strtok(NULL, " ");
    char *host = strtok(NULL, "\n");
    strtok(NULL, " ");
    char *connection = strtok(NULL, "\n");

    if (method == NULL || uri == NULL || host == NULL || connection == NULL)
    {
        perror("[ERR] Failed to identify method, URI.\n");
        handle_error(sock, 500);
        return 1;
    }

    char conet[20];
    strcpy(conet, connection);

    printf("[info] Handling Request %s  %s \n", method, uri);

    char safe_uri[BUF_SIZE] = BASE_STATIC_PATH;
    char *local_uri;
    struct stat st;

    if (!strcmp(uri, "/"))
        strcpy(uri, "/index.html");
    strcat(safe_uri, uri);

    local_uri = safe_uri + 1;
    if (stat(local_uri, &st) < 0)
    {
        perror("[WARN] No file found matching URI.\n");
        handle_error(sock, 404);
        return 1;
    }

    int fd = open(local_uri, O_RDONLY);
    if (fd < 0)
    {
        perror("[ERR] Failed to open file.\n");
        handle_error(sock, 500);
        return 1;
    }

    int ct_len = st.st_size;
    char ct_type[40];
    find_type(ct_type, local_uri);
    fill_header(header, 200, ct_len, ct_type);

    write(sock, header, strlen(header));

    int cnt;
    while ((cnt = read(fd, buffer, BUF_SIZE)) > 0)
        write(sock, buffer, cnt);

    return strncmp(KEEP_ALIVE, conet, 10);
}

void handle_error(int sock, int err_code)
{
    char header[BUF_SIZE];
    struct stat st;
    char local_url[40];
    char buffer[BUF_SIZE];

    sprintf(local_url, "static/%d.html", err_code);
    stat(local_url, &st);
    fill_header(header, err_code, st.st_size, "text/html");
    // 헤더 채우기
    write(sock, header, strlen(header));
    printf("handle error %d \n\n", err_code);
    int fd = open(local_url, O_RDONLY);
    if (fd < 0)
        return;

    int cnt;
    while ((cnt = read(fd, buffer, BUF_SIZE)) > 0)
        write(sock, buffer, cnt);

    return;
}

void fill_header(char *header, int status, long len, char *type)
{
    char status_text[40];
    switch (status)
    {
    case 200:
        strcpy(status_text, "OK");
        break;
    case 404:
        strcpy(status_text, "Not Found");
        break;
    case 500:
    default:
        strcpy(status_text, "Internal Server Error");
        break;
    }
    sprintf(header, HEADER_FMT, status, status_text, len, type);
}
void find_type(char *type, char *uri)
{
    char *ext = strrchr(uri, '.');
    if (!strcmp(ext, ".html"))
        strcpy(type, "text/html");
    else if (!strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg"))
        strcpy(type, "image/jpeg");
    else if (!strcmp(ext, ".png"))
        strcpy(type, "image/png");
    else if (!strcmp(ext, ".css"))
        strcpy(type, "text/css");
    else if (!strcmp(ext, ".js"))
        strcpy(type, "text/javascript");
    else
        strcpy(type, "text/plain");
}