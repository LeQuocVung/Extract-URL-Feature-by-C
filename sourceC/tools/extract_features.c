#include "extract_features.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <omp.h>
#define MAX_URL_LEN 4096
static int cnt_query_params(const char *query){
    if(!query || !*query) return 0;
    int cnt = 0;
    const char *p = query;
    while(*p){
        const char *segment_end = p;
        while(*segment_end && *segment_end != '&' && *segment_end != ';') segment_end++; // hết một tham số truy vấn
        const char *eq = p;
        while(eq < segment_end && *eq != '=') eq++; // dấu bằng
        if(eq > p && eq < segment_end - 1)
            cnt++;
        p = (*segment_end) ? segment_end + 1 : segment_end;
    }
    return cnt;
}
static void find_ext(const char *path, char *ext, size_t ext_size){
    ext[0] = '\0';
    if(!path && !*path) return;
    const char *last = strrchr(path, '/');
    last = last ? last + 1 : path;
    const char *dot = strrchr(last, '.');
    if(!dot || dot == last) return;
    size_t len = strlen(dot + 1);
    if(len == 0 || len >= ext_size) return;
    memcpy(ext, dot + 1, len + 1);
    str_tolower(ext);
}
static int in_list(const char *ext, const char **list){
    if(!ext || !*ext) return 0;
    for(int i = 0; list[i]; i++)
        if(strcmp(ext, list[i]) == 0) return 1;
    return 0;
}
static int has_word(const char *text, const char *word){
    if(!text || !word) return 0;
    int wlen = (int)strlen(word);
    const char *p = text;
    while((p = stristr(p, word)) != NULL){
        int left_ok  = (p == text) || !isalnum((unsigned char)*(p - 1)); //trước không có gì hoặc không có chữ/số
        int right_ok = !isalnum((unsigned char)*(p + wlen)); //sau không phải chữ hoặc số
        if(left_ok && right_ok) return 1;
        p++;
    }
    return 0;
}
static int get_depth_path(const char *path){
    if(!path || !*path) return 0;
    int d = 0, in_segment = 0;
    for(int i = 0; path[i]; i++){
        if (path[i] == '/') in_segment = 0;
        else if(!in_segment){ 
            d++;
            in_segment = 1;
        }
    }
    return d;
}
static int has_malware(const char *url_lower){
    if(has_keyword(url_lower, KW_MALWARE)) return 1;
    for(int i = 0; KW_MALWARE_WORD[i]; i++){
        if (has_word(url_lower, KW_MALWARE_WORD[i])) return 1;
    }
    return 0;
}
DLL_EXPORT int extract(const char *url, UrlFeatures *res){
    if (!url || !res) return -1;
    memset(res, 0, sizeof(*res));
    parseURL p;
    if (parse_URL(url, &p) != 0) return -1;
    const char *orig   = url;
    int orig_len       = (int)strlen(orig);
    char netloc[512]; // netloc=host+port
    if(p.port != -1){
        int hlen = (int)strlen(p.hostname);
        memcpy(netloc, p.hostname, hlen);
        netloc[hlen] = ':';
        char port_str[16];
        int port = p.port;
        int len = 0;
        if(port == 0) port_str[len++] = '0';
        else{
            int tmp = port;
            while(tmp){
                port_str[len++] = (char)('0' + tmp%10); // chuyển số thành chữ
                tmp /= 10;
            } 
        }
        for(int i = 0; i < len / 2; i++){ // đảo ngược
            char t = port_str[i];
            port_str[i] = port_str[len - 1 - i];
            port_str[len - 1 - i] = t;
        }
        memcpy(netloc + hlen + 1, port_str, len);
        netloc[hlen + 1 + len] = '\0';
    }
    else{
        strncpy(netloc, p.hostname, sizeof(netloc) - 1);
        netloc[sizeof(netloc) - 1] = '\0';
    }
    int netloc_len = (int)strlen(netloc);
    int path_len   = (int)strlen(p.path);
    int query_len  = (int)strlen(p.query);
    char ext[32];
    find_ext(p.path, ext, sizeof(ext));
    char core[256];
    char tld_buf[64]; 
    char sub[256];
    extract_domain(p.hostname, sub, core, tld_buf);
    int core_len = (int)strlen(core);
    UrlStat url_stat, netloc_stat, path_stat, query_stat;
    int url_freq[256];
    int netloc_freq[256];
    int path_freq[256];
    int query_freq[256];
    char url_lower[MAX_URL_LEN];
    char netloc_lower[512];
    char path_lower[2048]; 
    char query_lower[2048];
    scan(orig, orig_len, &url_stat, url_freq, url_lower);
    scan(netloc, netloc_len, &netloc_stat, netloc_freq, netloc_lower);
    scan(p.path, path_len, &path_stat, path_freq, path_lower);
    scan(p.query, query_len, &query_stat, query_freq, query_lower);
    res->url_special_ratio     = url_stat.len > 0 ? (double)url_stat.special / url_stat.len : 0.0;
    res->dot_count             = url_stat.dot;
    res->domain_digit_ratio    = netloc_stat.len > 0 ? (double)netloc_stat.digit   / netloc_stat.len : 0.0;
    res->url_letter_ratio      = url_stat.len > 0 ? (double)url_stat.letter  / url_stat.len : 0.0;
    res->url_digit_ratio       = url_stat.len > 0 ? (double)url_stat.digit   / url_stat.len : 0.0;
    res->query_param_count     = cnt_query_params(p.query);
    res->path_length           = path_len;
    res->ext_is_exe            = in_list(ext, EXT_EXE);
    res->has_defacement_kw     = has_keyword(url_lower, KW_DEFACEMENT);
    res->path_entropy          = entropy(path_freq, path_len);
    res->path_hyphen_count     = path_stat.dash;
    res->hostname_length       = netloc_len;
    res->path_dot_count        = path_stat.dot;
    res->path_depth            = get_depth_path(p.path);
    res->domain_core_length    = core_len;
    res->domain_entropy        = entropy(netloc_freq, netloc_len);
    res->query_entropy         = entropy(query_freq, query_len);
    res->query_length          = query_len;
    res->has_malware_kw        = has_malware(url_lower);
    res->url_entropy           = entropy(url_freq, orig_len);
    res->has_phishing_kw       = has_keyword(url_lower, KW_PHISHING);
    res->path_digit_ratio      = path_stat.len > 0 ? (double)path_stat.digit / path_stat.len : 0.0;
    res->dash_count            = url_stat.dash;
    res->ext_is_doc            = in_list(ext, EXT_DOC);
    res->query_digit_ratio     = query_stat.len > 0 ? (double)query_stat.digit / query_stat.len : 0.0;
    res->ext_is_script         = in_list(ext, EXT_SCRIPT);
    res->has_file_ext          = (ext[0] != '\0') ? 1.0 : 0.0;
    res->has_benign_signal     = has_keyword(url_lower, KW_BENIGN);
    res->underscore_count      = url_stat.underscore;
    res->tilde_count           = url_stat.tilde;
    res->path_underscore_count = path_stat.underscore;
    res->domain_hyphen_count   = netloc_stat.dash;
    res->is_ip                 = is_ipv4(p.hostname);
    res->is_shortener          = has_keyword(p.hostname, KW_SHORTENER);
    res->is_https              = (strcmp(p.scheme, "https") == 0) ? 1.0 : 0.0;
    res->has_port              = (p.port != -1) ? 1.0 : 0.0;
    return 0;
}
DLL_EXPORT int features_to_array(const UrlFeatures *f, double *arr, int n){
    if (!f || !arr || n < NUM_FEATURES) return -1;
    arr[0]=f->url_special_ratio;
    arr[1]=f->dot_count;
    arr[2]=f->domain_digit_ratio;     
    arr[3]=f->url_letter_ratio;
    arr[4]=f->url_digit_ratio;        
    arr[5]=f->query_param_count;
    arr[6]=f->path_length;            
    arr[7]=f->ext_is_exe;
    arr[8]=f->has_defacement_kw;      
    arr[9]=f->path_entropy;
    arr[10]=f->path_hyphen_count;     
    arr[11]=f->hostname_length;
    arr[12]=f->path_dot_count;        
    arr[13]=f->path_depth;
    arr[14]=f->domain_core_length;    
    arr[15]=f->domain_entropy;
    arr[16]=f->query_entropy;         
    arr[17]=f->query_length;
    arr[18]=f->has_malware_kw;        
    arr[19]=f->url_entropy;
    arr[20]=f->has_phishing_kw;       
    arr[21]=f->path_digit_ratio;
    arr[22]=f->dash_count;            
    arr[23]=f->ext_is_doc;
    arr[24]=f->query_digit_ratio;     
    arr[25]=f->ext_is_script;
    arr[26]=f->has_file_ext;          
    arr[27]=f->has_benign_signal;
    arr[28]=f->underscore_count;      
    arr[29]=f->tilde_count;
    arr[30]=f->path_underscore_count; 
    arr[31]=f->domain_hyphen_count;
    arr[32]=f->is_ip;                 
    arr[33]=f->is_shortener;
    arr[34]=f->is_https;              
    arr[35]=f->has_port;
    return 0;
}
DLL_EXPORT void extract_batch(const char *raw_buffer, const StringView *views, double *res, int count, int n_threads){
    if(!raw_buffer || !views || !res || count <= 0) return;
    #pragma omp parallel for num_threads(n_threads) schedule(static)
    for(int i = 0; i < count; i++){
        const char *url_ptr = raw_buffer + views[i].offset;
        size_t len = views[i].length;
        char url_safe[MAX_URL_LEN];
        size_t safe_len = (len < MAX_URL_LEN - 1) ? len : MAX_URL_LEN - 1;
        memcpy(url_safe, url_ptr, safe_len);
        url_safe[safe_len] = '\0';
        UrlFeatures f;
        double *dst = res + (size_t)i * NUM_FEATURES;
        if(extract(url_safe, &f) == 0) features_to_array(&f, dst, NUM_FEATURES);
        else memset(dst, 0, NUM_FEATURES * sizeof(double));
    }
}