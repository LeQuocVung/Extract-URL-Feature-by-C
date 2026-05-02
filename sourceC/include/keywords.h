#ifndef KEYWORDS_H
#define KEYWORDS_H
#include<stddef.h>
static const char *EXT_EXE[] = {
    "exe","bat","cmd","scr","pif","vbs","sh","msi", NULL
};
static const char *EXT_DOC[] = {
    "pdf","doc","docx","xls","xlsx","txt","html","htm", NULL
};
static const char *EXT_SCRIPT[] = {
    "php","asp","aspx","cgi","py","pl","rb","js", NULL
};
static const char *KW_PHISHING[] = {
    "login","signin","secure","account","update","verify",
    "banking","paypal","ebay","apple","amazon","confirm",
    "password","credential","wallet","alert","suspend",
    "webscr","cmd=","cgi-bin","redirect","token","auth",
    "validate","recover","restore",
    NULL
};
static const char *KW_MALWARE[] = {
    ".exe",".php?",".bat",".cmd",".scr",".pif",".vbs",
    "download","install","setup","payload","shell","sh4",
    "dropper","spam","cnc",".onion","porn","xxx","adult",
    NULL
};
static const char *KW_MALWARE_WORD[] = {
    "rat","bot","c2","js", NULL
};
static const char *KW_DEFACEMENT[] = {
    "index.php?option=com_","/wp-content/","/wp-admin/",
    "joomla","drupal","wordpress","/administrator/",
    "/modules/","/components/","view=article",
    "catid=","itemid=",
    NULL
};
static const char *KW_BENIGN[] = {
    "wikipedia","github","google","microsoft","youtube",
    "stackoverflow","reddit","twitter","linkedin","arxiv",
    "mozilla","python.org","npmjs",
    NULL
};
static const char *KW_SHORTENER[] = {
    "bit.ly","tinyurl","goo.gl","t.co","ow.ly","buff.ly",
    "is.gd","short.io","rebrand.ly","cutt.ly","tiny.cc","su.pr",
    NULL
};
static const char *MULTI_PART_TLD[] = {
    "com.vn","net.vn","org.vn","edu.vn","gov.vn","int.vn",
    "ac.vn","biz.vn","info.vn","name.vn","pro.vn","health.vn",
    "co.uk","org.uk","me.uk","net.uk","ltd.uk","plc.uk",
    "ac.uk","gov.uk","sch.uk","nhs.uk","police.uk",
    "com.au","net.au","org.au","edu.au","gov.au","asn.au","id.au",
    "co.jp","ne.jp","or.jp","ac.jp","go.jp","ed.jp","gr.jp",
    "co.kr","ne.kr","or.kr","go.kr","ac.kr","re.kr","pe.kr",
    "com.cn","net.cn","org.cn","gov.cn","edu.cn","ac.cn",
    "com.br","net.br","org.br","gov.br","edu.br","mil.br",
    "co.in","net.in","org.in","gen.in","firm.in","ind.in","res.in",
    "co.nz","net.nz","org.nz","govt.nz","ac.nz","geek.nz",
    "co.za","net.za","org.za","gov.za","ac.za","mil.za",
    "com.sg","com.my","com.ph","com.hk","com.tw",
    "edu.sg","edu.my","edu.ph","edu.hk","edu.tw",
    "gov.sg","gov.my","gov.ph","gov.hk",
    "net.sg","net.my","net.ph",
    "com.ar","com.mx","com.pe","com.co","com.ec","com.cl",
    "com.uy","com.bo","com.py","com.ve","com.gt","com.cu",
    "com.eg","com.ng","com.gh","com.sa","com.ae","com.pk",
    "com.tr","com.ua","com.pl","com.ro",
    NULL
};
#endif 