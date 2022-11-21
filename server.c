#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define access_right "server_data/access_table.txt"

int create_file(char *filename,char *permission){// file exist return 1 ; or return 0
    FILE *fp;
    char file_path[50] = "server_data/";
    strcat(file_path,filename);
    printf("file path :%s\n",file_path);
    if(access(filename,F_OK)==0){
        return 1;
    }else{
        fp = fopen(file_path ,"w");
        fclose(fp);
        fp = fopen(access_right , "a");
        fputs(filename,fp);
        fputs(" ",fp);
        fputs(permission,fp);
        fputs("\n",fp);
        fclose(fp);
        return 0;
    }
}

int check_permission(char *filename,int action){
    
}

int main(int argc ,char* argv[]){
    int sockfd,bind_result,new_socket;
    struct sockaddr_in addr,client_addr;
    pid_t child_p;
    char recv_buffer[512],send_buffer[512],cmd[10],filename[20],argu[10];

    socklen_t addr_len = sizeof(client_addr);
    if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        perror("open socket failed !\n");
        exit(1);
    }

    memset(&addr,'\0',sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1234);
    if((bind_result = bind(sockfd,(struct sockaddr*)&addr,sizeof(addr)))<0){//binding
        perror("bind failed !\n");
        exit(1);
    }

    if(listen(sockfd,6)<0){ //listening
        printf("error when listening! \n");
    }

    while(1){
        if((new_socket = accept(sockfd,(struct sockaddr*)&client_addr,&addr_len))<0){ // accept connection
            perror("error occurs when accept connection \n ");
            exit(1);
        }
        printf("connection accept from %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        if((child_p = fork())==0){ // child process do
            close(sockfd);//close child process`s sockfd
            while(1){// start to recv data
                recv(new_socket,recv_buffer,sizeof(recv_buffer),0);
                switch(recv_buffer[0]){
                    case 'c': //create file
                        sscanf(recv_buffer,"%s %s %s",cmd,filename,argu);
                        printf("get request to create %s! \n",filename);
                        if(create_file(filename,argu)==0){
                            strcpy(send_buffer,"file created success! \n");
                            send(new_socket,send_buffer,sizeof(send_buffer),0);
                        }else{
                            strcpy(send_buffer,"file is already exist! \n");
                            send(new_socket,send_buffer,sizeof(send_buffer),0);
                        }
                        break;
                    case 'r': // read file
                        break;
                    case 'w': // write file
                        break;
                    case 'm': // modify permission
                        break;
                    case ':': // client disconnected
                        printf("closed\n");
                        exit(1);
                        printf("I still alive\n");
                }
            }
        }
    }
    return 0;
}