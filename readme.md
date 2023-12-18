# 使用指南
* compile 指令: make
* 注意事項:
    * server底下需有以下資料夾:
        * server_data/file/
        * server_data/all/
        * server_data/group/
        * server_data/user/
    * client底下需有以下資料夾:
        * client_data/
    * 每個人所屬組別寫在server_data/group.txt中
* 開啟server: ./server
* 開啟client: ./client

* client 可用指令:
    * 新增檔案: ```create <檔名> <權限>```
        * 範例: ```create test.txt rwx---rw-```
    * 下載檔案: ```read <檔名>```
    * 修改權限: ```changemode <檔名> <新權限>```
    * 修改檔案: ```write <檔名> <a/o>```
        * a代表append, o代表overwrite
        * 範例1: ```write text.txt a```,此指令會把本地的text.txt內容append到遠端的text.txt後面
        * 範例2: ```write text.txt o```,此指令會把本地的text.txt內容取代遠端的text.txt

# code簡易解釋:
* 使用capability list來紀錄權限
* 在server_data/user/底下會有每個用戶名稱為的檔名的檔案,裡面的每一行的格式為:<檔名> <擁有的權限>
    * ex.server_data/user/Jack.txt裡面的第一行若是test1.txt rwx代表Jack是這test1.txt的owner,並且他對test.txt擁有rwx的權限
* 在server_data/group底下會有每個group名稱為的檔名的檔案,檔案裡面的每一行的格式為:<檔名> <擁有的權限>,代表該group對該檔案擁有的權限
    * ex. server_data/gruop/CES.txt裡面的第一行若是test1.txt r-x,代表CES這個group對test.txt擁有r-x的權限
* 在server_data/group底下會有all.txt這個檔案,裡面的每一行的格式為:<檔名> <擁有的權限>,代表other對這個檔案擁有的權限
    * ex. server_data/all/all.txt裡面的第一行若是test1.txt r--,代表other對test.txt擁有r--的權限