# 習題2
修改自[cpu2os/02-軟體/02-編譯器/01-diy/03a-compiler/compiler.c](https://github.com/ccc111b/cpu2os/blob/master/02-%E8%BB%9F%E9%AB%94/02-%E7%B7%A8%E8%AD%AF%E5%99%A8/01-diy/03a-compiler/compiler.c)  
--------  

測試檔案: `dowhile.c`
```
i = 1;
do {
	i = i + 1;
} while ( i < 10 );
```

## 執行結果
```
neil@B660M12400:/mnt/c/Users/mikel/Desktop/College/cpu2os/Mine/sp111b/ex2$ ./compiler dowhile.c 
i = 1;
do {
        i = i + 1;
} while ( i < 10 );

========== lex ==============
token=i
token==
token=1
token=;
token=do
token={
token=i
token==
token=i
token=+
token=1
token=;
token=}
token=while
token=(
token=i
token=<
token=10
token=)
token=;
========== dump ==============
0:i
1:=
2:1
3:;
4:do
5:{
6:i
7:=
8:i
9:+
10:1
11:;
12:}
13:while
14:(
15:i
16:<
17:10
18:)
19:;
============ parse =============
t0 = 1
i = t0
(L0)
t1 = i
t2 = 1
t3 = t1 + t2
i = t3
t4 = i
t5 = 10
t6 = t4 < t5
if not t6 goto L1
goto L0
(L1)
```