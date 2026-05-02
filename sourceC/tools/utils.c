#include "utils.h"
#include "keywords.h"
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
static unsigned char s_digit[256];
static unsigned char s_alpha[256];
static unsigned char s_lower[256]; 
static volatile int s_init = 0; 
static void init_table(){
    if(s_init) return;
    for (int i = 0; i < 256; i++) {
        s_digit[i] = (i >= '0' && i <= '9') ? 1 : 0;
        s_alpha[i] = ((i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z')) ? 1 : 0;
        s_lower[i] = (unsigned char)tolower(i);
    }
    s_init = 1;
}
void scan(const char *s, int len, UrlStat *st, int freq[256], char *res){
    if(!s || !freq || !st || !res) return;
    init_table();
    memset(st, 0, sizeof(*st));
    memset(freq, 0, 256 * sizeof(int));
    const unsigned char *p = (const unsigned char *)s;
    unsigned char *dest = (unsigned char *)res;
    for (int i = 0; i < len; i++){
        unsigned char c = p[i];
        unsigned char l_c = s_lower[c];
        dest[i] = l_c;
        freq[(int)c]++;
        st->len++;
        if(c == '.') st->dot++;
        else if(c == '-') st->dash++;
        else if(c == '_') st->underscore++;
        else if(c == '~') st->tilde++;
        if(s_digit[c]) st->digit++;
        else if(s_alpha[c]) st->letter++;
        else st->special++;
    }
    dest[len] = '\0';
}
double entropy(const int freq[256], int len){
    if(len <= 0) return 0.0;
    double h = 0.0, inv = 1.0 / len;
    for(int i = 0; i < 256; i++){
        if(!freq[i]) continue;
        double p = freq[i] * inv;
        h -= p * log2(p);
    }
    return h;
}
void extract_domain(const char *hostname, char *sub, char *core, char *tld){
    sub[0] = core[0] = tld[0] = '\0';
    if(!hostname || !*hostname) return;
    int dots[64], ndots = 0; // lưu vị trí các dấu chấm
    int h_len;
    for(h_len = 0; hostname[h_len]; h_len++){
        if (hostname[h_len] == '.' && ndots < 63) dots[ndots++] = h_len;
    }
    if(ndots == 0){ // nếu không có dấu chấm nào thì toàn bộ là core
        strncpy(core, hostname, 255);
        core[255] = '\0';
        return;
    }
    int second_last_dot = -1;
    if(ndots >= 2){
        second_last_dot = dots[ndots - 2] +1;
    }
    int tld_parts = 1;
    if(second_last_dot >= 0){
        char tmp[64];
        int c_len = h_len - second_last_dot;
        if(c_len > 0 && c_len < 64){
            memcpy(tmp, hostname + second_last_dot, c_len);
            tmp[c_len] = '\0';
            for(int i = 0; tmp[i]; i++)
                tmp[i] = (char)tolower((unsigned char)tmp[i]);
            for(int i = 0; MULTI_PART_TLD[i]; i++){
                if (strcmp(tmp, MULTI_PART_TLD[i]) == 0){ // check multi
                    tld_parts = 2;
                    break;
                }
            }
        }
    }
    int tld_dot_idx  = ndots - tld_parts; // chấm bắt đầu TLD
    int core_dot_idx = tld_dot_idx - 1; // chấm trước core 
    int full_tld_start = dots[tld_dot_idx] + 1; 
    int full_tld_len   = h_len - full_tld_start; // độ dài
    if(full_tld_len > 0 && full_tld_len < 64){
        memcpy(tld, hostname + full_tld_start, full_tld_len);
        tld[full_tld_len] = '\0';
    }
    int core_start = 0;
    if(core_dot_idx >= 0){
        core_start = dots[core_dot_idx] + 1;
    }
    int core_end = dots[tld_dot_idx];
    int core_len = core_end - core_start;
    if(core_len > 0 && core_len < 256) {
        memcpy(core, hostname + core_start, core_len);
        core[core_len] = '\0';
    }
    if(core_dot_idx >= 0){
        int sub_len = dots[core_dot_idx];
        if(sub_len > 0){
            if (sub_len < 256) {
                memcpy(sub, hostname, sub_len);
                sub[sub_len] = '\0';
            }
        }
    }
}
const char *stristr(const char *big, const char *sub){
    if (!big || !sub) return NULL;
    int slen = (int)strlen(sub);
    if (!slen) return big;
    init_table();
    for(; *big; big++){
        int i = 0;
        while(i < slen && big[i] && (s_lower[(unsigned char)big[i]] == s_lower[(unsigned char)sub[i]]))
            i++;
        if (i == slen) return big;
    }
    return NULL;
}
int has_keyword(const char *big, const char **kw) {
    if(!big || !kw) return 0;
    for(int i = 0; kw[i]; i++)
        if (stristr(big, kw[i])) return 1;
    return 0;
}
int is_ipv4(const char *h) {
    if(!h) return 0;
    int dots = 0, nums = 0, in_num = 0, val = 0;
    for(const char *c = h; *c; c++) {
        if(isdigit((unsigned char)*c)) {
            val = val * 10 + (*c - '0');
            if (val > 255) return 0;
            in_num = 1;
        } 
        else if(*c == '.') {
            if(!in_num) return 0;
            dots++; val = 0; in_num = 0; nums++;
        } 
        else return 0;
    }
    if (in_num) nums++;
    if(dots == 3 && nums == 4) return 1;
    return 0;
}
void str_tolower(char *s) {
    if (!s) return;
    init_table();
    for (; *s; s++) *s = (char)s_lower[(unsigned char)*s];
}