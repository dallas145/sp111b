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
