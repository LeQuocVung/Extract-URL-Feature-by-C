#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include "url_parser.h"
static void str_tolower(char *str){
    if(!str) return;
    for(int i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
}
static void copy(char *dest, const char *src, int n,int len){
    if (!dest || !src || n <= 0) return;
    int cpy;
    if(len < n-1){
        cpy = len;
    }
    else cpy = n - 1;
    for(int i = 0; i < cpy; i++){
        dest[i] = src[i];
    }
    dest[cpy] = '\0';
}
int parse_URL(const char *URL, parseURL *res ){
    if(!URL || !res) return -1;
    char new_url[MAX_URL];
    memset(res,0,sizeof(*res));
    res->port = -1;
    const char *end_scheme = strstr(URL, "://");
    if(end_scheme){
        strncpy(new_url, URL,sizeof(new_url)-1);
        new_url[sizeof(new_url)-1] = '\0';
    }
    else{
        strcpy(new_url,"http://"); // thêm nếu không có
        strncat(new_url, URL,sizeof(new_url) - strlen(new_url) -1 );
    }
    const char *p = new_url;
    end_scheme = strstr(p, "://");
    copy(res->scheme, p,sizeof(res->scheme),(int)(end_scheme - p));
    p = end_scheme + 3;
    str_tolower(res->scheme);
    const char *at = strchr(p, '@');
    const char *slash = strchr(p,'/');
    if( at && (!slash || at < slash)){
        p = at+1;
    }
    const char *host = p;
    while(*host && *host != '/' && *host != '?' && *host != '#'){
        host++;
    }
    const char *colon = NULL;
    for (const char *c = host -1; c >= p; c--){
        if(*c == ':'){
            colon = c;
            break;
        }
    }
    if(colon){
        const char *port_str = colon +1;
        int flag_digit = 1;
        for(const char *c = port_str; c < host; c++){
            if(!isdigit((unsigned char)*c)){
                flag_digit = 0;
                break;
            }
        }
        if(flag_digit && port_str < host){
            res->port = atoi(port_str);
            copy(res->hostname, p, sizeof(res->hostname), (int)(colon - p));
        }
        else{
            copy(res->hostname, p, sizeof(res->hostname),(int)(host - p));
        }
    }
    else{
        copy(res->hostname, p, sizeof(res->hostname),(int)(host - p));
    }
    str_tolower(res->hostname);
    p = host;
    const char *path = p;
    while(*path && *path != '?' && *path != '#'){
        path++;
    }
    copy(res->path,p, sizeof(res->path), (int)(path - p));
    p = path;
    if(*p == '?'){
        p++;
        const char *query = p;
        while(*query && *query != '#') query++;
        copy(res->query, p, sizeof(res->query), (int)(query - p));
        p = query;
    }
    if(*p == '#'){
        p++;
        strncpy(res->fragment, p, sizeof(res->fragment) - 1);
        res->fragment[sizeof(res->fragment)-1] = '\0';
    }
    return 0;
}
