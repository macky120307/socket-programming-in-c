#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define PORT      10000
#define BUF_LEN   512

char caller_name[BUF_LEN+1];     /* 自端末のユーザの名前 */
char receiver_name[BUF_LEN+1];   /* 接続相手のユーザの名前 */


int  port  = PORT;               /* 接続相手のポート番号(=10000) */
char host[BUF_LEN+1];            /* 接続相手のIPアドレスを格納する変数 */
struct sockaddr_in  server;
int  soc;
char buf[BUF_LEN+1];             /* 送受信用データバッファ */ 
int  flag;

/**************************************
プログラムで新たに使う変数をここで定義する
**************************************/
char receiver_msg[BUF_LEN+1];
char caller_msg[BUF_LEN+1];


int main(void)
{
    /* 自分の名前を入力 */
    printf("あなたの名前は ? ");
    fgets(caller_name,BUF_LEN,stdin);               /* キーボードから一行読み込む(改行コード付き)*/
    caller_name[strlen(caller_name)-1] = '\0';      /* 改行コードを削除 */ 

    /* 接続相手のIPアドレスを入力 */
    printf("接続先のIPアドレスは? ");
    fgets(host,BUF_LEN,stdin);      /* キーボードから一行読み込む (改行 コード付き )*/
    host[strlen(host)-1] = '\0';    /* 改行コードを削除 */ 

    /* 相手と通信を行うためのソケット soc を生成 */
    soc = socket(AF_INET, SOCK_STREAM, 0); 
    if(soc < 0){
        perror("socket");
        exit(1);
    }

    /* 変数 server に接続相手のIPアドレスとポート番号を設定 */
    inet_aton(host,(struct in_addr *)&server.sin_addr);  /* IPアドレス */
    server.sin_port = htons(port);             /* ポート番号 */
    server.sin_family = AF_INET;               /* 通信モード */

    /* ソケット soc を使って変数 server に示した相手と接続 */
    flag = connect(soc, (struct sockaddr *)&server, sizeof(server));
    if(flag < 0){   /* 接続失敗 */
        perror("connect");
        exit(1);
    }

    /* 相手に自分の名前を送信 */
    send(soc,caller_name,strlen(caller_name)+1,0);

    /* 相手から名前を受信し、モニタに表示 */
    recv(soc,receiver_name,BUF_LEN,0);
    printf("%s さんから応答がありました\n",receiver_name);

    fcntl(0, F_SETFL, O_NONBLOCK);
    fcntl(soc, F_SETFL, O_NONBLOCK);

    /* 無限ループ開始 */
    while (1) {
        char *fgetsRes = fgets(caller_msg, BUF_LEN, stdin);
        if (fgetsRes) {
            printf("%s: ", caller_name);
            caller_msg[strlen(caller_msg)-1] = '\0';
            send(soc, caller_msg, strlen(caller_msg)+1, 0);
        }
        if (!strcmp(caller_msg, "end") || !strcmp(caller_msg, "quit")) break;

        int recvRes = recv(soc, receiver_msg, BUF_LEN,0);
        if (recvRes > 0) {
            printf("%s: %s \n", receiver_name, receiver_msg);
        }
        if (!strcmp(receiver_msg, "end") || !strcmp(receiver_msg, "quit")) break;
    }
    /* 無限ループ終了 */

    close(soc);
    return 0;
}
