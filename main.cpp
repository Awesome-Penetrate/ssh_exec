#include <iostream>
#include <libssh2.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>
#include <vector>
#include <stdio.h>


LIBSSH2_SESSION * session;
LIBSSH2_CHANNEL * channel;
int rc;

/**
 * ssh参数
 */
struct ssh_config{
    unsigned int port;
    std::string username;
    std::string password;
    std::string address;
    std::string hosts_file;
    std::string command;
};

/**
 * 多台主机
 */
struct hosts{
    int sock;
    unsigned int port;
    std::string username;
    std::string password;
    std::string address;
};

/**
 * 处理错误
 * @param opt
 */
void error_handle(struct ssh_config & opt){
    std::string start("\033[41;36m");

    std::string end("\033[0m");

    if(opt.username.empty()){
        std::cout <<start<<"[!]Username is empty "<<end << std::endl;
    }

    if(opt.password.empty()){
        std::cout <<start << "[!]Password is empty "<<end << std::endl;
    }

    if(opt.address.empty()){
        std::cout << start <<"[!]Host address is empty "<<end << std::endl;
    }

}

/**
 * 加载主机文件
 * @param hosts_file
 * @return
 * 格式：[IP] : [用户名] : [密码] : [端口]
 */
std::vector<struct hosts> loadhosts(std::string hosts_file){
    std::vector<struct hosts> remote_hosts;

    std::fstream IO(hosts_file);
    if(!IO.is_open()){
        std::cout << "Can't Read Hosts file :" << hosts_file << std::endl;
        exit(EXIT_FAILURE);
    }

    while(!IO.eof()){

        struct hosts tmp;

        char  address[100],port[100],username[100],password[100];

        std::string line;

        getline(IO,line);

        sscanf(line.c_str(),"%s - %s - %s - %s",address,username,password,port);

        tmp.address = address;

        tmp.username = username;

        tmp.password = password;

        tmp.port = atoi(port);

        remote_hosts.push_back(tmp);
    }
    IO.close();
    // std::cout << bash_connect << std::endl;
    return remote_hosts;
}


/**
 * 输出帮助信息
 * @param program_name
 */
void help(char * program_name){
    std::cout << "Usage: " << program_name << " -t <Address> -u <Username> -p <Password> -e <Bash_file>"<<std::endl<<std::endl;
    std::cout << program_name << " -e <Bash Script> -l <Hosts File>"<<std::endl<< std::endl;
    std::cout
            <<"\t-t \t<Address>\tSSH主机地址"<<std::endl
            <<"\t-u \t<Username>\tSSH登录用户名"<<std::endl
            <<"\t-p \t<Password>\tSSH登录密码"<<std::endl
            <<"\t-e \t<Bash_file>\tSSH执行脚本"<<std::endl
            <<"\t-s \t<Port>\tSSH端口(默认22)"<<std::endl
            <<"\t-c \t<Command>\t执行命令"<<std::endl
            <<"\t-l \t<Hosts File>\t多台主机文件"<<std::endl
            <<"\t-h \t<Help>\t输出帮助信息"<<std::endl
            <<"\t-v \t<Version>\t输出版本信息"<<std::endl<<std::endl;
}


/**
 * 读取bash file
 * @param filename
 * @return
 */
