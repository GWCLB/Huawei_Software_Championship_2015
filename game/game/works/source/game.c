#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "texasv30g.h"

#define BUF_SIZE 50
#define READBUF_SIZE 1024


extern char *deck;//一副牌的值和花色
extern char *suit;
extern int deal[7];
extern int dealtmp[7];
extern char card[50]; //传递给我的牌
int my_blind;

extern int bet;
extern int pot;
extern int blind;
extern int sum;

int fold_num;//累计统计fold的队友数
int check_num;//累计check的人数
int raise_num;
int inquire_num_players;
int all_num;

int hold_flag = 0;
int flop_flag = 0;
int turn_flag = 0;
int river_flag = 0;

int game_round_num;

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int clnt_sock; //新创建的套接字的描述符
    int i;
    int str_len; //socket read函数接收字节大小
    int max_bet;
    char *p_hold; //手牌消息指针
    char *p_flop_1; //公牌消息指针
    char *p_seat; //座位消息指针
    char *p_seat_end;
	char *p_blind;//盲注消息指针
	char *p_blind_end;//
    char *p_inquire;//询问消息指针
    char *p_turn;//转牌指针
    char *p_river;//河牌指针
    char *temp;//临时指针
	char *temp_2;//

    struct sockaddr_in serv_adr; //服务器地址
    struct sockaddr_in clnt_adr; //客户端地址

    char* name = "lx";//队名

    char hold[4] = {'\0'}; //手牌信息
	char small_blind[7] = {'\0'};//小盲注信息
	char big_blind[7] = {'\0'};//大盲注信息
    char flop[7] = {'\0'}; //公牌消息
    char turn[3] = {'\0'}; //转牌消息
    char river[3] = {'\0'};//河牌消息
	char total_pot[10] = {'\0'};//彩池金钱
    char jetton[5] = {'\0'};//开始时筹码
    char bet_1[10] = {'\0'};//本局中收到已玩玩家下注的下注额
    char bet_2[10] = {'\0'};
    char bet_3[10] = {'\0'};
    char bet_4[10] = {'\0'};
    char bet_5[10] = {'\0'};
    char bet_6[10] = {'\0'};
    char bet_7[10] = {'\0'};
    char bet_8[10] = {'\0'};

    char last_jetton[15] = {'\0'};
    char last_bet[10] = {'\0'};
    char char_temp[240];//存放临时字符数组

	
    char buf[BUF_SIZE]; // 发送消息存放的字符串
    char readbuf[READBUF_SIZE]; //接收消息存放的字符串
    if(argc!=6)
    {
        printf("Usagge: %s<port>\n", argv[1]);
        exit(1);
    }


    clnt_sock=socket(AF_INET, SOCK_STREAM, 0);

    int option = 1;
    setsockopt(clnt_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, sizeof(option));


    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port=htons(atoi(argv[2]));

    memset(&clnt_adr, 0, sizeof(clnt_adr));
    clnt_adr.sin_family = AF_INET;
    clnt_adr.sin_addr.s_addr = inet_addr(argv[3]);
    clnt_adr.sin_port=htons(atoi(argv[4]));

    if(-1 == bind(clnt_sock, (struct sockaddr*) &clnt_adr, sizeof(clnt_adr)))
    {
        error_handling("bind() error");
    }

    while(connect(clnt_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) < 0)
    {
        usleep(10*1000);
    }

    char regmsg[BUF_SIZE] = {'\0'};
    //strcpy(regmsg, "reg: ");
    strcat(regmsg, "reg: ");
    strcat(regmsg, argv[5]);
    strcat(regmsg, " ");
    strcat(regmsg, name);
    strcat(regmsg, "\n");


    write(clnt_sock, regmsg, strlen(regmsg)+1);
	game_round_num = 0;
    while(1)
    {
        if(0 == (str_len = read(clnt_sock, readbuf, READBUF_SIZE-1)))
        {
            break;
        }
        if(NULL != strstr(readbuf, "game-over \n"))
        {
            close(clnt_sock);
            return 0;
        }

        readbuf[str_len] = 0;
        //一局开始时获取筹码（必须把seat拷贝出来，因为可能服务器发来上一局结束发来判断牌的那部分）
        p_seat = strstr(readbuf, "seat/ \n");

        if(NULL != p_seat)
        {
            game_round_num++;//表示一局游戏开始.
			memset(char_temp, '\0', 240);
            p_seat_end = strstr(readbuf, "/seat \n");
            i = 0;
            while(p_seat != p_seat_end)
            {
                char_temp[i++] = *p_seat++;
            }

            temp = strstr(char_temp, argv[5]);
            if(NULL != temp)
            {
                //将上一次的值先清空
                memset(last_jetton, '\0', 15);
                while(*temp != ' ')//寻找第一个空格
                    temp++;
                temp = temp + 1;//跳过第一个空格
                for(i = 0;*temp != ' '; temp++,i++)
                {
                    last_jetton[i] = *temp;
                }
                temp = NULL;
            }
            //新一轮开始，清空值
            memset(hold, '\0', 4);
            memset(flop, '\0', 7);
            memset(turn, '\0', 3);
            memset(river, '\0', 3);
			memset(total_pot, '\0', 10);
            memset(bet_1, '\0', 10);
            memset(bet_2, '\0', 10);
            memset(bet_3, '\0', 10);
            memset(bet_4, '\0', 10);
            memset(bet_5, '\0', 10);
            memset(bet_6, '\0', 10);
            memset(bet_7, '\0', 10);
            memset(bet_8, '\0', 10);
            memset(last_bet, '\0', 10);
            memset(card, '\0', 50);
			bet = 0;//开始下注额为零
			pot = 0;//彩池为零
            hold_flag = 0;
            flop_flag = 0;
            turn_flag = 0;
            river_flag = 0;
			fold_num = 0;
			check_num = 0;
			raise_num = 0;
			inquire_num_players = 0;
			all_num = 0;
			my_blind = 0;//是不是盲注清空
			blind = 0;//小盲注一句开始为零
			p_seat = NULL;
        }
		memset(char_temp, '\0', 240);//用完后清空；方便下面的使用
        //获取盲注信息,并从盲注信息中找到看自己这局是不是大盲注或小盲注
		p_blind = strstr(readbuf, "blind/ \n");
		temp_2 = p_blind;
		
		if(NULL != p_blind)
		{
			memset(char_temp, '\0', 240);
            p_blind_end = strstr(readbuf, "/blind \n");
            i = 0;
            while(p_blind != p_blind_end)
            {
                char_temp[i++] = *p_blind++;
            }

            temp = strstr(char_temp, argv[5]);	
			if(NULL != temp)
			{
				my_blind = 1;
			}
			temp = NULL;
			p_blind = temp_2 + 8;//指向盲注信息的首行首字母
			while(*p_blind != ' ')//寻找第一个空格
				p_blind++;
			p_blind = p_blind + 1;//跳过空格。指向盲注信息
			i = 0;
			while(*p_blind != ' ')
				small_blind[i++] = *p_blind++;
		}
		blind = atoi(small_blind);
		memset(char_temp, '\0', 240);//用完后清空；方便下面的使用
		//获取盲注信息结束
		// 获取手牌消息
        p_hold = strstr(readbuf, "hold/ \n");
        if(NULL != p_hold)
        {
            p_hold = p_hold + 7;
            //分割第一个手牌花色和点数
            if(*p_hold == 'S') //SPADES黑桃
            {
                hold[0] = 'S';
                if(*(p_hold + 8) == ' ')
                    hold[1] = *(p_hold + 7);
                else
                    hold[1] = 'T';
            }
            if(*p_hold == 'H') //HEARTS红桃
            {
                hold[0] = 'H';
                if(*(p_hold + 8) == ' ')
                    hold[1] = *(p_hold + 7);
                else
                    hold[1] = 'T';
            }
            if(*p_hold == 'C') //CLUBS梅花
            {
                hold[0] = 'C';
                if( *(p_hold + 7) == ' ')
                    hold[1] = *(p_hold + 6);
                else
                    hold[1] = 'T';
            }
            if(*p_hold == 'D') //DIAMONDS方片
            {
                hold[0] = 'D';
                if(*(p_hold + 10) == ' ')
                    hold[1] = *(p_hold + 9);
                else
                    hold[1] = 'T';
            }
            //分割第二个手牌花色和点数
            p_hold = strstr(readbuf, "/hold \n");//定位字符串指针
            p_hold = p_hold - 3; //从后面开始指
            if(*(p_hold - 1) != ' ')//此时的点数为10
            {
                if(*(p_hold - 4) == 'E') //SPADES黑桃 倒数第二个字母
                    hold[2] = 'S';
                if(*(p_hold - 4) == 'T') //HEARTS红桃 倒数第二个字母
                    hold[2] = 'H';
                if(*(p_hold - 4) == 'B') //CLUBS梅花 倒数第二个字母
                    hold[2] = 'C';
                if(*(p_hold - 4) == 'D') //DIAMONDS方片 倒数第二个字母
                    hold[2] = 'D';
                hold[3] = 'T';
            }
            else
            {
                if(*(p_hold - 3) == 'E') //SPADES黑桃 倒数第二个字母
                    hold[2] = 'S';
                if(*(p_hold - 3) == 'T') //HEARTS红桃 倒数第二个字母
                    hold[2] = 'H';
                if(*(p_hold - 3) == 'B') //CLUBS梅花 倒数第二个字母
                    hold[2] = 'C';
                if(*(p_hold - 3) == 'D') //DIAMONDS方片 倒数第二个字母
                    hold[2] = 'D';
                hold[3] = *p_hold;
            }

        }
        //获取手牌结束
        //获取公牌消息开始
        p_flop_1 = strstr(readbuf, "flop/ \n");
        if(NULL != p_flop_1 )
        {
            p_flop_1 = p_flop_1 + 7;

            //分割第一个公共牌
            if(*p_flop_1 == 'S') //SPADES黑桃
            {
                flop[0] = 'S';
                if(*(p_flop_1 + 8) == ' ')
                    flop[1] = *(p_flop_1 + 7);
                else
                    flop[1] = 'T';
            }
            if(*p_flop_1 == 'H') //HEARTS红桃
            {
                flop[0] = 'H';
                if(*(p_flop_1 + 8) == ' ')
                    flop[1] = *(p_flop_1 + 7);
                else
                    flop[1] = 'T';
            }
            if(*p_flop_1 == 'C') //CLUBS梅花
            {
                flop[0] = 'C';
                if( *(p_flop_1 + 7) == ' ')
                    flop[1] = *(p_flop_1 + 6);
                else
                    flop[1] = 'T';
            }
            if(*p_flop_1 == 'D') //DIAMONDS方片
            {
                flop[0] = 'D';
                if(*(p_flop_1 + 10) == ' ')
                    flop[1] = *(p_flop_1 + 9);
                else
                    flop[1] = 'T';
            }

            while(*p_flop_1 != '\n')
                p_flop_1++;
            p_flop_1 = p_flop_1 + 1; //定位到第二个公共牌首字母

            //分割第二个公共牌首字母

            if(*p_flop_1 == 'S') //SPADES黑桃
            {
                flop[2] = 'S';
                if(*(p_flop_1 + 8) == ' ')
                    flop[3] = *(p_flop_1 + 7);
                else
                    flop[3] = 'T';
            }
            if(*p_flop_1 == 'H') //HEARTS红桃
            {
                flop[2] = 'H';
                if(*(p_flop_1 + 8) == ' ')
                    flop[3] = *(p_flop_1 + 7);
                else
                    flop[3] = 'T';
            }
            if(*p_flop_1 == 'C') //CLUBS梅花
            {
                flop[2] = 'C';
                if( *(p_flop_1 + 7) == ' ')
                    flop[3] = *(p_flop_1 + 6);
                else
                    flop[3] = 'T';
            }
            if(*p_flop_1 == 'D') //DIAMONDS方片
            {
                flop[2] = 'D';
                if(*(p_flop_1 + 10) == ' ')
                    flop[3] = *(p_flop_1 + 9);
                else
                    flop[3] = 'T';
            }

            while(*p_flop_1 != '\n')
                p_flop_1++;
            p_flop_1 = p_flop_1 + 1; //定位到第三个公共牌首字母

            //分割第三个公共牌首字母
            if(*p_flop_1 == 'S') //SPADES黑桃
            {
                flop[4] = 'S';
                if(*(p_flop_1 + 8) == ' ')
                    flop[5] = *(p_flop_1 + 7);
                else
                    flop[5] = 'T';
            }
            if(*p_flop_1 == 'H') //HEARTS红桃
            {
                flop[4] = 'H';
                if(*(p_flop_1 + 8) == ' ')
                    flop[5] = *(p_flop_1 + 7);
                else
                    flop[5] = 'T';
            }
            if(*p_flop_1 == 'C') //CLUBS梅花
            {
                flop[4] = 'C';
                if( *(p_flop_1 + 7) == ' ')
                    flop[5] = *(p_flop_1 + 6);
                else
                    flop[5] = 'T';
            }
            if(*p_flop_1 == 'D') //DIAMONDS方片
            {
                flop[4] = 'D';
                if(*(p_flop_1 + 10) == ' ')
                    flop[5] = *(p_flop_1 + 9);
                else
                    flop[5] = 'T';
            }
        }
        //获取公牌消息结束

        //获取转牌消息
        p_turn = strstr(readbuf, "turn/ \n");
        if(NULL != p_turn && NULL == p_seat)
        {
            p_turn = p_turn + 7;
            //分割第一个手牌花色和点数
            if(*p_turn == 'S') //SPADES黑桃
            {
                turn[0] = 'S';
                if(*(p_turn + 8) == ' ')
                    turn[1] = *(p_turn + 7);
                else
                    turn[1] = 'T';
            }
            if(*p_turn == 'H') //HEARTS红桃
            {
                turn[0] = 'H';
                if(*(p_turn + 8) == ' ')
                    turn[1] = *(p_turn + 7);
                else
                    turn[1] = 'T';
            }
            if(*p_turn == 'C') //CLUBS梅花
            {
                turn[0] = 'C';
                if( *(p_turn + 7) == ' ')
                    turn[1] = *(p_turn + 6);
                else
                    turn[1] = 'T';
            }
            if(*p_turn == 'D') //DIAMONDS方片
            {
                turn[0] = 'D';
                if(*(p_turn + 10) == ' ')
                    turn[1] = *(p_turn + 9);
                else
                    turn[1] = 'T';
            }
        }
        //获取转牌消息结束
        //获取河牌消息
        p_river = strstr(readbuf, "river/ \n");
        if(NULL != p_river && NULL == p_seat)
        {
            p_river = p_river + 8;
            //分割第一个手牌花色和点数
            if(*p_river == 'S') //SPADES黑桃
            {
                river[0] = 'S';
                if(*(p_river + 8) == ' ')
                    river[1] = *(p_river + 7);
                else
                    river[1] = 'T';
            }
            if(*p_river == 'H') //HEARTS红桃
            {
                river[0] = 'H';
                if(*(p_river + 8) == ' ')
                    river[1] = *(p_river + 7);
                else
                    river[1] = 'T';
            }
            if(*p_river == 'C') //CLUBS梅花
            {
                river[0] = 'C';
                if( *(p_river + 7) == ' ')
                    river[1] = *(p_river + 6);
                else
                    river[1] = 'T';
            }
            if(*p_river == 'D') //DIAMONDS方片
            {
                river[0] = 'D';
                if(*(p_river + 10) == ' ')
                    river[1] = *(p_river + 9);
                else
                    river[1] = 'T';
            }


        }

        //获取河牌消息结束
        if( (NULL != p_river && NULL != p_seat) || (NULL != p_turn && NULL != p_seat) )//此时发的消息里既有转牌河牌也有seat等,清空不需要上一局的信息带入这一局
        {
            memset(flop, '\0', 7);
            memset(turn, '\0', 3);
            memset(river, '\0', 3);
        }

        //传给card手牌
            for(i = 0; i < 4;i++)
            {
                card[i] = hold[i];
                hold_flag = 1;
            }
        //传给card三张公共牌
            if(flop[0] != '\0')
            {
                for(i = 4; i < 10;i++)
                {
                    card[i] = flop[i - 4];
                    flop_flag = 1;
                }
            }
        //传给card一张转牌
            if(turn[0] != '\0')
            {
                for(i = 10; i < 12;i++)
                {
                    card[i] = turn[i - 10];
                    turn_flag = 1;
                }
            }
        //传给card一张河牌
            if(river[0] != '\0')
            {
                for(i = 12; i < 14;i++)
                {
                    card[i] = river[i - 12];
                    river_flag = 1;
                }
            }

        //本手牌已行动过的所有玩家（包括被询问者和盲注）的手中筹码、剩余金币数、
        //本手牌累计投注额、及最近的一次有效action，按逆时针座次由近及远排列，上家排在第一个
        //当前底池总金额（本轮投注已累计）
        p_inquire = strstr(readbuf, "inquire/ \n");
		
	
        if( NULL != p_inquire  )//如果我弃牌，则询问消息不再处理，特别是不再给服务器发送消息，如果发送了，会造成服务器消息阻塞
        {
            
			fold_num = 0;//累计统计fold的队友数
			check_num = 0;//累计check的人数
			raise_num = 0;
			inquire_num_players = 0;
			all_num = 0;
			
			//从询问消息中得到累计的下注额最大者（并且得出fold的玩家数，如果七个，则必须叫牌，否则服务器不会把彩池的筹码给自己）
            p_inquire = p_inquire + 10;//指向询问消息的第一行首字母
            while(1)
            {
                while(*p_inquire != '\n')//定位到第一行回车处
                    p_inquire++;
                temp = p_inquire;//记录回车的位置
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;
					
                {//代码块计算出询问消息中第一行的累计下注额
                    p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
                    p_inquire = p_inquire -1;//跳过空格
                    while( *p_inquire != ' ') //寻找第二个空格
                        p_inquire--;
                    p_inquire = p_inquire +1;//指向最大下注额的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_1[i++] = *p_inquire++;
                    }
                }

                p_inquire = temp + 1; //重新指回第一个回车,并跳过第一个回车

                if( *p_inquire == 't')//定位到最后一行
                {
                    while( *p_inquire != '\n') //寻找total pot结尾
						p_inquire++;
					p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
					p_inquire = p_inquire +1;//指向total pot的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }


					break;//以进行到最后一行total pot，跳出while循环否则继续寻找下一行
                }
                //寻找第二个回车
                while( *p_inquire != '\n') //寻找第二行结尾
                    p_inquire++;
                temp = p_inquire;//记录回车的位置
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;	
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;				
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;
				
                {
                    p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
                    p_inquire = p_inquire -1;//跳过空格
                    while( *p_inquire != ' ') //寻找第二个空格
                        p_inquire--;
                    p_inquire = p_inquire +1;//指向最大下注额的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_2[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //重新指回第二个回车,并跳过第二个回车
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //寻找total pot结尾
						p_inquire++;
					p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
					p_inquire = p_inquire +1;//指向total pot的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//以进行到最后一行total pot，跳出while循环否则继续寻找下一行
                }
                //寻找第三个回车
                while( *p_inquire != '\n') //寻找第三行结尾
                    p_inquire++;				
				temp = p_inquire;//记录回车的位置
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;	
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;					
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;

					
                {
                    p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
                    p_inquire = p_inquire -1;//跳过空格
                    while( *p_inquire != ' ') //寻找第二个空格
                        p_inquire--;
                    p_inquire = p_inquire +1;//指向最大下注额的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_3[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //重新指回第三个回车,并跳过第三个回车
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //寻找total pot结尾
						p_inquire++;
					p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
					p_inquire = p_inquire +1;//指向total pot的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//以进行到最后一行total pot，跳出while循环否则继续寻找下一行
                }
                //寻找第四个回车
                while( *p_inquire != '\n') //寻找第三行结尾
                    p_inquire++;
					
				temp = p_inquire;//记录回车的位置
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;	
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;					
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;

					
                {
                    p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
                    p_inquire = p_inquire -1;//跳过空格
                    while( *p_inquire != ' ') //寻找第二个空格
                        p_inquire--;
                    p_inquire = p_inquire +1;//指向最大下注额的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_4[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //重新指回第四个回车,并跳过第四个回车
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //寻找total pot结尾
						p_inquire++;
					p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
					p_inquire = p_inquire +1;//指向total pot的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//以进行到最后一行total pot，跳出while循环否则继续寻找下一行
                }
                //寻找第五个回车
                while( *p_inquire != '\n') //寻找第三行结尾
                    p_inquire++;
				temp = p_inquire;//记录回车的位置	
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;  
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;
					
                {
                    p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
                    p_inquire = p_inquire -1;//跳过空格
                    while( *p_inquire != ' ') //寻找第二个空格
                        p_inquire--;
                    p_inquire = p_inquire +1;//指向最大下注额的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_5[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //重新指回第五个回车,并跳过第五个回车
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //寻找total pot结尾
						p_inquire++;
					p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
					p_inquire = p_inquire +1;//指向total pot的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//以进行到最后一行total pot，跳出while循环否则继续寻找下一行
                }
                //寻找第六个回车
                while( *p_inquire != '\n') //寻找第三行结尾
                    p_inquire++;
				
				temp = p_inquire;//记录回车的位置
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;	
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;					
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;

					
                {
                    p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
                    p_inquire = p_inquire -1;//跳过空格
                    while( *p_inquire != ' ') //寻找第二个空格
                        p_inquire--;
                    p_inquire = p_inquire +1;//指向最大下注额的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_6[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //重新指回第六个回车,并跳过第六个回车
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //寻找total pot结尾
						p_inquire++;
					p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
					p_inquire = p_inquire +1;//指向total pot的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//以进行到最后一行total pot，跳出while循环否则继续寻找下一行
                }
                //寻找第七个回车
                while( *p_inquire != '\n') //寻找第三行结尾
                    p_inquire++;
				temp = p_inquire;//记录回车的位置
				
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;					
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;
					
                {
                    p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
                    p_inquire = p_inquire -1;//跳过空格
                    while( *p_inquire != ' ') //寻找第二个空格
                        p_inquire--;
                    p_inquire = p_inquire +1;//指向最大下注额的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_7[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //重新指回第七个回车,并跳过第七个回车
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //寻找total pot结尾
						p_inquire++;
					p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
					p_inquire = p_inquire +1;//指向total pot的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//以进行到最后一行total pot，跳出while循环否则继续寻找下一行
                }
                //寻找第八个回车
                while( *p_inquire != '\n') //寻找第八行结尾
                    p_inquire++;
				temp = p_inquire;
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;

					
                {
                    p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
                    p_inquire = p_inquire -1;//跳过空格
                    while( *p_inquire != ' ') //寻找第二个空格
                        p_inquire--;
                    p_inquire = p_inquire +1;//指向最大下注额的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_8[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //重新指回第八个回车,并跳过第八个回车
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //寻找total pot结尾
						p_inquire++;
					p_inquire= p_inquire -2;//跳过空格
                    while( *p_inquire != ' ') //寻找第一个空格
                        p_inquire--;
					p_inquire = p_inquire +1;//指向total pot的第一个字符
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }
					break;//以进行到最后一行total pot，跳出while循环否则继续寻找下一行
                }
            }
			p_inquire = NULL;//将指针滞空
            //询问消息中查找出来了累计下注额在此之中 查找下注额最大结束
            max_bet = atoi(bet_1);
			if(bet_2[0] != '\0')
			{
				if(max_bet < atoi(bet_2))
					max_bet = atoi(bet_2);
			}
			if(bet_3[0] != '\0')
			{
            if(max_bet < atoi(bet_3))
                max_bet = atoi(bet_3);
			}
			if(bet_4[0] != '\0')
			{			
				if(max_bet < atoi(bet_4))
					max_bet = atoi(bet_4);
			}
			if(bet_5[0] != '\0')
			{				
				if(max_bet < atoi(bet_5))
					max_bet = atoi(bet_5);
			}
			if(bet_6[0] != '\0')
			{			
				if(max_bet < atoi(bet_6))
					max_bet = atoi(bet_6);
			}
			if(bet_7[0] != '\0')
			{				
				if(max_bet < atoi(bet_7))
					max_bet = atoi(bet_7);
			}
			if(bet_8[0] != '\0')
			{				
				if(max_bet < atoi(bet_8))
					max_bet = atoi(bet_8);
			}
			//一次询问消息结束后立即清空
			memset(bet_1, '\0', 10);
            memset(bet_2, '\0', 10);
            memset(bet_3, '\0', 10);
            memset(bet_4, '\0', 10);
            memset(bet_5, '\0', 10);
            memset(bet_6, '\0', 10);
            memset(bet_7, '\0', 10);
            memset(bet_8, '\0', 10);
			bet = max_bet;
			
			if(total_pot[0] != '\0')
			{
				pot = atoi(total_pot);
			}
			
            //从询问消息中得到自己现在剩余的筹码和已下的投注额，如若未找到，则表明刚开始，此时自己还没有下注，筹码为起始值
            temp = strstr(readbuf, argv[5]);
            if(NULL != temp)
            {
                //将上一次的值先清空
                memset(last_jetton, '\0', 15);
                memset(last_bet, '\0', 10);

                while(*temp != ' ')//寻找第一个空格
                    temp++;
                temp = temp + 1;//跳过第一个空格
                for(i = 0;*temp != ' '; temp++,i++)
                {
                    last_jetton[i] = *temp;
                }
                temp = temp + 1;//跳过第二个空格
                while(*temp != ' ')//寻找第三个空格
                    temp++;
                temp = temp + 1;//跳过第三个空格
                for(i = 0;*temp != ' '; temp++,i++)
                {
                    last_bet[i] = *temp;
                }
                temp = NULL;
            }



            /*此处调用外面的决策程序 接着供下面返回服务器用*/
            //作出决定，发给服务器的回应
            //发给两张牌的程序
			if(inquire_num_players - fold_num == 1)	//所有玩家减去弃牌的玩家若等一就说明其他人都弃牌，此时自己必须加注获得彩池金额
			{
				memset(buf, 0, BUF_SIZE);
				strcat(buf, "raise \n");
				write(clnt_sock, buf, strlen(buf) + 1);
				fold_num = 0;//重新清零，迎接下一个询问消息
			}
			else
			{
				//发给两张牌的程序
				if(hold_flag == 1 && flop_flag == 0 && turn_flag == 0 && river_flag == 0)
				{
					if(judgemypork(2) == 0)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "fold \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}					
					if(judgemypork(2) == 1)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "call \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(2) == 2)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "raise \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(2) == 3)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "rasie 200 \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(2) == 4)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "check \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					
				}
				//发给五张牌的程序
				if(hold_flag == 1 && flop_flag == 1 && turn_flag == 0 && river_flag == 0)
				{
					if(judgemypork(5) == 0)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "fold \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(5) == 1)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "call \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(5) == 2)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "raise \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(5) == 3)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "raise 200 \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(5) == 4)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "check \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
				}
				//发给六张牌的程序
				if(hold_flag == 1 && flop_flag == 1 && turn_flag == 1 && river_flag == 0)
				{
					if(judgemypork(6) == 0)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "fold \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(6) == 1)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "call \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(6) == 2)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "raise \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(6) == 3)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "raise 200 \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(6) == 4)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "check \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}

				}

				//发给七张牌的程序
				if(hold_flag == 1 && flop_flag == 1 && turn_flag == 1 && river_flag == 1)
				{
					if(judgemypork(7) == 0)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "fold \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(7) == 1)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "call \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(7) == 2)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "raise \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(7) == 3)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "raise 217 \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}
					if(judgemypork(7) == 4)
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "check \n");
						write(clnt_sock, buf, strlen(buf) + 1);
					}					

				}

				fold_num = 0;//重新清零，迎接下一个询问消息
				inquire_num_players = 0;//询问消息接受后总的玩家（包括弃牌的玩家）
			}

        }
        //这一局结束
        if( NULL != strstr(readbuf, "pot-win/ \n") )
        {
		
        }
        //这一局结束
		
        memset(readbuf, 0, READBUF_SIZE);
    }
    close(clnt_sock);
    return 0;
}



void error_handling(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
