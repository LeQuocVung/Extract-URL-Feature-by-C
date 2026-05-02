## LƯU Ý: đây không phải hướng dẫn của một môn học, đây là một PROJECT CÁ NHÂN phục vụ cho một PROJECT CÁ NHÂN KHÁC, độ chính xác chưa được các người có chuyên môn kiểm chứng. Nếu có thắc mắc hay đề xuất cải tiền mới, vui lòng liên hệ qua email ở dưới file README này!
## Người thực hiện: Lê Quốc Vững - School of Infomation and Communication Technology - Ha Noi University of Sience and Technology
## ĐÂY LÀ PHẦN TIẾP THEO SAU "PIPELINE URL DETECTION" 
1. Trình bày kết quả và đánh giá cá nhân:
    - nhằm phục vụ cho mục đích xây dựng web/app cảnh báo mã độc và thông báo cho user trên máy điện thoại/ máy tính CẤU HÌNH THẤP, tôi đã thực hiện phần xây dựng một model Machine Learning là XGBoost với ưu điểm tốc độ nhanh, nhẹ, không gây áp lực cho bộ nhớ. 
    - Ý tưởng của tôi: sử dụng C để tăng tốc độ và giảm bộ nhớ, nhưng nếu chạy chương trình 1 luồng thì chắc chắn không tối ưu được so với tính toán đa luồng của python, vì vậy tôi đã nghĩ đến trích xuất đa luồng và sử dụng cơ chế zero-copy (được giải thích trong file explain.md)
    - Ở phần này, tôi sẽ viết lại mã nguồn trích xuất bằng C nhằm giảm áp lực lên bộ nhớ và cải thiện tốc độ so với Python khi sử dụng các thư viện có sẵn. Máy tôi có 12 luồng, tôi sử dụng 6 luồng để tính toán, tránh xung đột về luồng.
    - Kết quả đạt được khá ấn tượng:
        - về tốc độ:
                + với 100 URL: tốc độ C nhanh hơn khoảng 21 lần (so sánh với python thuần)
                + với 1000 URL: tốc độ C nhanh hơn khoảng 50 lần (so sánh với python thuần)
                + với 10000 URL: tốc độ C nhanh hơn khoảng 17 lần (so sánh với py xử lý nhiều luồng)
                + với toàn bộ dataset đã xử lý ở phần trước (163408 URLs): C nhanh hơn 8 lần (py nhiều luồng) 
        - về độ chính xác: tôi đã thử nghiệm với toàn bộ dataset phần trước (163408 URLs):

                | Feature    | Dòng lệch | %     |
                | :---       | :---      | :---  |
                | Feature 0  | 301       | 0.18% |
                | Feature 2  | 8         | 0.00% |
                | Feature 3  | 301       | 0.18% |
                | Feature 4  | 248       | 0.15% |
                | Feature 5  | 161       | 0.10% |
                | Feature 6  | 530       | 0.32% |
                | Feature 7  | 20        | 0.01% |
                | Feature 9  | 530       | 0.32% |
                | Feature 10 | 117       | 0.07% |
                | Feature 11 | 276       | 0.17% |
                | Feature 12 | 301       | 0.18% |
                | Feature 13 | 275       | 0.17% |
                | Feature 14 | 14110     | 8.63% |
                | Feature 15 | 276       | 0.17% |
                | Feature 16 | 114       | 0.07% |
                | Feature 17 | 114       | 0.07% |
                | Feature 18 | 649       | 0.40% |
                | Feature 19 | 301       | 0.18% |
                | Feature 21 | 426       | 0.26% |
                | Feature 23 | 30        | 0.02% |
                | Feature 24 | 103       | 0.06% |
                | Feature 25 | 16        | 0.01% |
                | Feature 26 | 779       | 0.48% |
                | Feature 30 | 3         | 0.00% |
                | Feature 31 | 108       | 0.07% |
        - về bộ nhớ: chắc chắn là C sẽ dùng ít bộ nhớ hơn Python do Python cần khởi tạo thành các object khi nạp dữ liệu sẽ tốn nhiều bộ nhớ hơn
    - kết quả cho thấy: lỗi khá nhỏ, nhưng không đều, tập trung chủ yếu ở domain và host_name
    - Nguyên nhân: do tôi không sử dụng các thuật toán thông minh như trong python, tôi sử dụng tách core domain đơn giản (được trình bày trong phần mã nguồn và giải thích ở phần 4 của file này)
2. Cấu trúc thư mục:
    ```text
        EXTRACT URL FEATURES BY C             
        ├── sourceC/                 
        │   ├── include/ -chứa các file.h
        │   │   
        │   └──tools/ -chứa mã nguồn C
        ├── cmpWithPy/
        │   ├──feature_extract.py -mã nguồn lấy từ PIPELINE ở phần trước
        │   └──bench.py -mã nguồn benchmark 
        ├──requirements.txt -chứa các thư viện (bao gồm cả thư viện có sẵn) cần khai báo
        ├── README-vi.md 
        └── README-en.md            
    
    - tôi xin phép không public các file csv vì nặng, file csv ở đây là tôi đã tách chỉ còn cột url để so sánh
3. Cách sử dụng mã nguồn:
    - sau khi clone mã nguồn về, hãy biên dịch ra file dll hoặc file o (tôi không sử dụng các hàm chỉ có ở Linux và ngược lại, vì vậy sẽ không có lỗi xảy ra)
    - nếu bạn sử dụng Windows giống tôi hoặc muốn tạo dll thì có thể sử dụng câu lệnh sau:
    "gcc -O3 -shared -o extract_core.dll sourceC/tools/extract_features.c sourceC/tools/url_parser.c sourceC/tools/utils.c -I sourceC/include -fopenmp -lm -DBUILD_DLL"
4. Góc nhìn và quan điểm về tương lai:
    - mã nguồn trên được dựa trên logic của feature_extract.py để viết, do phần keyword và phần tách url không được đầy đủ như python với data khổng lồ, nhưng kết quả đạt được sau khi kiểm tra thấy sai số có thể chấp nhận được (dưới 10%) và feature core không đóng góp quá nhiều vào model.
    - dựa vào phần mã nguồn này, có thể ứng dụng làm các app hay web thời gian thực, hữu ích cho tương lai
    - mã nguồn có sử dụng AI để hỗ trợ, có thể sẽ gây khó chịu cho người đọc, nếu có ý kiến hay đóng góp, hãy liên hệ email phía dưới
    - ngoài ra mã nguồn này chưa được tối ưu hoàn toàn, chưa sử dụng các cấu trúc dữ liệu như hash hay trie có thể tăng tốc độ truy vấn các từ hoặc sẽ có nhiều phương pháp tối ưu hơn
    - nếu có đóng góp hoặc muốn sử dụng mã nguồn, vui lòng liên hệ qua email dưới đây
                                                            email: lequocvung111@gmail.com