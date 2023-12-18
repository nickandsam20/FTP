#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define access_right "server_data/access_table.txt"
#define group_right "server_data/group/"
#define user_right "server_data/user/"
#define all_right "server_data/all/all.txt"
#define group_member_path "server_data/group.txt"

int fileExists(const char *fname) {
  FILE *fp;
  if (fp = fopen(fname, "r")) {
    fclose(fp);
    return 1;
  }
  return 0;
}

int create_file(char *filename, char *permission, char *group,
                char *owner) {  // file exist return 1 ; or return 0
  FILE *fp;
  char file_path[50] = "server_data/file/";
  strcat(file_path, filename);

  if (fileExists(file_path)) return 1;

  printf("[create file]user group:%s, permission:%s, file name:%s\n", group,
         permission, filename);

  // char u_p[4]="rwx", g_p[4]="rwx", a_p[4]="rwx";
  char u_p[4], g_p[4], a_p[4];
  memcpy(u_p, &permission[0], 3);
  memcpy(g_p, &permission[3], 3);
  memcpy(a_p, &permission[6], 3);

  // for user permission
  char p_name[100] = "";
  char content[100];
  sprintf(p_name, "%s%s.txt", user_right, owner);
  printf("[create file]writing permission to file %s\n", p_name);
  fp = fopen(p_name, "a");
  if (!fp) {
    printf("[create file]open file 1 error\n");
  } else {
    u_p[3] = '\0';
    sprintf(content, "%s %s\n", filename, u_p);
    fputs(content, fp);
    fclose(fp);
  }

  // for group permission
  sprintf(p_name, "%s%s.txt", group_right, group);
  printf("[create file]writing permission to file %s\n", p_name);
  fp = fopen(p_name, "a");
  if (!fp) {
    printf("[create file]open file 2 error\n");
  } else {
    g_p[3] = '\0';
    sprintf(content, "%s %s\n", filename, g_p);
    fputs(content, fp);
    fclose(fp);
  }

  // for other permission
  sprintf(p_name, "%s", all_right);
  printf("[create file]writing permission to file %s\n", p_name);
  fp = fopen(p_name, "a");
  if (!fp) {
    printf("[create file]open file 3 error\n");
  } else {
    a_p[3] = '\0';
    sprintf(content, "%s %s\n", filename, a_p);
    fputs(content, fp);
    fclose(fp);
  }

  fp = fopen(file_path, "w");
  fclose(fp);

  return 0;
}
void search_group(char *username, char *group) {
  FILE *fp;
  char line[30], member[20], g[10];
  int exist = 0;
  fp = fopen(group_member_path, "r");
  while (fgets(line, sizeof(line), fp) != NULL) {
    sscanf(line, "%s %s", member, g);
    if (strcmp(member, username) == 0) {
      exist = 1;
      strcpy(group, g);
    }
  }
  if (exist == 0) {
    strcmp(group, "None");
  }
  fclose(fp);
}

