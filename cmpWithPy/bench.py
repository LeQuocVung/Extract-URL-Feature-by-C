import os
import ctypes
import sys
import mmap
import time
import tracemalloc
import numpy as np
from multiprocessing import Pool, cpu_count
from feature_extract import extract_36_features
CURRENT_DIR  = os.path.dirname(os.path.abspath(__file__))
PARENT_DIR   = os.path.abspath(os.path.join(CURRENT_DIR, ".."))
DLL_PATH     = os.path.join(PARENT_DIR, "extract_core.dll") 
CSV_PATH     = os.path.join(PARENT_DIR, "url_only_dataset.csv") 
MINGW_BIN    = r"C:\msys64\ucrt64\bin"
NUM_FEATURES = 36
N_THREADS    = 6
if PARENT_DIR not in sys.path:
    sys.path.append(PARENT_DIR)
if sys.platform == "win32":
    if os.path.exists(MINGW_BIN):
        if hasattr(os, "add_dll_directory"):
            os.add_dll_directory(MINGW_BIN)
        os.environ["PATH"] = MINGW_BIN + os.pathsep + os.environ["PATH"]
    if hasattr(os, "add_dll_directory"):
        os.add_dll_directory(PARENT_DIR)
class StringView(ctypes.Structure):
    _fields_ = [("offset", ctypes.c_size_t), ("length", ctypes.c_size_t)]
lib = ctypes.CDLL(DLL_PATH, winmode=0)
lib.extract_batch.argtypes = [
    ctypes.c_char_p,
    ctypes.POINTER(StringView),
    ctypes.POINTER(ctypes.c_double),
    ctypes.c_int,
    ctypes.c_int,
]
lib.extract_batch.restype = None
class _PMC(ctypes.Structure):
    _fields_ = [
        ("cb",                         ctypes.c_ulong),
        ("PageFaultCount",             ctypes.c_ulong),
        ("PeakWorkingSetSize",         ctypes.c_size_t),
        ("WorkingSetSize",             ctypes.c_size_t),
        ("QuotaPeakPagedPoolUsage",    ctypes.c_size_t),
        ("QuotaPagedPoolUsage",        ctypes.c_size_t),
        ("QuotaNonPagedPoolUsage",     ctypes.c_size_t),
        ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
        ("PagefileUsage",              ctypes.c_size_t),
        ("PeakPagefileUsage",          ctypes.c_size_t),
    ]
def current_ram_mb():
    pmc = _PMC()
    pmc.cb = ctypes.sizeof(pmc)
    ctypes.windll.psapi.GetProcessMemoryInfo(
        ctypes.windll.kernel32.GetCurrentProcess(),
        ctypes.byref(pmc), pmc.cb
    )
    return pmc.WorkingSetSize / 1024 / 1024
def _mp_worker(chunk):
    """Chạy trong process con — mỗi process xử lý 1 chunk URLs"""
    return [extract_36_features(url) for url in chunk]
def _split_chunks(lst, n):
    """Chia list thành n chunk gần bằng nhau"""
    k, m = divmod(len(lst), n)
    return [lst[i*k + min(i,m):(i+1)*k + min(i+1,m)] for i in range(n)]
def bench_c(raw_ptr, c_views, check_num):
    out = np.zeros((check_num, NUM_FEATURES), dtype=np.float64)
    tracemalloc.start()
    ram_before = current_ram_mb()
    t = time.perf_counter()
    lib.extract_batch(
        raw_ptr, c_views,
        out.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
        check_num, N_THREADS,
    )
    elapsed = time.perf_counter() - t
    ram_after = current_ram_mb()
    _, peak_traced = tracemalloc.get_traced_memory()
    tracemalloc.stop()
    return elapsed, max(0, ram_after - ram_before), peak_traced / 1024 / 1024, out
def bench_py_single(urls):
    out = np.zeros((len(urls), NUM_FEATURES), dtype=np.float64)
    tracemalloc.start()
    ram_before = current_ram_mb()
    t = time.perf_counter()
    for i, url in enumerate(urls):
        out[i] = extract_36_features(url)
    elapsed = time.perf_counter() - t
    ram_after = current_ram_mb()
    _, peak_traced = tracemalloc.get_traced_memory()
    tracemalloc.stop()
    return elapsed, max(0, ram_after - ram_before), peak_traced / 1024 / 1024, out
