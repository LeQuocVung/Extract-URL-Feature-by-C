#ifndef URL_PARSER_H
#define URL_PARSER_H
#include <stddef.h>
#define MAX_URL 4096
#define MAX_PART 2048
typedef struct parseURL {
    char scheme[10];
    char hostname[MAX_PART];
    char path[MAX_PART];
    char query[MAX_PART];
    char fragment[MAX_PART];
    int port;
} parseURL;
// example: https://sub.example.com:80/path/to/page?id=123#section
int parse_URL(const char *URL, parseURL *res);
#endif