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

#define access_right "server_data/access_table.txt"
#define group_member_path "server_data/group.txt"

int create_file(char *filename ,char *permission ,char *group ,char *owner){// file exist return 1 ; or return 0
    FILE *fp;
    char file_path[50] = "server_data/file/";
    strcat(file_path,filename);
    //printf("file path :%s\n",file_path);
    if(access(file_path,F_OK)==0){
        return 1;
    }else{ // write to access table
        fp = fopen(file_path ,"w");
        fclose(fp);
        fp = fopen(access_right , "a");
        fputs(filename,fp);
        fputs(" ",fp);
        fputs(permission,fp);
        fputs(" ",fp);
        fputs(owner,fp);
        fputs(" ",fp);
        fputs(group,fp);
        fputs("\n",fp);
        fclose(fp);
        return 0;
    }
}

void search_group(char *username, char *group){
    FILE *fp;
    char line[30],member[20],g[10];
    int exist = 0;
    fp = fopen(group_member_path,"r");
    while(fgets(line,sizeof(line),fp)!=NULL){
        sscanf(line,"%s %s",member,g);
        if(strcmp(member,username)==0){
            exist = 1;
            strcpy(group,g);
        }
    }
    if(exist==0){
        strcmp(group,"None");
    }
}

int modify_access_right(char *filename,char *permission,char *username){
    FILE *fp,*ftmp;
    char line[80],f_owner[20],f_name[20],f_permission[15],f_group[10];
    char buffer[80];
    int exist=0;
    fp = fopen(access_right,"r");
    while(fgets(line,sizeof(line),fp)!=NULL){
        sscanf(line,"%s %s %s %s",f_name,f_permission,f_owner,f_group);
        if(strcmp(f_name,filename)==0){ // find the file
            exist = 1;
            if(strcmp(username,f_owner)==0){ // if user is owner
                ftmp = fopen("server_data/replace.tmp","w");   
                sprintf(buffer,"%s %s %s %s\n",f_name,permission,f_owner,f_group);
                fputs(buffer,ftmp);
            }else{
                return -1;
            }
        }else{
            fputs(line,ftmp);
        }
    }
    remove(access_right);
    rename("server_data/replace.tmp",access_right);
    fclose(fp);
    fclose(ftmp);
    if(exist==0){
        return -2;
    }else{
        return 0;
    }
}
void read_file(char *filename,char *username,char *group,int socket){
    int p;
    FILE *fp;
    char file_path[50] = "server_data/file/";
    char line[1024];
    char send_buffer[512];
    p = check_permission(filename,0,group,username);
    if(p == -1){
        strcpy(send_buffer,"file isn`t exist! \n");
        send(socket,send_buffer,sizeof(send_buffer),0);
        return;
    }else if(p == -2){
        strcpy(send_buffer,"you don`t have permission to read this file! \n");
        send(socket,send_buffer,sizeof(send_buffer),0);
        return;
    }
    if(p==0){
        //printf("allow to read\n");
        strcat(file_path,filename);
        fp = fopen(file_path,"r");
        if(flock(fp->_fileno,LOCK_SH|LOCK_NB)==0){//lock the file
            strcpy(send_buffer,"read file success! \n");
            send(socket,send_buffer,sizeof(send_buffer),0);
            printf("sending %s to %s !\n",filename,username);
            sleep(1);
            while(fgets(line,sizeof(line),fp)!=NULL){
                if(send(socket,line,sizeof(line),0)==-1){
                    perror("Error occurs when sending file to client\n");
                    exit(1);
                }
                memset(line,'\0',sizeof(line));
            }
            sleep(5);
            fclose(fp);
            flock(fp->_fileno,LOCK_UN);
            memset(send_buffer,'\0',sizeof(send_buffer));
            strcpy(send_buffer,"EOF");
            send(socket,send_buffer,sizeof(send_buffer),0);
        }else{
            strcpy(send_buffer,"somebody is writing/reading this file,please try again later! \n");
            send(socket,send_buffer,sizeof(send_buffer),0);
            printf("somebody is writing this file!\n");
        }
    }
}

void write_file(char *filename,char *username,char *group,int socket,char* mode){
    int p,finish;
    FILE *fp;
    char send_buffer[512];
    char recv_buffer[1024];
    char file_path[50] = "server_data/file/";
    p = check_permission(filename,1,group,username);
    if(p == -1){
        strcpy(send_buffer,"file isn`t exist! \n");
        send(socket,send_buffer,sizeof(send_buffer),0);
        return;
    }else if(p == -2){
        strcpy(send_buffer,"you don`t have permission to read this file! \n");
        send(socket,send_buffer,sizeof(send_buffer),0);
        return;
    }
    if(p==0){
        strcat(file_path,filename);
        fp = fopen(file_path,mode);
        if(flock(fp->_fileno,LOCK_EX|LOCK_NB)==0){//lock the file
            strcpy(send_buffer,"write file success! \n");
            send(socket,send_buffer,sizeof(send_buffer),0);//reply can it is ok to send file
            while(1){
                recv(socket,recv_buffer,sizeof(recv_buffer),0);
                if(strcmp(recv_buffer,"EOF")==0){
                    break;
                }
                fputs(recv_buffer,fp);
                memset(recv_buffer,'\0',sizeof(recv_buffer));
            }
            fclose(fp);
            flock(fp->_fileno,LOCK_UN);
        }else{
            strcpy(send_buffer,"somebody is writing/reading this file\n");
            send(socket,send_buffer,sizeof(send_buffer),0);
        }
    }
}

