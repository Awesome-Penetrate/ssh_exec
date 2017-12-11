# ssh_exec

## 安装 

Ubuntu/Kali linux :

* `apt-get install libssh2-1-dev`
* `make`


## 使用方法

```

Usage: ./ssh_exec -t <Address> -u <Username> -p <Password> -e <Bash_file>

       ./ssh_exec -e <Bash Script> -l <Hosts File>


	-t 	<Address>	SSH主机地址
	-u 	<Username>	SSH登录用户名
	-p 	<Password>	SSH登录密码
	-e 	<Bash_file>	SSH执行脚本
	-s 	<Port>	SSH端口(默认22)
	-c 	<Command>	执行命令
	-l 	<Hosts File>	多台主机文件
	-h 	<Help>	输出帮助信息
	-v 	<Version>	输出版本信息
```
## 使用过程

### 单台主机执行

```bash
payloads@koone:~/CLionProjects/untitled6/ssh_exec$ ./ssh_exec -t 118.***.200.***  -u root -p *** -c "netstat -at" -s 5432
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 localhost:mysql         *:*                     LISTEN     
tcp        0      0 *:http                  *:*                     LISTEN     
tcp        0      0 *:postgresql            *:*                     LISTEN     
tcp        0      0 localhost:5433          *:*                     LISTEN     
tcp        0    112 *:postgresql *:49666     ESTABLISHED
tcp6       0      0 [::]:http               [::]:*                  LISTEN     
tcp6       0      0 [::]:8089               [::]:*                  LISTEN     
```

### 多台主机执行

```
payloads@koone:~$ cat hosts.ini 
127.0.0.1 - payloads - *** - 22
127.0.0.1 - payloads - *** - 22
payloads@koone:~$ cat test.sh 
#!/bin/bash
echo hello
payloads@koone:~$ ./ssh_exec -e ./test.sh -l hosts.ini
[!]Username is empty 
[!]Password is empty 
[!]Host address is empty 
hello

hello
```

## 多台主机格式

`IP - 用户名 - 密码 - 端口`

例如：127.0.0.1 - root - root - 22



## 关于

Koons Tools |  payloads@aliyun.com

