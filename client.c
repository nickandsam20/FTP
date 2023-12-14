#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>

int get_file(int socket, char* filename) {
  FILE* fp;
  char file_path[50] = "client_data/";
  char file_buffer[1024];
  char recv_buffer[512];
  int n;
  strcat(file_path, filename);
  recv(socket, recv_buffer, sizeof(recv_buffer),
       0);  // wait for server reply if user can read or not
  if (strcmp(recv_buffer, "read file success! \n") != 0) {
    printf("%s", recv_buffer);
    return -1;
  }
  fp = fopen(file_path, "w");
  while (1) {
    recv(socket, file_buffer, sizeof(file_buffer), 0);
    if (strcmp(file_buffer, "EOF") == 0) {
      break;
    }
    fprintf(fp, "%s", file_buffer);
    memset(file_buffer, '\0', sizeof(file_buffer));
  }
  fclose(fp);
  return 0;
}
int ul_file(int socket, char* filename) {
  FILE* fp;
  char file_path[50] = "client_data/";
  char line[1024];
  char recv_buffer[512];
  strcat(file_path, filename);
  recv(socket, recv_buffer, sizeof(recv_buffer),
       0);  // wait for server reply if user can ul or not
  if (strcmp(recv_buffer, "write file success! \n") != 0) {
    printf("%s", recv_buffer);
    return -1;
  }
  fp = fopen(file_path, "r");
  while (fgets(line, sizeof(line), fp) != NULL) {  // start to upload the file
    if (send(socket, line, sizeof(line), 0) == -1) {
      perror("Error occurs when sending file to client\n");
      exit(1);
    }
    memset(line, '\0', sizeof(line));
  }
  memset(line, '\0', sizeof(line));
  strcpy(line,
         "EOF");  // send to EOF to let server know file is transmit finished
  sleep(5);
  send(socket, line, sizeof(line), 0);
  fclose(fp);
  return 0;
}

int check_file_exist(char* filename, int action) {
  FILE* fp;
  char overwrite;
  char file_path[50] = "client_data/";
  strcat(file_path, filename);
  if (action == 1) {  // check file exist for write
    if (access(file_path, F_OK) == 0)
      return 0;
    else {
      return -1;
    }
  }
  if (access(file_path, F_OK) ==
      0) {  // file is already exist , ask user want to overwrite it or not
    printf("%s is already exist do you want to overwrite it? [Y/N]\n",
           filename);
    while (1) {
      scanf("%c", &overwrite);
      if (overwrite == 'Y' || overwrite == 'y') {
        return 0;
      } else if (overwrite == 'N' || overwrite == 'n') {
        return -1;
      }
    }
  }
  return 0;
}

int main() {
  struct sockaddr_in server_addr;
  int c_socket, connect_result, argc;
  char input[40], send_buffer[512], recv_buffer[512];
  char cmd[10], filename[20], argu[15];
  char username[20];
  if ((c_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("open socket failed !\n");
    exit(1);
  }
  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(1234);

  if ((connect_result = connect(c_socket, (struct socketaddr*)&server_addr,
                                sizeof(server_addr))) < 0) {
    perror("connect to server failed !\n");
    exit(1);
  }
  printf("please Enter username :");
  scanf("%s", username);
  send(c_socket, username, sizeof(username), 0);  // send username to server
  printf("usage : operator filename [permission/argument]\n");
  while (1) {
    memset(send_buffer, '\0', sizeof(send_buffer));
    memset(recv_buffer, '\0', sizeof(recv_buffer));
    fgets(input, sizeof(input), stdin);
    argc = sscanf(input, "%s %s %s", cmd, filename, argu);
    if (argc == 1) {
      if (strcmp(cmd, ":exit") == 0) {  // disconnect
        strcpy(send_buffer, input);
        send(c_socket, send_buffer, sizeof(send_buffer), 0);
        exit(1);
      }
    } else if (argc == 2) {
      if (strcmp(cmd, "read") == 0) {  // read file
        if (check_file_exist(filename, 0) == 0) {
          strcpy(send_buffer, input);
          send(c_socket, send_buffer, sizeof(send_buffer),
               0);  // send read request
          if (get_file(c_socket, filename) == 0) {
            printf("file read finished!\n");
          }
        }
      }
    } else if (argc == 3) {
      if (strcmp(cmd, "write") == 0) {  // write file
        if (strcmp(argu, "o") == 0 || strcmp(argu, "a") == 0) {
          if (check_file_exist(filename, 1) == 0) {
            strcpy(send_buffer, input);
            send(c_socket, send_buffer, sizeof(send_buffer),
                 0);  // send write request
            if (ul_file(c_socket, filename) == 0) {
              printf("file write finished!\n");
            }
          } else {
            printf("doesnt have %s\n", filename);
          }
        } else {
          printf("usage : write [filename] [w/a]\n");
        }
      }
      if (strcmp(cmd, "create") == 0) {  // create file
        printf("create file %s with premission %s\n", filename, argu);
        strcpy(send_buffer, input);
        send(c_socket, send_buffer, sizeof(send_buffer), 0);
        recv(c_socket, recv_buffer, sizeof(recv_buffer), 0);
        printf("%s", recv_buffer);
      }
      if (strcmp(cmd, "changemode") == 0) {  // modify access right
        printf("modify %s premission to %s\n", filename, argu);
        sprintf(send_buffer, "mode %s %s", filename, argu);
        // strcpy(send_buffer, input);
        send(c_socket, send_buffer, sizeof(send_buffer), 0);
        recv(c_socket, recv_buffer, sizeof(recv_buffer), 0);
        printf("%s", recv_buffer);
      }
    }
    memset(cmd, '\0', sizeof(cmd));
    memset(filename, '\0', sizeof(filename));
    memset(argu, '\0', sizeof(argu));
  }
  return 0;
}