std::string get_bash_file(std::string filename){
    std::fstream IO(filename);
    std::string bash_connect;
    if(!IO.is_open()){
        std::cout << "Can't Read Bash file :" << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    while(!IO.eof()){
        std::string line;
        getline(IO,line);
        bash_connect+="\n"+line;
    }
    IO.close();
    // std::cout << bash_connect << std::endl;
    return bash_connect;
}


/**
 * 执行命令
 * @param sock
 * @param opt
 */
void ssh_exec(int sock,struct ssh_config & opt){

    libssh2_channel_exec(channel,opt.command.c_str());

    char result[1024] ="";
    std::string commandRes="";
    while(libssh2_channel_read(channel,result,1000) > 0){
        std::cout << result;
        commandRes+=result;
    }
    std::cout << std::endl;
}



/**
 * 连接SSH
 * @param host
 * @return
 */
int connect_ssh(struct hosts & host){
    int sock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in ser_sock;
    ser_sock.sin_addr.s_addr = inet_addr(host.address.c_str());
    ser_sock.sin_port = htons(host.port);
    ser_sock.sin_family = AF_INET;
    if(connect(sock,(sockaddr *)&ser_sock,sizeof(ser_sock)) == -1){
        std::cout <<"[!]Can't Connect to " << inet_ntoa(ser_sock.sin_addr) << std::endl;
        return -1;
    }
    session = libssh2_session_init();

    if(!session){
        return -1;
    }

    rc = libssh2_session_handshake(session,sock);

    if(rc) {
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
        return -1;
    }

    if (libssh2_userauth_password(session, host.username.c_str(), host.password.c_str())) {
        fprintf(stderr, "Authentication by password failed.\n");
        return -1;
    }

    channel = libssh2_channel_open_session(session);
    return sock;
}

/**
 * 主函数
 * @param argc
 * @param argv
 * @return
 */
int main(int argc,char * argv[]) {
    int opt; // 用于匹配参数

    // 多台主机结构体
    std::vector<struct hosts> hosts;

    // 输出彩色～
    std::string start("\033[41;36m");
    std::string end("\033[0m");

    // 单台主机SSH配置
    struct ssh_config sshConfig;

    // 默认端口 22
    sshConfig.port = 22;

    // 参数序列
    std::string options("t:u:p:l:e:s:r:c:h::v::");

    //初始化libssh2库
    rc = libssh2_init(0);

    if(rc !=0){
        fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
        return 1;
    }
/*
    /**
     * 处理参数
     */
    while((opt = getopt(argc,argv,options.data()))!=-1){
        switch(opt){
                // 加载bash脚本
            case 'e':
                sshConfig.command = get_bash_file(optarg);
                break;
                // 从配置文件读取多个主机
            case 'l':
                sshConfig.hosts_file  = optarg;
                break;
                // 设置单个主机
            case 't':
                sshConfig.address  = optarg;
                break;
                // 设置执行命令
            case 'c':
                sshConfig.command  = optarg;
                break;
                // 设置用户名
            case 'u':
                sshConfig.username = optarg;
                break;
                // 设置密码
            case 'p':
                sshConfig.password = optarg;
                break;
                // 设置端口
            case 's':
                sshConfig.port = atoi(optarg);
                break;
                // 输出版本信息
            case 'v':
                std::cout << "[*]Version : 0.01 By payloads@aliyun.com" << std::endl;
                exit(EXIT_FAILURE);
                // 默认输出帮助信息
            default:
                help(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /**
     * 如果没有参数则输出帮助信息
     */
// std::cout << optind << std::endl;
    if(optind < 5 ){
        help(argv[0]);
        exit(EXIT_FAILURE);
    }

    /**
     * 处理错误
     */
    error_handle(sshConfig);

    /**
     * 批量模式
     */
    if(!sshConfig.hosts_file.empty()){

        /**
         * 获取多台主机
         */
        hosts = loadhosts(sshConfig.hosts_file);
        for (int i = 0; i < hosts.size(); ++i) {
            // 循环获取每个主机的socket套接字
            int sock = connect_ssh(hosts[i]);
            if(sock != -1){
                // 执行
                ssh_exec(sock,sshConfig);
            }
        }

    }else{

        /**
         * 单任务模式
         */
        struct hosts host;

        sshConfig.username.swap(host.username); //用户名

        sshConfig.address.swap(host.address); //IP地址

        host.port = sshConfig.port; //端口

        sshConfig.password.swap(host.password); //密码

        // 获取套接字
        int sock = connect_ssh(host);

        if(sock != -1){
            // 执行命令
            ssh_exec(sock,sshConfig);
        }

    }

    /**
     * 释放资源
     */
    libssh2_channel_free(channel);
    libssh2_session_disconnect(session,"Bye");
    libssh2_session_free(session);


    return 0;
}