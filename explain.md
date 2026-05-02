# sử dụng kỹ thuật zero-copy để truyền dữ liệu từ Python sang C mà không tạo bản sao trong bộ nhớ. Toàn bộ 163,408 URL được xử lý trên một vùng nhớ duy nhất, được chia sẻ giữa Python runtime, numpy, ctypes và thư viện C. Thay vì đọc toàn bộ file vào RAM bằng f.read(), hệ thống sử dụng mmap để ánh xạ file trực tiếp vào không gian địa chỉ ảo của tiến trình. Cơ chế này hoạt động như sau:
 ```text:
        ┌─────────────────────────────────────────────────────┐
        │                    Ổ đĩa (Disk)                     │
        │  [url1\nurl2\nurl3\n...]   ← file CSV               │
        └──────────────────┬──────────────────────────────────┘
                           │ OS ánh xạ
                           ▼
        ┌─────────────────────────────────────────────────────┐
        │                RAM (Virtual Address Space)          │
        │  Địa chỉ 0x7F00: [url1\nurl2\nurl3\n...]            │
        │                   ↑                                 │
        │                   mm                                │
        └─────────────────────────────────────────────────────┘
    
- np.frombuffer tạo một numpy array trỏ thẳng vào vùng nhớ của mmap, không tạo bản sao:
    
        RAM:  [url1\nurl2\nurl3\n...]
        ↑
        Địa chỉ 0x7F00
        │
        ├── mm.data      → 0x7F00  
        └── buf_np.data  → 0x7F00  
    
- ctypes.cast chuyển địa chỉ số nguyên thành kiểu const char* tạo ra 1 object chỉ chứa địa chỉ 8 byte:
    
        RAM:  [url1\nurl2\nurl3\n...]
        ↑
        Địa chỉ 0x7F00
        │
        ├── mm.data          → 0x7F00
        ├── buf_np.data      → 0x7F00
        └── raw_ptr (c_char_p) → 0x7F00  ← truyền cho C
    
- C chỉ lưu vị trí và độ dài của URL trong một vùng nhớ chung
    
        Vùng nhớ:  [https://ex.com\nhttp://evil.com\n...]
                    ↑               ↑
                    offset=0        offset=15
                    length=14       length=14
        views[0] = {offset: 0,  length: 14}
        views[1] = {offset: 15, length: 14}
        ...
    
- lý do tại sao cần memcpy vào một buffer là url_safe nữa là vì: khi ta lưu trong một vùng nhớ, ta lưu không thể có kí tự '\0' giữa 2 url, vì vậy ta cầm memcpy vào buffer và thêm '\0' vào cuối buffer để sử dụng. Điều này không tốn bộ nhớ vì buffer chỉ được gọi 1 lần và không có cấp phát động nên tự đồng giải phóng sau mỗi lần lặp
- sơ đồ luồng dữ liệu:
    
        ┌──────────────────────────────────────────────────────────────┐
        │                        File CSV (Disk)                       │
        └──────────────────────────┬───────────────────────────────────┘
                                │ mmap() — OS ánh xạ, không copy
                                ▼
        ┌──────────────────────────────────────────────────────────────┐
        │              Vùng nhớ RAM duy nhất (địa chỉ 0x7F00)          │
        │                                                              │
        │  [url1\0url2\0url3\0 ... url163408\0]                        │
        │   ↑                                                          │
        │   │   Python layer:                                          │
        │   ├── mm          (mmap object)     — quản lý vòng đời       │
        │   ├── buf_np.data (numpy array)     — không copy             │
        │   └── raw_ptr     (ctypes c_char_p) — địa chỉ truyền cho C   │
        │                                                              │
        │   C layer (OpenMP 6 threads):                                │
        │   ├── Thread 0: raw_buffer + views[0].offset → url_safe[0]   │
        │   ├── Thread 1: raw_buffer + views[1].offset → url_safe[1]   │
        │   ├── Thread 2: raw_buffer + views[2].offset → url_safe[2]   │
        │   └── ...                                                    │
        └──────────────────────────────────────────────────────────────┘
                                │ features_to_array()
                                ▼
        ┌──────────────────────────────────────────────────────────────┐
        │         out_c: np.zeros((163408, 36), dtype=float64)         │
        │         → Ma trận kết quả 163,408 × 36 đặc trưng             │
        └──────────────────────────────────────────────────────────────┘