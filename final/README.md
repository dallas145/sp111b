# 期末報告

## 本程式非原創

這份報告全部程式碼皆來自[lotabout/write-a-C-interpreter][githubpage]  

僅作為練習用途。  

參考文章：[手把手教你構建C語言編譯器][1]  

## 研究目標
想藉由跟著文章教學，一步一步寫出C語言編譯器的方式來進一步了解C語言。

-----
目前進度：  
- [x] [手把手教你構建C語言編譯器（1）][2]
- [x] [手把手教你構建C語言編譯器（2）][3]
- [x] [手把手教你構建C語言編譯器（3）][4]
- [x] [手把手教你構建C語言編譯器（4）][5]
- [x] [手把手教你構建C語言編譯器（5）][6]
- [x] [手把手教你構建C語言編譯器（6）][7]
- [x] [手把手教你構建C語言編譯器（7）][8]
- [x] [手把手教你構建C語言編譯器（8）][9]

## 研究心得
會選這個專案進行研究的原因，是在搜尋研究主題時，找到這個專案，發現它也是 inspire by c4 ，而且作者好像解釋得淺顯易懂、步調好像也不快，就把主題訂了下來。  
沒想到我的C語言基礎比我想得還薄弱，還沒到一半我就覺得看不懂了，只能硬著頭皮照著文章的步驟把程式碼完成。  
完成程式碼後又經歷了痛苦又漫長的debug，才得以成功執行，但還是沒辦法自我編譯。
結論：C語言遠比我所想得還複雜。

-----

程式碼在`ch8/compiler.c`  

### 執行結果

Shell: `PowerShell 7.3.4`

------
```
> make
gcc -std=c99 -O0 -w compiler.c -o compiler

> ls

        Directory: C:\Users\mikel\Desktop\College\cccclass\sp111b\mid\ch8


Mode                LastWriteTime         Length Name
----                -------------         ------ ----
-a---        2023/6/5  下午 01:57          28314 ﭰ  compiler.c
-a---        2023/6/5  下午 03:20          76789 ﬓ  compiler.exe
-a---        2023/6/5  上午 12:39            274 ﭰ  hello.c
-a---        2023/6/4  下午 07:38            166   Makefile
-a---        2023/6/5  下午 03:19              0   README.md
-a---        2023/6/5  上午 12:36             79 ﭰ  test.c

> ./compiler.exe test.c
Hello World!
exit(0)

> ./compiler.exe hello.c
fibonacci( 0) = 1
fibonacci( 1) = 1
fibonacci( 2) = 2
fibonacci( 3) = 3
fibonacci( 4) = 5
fibonacci( 5) = 8
fibonacci( 6) = 13
fibonacci( 7) = 21
fibonacci( 8) = 34
fibonacci( 9) = 55
fibonacci(10) = 89
exit(0)

>
```

螢幕截圖：
![](https://github.com/dallas145/sp111b/blob/main/final/screenshot/20230605final.png?raw=true)

-----
[1]:https://lotabout.me/2015/write-a-C-interpreter-0/
[2]:https://lotabout.me/2015/write-a-C-interpreter-1/
[3]:https://lotabout.me/2015/write-a-C-interpreter-2/
[4]:https://lotabout.me/2015/write-a-C-interpreter-3/
[5]:https://lotabout.me/2016/write-a-C-interpreter-4/
[6]:https://lotabout.me/2016/write-a-C-interpreter-5/
[7]:https://lotabout.me/2016/write-a-C-interpreter-6/
[8]:https://lotabout.me/2016/write-a-C-interpreter-7/
[9]:https://lotabout.me/2016/write-a-C-interpreter-8/
[10]:https://lotabout.me/2016/write-a-C-interpreter-9/
[githubpage]:https://github.com/lotabout/write-a-C-interpreter/tree/master
