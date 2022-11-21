#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    struct sockaddr_in server_addr; 
    int c_socket,connect_result,argc;
    char input[40],send_buffer[512],recv_buffer[512];
    char cmd[10],filename[20],argu[10];
    if((c_socket = socket(AF_INET,SOCK_STREAM,0))<0){
        perror("open socket failed !\n");
        exit(1);
    }
    memset(&server_addr,'\0',sizeof(server_addr));
    
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1234);

    if((connect_result = connect(c_socket,(struct socketaddr*)&server_addr,sizeof(server_addr)))<0){
        perror("connect to server failed !\n");
        exit(1);
    }
    while(1){
        fgets(input,sizeof(input),stdin);
        argc = sscanf(input,"%s %s %s",cmd,filename,argu);
        if(argc == 1){
            if(strcmp(cmd,":exit")==0){ // disconnect
                strcpy(send_buffer,input);
                send(c_socket,send_buffer,sizeof(send_buffer),0);
                exit(1);
            }
        }else if(argc == 2){
            if(strcmp(cmd,"read")==0){ // read file
                printf("read file %s \n",filename);
                strcpy(send_buffer,input);
                send(c_socket,send_buffer,sizeof(send_buffer),0);
            }
        }else if(argc == 3){
            if(strcmp(cmd,"create")==0){ // create file
                printf("create file %s with premission %s\n",filename,argu);
                strcpy(send_buffer,input);
                send(c_socket,send_buffer,sizeof(send_buffer),0);
                recv(c_socket,recv_buffer,sizeof(recv_buffer),0);
                printf("%s\n",recv_buffer);
            }
        }else{
            printf("invalid command\n");
        }
        memset(cmd,'\0',sizeof(cmd));
        memset(filename,'\0',sizeof(filename));
        memset(argu,'\0',sizeof(argu));
    }
    return 0;
}