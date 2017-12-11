#include <iostream>
#include <libssh2.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>


LIBSSH2_SESSION * session;
LIBSSH2_CHANNEL * channel;

/**
 * ssh参数
 */
struct ssh_config{
    unsigned int port;
    std::string username;
    std::string password;
    std::string address;
    std::string sh_file;
    std::string command;
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
        exit(EXIT_FAILURE);
    }

    if(opt.password.empty()){
        std::cout <<start << "[!]Password is empty "<<end << std::endl;
        exit(EXIT_FAILURE);
    }

    if(opt.address.empty()){
        std::cout << start <<"[!]Host address is empty "<<end << std::endl;
        exit(EXIT_FAILURE);
    }

}


/**
 * 输出帮助信息
 * @param program_name
 */
void help(char * program_name){
    std::cout << "Usage: " << program_name << " -t <Address> -u <Username> -p <Password> -e <Bash_file>"<<std::endl<<std::endl;
    std::cout
            <<"\t-t \t<Address>\tSSH主机地址"<<std::endl
            <<"\t-u \t<Username>\tSSH登录用户名"<<std::endl
            <<"\t-p \t<Password>\tSSH登录密码"<<std::endl
            <<"\t-e \t<Bash_file>\tSSH执行脚本"<<std::endl
            <<"\t-s \t<Port>\tSSH端口(默认22)"<<std::endl
            <<"\t-c \t<Command>\t执行命令"<<std::endl
            <<"\t-h \t<Help>\t输出帮助信息"<<std::endl<<std::endl;
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

    char result[500] ="";
    std::string commandRes="";
    while(libssh2_channel_read(channel,result,100) > 0){
        commandRes+=result;
    }
    std::cout << commandRes << std::endl;
}




int main(int argc,char * argv[]) {
    int opt,sock,rc;
    std::string start("\033[41;36m");
    std::string end("\033[0m");
    struct ssh_config sshConfig;
    sshConfig.port = 22;

    std::string options("t:u:p:e:s:c:h::");

    struct sockaddr_in ServerSock;

    rc = libssh2_init(0);

    if(rc !=0){
        fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
        return 1;
    }

    /**
     * 处理参数
     */
    while((opt = getopt(argc,argv,options.data()))!=-1){
        switch(opt){
            case 't':
                sshConfig.address  = optarg;
                break;
            case 'c':
                sshConfig.command  = optarg;
                break;
            case 'u':
                sshConfig.username = optarg;
                break;
            case 'p':
                sshConfig.password = optarg;
                break;
            case 'e':
                sshConfig.command = get_bash_file(optarg);
                break;
            case 's':
                sshConfig.port = atoi(optarg);
                break;
            default:
                help(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    /**
     * 处理错误
     */
    error_handle(sshConfig);


    if(optind < 8){
        help(argv[0]);
        exit(EXIT_FAILURE);
    }
/*
    std::cout << "Username : " << sshConfig.username
              << " Password :" << sshConfig.password
              << " Address : " << sshConfig.address
              << " Bash file : " << sshConfig.sh_file
              << " Port : " << sshConfig.port
              << std::endl;
*/
    ServerSock.sin_addr.s_addr = inet_addr(sshConfig.address.c_str());
    ServerSock.sin_family = AF_INET;
    ServerSock.sin_port = htons(sshConfig.port);

    sock = socket(AF_INET,SOCK_STREAM,0);

    /**
     * 连接目标主机
     */
    if(connect(sock,(sockaddr *)&ServerSock,sizeof(ServerSock)) == -1){
        std::cout << start << "[!]Can't Connect to " << inet_ntoa(ServerSock.sin_addr) << end << std::endl;
        exit(EXIT_FAILURE);
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

    if (libssh2_userauth_password(session, sshConfig.username.c_str(), sshConfig.password.c_str())) {
        fprintf(stderr, "Authentication by password failed.\n");
        exit(0);
    }

    channel = libssh2_channel_open_session(session);

    ssh_exec(sock,sshConfig);



    libssh2_channel_free(channel);
    libssh2_session_disconnect(session,"Bye");
    libssh2_session_free(session);

    return 0;
}