int check_permission(char *filename,int action,char *group, char *username){ //action 0 read ; 1 write
    FILE *fp;
    int exist = 0;
    char line[80],f_owner[20],f_name[20],f_permission[15],f_group[10];
    fp = fopen(access_right,"r");
    while(fgets(line,sizeof(line),fp)!=NULL){
        sscanf(line,"%s %s %s %s",f_name,f_permission,f_owner,f_group);
        if(strcmp(f_name,filename)==0){
            exist = 1;
            if(action==0){ // check read premission
                if(f_permission[6]=='r') // check if everyone can read or not
                    return 0;
                if(strcmp(f_group,group)==0){ // if he/she is group member
                    if(f_permission[3]=='r')
                        return 0;
                }
                if(strcmp(f_owner,username)==0){
                    if(f_permission[0]=='r')
                        return 0;
                }
                return -2;// doesnt have permission to read
            }else{ //write
                if(f_permission[7]=='w') // check if everyone can write or not
                    return 0;
                if(strcmp(f_group,group)==0){ // if he/she is group member
                    if(f_permission[4]=='w')
                        return 0;
                }
                if(strcmp(f_owner,username)==0){
                    if(f_permission[1]=='w')
                        return 0;
                }
                return -2;// doesnt have permission to write
            }
        }
    }
    if(exist==0){
        return -1;
    }
}

int main(int argc ,char* argv[]){
    int sockfd,bind_result,new_socket;
    int read_result;
    struct sockaddr_in addr,client_addr;
    pid_t child_p;
    char recv_buffer[512],send_buffer[512],cmd[10],filename[20],argu[15];

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
            char username[20],group[20];
            close(sockfd);//close child process`s sockfd
            recv(new_socket,recv_buffer,sizeof(recv_buffer),0);
            strcpy(username,recv_buffer); //get client username
            search_group(username,group); //find user`s group
            printf("find %s is belong %s group! \n",username,group);
            while(1){// start to recv data
                memset(recv_buffer,'\0',sizeof(recv_buffer));
                memset(send_buffer,'\0',sizeof(send_buffer));
                recv(new_socket,recv_buffer,sizeof(recv_buffer),0);
                switch(recv_buffer[0]){
                    case 'c': //create file
                        sscanf(recv_buffer,"%s %s %s",cmd,filename,argu);
                        printf("get request to create %s by %s! \n",filename,username);
                        if(create_file(filename,argu,group,username)==0){
                            strcpy(send_buffer,"file created success! \n");
                            send(new_socket,send_buffer,sizeof(send_buffer),0);
                        }else{
                            strcpy(send_buffer,"file is already exist! \n");
                            send(new_socket,send_buffer,sizeof(send_buffer),0);
                        }
                        break;
                    case 'r': // read file
                        sscanf(recv_buffer,"%s %s",cmd,filename);
                        printf("get a read %s request from %s\n",filename,username);
                        read_file(filename,username,group,new_socket);
                        break;
                    case 'w': // write file
                        sscanf(recv_buffer,"%s %s %s",cmd,filename,argu);
                        printf("get a write %s request from %s\n",filename,username);
                        write_file(filename,username,group,new_socket,argu);
                        break;
                    case 'm': // modify permission
                        sscanf(recv_buffer,"%s %s %s",cmd,filename,argu);
                        int modify_result = modify_access_right(filename,argu,username);
                        if(modify_result==0){
                            strcpy(send_buffer,"access right was modified successfully! \n");
                            send(new_socket,send_buffer,sizeof(send_buffer),0);
                        }else if(modify_result==-1){
                            strcpy(send_buffer,"you don`t have permission to modify access right of this file! \n");
                            send(new_socket,send_buffer,sizeof(send_buffer),0);
                        }else if(modify_result==-2){
                            strcpy(send_buffer,"this file isn`t exist! \n");
                            send(new_socket,send_buffer,sizeof(send_buffer),0);
                        }
                        break;
                    case ':': // client disconnected
                        printf("disconnect from %s\n",username);
                        exit(1);
                        printf("I still alive\n");
                }
            }
        }
    }
    return 0;
}