int modify_access_right(char *filename, char *permission, char *username,
                        char *group) {
  FILE *fp, *ftmp;
  char line[80], f_owner[20], f_name[20], f_permission[15], f_group[10];
  char buffer[80];
  int exist = 0;  // 用exist代表是否這個username是檔案擁有者(owner)
  char p_name[100], p_tmp_name[100];

  printf(
      "[modify] start modify access right,filename:%s,permission:%s,group:%s\n",
      filename, permission, group);
  sprintf(p_name, "%s%s.txt", user_right, username);
  sprintf(p_tmp_name, "%s%stmp.txt", user_right, username);

  fp = fopen(p_name, "r");
  ftmp = fopen(p_tmp_name, "w");
  while (fgets(line, sizeof(line), fp) != NULL) {
    sscanf(line, "%s %s", f_name, f_permission);
    if (strcmp(f_name, filename) == 0) {  // find the file
      exist = 1;
      char new_line[50];
      sprintf(new_line, "%s %c%c%c", f_name, permission[0], permission[1],
              permission[2]);
      fputs(new_line, ftmp);
    } else {
      fputs(line, ftmp);
    }
    memset(line, '\0', sizeof(line));
  }

  // 檢查是不是owner,不是owner代表沒有權限修改權限,就直接return不做權限修改
  if (!exist) {
    printf("[modify] you are not file owner,can't change mode:%s\n", p_name);
    fclose(fp);
    fclose(ftmp);
    remove(p_tmp_name);
    return -1;
  }
  fclose(fp);
  fclose(ftmp);
  remove(p_name);
  rename(p_tmp_name, p_name);
  printf("[modify] finish modify user file:%s\n", p_name);

  sprintf(p_name, "%s%s.txt", group_right, group);
  sprintf(p_tmp_name, "%s%stmp.txt", user_right, group);

  fp = fopen(p_name, "r");
  ftmp = fopen(p_tmp_name, "w");
  while (fgets(line, sizeof(line), fp) != NULL) {
    sscanf(line, "%s %s", f_name, f_permission);
    if (strcmp(f_name, filename) == 0) {  // find the file
      char new_line[50];
      sprintf(new_line, "%s %c%c%c", f_name, permission[3], permission[4],
              permission[5]);
      fputs(new_line, ftmp);
    } else {
      fputs(line, ftmp);
    }
    memset(line, '\0', sizeof(line));
  }
  fclose(fp);
  fclose(ftmp);
  remove(p_name);
  rename(p_tmp_name, p_name);
  printf("[modify] finish modify group file:%s\n", p_name);

  sprintf(p_name, "%s", all_right);
  sprintf(p_tmp_name, "server_data/all/all_right_tmp.txt");

  fp = fopen(p_name, "r");
  ftmp = fopen(p_tmp_name, "w");
  while (fgets(line, sizeof(line), fp) != NULL) {
    sscanf(line, "%s %s", f_name, f_permission);
    if (strcmp(f_name, filename) == 0) {  // find the file
      char new_line[50];
      sprintf(new_line, "%s %c%c%c", f_name, permission[6], permission[7],
              permission[8]);
      fputs(new_line, ftmp);
    } else {
      fputs(line, ftmp);
    }
    memset(line, '\0', sizeof(line));
  }

  fclose(fp);
  fclose(ftmp);
  remove(p_name);
  rename(p_tmp_name, p_name);
  printf("[modify] finish modify all file:%s\n", p_name);
  return 0;
}
void read_file(char *filename, char *username, char *group, int socket) {
  int p;
  FILE *fp;
  char file_path[50] = "server_data/file/";
  char line[1024];
  char send_buffer[512];
  p = check_permission(filename, 0, group, username);
  if (p == -1) {
    strcpy(send_buffer, "file isn`t exist! \n");
    send(socket, send_buffer, sizeof(send_buffer), 0);
    return;
  } else if (p == -2) {
    strcpy(send_buffer, "you don`t have permission to read this file! \n");
    send(socket, send_buffer, sizeof(send_buffer), 0);
    return;
  }
  if (p == 0) {
    // printf("allow to read\n");
    strcat(file_path, filename);
    fp = fopen(file_path, "r");
    if (flock(fp->_fileno, LOCK_SH | LOCK_NB) == 0) {  // lock the file
      strcpy(send_buffer, "read file success! \n");
      send(socket, send_buffer, sizeof(send_buffer), 0);
      printf("sending %s to %s !\n", filename, username);
      sleep(1);
      while (fgets(line, sizeof(line), fp) != NULL) {
        if (send(socket, line, sizeof(line), 0) == -1) {
          perror("Error occurs when sending file to client\n");
          exit(1);
        }
        memset(line, '\0', sizeof(line));
      }
      sleep(5);
      // fclose(fp);
      flock(fp->_fileno, LOCK_UN);
      memset(send_buffer, '\0', sizeof(send_buffer));
      strcpy(send_buffer, "EOF");
      send(socket, send_buffer, sizeof(send_buffer), 0);
      fclose(fp);
    } else {
      strcpy(send_buffer,
             "somebody is writing this file,please try again later! \n");
      send(socket, send_buffer, sizeof(send_buffer), 0);
      printf("somebody is writing this file!\n");
    }
  }
}

