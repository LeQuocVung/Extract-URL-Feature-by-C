#ifndef EXTRACT_FEATURES_H
#define EXTRACT_FEATURES_H
#ifdef __cplusplus
extern "C"{
#endif
#include<stddef.h>
#include"keywords.h"
#include"url_parser.h"
#include"utils.h"
#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef BUILD_DLL
        #define DLL_EXPORT __declspec(dllexport)
    #elif defined(USE_DLL)
        #define DLL_EXPORT __declspec(dllimport)
    #else
        #define DLL_EXPORT
    #endif
#else
    #define DLL_EXPORT
#endif
#define NUM_FEATURES 36
typedef struct {
    double url_special_ratio;
    double dot_count;
    double domain_digit_ratio;
    double url_letter_ratio;
    double url_digit_ratio;
    double query_param_count;
    double path_length;
    double ext_is_exe;
    double has_defacement_kw;
    double path_entropy;
    double path_hyphen_count;
    double hostname_length;
    double path_dot_count;
    double path_depth;
    double domain_core_length;
    double domain_entropy;
    double query_entropy;
    double query_length;
    double has_malware_kw;
    double url_entropy;
    double has_phishing_kw;
    double path_digit_ratio;
    double dash_count;
    double ext_is_doc;
    double query_digit_ratio;
    double ext_is_script;
    double has_file_ext;
    double has_benign_signal;
    double underscore_count;
    double tilde_count;
    double path_underscore_count;
    double domain_hyphen_count;
    double is_ip;
    double is_shortener;
    double is_https;
    double has_port;
} UrlFeatures;
typedef struct {
    size_t offset;
    size_t length;
} StringView;
DLL_EXPORT int extract(const char *url, UrlFeatures *res);
DLL_EXPORT int features_to_array(const UrlFeatures *f, double *arr, int n);
DLL_EXPORT void extract_batch(const char *raw_buffer, const StringView *views, double *res, int count, int n_threads);
#ifdef __cplusplus
}
#endif
#endif