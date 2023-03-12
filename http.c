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

void handle_error(int sock, int err_code);
void handle_500(int sock);
void fill_response(char *header, int status, long len, char *type);
void find_type(char *ct_type, char *uri);

void handle_http(int sock)
{
    char req[BUF_SIZE];
    char res[BUF_SIZE];

    int str_len;

    if ((read(sock, req, BUF_SIZE)) < 0)
    {
        perror("[error] Failed to read request. \n");
    }

    printf("%s \n", req);
    char *method = strtok(req, " ");
    char *uri = strtok(NULL, " ");

    printf("[info] Handling Request %s  %s \n", method, uri);

    char safe_uri[BUF_SIZE] = BASE_STATIC_PATH;
    char *local_uri;
    struct stat st;

    if (!strcmp(uri, "/"))
        strcpy(uri, "/index.html");
    strcat(safe_uri, uri);

    printf("uri : %s \n", uri);
    local_uri = safe_uri + 1;
    if (stat(local_uri, &st) < 0)
    {
        perror("[WARN] No file found matching URI.\n");
        handle_error(sock, 404);
        return;
    }

    int fd = open(local_uri, O_RDONLY);
    if (fd < 0)
    {
        perror("[ERR] Failed to open file.\n");
        handle_error(sock, 500);
        return;
    }

    int ct_len = st.st_size;
    char ct_type[40];
    find_type(ct_type, local_uri);
    fill_response(res, 200, ct_len, ct_type);
    printf("res : %s \n", res);

    write(sock, res, strlen(res));

    int cnt;
    while ((cnt = read(fd, req, BUF_SIZE)) > 0)
        write(sock, req, cnt);
}

void handle_error(int sock, int err_code)
{
    char res[BUF_SIZE];
    struct stat st;
    char local_url[40];

    sprintf(local_url, "static/%d.html", err_code);
    stat(local_url, &st);
    fill_response(res, err_code, st.st_size, "text/html");

    write(sock, res, strlen(res));
}

void fill_response(char *header, int status, long len, char *type)
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
void find_type(char *ct_type, char *uri)
{
    char *ext = strrchr(uri, '.');
    if (!strcmp(ext, ".html"))
        strcpy(ct_type, "text/html");
    else if (!strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg"))
        strcpy(ct_type, "image/jpeg");
    else if (!strcmp(ext, ".png"))
        strcpy(ct_type, "image/png");
    else if (!strcmp(ext, ".css"))
        strcpy(ct_type, "text/css");
    else if (!strcmp(ext, ".js"))
        strcpy(ct_type, "text/javascript");
    else
        strcpy(ct_type, "text/plain");
}