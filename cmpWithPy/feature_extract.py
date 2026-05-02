import re
import math
import tldextract
from collections import Counter
from urllib.parse import urlparse, parse_qs
def calculate_entropy(s):
    if not s:
        return 0.0
    cnt = Counter(s)
    total = len(s)
    return -sum((c / total) * math.log2(c / total) for c in cnt.values())
def extract_36_features(url):
    original_url = url
    if "://" in url:
        parsed      = urlparse(url)
        ext         = tldextract.extract(url)
        hostname    = parsed.netloc or ""
        path        = parsed.path or ""
        query       = parsed.query or ""
        scheme      = parsed.scheme or ""
        has_port    = 1.0 if parsed.port is not None else 0.0
        is_https    = 1.0 if scheme == "https" else 0.0
        domain_full = hostname.lower()
        domain_core = ext.domain
    else:
        assumed_url = "http://" + url
        parsed      = urlparse(assumed_url)
        ext         = tldextract.extract(assumed_url)
        hostname    = parsed.netloc or ""
        path        = parsed.path or ""
        query       = parsed.query or ""
        scheme      = "http"
        has_port    = 1.0 if parsed.port is not None else 0.0
        is_https    = 0.0
        domain_full = hostname.lower()
        domain_core = ext.domain
    def count_digits(s): return sum(c.isdigit() for c in s)
    def count_letters(s): return sum(c.isalpha() for c in s)
    def has_kw(s, keywords): return 1.0 if any(kw in s.lower() for kw in keywords) else 0.0
    url_special_ratio = sum(not c.isalnum() for c in original_url) / len(original_url) if original_url else 0.0
    dot_count = original_url.count('.')
    domain_digit_ratio = count_digits(domain_full) / len(domain_full) if domain_full else 0.0
    url_letter_ratio = count_letters(original_url) / len(original_url) if original_url else 0.0
    url_digit_ratio = count_digits(original_url) / len(original_url) if original_url else 0.0
    query_param_count = len(parse_qs(query))
    path_length = len(path)
    path_parts = [p for p in path.split('/') if p]
    ext_str = ""
    if path_parts and '.' in path_parts[-1]:
        ext_str = path_parts[-1].rsplit('.', 1)[-1].lower()
    ext_is_exe = 1.0 if ext_str in {"exe","bat","cmd","scr","pif","vbs","sh","msi"} else 0.0
    DEFACEMENT_KW = re.compile(
        r"index\.php\?option=com_|/wp-content/|/wp-admin/|"
        r"joomla|drupal|wordpress|/administrator/|/modules/|"
        r"/components/|view=article|catid=|itemid="
    )
    has_defacement_kw = 1.0 if DEFACEMENT_KW.search(original_url.lower()) else 0.0
    path_entropy = calculate_entropy(path)
    path_hyphen_count = path.count('-')
    hostname_length = len(hostname)
    path_dot_count = path.count('.')
    path_depth = len(path_parts)
    domain_core_length = len(domain_core)
    domain_entropy = calculate_entropy(domain_full)
    query_entropy = calculate_entropy(query)
    query_length = len(query)
    MALWARE_KW = re.compile(
        r"\.exe|\.php\?|\.bat|\.cmd|\.scr|\.pif|\.vbs|\.js\b|"
        r"download|install|setup|payload|shell|sh4|dropper|rat\b|"
        r"bot\b|spam|cnc|c2\b|\.onion|porn|xxx|adult"
    )
    has_malware_kw = 1.0 if MALWARE_KW.search(original_url.lower()) else 0.0
    url_entropy = calculate_entropy(original_url)
    PHISH_KW = re.compile(
        r"login|signin|secure|account|update|verify|banking|paypal|ebay|"
        r"apple|amazon|confirm|password|credential|wallet|alert|suspend|"
        r"webscr|cmd=|cgi-bin|redirect|token|auth|validate|recover|restore"
    )
    has_phishing_kw = 1.0 if PHISH_KW.search(original_url.lower()) else 0.0
    path_digit_ratio = count_digits(path) / len(path) if path else 0.0
    dash_count = original_url.count('-')
    ext_is_doc = 1.0 if ext_str in {"pdf","doc","docx","xls","xlsx","txt","html","htm"} else 0.0
    query_digit_ratio = count_digits(query) / len(query) if query else 0.0
    ext_is_script = 1.0 if ext_str in {"php","asp","aspx","cgi","py","pl","rb","js"} else 0.0
    has_file_ext = 1.0 if ext_str else 0.0
    BENIGN_SIGNAL = re.compile(
        r"wikipedia|github|google|microsoft|youtube|stackoverflow|"
        r"reddit|twitter|linkedin|arxiv|mozilla|python\.org|npmjs"
    )
    has_benign_signal = 1.0 if BENIGN_SIGNAL.search(original_url.lower()) else 0.0
    underscore_count = original_url.count('_')
    tilde_count = original_url.count('~')
    path_underscore_count = path.count('_')
    domain_hyphen_count = domain_full.count('-')
    IP_RE = re.compile(r"^(?:\d{1,3}\.){3}\d{1,3}$")
    is_ip = 1.0 if (hostname and IP_RE.match(hostname.split(':')[0])) else 0.0
    SHORTENER_RE = re.compile(
        r"bit\.ly|tinyurl|goo\.gl|t\.co|ow\.ly|buff\.ly|"
        r"is\.gd|short\.io|rebrand\.ly|cutt\.ly|tiny\.cc|su\.pr"
    )
    is_shortener = 1.0 if (hostname and SHORTENER_RE.search(hostname)) else 0.0
    return [
        url_special_ratio, dot_count, domain_digit_ratio, url_letter_ratio,
        url_digit_ratio, query_param_count, path_length, ext_is_exe,
        has_defacement_kw, path_entropy, path_hyphen_count, hostname_length,
        path_dot_count, path_depth, domain_core_length, domain_entropy,
        query_entropy, query_length, has_malware_kw, url_entropy,
        has_phishing_kw, path_digit_ratio, dash_count, ext_is_doc,
        query_digit_ratio, ext_is_script, has_file_ext, has_benign_signal,
        underscore_count, tilde_count, path_underscore_count, domain_hyphen_count,
        is_ip, is_shortener, is_https, has_port
    ]