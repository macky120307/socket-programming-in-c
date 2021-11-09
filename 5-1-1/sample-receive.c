#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define PORT     10000
#define BUF_LEN  512

char receiver_name[BUF_LEN+1]; /* 自端末のユーザの名前 */
char caller_name[BUF_LEN+1];   /* 発信する端末のユーザの名前 */ 

int  soc_waiting;              /* 自端末が着信を待ち受けるためのソケット */
int  port = PORT;              /* 着信を待ち受けるためのポート番号(10000) */
struct sockaddr_in me;
int  soc;                      /* データ送信用ソケット */
char buf[BUF_LEN+1];           /* データ送受信用バッファ */
int flag;

/****************************************
プログラムで新たに使う変数をこの下で定義する
****************************************/
char receiver_msg[BUF_LEN+1];
char caller_msg[BUF_LEN+1];


int main()
{
    /* 自分の名前を入力 */
    printf("あなたの名前は ? ");
    fgets(receiver_name, BUF_LEN, stdin);            /* キーボードから一行読み込む(改行コード付き) */
    receiver_name[strlen(receiver_name)-1] = '\0';   /* 改行コードを削除 */

    /* 相手から着信するのに必要なソケット soc_waiting を生成する */
    soc_waiting = socket(AF_INET, SOCK_STREAM, 0);
    if (soc_waiting < 0)      /* ソケット生成失敗 */
    {
        perror("socket");     /* エラーメッセージを出力して終了 */
        exit(1);
    }
    /*ソケットの再利用（EADDRINUSE対策）*/
    int on = 1;
    if(setsockopt(soc_waiting,SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) )<0){
        perror("ERROR on setsockopt");
        exit(1);
    }

    /* 生成したソケット soc_waiting に相手から着信を待ち受けるための */
    /* 自分のポート番号などを設定する                               */
    /* ポート番号等の情報をまず変数 me に設定する                   */
    memset((char *)&me,0,sizeof(me));     
    me.sin_addr.s_addr = htonl(INADDR_ANY);  /* すべてのIPアドレスから待ち受ける */
    me.sin_port = htons(PORT);               /* 待ち受けるポート番号 */
    me.sin_family = AF_INET;                 /* 通信モード */

    /* 変数 me と ソケット soc_waiting を結びつける */
	flag = bind(soc_waiting, (struct sockaddr *)&me, sizeof(me));
    if( flag <0 ){
		perror("bind(REUSEPORT):");
        exit(1);
    }

    /* soc_waitingによって相手からの着信を待つ */
    printf("着信を待っています...\n");
    listen(soc_waiting, 1);

    /* 相手から着信があったら 相手との通信のために */
    /* 別のソケット soc を生成する                 */ 
    soc = accept(soc_waiting, NULL, NULL);   
    close(soc_waiting);

    /* soc を使って相手から名前を受信し、モニタに表示 */
    recv(soc,caller_name,BUF_LEN,0);
    printf("%s さんから着信しました\n", caller_name);

    /* soc を使って相手に自分の名前を送る */
    send(soc, receiver_name, strlen(receiver_name)+1, 0);

    /*      無限ループ開始　      */
    while (1) {
        /* recv関数を用いてメッセージを受信する */
        recv(soc, caller_msg, BUF_LEN,0);

        /* 接続相手のユーザの名前と受信したメッセージを表示する */
        printf("%s: %s \n", caller_name, caller_msg);

        /* 自端末のユーザの名前を表示 */
        printf("%s: ", receiver_name);

        /* fgets関数でメッセージを取得する */
        fgets(receiver_msg, BUF_LEN, stdin);

         /* fgets関数で取得したメッセージの最後の文字をnull文字にする */
        receiver_msg[strlen(receiver_msg)-1] = '\0';

        /* send関数を用いてreceiver_msgを送信する */
        send(soc, receiver_msg, strlen(receiver_msg)+1, 0);
    }
    /*      無限ループ終了        */
    /* 着信を待つために生成したソケット soc_waiting を閉じる */
    close(soc);
    
    return 0;
}


