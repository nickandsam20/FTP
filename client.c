#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/file.h>


int get_file(int socket,char* filename,char *send_buffer){
    FILE *fp;
    char file_path[50] = "client_data/";
    char overwrite;
    char file_buffer[1024];
    int n;
    strcat(file_path,filename);
    if(access(file_path,F_OK) == 0){ // file is already exist , ask user want to overwrite it or not
        printf("%s is already exist do you want to overwrite it? [Y/N]\n",filename);
        while(1){
            scanf("%c",&overwrite);
            if(overwrite=='Y'||overwrite=='y'){
                break;
            }else if(overwrite=='N'||overwrite=='n'){
                return -1;
            }
        }
    }
    fp = fopen(file_path,"w");
    send(socket,send_buffer,sizeof(send_buffer),0);
    while(1){
        n = recv(socket,file_buffer,sizeof(file_buffer),0);
        if(n<=0){
            break;
        }
        fputs(file_buffer,fp);
        memset(file_buffer,'\0',sizeof(file_buffer));
    }
    return 0;
}

int main(){
    struct sockaddr_in server_addr; 
    int c_socket,connect_result,argc;
    char input[40],send_buffer[512],recv_buffer[512];
    char cmd[10],filename[20],argu[15];
    char username[20];
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
    printf("please Enter username :");
    scanf("%s",username);
    send(c_socket,username,sizeof(username),0); // send username to server
    while(1){
        memset(send_buffer,'\0',sizeof(send_buffer));
        memset(recv_buffer,'\0',sizeof(recv_buffer));
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
                if(get_file(c_socket,filename,send_buffer)==0){
                    recv(c_socket,recv_buffer,sizeof(recv_buffer),0);
                    printf("%s",recv_buffer);
                }
            }
            if(strcmp(cmd,"write")==0){ // write file
                printf("write file %s \n",filename);
                strcpy(send_buffer,input);
                
                recv(c_socket,recv_buffer,sizeof(recv_buffer),0);
                printf("%s",recv_buffer);
            }
        }else if(argc == 3){
            if(strcmp(cmd,"create")==0){ // create file
                printf("create file %s with premission %s\n",filename,argu);
                strcpy(send_buffer,input);
                send(c_socket,send_buffer,sizeof(send_buffer),0);
                recv(c_socket,recv_buffer,sizeof(recv_buffer),0);
                printf("%s",recv_buffer);
            }
            if(strcmp(cmd,"mode")==0){ // modify access right
                printf("modify %s premission to %s\n",filename,argu);
                strcpy(send_buffer,input);
                send(c_socket,send_buffer,sizeof(send_buffer),0);
                recv(c_socket,recv_buffer,sizeof(recv_buffer),0);
                printf("%s",recv_buffer);
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