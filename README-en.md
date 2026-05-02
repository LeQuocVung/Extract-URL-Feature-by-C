## NOTE: This is not a course tutorial; this is a PERSONAL PROJECT intended to support another PERSONAL PROJECT. The accuracy has not been verified by professionals. If you have any questions or suggestions for improvement, please contact me via the email listed at the bottom of this README!
## Author: Le Quoc Vung - School of Information and Communication Technology - Hanoi University of Science and Technology
## THIS IS THE CONTINUATION OF "PIPELINE URL DETECTION"
1. Results and Personal Evaluation:
    - To serve the goal of building a malware warning web/app and notifying users on low-configuration mobile/computer devices, I implemented an XGBoost Machine Learning model. Its advantages include high speed, being lightweight, and not putting pressure on memory
    - My idea: Use C to increase speed and reduce memory usage. However, running a single-threaded program is certainly not as optimal as Python's multi-threaded computation. Therefore, I considered multi-threaded extraction and utilized a zero-copy mechanism (explained in explain.md)
    - In this section, I rewritten the extraction source code in C to reduce memory pressure and improve speed compared to Python when using available libraries. My machine has 12 threads; I used 6 threads for computation to avoid thread conflicts.The results achieved are quite impressive:
        - Speed:
            + With 100 URLs: C is approximately 21 times faster (compared to pure Python)
            + With 1000 URLs: C is approximately 50 times faster (compared to pure Python).
            + With 10,000 URLs: C is approximately 17 times faster (compared to multi-threaded Python)
            + With the entire processed dataset (163,408 URLs): C is 8 times faster (compared to multi-threaded Python)
        - Accuracy: I tested it with the entire dataset from the previous part (163,408 URLs):
                
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
        - Memory: C undoubtedly uses less memory than Python, as Python needs to initialize objects when loading data, which consumes significantly more memory.
    - The results show that the error is quite small but unevenly distributed, concentrated mainly in `domain` and `host_name`.
    - Reason: Since I do not use intelligent algorithms as in Python, I used a simple core domain separation (presented in the source code and explained in Section 4 of this file).
2. Directory Structure:
    ```text
        EXTRACT URL FEATURES BY C             
        ├── sourceC/                 
        │   ├── include/ - contains .h files
        │   └── tools/ - contains C source code
        ├── cmpWithPy/
        │   ├── feature_extract.py - source code taken from the previous PIPELINE
        │   └── bench.py - benchmark source code 
        ├── requirements.txt - contains required libraries (including built-in ones)
        ├── README-vi.md 
        └── README-en.md
        
    - I apologize for not making the CSV files public as they are heavy; the CSV files here have been stripped to only the URL column for comparison.
3. How to Use the Source Code:
    - After cloning the source code, please compile it into a .dll or .o file (I did not use Linux-only functions or vice-versa, so no errors should occur).
    - If you are using Windows like me or want to create a DLL, you can use the following command:"gcc -O3 -shared -o extract_core.dll sourceC/tools/extract_features.c sourceC/tools/url_parser.c sourceC/tools/utils.c -I sourceC/include -fopenmp -lm -DBUILD_DLL"
4. Perspectives and Future Outlook:
    - The above source code is based on the logic of feature_extract.py. Because the keyword and URL separation parts are not as comprehensive as Python's with massive data, the results show an acceptable margin of error (under 10%), and the core features do not contribute too much to the model.
    - Based on this source code, it can be applied to real-time apps or websites, which is useful for the future.
    -The source code used AI for support, which might be uncomfortable for some readers. 
    - Additionally, this source code is not fully optimized; data structures like Hash or Trie have not been used to speed up word queries, and there may be many other optimization methods.
    - If you have contributions, please contact me via the email below.                                                          
                                                    email: lequocvung111@gmail.com