void write_file(char *filename, char *username, char *group, int socket,
                char *mode) {
  int p, finish;
  FILE *fp;
  char send_buffer[512];
  char recv_buffer[1024];
  char file_path[50] = "server_data/file/";
  p = check_permission(filename, 1, group, username);
  if (p == -1) {
    strcpy(send_buffer, "file isn`t exist! \n");
    send(socket, send_buffer, sizeof(send_buffer), 0);
    return;
  } else if (p == -2) {
    strcpy(send_buffer, "you don`t have permission to read this file! \n");
    send(socket, send_buffer, sizeof(send_buffer), 0);
    return;
  }
  if (p == 0) {
    strcat(file_path, filename);
    fp = fopen(file_path, mode);
    if (flock(fp->_fileno, LOCK_EX | LOCK_NB) == 0) {  // lock the file
      strcpy(send_buffer, "write file success! \n");
      send(socket, send_buffer, sizeof(send_buffer),
           0);  // reply can it is ok to send file
      while (1) {
        recv(socket, recv_buffer, sizeof(recv_buffer), 0);
        if (strcmp(recv_buffer, "EOF") == 0) {
          break;
        }
        fputs(recv_buffer, fp);
        memset(recv_buffer, '\0', sizeof(recv_buffer));
      }
      fclose(fp);
      flock(fp->_fileno, LOCK_UN);
    } else {
      strcpy(send_buffer, "somebody is writing/reading this file\n");
      send(socket, send_buffer, sizeof(send_buffer), 0);
    }
  }
}

// return 0:有權限,-1:沒有這個檔案,-2:沒有權限
int check_permission(char *filename, int action, char *group,
                     char *username) {  // action 0 read ; 1 write
  char tmp_path[100];
  sprintf(tmp_path, "server_data/file/%s.txt", filename);
  printf("file path:%s\n", tmp_path);
  if (!fileExists(tmp_path)) {
    printf("[check_permission] file not exist\n");
    return -1;
  }
  FILE *fp;

  int exist = 0;

  char line[80], f_owner[20], f_name[20], f_permission[15], f_group[10];

  char p_file_name[100];
  printf(
      "[check_permission] start checking "
      "permission,filename:%s,action:%d,group:%s,username:%s\n",
      filename, action, group, username);
  // check whether user is file owner ,and check by owner permission
  sprintf(p_file_name, "%s%s.txt", user_right, username);
  printf("[check_permission]checking file:%s\n", p_file_name);
  fp = fopen(p_file_name, "r");
  if (fp) {
    while (fgets(line, sizeof(line), fp) != NULL) {
      sscanf(line, "%s %s", f_name, f_permission);
      printf("%s permission: %s-%s\n", username, f_name, f_permission);
      if (strcmp(f_name, filename) == 0) {
        exist = 1;
        if (action == 0) {  // read operation
          if (f_permission[0] == 'r') {
            printf("[check_permission] enable by user permission\n");
            return 0;
          }
        }

        if (action == 1) {
          if (f_permission[1] == 'w') {
            printf("[check_permission] enable by user permission\n");
            return 0;
          }
        }
        break;
      }
    }
    fclose(fp);
  } else {
    printf(
        "[check_permission]don't have perrmision file,pass checking user "
        "permission\n");
  }

  // check whether the group user belong to has the right to access
  sprintf(p_file_name, "%s%s.txt", group_right, group);
  printf("[check_permission]checking file:%s\n", p_file_name);
  fp = fopen(p_file_name, "r");
  while (fgets(line, sizeof(line), fp) != NULL) {
    sscanf(line, "%s %s", f_name, f_permission);
    printf("%s permission: %s-%s\n", group, f_name, f_permission);
    if (strcmp(f_name, filename) == 0) {
      if (action == 0) {  // read operation
        if (f_permission[0] == 'r') {
          printf("[check_permission] enable by group permission\n");
          return 0;
        }
      }

      if (action == 1) {
        if (f_permission[1] == 'w') {
          printf("[check_permission] enable by group permission\n");
          return 0;
        }
      }
      break;
    }
  }
  fclose(fp);

  // 如果是檔案擁有者,就不能檢查other權限
  if (exist) {
    printf(
        "[check_permission] is file owner but dusable by owner and group "
        "permission\n");
    return -2;
  }

  sprintf(p_file_name, "%s", all_right);
  printf("[check_permission]checking file:%s\n", p_file_name);
  fp = fopen(p_file_name, "r");
  while (fgets(line, sizeof(line), fp) != NULL) {
    sscanf(line, "%s %s", f_name, f_permission);
    printf("%s permission: %s-%s\n", "all", f_name, f_permission);
    if (strcmp(f_name, filename) == 0) {
      if (action == 0) {  // read operation
        if (f_permission[0] == 'r') {
          printf("[check_permission] enable by all permission\n");
          return 0;
        }
      }

      if (action == 1) {
        if (f_permission[1] == 'w') {
          printf("[check_permission] enable by all permission\n");
          return 0;
        }
      }
      break;
    }
  }
  fclose(fp);

  return -2;
}

