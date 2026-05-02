#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
typedef struct {
    int len; 
    int dot; 
    int dash; 
    int underscore;
    int tilde; 
    int digit; 
    int letter;
    int special;
} UrlStat;
void scan(const char *s, int len, UrlStat *st, int freq[256], char *res); // thống kê, đưa về chữ thường
double entropy(const int freq[256], int len);
void extract_domain(const char *hostname, char *sub, char *core, char *tld);
const char *stristr(const char *big, const char *sub);
int has_keyword(const char *big, const char **kw);
int is_ipv4(const char *h);
void str_tolower(char *s);
#endif