def bench_py_multi(urls, n_proc):
    chunks  = _split_chunks(urls, n_proc)
    ram_before = current_ram_mb()
    t = time.perf_counter()
    # multiprocessing không dùng tracemalloc được (process riêng)
    with Pool(processes=n_proc) as pool:
        results = pool.map(_mp_worker, chunks)
    elapsed  = time.perf_counter() - t
    ram_after = current_ram_mb()
    # Ghép kết quả
    flat = [feat for chunk_res in results for feat in chunk_res]
    out  = np.array(flat, dtype=np.float64)
    return elapsed, max(0, ram_after - ram_before), None, out
def run_benchmark():
    if not os.path.exists(CSV_PATH):
        print(f"[LỖI] Không tìm thấy CSV: {CSV_PATH}")
        return
    with open(CSV_PATH, "rb") as f:
        mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
    buf_np  = np.frombuffer(mm, dtype=np.uint8)
    raw_ptr = ctypes.cast(buf_np.ctypes.data, ctypes.c_char_p)
    views_list, offset = [], 0
    mm.seek(0)
    for line in iter(mm.readline, b""):
        slen = len(line.rstrip(b"\r\n"))
        views_list.append(StringView(offset, slen))
        offset += len(line)
    check_num = min(len(views_list), 163_408)
    c_views   = (StringView * check_num)(*views_list[:check_num])
    mm.seek(0)
    urls = []
    for line in iter(mm.readline, b""):
        urls.append(line.decode("utf-8", errors="ignore").strip())
        if len(urls) == check_num:
            break
    print("Warm-up...")
    extract_36_features("http://warmup.example.com/path?q=1")
    _dummy = np.zeros((1, NUM_FEATURES), dtype=np.float64)
    lib.extract_batch(raw_ptr, c_views,
                      _dummy.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
                      1, 1)
    n_proc = min(N_THREADS, cpu_count())
    print(f"Benchmark {check_num:,} URLs | C threads: {N_THREADS} | Python processes: {n_proc}\n")
    print("Đang chạy C + OpenMP ...")
    t_c,  ram_c,  traced_c,  out_c  = bench_c(raw_ptr, c_views, check_num)
    print("Đang chạy Python single-thread ...")
    t_py, ram_py, traced_py, out_py = bench_py_single(urls)
    print("Đang chạy Python multiprocessing ...")
    t_mp, ram_mp, _,          out_mp = bench_py_multi(urls, n_proc)
    W = 85
    def row(label, c, py, mp):
        print(f"  {label:<32} | {str(c):<18} | {str(py):<18} | {str(mp):<18}")
    print("\n" + "=" * W)
    print(f"  {'Thông số':<32} | {'C + OpenMP':<18} | {'Python 1 thread':<18} | {f'Python {n_proc} process':<18}")
    print("-" * W)
    row("Thời gian (s)",
        f"{t_c:.4f}",
        f"{t_py:.4f}",
        f"{t_mp:.4f}")
    row("Tốc độ so với C",
        "1.0x (baseline)",
        f"{t_py/t_c:.1f}x chậm hơn",
        f"{t_mp/t_c:.1f}x chậm hơn")
    print("-" * W)
    row("RAM tăng thực tế (MB)",
        f"{ram_c:.2f}",
        f"{ram_py:.2f}",
        f"{ram_mp:.2f}")
    row("RAM Python objects peak (MB)",
        f"{traced_c:.3f}",
        f"{traced_py:.3f}",
        "N/A (process riêng)")
    row("RAM output numpy (MB)",
        f"{out_c.nbytes/1024/1024:.2f}",
        f"{out_py.nbytes/1024/1024:.2f}",
        f"{out_mp.nbytes/1024/1024:.2f}")
    print("=" * W)
    def analyse(label, out_ref, out_cmp):
        diff  = np.abs(out_ref[:len(out_cmp)] - out_cmp)
        wrong = np.sum(diff > 1e-4, axis=0)
        total_wrong = int(np.sum(wrong > 0))
        print(f"\n  [Sai số: C vs {label}]  —  {total_wrong} feature(s) có lệch")
        print(f"  {'Feature':<10} | {'Dòng lệch':<12} | {'%':<8}")
        print("  " + "-" * 35)
        found = False
        for i, cnt in enumerate(wrong):
            if cnt > 0:
                print(f"  Feature {i:<3} | {int(cnt):<12} | {cnt/check_num*100:.2f}%")
                found = True
        if not found:
            print("  => Khớp hoàn toàn!")
    analyse("Python",   out_c, out_py)
    del raw_ptr
    del buf_np
    del c_views
    mm.close()
if __name__ == "__main__":
    run_benchmark()