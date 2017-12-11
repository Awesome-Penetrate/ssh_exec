# ssh_exec

## install 

Ubuntu/Kali linux :

* `apt-get install libssh2-1-dev`
* `make`
* `make install`


## usage

```
Usage: ./ssh_exec -t <Address> -u <Username> -p <Password> -e <Bash_file>

	-t 	<Address>	SSH主机地址
	-u 	<Username>	SSH登录用户名
	-p 	<Password>	SSH登录密码
	-e 	<Bash_file>	SSH执行脚本
	-s 	<Port>	SSH端口(默认22)
	-c 	<Command>	执行命令
	-h 	<Help>	输出帮助信息

```

## about

Koons Tools |  payloads@aliyun.com