int main(int argc, char *argv[]) {
  int sockfd, bind_result, new_socket;
  int read_result;
  struct sockaddr_in addr, client_addr;
  pid_t child_p;
  char recv_buffer[512], send_buffer[512], cmd[10], filename[20], argu[15];

  socklen_t addr_len = sizeof(client_addr);
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("open socket failed !\n");
    exit(1);
  }

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(1234);
  if ((bind_result = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) <
      0) {  // binding
    perror("bind failed !\n");
    exit(1);
  }

  if (listen(sockfd, 6) < 0) {  // listening
    printf("error when listening! \n");
  }

  while (1) {
    if ((new_socket = accept(sockfd, (struct sockaddr *)&client_addr,
                             &addr_len)) < 0) {  // accept connection
      perror("error occurs when accept connection \n ");
      exit(1);
    }
    printf("connection accept from %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
    if ((child_p = fork()) == 0) {  // child process do
      char username[20], group[20];
      close(sockfd);  // close child process`s sockfd
      recv(new_socket, recv_buffer, sizeof(recv_buffer), 0);
      strcpy(username, recv_buffer);  // get client username
      search_group(username, group);  // find user`s group
      printf("find %s is belong %s group! \n", username, group);
      while (1) {  // start to recv data
        memset(recv_buffer, '\0', sizeof(recv_buffer));
        memset(send_buffer, '\0', sizeof(send_buffer));
        recv(new_socket, recv_buffer, sizeof(recv_buffer), 0);
        printf("recv:%s\n", recv_buffer);
        switch (recv_buffer[0]) {
          case 'c':  // create file
            sscanf(recv_buffer, "%s %s %s", cmd, filename, argu);
            printf("get request to create %s by %s! \n", filename, username);
            if (create_file(filename, argu, group, username) == 0) {
              strcpy(send_buffer, "file created success! \n");
              send(new_socket, send_buffer, sizeof(send_buffer), 0);
            } else {
              strcpy(send_buffer, "file is already exist! \n");
              send(new_socket, send_buffer, sizeof(send_buffer), 0);
            }
            break;
          case 'r':  // read file
            sscanf(recv_buffer, "%s %s", cmd, filename);
            printf("get a read %s request from %s\n", filename, username);
            read_file(filename, username, group, new_socket);
            break;
          case 'w':  // write file
            sscanf(recv_buffer, "%s %s %s", cmd, filename, argu);
            if (strcmp(argu, "o") == 0) {
              strcpy(argu, "w");
            }
            printf("get a write %s request from %s\n", filename, username);
            write_file(filename, username, group, new_socket, argu);
            break;
          case 'm':  // modify permission
            sscanf(recv_buffer, "%s %s %s", cmd, filename, argu);
            printf("%s %s %s\n", cmd, filename, argu);
            int modify_result =
                modify_access_right(filename, argu, username, group);
            // int modify_result = 0;
            if (modify_result == 0) {
              strcpy(send_buffer, "access right was modified successfully! \n");
              send(new_socket, send_buffer, sizeof(send_buffer), 0);
            } else if (modify_result == -1) {
              strcpy(send_buffer,
                     "you don`t have permission to modify access right of this "
                     "file! \n");
              send(new_socket, send_buffer, sizeof(send_buffer), 0);
            } else if (modify_result == -2) {
              strcpy(send_buffer, "this file isn`t exist! \n");
              send(new_socket, send_buffer, sizeof(send_buffer), 0);
            }
            break;
          case ':':  // client disconnected
            printf("disconnect from %s\n", username);
            exit(1);
            printf("I still alive\n");
        }
      }
    }
  }
  return 0;
}
