# 期中報告 
修改自[cpu2os/02-軟體/02-編譯器/01-diy/03a-compiler/compiler.c](https://github.com/ccc111b/cpu2os/blob/master/02-%E8%BB%9F%E9%AB%94/02-%E7%B7%A8%E8%AD%AF%E5%99%A8/01-diy/03a-compiler/compiler.c)  

shell: PowerShell 7

--------

嘗試在老師的簡易編譯器中加入for迴圈的功能，但只是土砲模仿for迴圈的功能，沒有完全實踐成功。
ex:
```c
for(i = 0; i < 10; i = i + 1) {
  //something
}

// i 會在something執行前就 +1
```

--------

測試檔案: `for.c`
```c
j = 0;
for(i = 0; i < 10; i = i + 1) {
  j = j + 1;
}
```

## 執行結果
```
 ./compiler.exe for.c
j = 0;
for(i = 0; i < 10;i = i + 1) {
  j = j + 1;
}

========== lex ==============
token=j
token==
token=0
token=;
token=for
token=(
token=i
token==
token=0
token=;
token=i
token=<
token=10
token=;
token=i
token==
token=i
token=+
token=1
token=)
token={
token=j
token==
token=j
token=+
token=1
token=;
token=}
========== dump ==============
0:j
1:=
2:0
3:;
4:for
5:(
6:i
7:=
8:0
9:;
10:i
11:<
12:10
13:;
14:i
15:=
16:i
17:+
18:1
19:)
20:{
21:j
22:=
23:j
24:+
25:1
26:;
27:}
============ parse =============
t0 = 0
j = t0
t1 = 0
i = t1
(L0)
t2 = i
t3 = 10
t4 = t2 < t3
if not t4 goto L1
t5 = i
t6 = 1
t7 = t5 + t6
i = t7
t8 = j
t9 = 1
t10 = t8 + t9
j = t10
goto L0
(L1)

```
