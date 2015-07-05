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


extern char *deck;//һ���Ƶ�ֵ�ͻ�ɫ
extern char *suit;
extern int deal[7];
extern int dealtmp[7];
extern char card[50]; //���ݸ��ҵ���
int my_blind;

extern int bet;
extern int pot;
extern int blind;
extern int sum;

int fold_num;//�ۼ�ͳ��fold�Ķ�����
int check_num;//�ۼ�check������
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
    int clnt_sock; //�´������׽��ֵ�������
    int i;
    int str_len; //socket read���������ֽڴ�С
    int max_bet;
    char *p_hold; //������Ϣָ��
    char *p_flop_1; //������Ϣָ��
    char *p_seat; //��λ��Ϣָ��
    char *p_seat_end;
	char *p_blind;//äע��Ϣָ��
	char *p_blind_end;//
    char *p_inquire;//ѯ����Ϣָ��
    char *p_turn;//ת��ָ��
    char *p_river;//����ָ��
    char *temp;//��ʱָ��
	char *temp_2;//

    struct sockaddr_in serv_adr; //��������ַ
    struct sockaddr_in clnt_adr; //�ͻ��˵�ַ

    char* name = "lx";//����

    char hold[4] = {'\0'}; //������Ϣ
	char small_blind[7] = {'\0'};//Сäע��Ϣ
	char big_blind[7] = {'\0'};//��äע��Ϣ
    char flop[7] = {'\0'}; //������Ϣ
    char turn[3] = {'\0'}; //ת����Ϣ
    char river[3] = {'\0'};//������Ϣ
	char total_pot[10] = {'\0'};//�ʳؽ�Ǯ
    char jetton[5] = {'\0'};//��ʼʱ����
    char bet_1[10] = {'\0'};//�������յ����������ע����ע��
    char bet_2[10] = {'\0'};
    char bet_3[10] = {'\0'};
    char bet_4[10] = {'\0'};
    char bet_5[10] = {'\0'};
    char bet_6[10] = {'\0'};
    char bet_7[10] = {'\0'};
    char bet_8[10] = {'\0'};

    char last_jetton[15] = {'\0'};
    char last_bet[10] = {'\0'};
    char char_temp[240];//�����ʱ�ַ�����

	
    char buf[BUF_SIZE]; // ������Ϣ��ŵ��ַ���
    char readbuf[READBUF_SIZE]; //������Ϣ��ŵ��ַ���
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
        //һ�ֿ�ʼʱ��ȡ���루�����seat������������Ϊ���ܷ�����������һ�ֽ��������ж��Ƶ��ǲ��֣�
        p_seat = strstr(readbuf, "seat/ \n");

        if(NULL != p_seat)
        {
            game_round_num++;//��ʾһ����Ϸ��ʼ.
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
                //����һ�ε�ֵ�����
                memset(last_jetton, '\0', 15);
                while(*temp != ' ')//Ѱ�ҵ�һ���ո�
                    temp++;
                temp = temp + 1;//������һ���ո�
                for(i = 0;*temp != ' '; temp++,i++)
                {
                    last_jetton[i] = *temp;
                }
                temp = NULL;
            }
            //��һ�ֿ�ʼ�����ֵ
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
			bet = 0;//��ʼ��ע��Ϊ��
			pot = 0;//�ʳ�Ϊ��
            hold_flag = 0;
            flop_flag = 0;
            turn_flag = 0;
            river_flag = 0;
			fold_num = 0;
			check_num = 0;
			raise_num = 0;
			inquire_num_players = 0;
			all_num = 0;
			my_blind = 0;//�ǲ���äע���
			blind = 0;//Сäעһ�俪ʼΪ��
			p_seat = NULL;
        }
		memset(char_temp, '\0', 240);//�������գ����������ʹ��
        //��ȡäע��Ϣ,����äע��Ϣ���ҵ����Լ�����ǲ��Ǵ�äע��Сäע
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
			p_blind = temp_2 + 8;//ָ��äע��Ϣ����������ĸ
			while(*p_blind != ' ')//Ѱ�ҵ�һ���ո�
				p_blind++;
			p_blind = p_blind + 1;//�����ո�ָ��äע��Ϣ
			i = 0;
			while(*p_blind != ' ')
				small_blind[i++] = *p_blind++;
		}
		blind = atoi(small_blind);
		memset(char_temp, '\0', 240);//�������գ����������ʹ��
		//��ȡäע��Ϣ����
		// ��ȡ������Ϣ
        p_hold = strstr(readbuf, "hold/ \n");
        if(NULL != p_hold)
        {
            p_hold = p_hold + 7;
            //�ָ��һ�����ƻ�ɫ�͵���
            if(*p_hold == 'S') //SPADES����
            {
                hold[0] = 'S';
                if(*(p_hold + 8) == ' ')
                    hold[1] = *(p_hold + 7);
                else
                    hold[1] = 'T';
            }
            if(*p_hold == 'H') //HEARTS����
            {
                hold[0] = 'H';
                if(*(p_hold + 8) == ' ')
                    hold[1] = *(p_hold + 7);
                else
                    hold[1] = 'T';
            }
            if(*p_hold == 'C') //CLUBS÷��
            {
                hold[0] = 'C';
                if( *(p_hold + 7) == ' ')
                    hold[1] = *(p_hold + 6);
                else
                    hold[1] = 'T';
            }
            if(*p_hold == 'D') //DIAMONDS��Ƭ
            {
                hold[0] = 'D';
                if(*(p_hold + 10) == ' ')
                    hold[1] = *(p_hold + 9);
                else
                    hold[1] = 'T';
            }
            //�ָ�ڶ������ƻ�ɫ�͵���
            p_hold = strstr(readbuf, "/hold \n");//��λ�ַ���ָ��
            p_hold = p_hold - 3; //�Ӻ��濪ʼָ
            if(*(p_hold - 1) != ' ')//��ʱ�ĵ���Ϊ10
            {
                if(*(p_hold - 4) == 'E') //SPADES���� �����ڶ�����ĸ
                    hold[2] = 'S';
                if(*(p_hold - 4) == 'T') //HEARTS���� �����ڶ�����ĸ
                    hold[2] = 'H';
                if(*(p_hold - 4) == 'B') //CLUBS÷�� �����ڶ�����ĸ
                    hold[2] = 'C';
                if(*(p_hold - 4) == 'D') //DIAMONDS��Ƭ �����ڶ�����ĸ
                    hold[2] = 'D';
                hold[3] = 'T';
            }
            else
            {
                if(*(p_hold - 3) == 'E') //SPADES���� �����ڶ�����ĸ
                    hold[2] = 'S';
                if(*(p_hold - 3) == 'T') //HEARTS���� �����ڶ�����ĸ
                    hold[2] = 'H';
                if(*(p_hold - 3) == 'B') //CLUBS÷�� �����ڶ�����ĸ
                    hold[2] = 'C';
                if(*(p_hold - 3) == 'D') //DIAMONDS��Ƭ �����ڶ�����ĸ
                    hold[2] = 'D';
                hold[3] = *p_hold;
            }

        }
        //��ȡ���ƽ���
        //��ȡ������Ϣ��ʼ
        p_flop_1 = strstr(readbuf, "flop/ \n");
        if(NULL != p_flop_1 )
        {
            p_flop_1 = p_flop_1 + 7;

            //�ָ��һ��������
            if(*p_flop_1 == 'S') //SPADES����
            {
                flop[0] = 'S';
                if(*(p_flop_1 + 8) == ' ')
                    flop[1] = *(p_flop_1 + 7);
                else
                    flop[1] = 'T';
            }
            if(*p_flop_1 == 'H') //HEARTS����
            {
                flop[0] = 'H';
                if(*(p_flop_1 + 8) == ' ')
                    flop[1] = *(p_flop_1 + 7);
                else
                    flop[1] = 'T';
            }
            if(*p_flop_1 == 'C') //CLUBS÷��
            {
                flop[0] = 'C';
                if( *(p_flop_1 + 7) == ' ')
                    flop[1] = *(p_flop_1 + 6);
                else
                    flop[1] = 'T';
            }
            if(*p_flop_1 == 'D') //DIAMONDS��Ƭ
            {
                flop[0] = 'D';
                if(*(p_flop_1 + 10) == ' ')
                    flop[1] = *(p_flop_1 + 9);
                else
                    flop[1] = 'T';
            }

            while(*p_flop_1 != '\n')
                p_flop_1++;
            p_flop_1 = p_flop_1 + 1; //��λ���ڶ�������������ĸ

            //�ָ�ڶ�������������ĸ

            if(*p_flop_1 == 'S') //SPADES����
            {
                flop[2] = 'S';
                if(*(p_flop_1 + 8) == ' ')
                    flop[3] = *(p_flop_1 + 7);
                else
                    flop[3] = 'T';
            }
            if(*p_flop_1 == 'H') //HEARTS����
            {
                flop[2] = 'H';
                if(*(p_flop_1 + 8) == ' ')
                    flop[3] = *(p_flop_1 + 7);
                else
                    flop[3] = 'T';
            }
            if(*p_flop_1 == 'C') //CLUBS÷��
            {
                flop[2] = 'C';
                if( *(p_flop_1 + 7) == ' ')
                    flop[3] = *(p_flop_1 + 6);
                else
                    flop[3] = 'T';
            }
            if(*p_flop_1 == 'D') //DIAMONDS��Ƭ
            {
                flop[2] = 'D';
                if(*(p_flop_1 + 10) == ' ')
                    flop[3] = *(p_flop_1 + 9);
                else
                    flop[3] = 'T';
            }

            while(*p_flop_1 != '\n')
                p_flop_1++;
            p_flop_1 = p_flop_1 + 1; //��λ������������������ĸ

            //�ָ����������������ĸ
            if(*p_flop_1 == 'S') //SPADES����
            {
                flop[4] = 'S';
                if(*(p_flop_1 + 8) == ' ')
                    flop[5] = *(p_flop_1 + 7);
                else
                    flop[5] = 'T';
            }
            if(*p_flop_1 == 'H') //HEARTS����
            {
                flop[4] = 'H';
                if(*(p_flop_1 + 8) == ' ')
                    flop[5] = *(p_flop_1 + 7);
                else
                    flop[5] = 'T';
            }
            if(*p_flop_1 == 'C') //CLUBS÷��
            {
                flop[4] = 'C';
                if( *(p_flop_1 + 7) == ' ')
                    flop[5] = *(p_flop_1 + 6);
                else
                    flop[5] = 'T';
            }
            if(*p_flop_1 == 'D') //DIAMONDS��Ƭ
            {
                flop[4] = 'D';
                if(*(p_flop_1 + 10) == ' ')
                    flop[5] = *(p_flop_1 + 9);
                else
                    flop[5] = 'T';
            }
        }
        //��ȡ������Ϣ����

        //��ȡת����Ϣ
        p_turn = strstr(readbuf, "turn/ \n");
        if(NULL != p_turn && NULL == p_seat)
        {
            p_turn = p_turn + 7;
            //�ָ��һ�����ƻ�ɫ�͵���
            if(*p_turn == 'S') //SPADES����
            {
                turn[0] = 'S';
                if(*(p_turn + 8) == ' ')
                    turn[1] = *(p_turn + 7);
                else
                    turn[1] = 'T';
            }
            if(*p_turn == 'H') //HEARTS����
            {
                turn[0] = 'H';
                if(*(p_turn + 8) == ' ')
                    turn[1] = *(p_turn + 7);
                else
                    turn[1] = 'T';
            }
            if(*p_turn == 'C') //CLUBS÷��
            {
                turn[0] = 'C';
                if( *(p_turn + 7) == ' ')
                    turn[1] = *(p_turn + 6);
                else
                    turn[1] = 'T';
            }
            if(*p_turn == 'D') //DIAMONDS��Ƭ
            {
                turn[0] = 'D';
                if(*(p_turn + 10) == ' ')
                    turn[1] = *(p_turn + 9);
                else
                    turn[1] = 'T';
            }
        }
        //��ȡת����Ϣ����
        //��ȡ������Ϣ
        p_river = strstr(readbuf, "river/ \n");
        if(NULL != p_river && NULL == p_seat)
        {
            p_river = p_river + 8;
            //�ָ��һ�����ƻ�ɫ�͵���
            if(*p_river == 'S') //SPADES����
            {
                river[0] = 'S';
                if(*(p_river + 8) == ' ')
                    river[1] = *(p_river + 7);
                else
                    river[1] = 'T';
            }
            if(*p_river == 'H') //HEARTS����
            {
                river[0] = 'H';
                if(*(p_river + 8) == ' ')
                    river[1] = *(p_river + 7);
                else
                    river[1] = 'T';
            }
            if(*p_river == 'C') //CLUBS÷��
            {
                river[0] = 'C';
                if( *(p_river + 7) == ' ')
                    river[1] = *(p_river + 6);
                else
                    river[1] = 'T';
            }
            if(*p_river == 'D') //DIAMONDS��Ƭ
            {
                river[0] = 'D';
                if(*(p_river + 10) == ' ')
                    river[1] = *(p_river + 9);
                else
                    river[1] = 'T';
            }


        }

        //��ȡ������Ϣ����
        if( (NULL != p_river && NULL != p_seat) || (NULL != p_turn && NULL != p_seat) )//��ʱ������Ϣ�����ת�ƺ���Ҳ��seat��,��ղ���Ҫ��һ�ֵ���Ϣ������һ��
        {
            memset(flop, '\0', 7);
            memset(turn, '\0', 3);
            memset(river, '\0', 3);
        }

        //����card����
            for(i = 0; i < 4;i++)
            {
                card[i] = hold[i];
                hold_flag = 1;
            }
        //����card���Ź�����
            if(flop[0] != '\0')
            {
                for(i = 4; i < 10;i++)
                {
                    card[i] = flop[i - 4];
                    flop_flag = 1;
                }
            }
        //����cardһ��ת��
            if(turn[0] != '\0')
            {
                for(i = 10; i < 12;i++)
                {
                    card[i] = turn[i - 10];
                    turn_flag = 1;
                }
            }
        //����cardһ�ź���
            if(river[0] != '\0')
            {
                for(i = 12; i < 14;i++)
                {
                    card[i] = river[i - 12];
                    river_flag = 1;
                }
            }

        //���������ж�����������ң�������ѯ���ߺ�äע�������г��롢ʣ��������
        //�������ۼ�Ͷע��������һ����Чaction������ʱ�������ɽ���Զ���У��ϼ����ڵ�һ��
        //��ǰ�׳��ܽ�����Ͷע���ۼƣ�
        p_inquire = strstr(readbuf, "inquire/ \n");
		
	
        if( NULL != p_inquire  )//��������ƣ���ѯ����Ϣ���ٴ����ر��ǲ��ٸ�������������Ϣ����������ˣ�����ɷ�������Ϣ����
        {
            
			fold_num = 0;//�ۼ�ͳ��fold�Ķ�����
			check_num = 0;//�ۼ�check������
			raise_num = 0;
			inquire_num_players = 0;
			all_num = 0;
			
			//��ѯ����Ϣ�еõ��ۼƵ���ע������ߣ����ҵó�fold�������������߸����������ƣ��������������Ѳʳصĳ�����Լ���
            p_inquire = p_inquire + 10;//ָ��ѯ����Ϣ�ĵ�һ������ĸ
            while(1)
            {
                while(*p_inquire != '\n')//��λ����һ�лس���
                    p_inquire++;
                temp = p_inquire;//��¼�س���λ��
				inquire_num_players =inquire_num_players +1;
				if(*(p_inquire - 2) == 'd' && *(p_inquire - 3) == 'l')
					fold_num++;
				else if(*(p_inquire - 2) == 'k' && *(p_inquire - 3) == 'c')
					check_num++;
				else if(*(p_inquire - 2) == 'e' && *(p_inquire - 3) == 's')	
					raise_num++;
				else if(*(p_inquire - 2) == 'n' && *(p_inquire - 3) == 'i')	
					all_num++;
					
                {//���������ѯ����Ϣ�е�һ�е��ۼ���ע��
                    p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
                    p_inquire = p_inquire -1;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵڶ����ո�
                        p_inquire--;
                    p_inquire = p_inquire +1;//ָ�������ע��ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_1[i++] = *p_inquire++;
                    }
                }

                p_inquire = temp + 1; //����ָ�ص�һ���س�,��������һ���س�

                if( *p_inquire == 't')//��λ�����һ��
                {
                    while( *p_inquire != '\n') //Ѱ��total pot��β
						p_inquire++;
					p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
					p_inquire = p_inquire +1;//ָ��total pot�ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }


					break;//�Խ��е����һ��total pot������whileѭ���������Ѱ����һ��
                }
                //Ѱ�ҵڶ����س�
                while( *p_inquire != '\n') //Ѱ�ҵڶ��н�β
                    p_inquire++;
                temp = p_inquire;//��¼�س���λ��
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
                    p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
                    p_inquire = p_inquire -1;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵڶ����ո�
                        p_inquire--;
                    p_inquire = p_inquire +1;//ָ�������ע��ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_2[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //����ָ�صڶ����س�,�������ڶ����س�
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //Ѱ��total pot��β
						p_inquire++;
					p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
					p_inquire = p_inquire +1;//ָ��total pot�ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//�Խ��е����һ��total pot������whileѭ���������Ѱ����һ��
                }
                //Ѱ�ҵ������س�
                while( *p_inquire != '\n') //Ѱ�ҵ����н�β
                    p_inquire++;				
				temp = p_inquire;//��¼�س���λ��
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
                    p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
                    p_inquire = p_inquire -1;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵڶ����ո�
                        p_inquire--;
                    p_inquire = p_inquire +1;//ָ�������ע��ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_3[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //����ָ�ص������س�,�������������س�
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //Ѱ��total pot��β
						p_inquire++;
					p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
					p_inquire = p_inquire +1;//ָ��total pot�ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//�Խ��е����һ��total pot������whileѭ���������Ѱ����һ��
                }
                //Ѱ�ҵ��ĸ��س�
                while( *p_inquire != '\n') //Ѱ�ҵ����н�β
                    p_inquire++;
					
				temp = p_inquire;//��¼�س���λ��
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
                    p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
                    p_inquire = p_inquire -1;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵڶ����ո�
                        p_inquire--;
                    p_inquire = p_inquire +1;//ָ�������ע��ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_4[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //����ָ�ص��ĸ��س�,���������ĸ��س�
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //Ѱ��total pot��β
						p_inquire++;
					p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
					p_inquire = p_inquire +1;//ָ��total pot�ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//�Խ��е����һ��total pot������whileѭ���������Ѱ����һ��
                }
                //Ѱ�ҵ�����س�
                while( *p_inquire != '\n') //Ѱ�ҵ����н�β
                    p_inquire++;
				temp = p_inquire;//��¼�س���λ��	
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
                    p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
                    p_inquire = p_inquire -1;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵڶ����ո�
                        p_inquire--;
                    p_inquire = p_inquire +1;//ָ�������ע��ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_5[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //����ָ�ص�����س�,������������س�
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //Ѱ��total pot��β
						p_inquire++;
					p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
					p_inquire = p_inquire +1;//ָ��total pot�ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//�Խ��е����һ��total pot������whileѭ���������Ѱ����һ��
                }
                //Ѱ�ҵ������س�
                while( *p_inquire != '\n') //Ѱ�ҵ����н�β
                    p_inquire++;
				
				temp = p_inquire;//��¼�س���λ��
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
                    p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
                    p_inquire = p_inquire -1;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵڶ����ո�
                        p_inquire--;
                    p_inquire = p_inquire +1;//ָ�������ע��ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_6[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //����ָ�ص������س�,�������������س�
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //Ѱ��total pot��β
						p_inquire++;
					p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
					p_inquire = p_inquire +1;//ָ��total pot�ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//�Խ��е����һ��total pot������whileѭ���������Ѱ����һ��
                }
                //Ѱ�ҵ��߸��س�
                while( *p_inquire != '\n') //Ѱ�ҵ����н�β
                    p_inquire++;
				temp = p_inquire;//��¼�س���λ��
				
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
                    p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
                    p_inquire = p_inquire -1;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵڶ����ո�
                        p_inquire--;
                    p_inquire = p_inquire +1;//ָ�������ע��ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_7[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //����ָ�ص��߸��س�,���������߸��س�
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //Ѱ��total pot��β
						p_inquire++;
					p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
					p_inquire = p_inquire +1;//ָ��total pot�ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }

					break;//�Խ��е����һ��total pot������whileѭ���������Ѱ����һ��
                }
                //Ѱ�ҵڰ˸��س�
                while( *p_inquire != '\n') //Ѱ�ҵڰ��н�β
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
                    p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
                    p_inquire = p_inquire -1;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵڶ����ո�
                        p_inquire--;
                    p_inquire = p_inquire +1;//ָ�������ע��ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        bet_8[i++] = *p_inquire++;
                    }
                }
                p_inquire = temp + 1; //����ָ�صڰ˸��س�,�������ڰ˸��س�
                if( *p_inquire == 't')
                {
                    while( *p_inquire != '\n') //Ѱ��total pot��β
						p_inquire++;
					p_inquire= p_inquire -2;//�����ո�
                    while( *p_inquire != ' ') //Ѱ�ҵ�һ���ո�
                        p_inquire--;
					p_inquire = p_inquire +1;//ָ��total pot�ĵ�һ���ַ�
                    i = 0;
                    while(*p_inquire != ' ')
                    {
                        total_pot[i++] = *p_inquire++;
                    }
					break;//�Խ��е����һ��total pot������whileѭ���������Ѱ����һ��
                }
            }
			p_inquire = NULL;//��ָ���Ϳ�
            //ѯ����Ϣ�в��ҳ������ۼ���ע���ڴ�֮�� ������ע��������
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
			//һ��ѯ����Ϣ�������������
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
			
            //��ѯ����Ϣ�еõ��Լ�����ʣ��ĳ�������µ�Ͷע�����δ�ҵ���������տ�ʼ����ʱ�Լ���û����ע������Ϊ��ʼֵ
            temp = strstr(readbuf, argv[5]);
            if(NULL != temp)
            {
                //����һ�ε�ֵ�����
                memset(last_jetton, '\0', 15);
                memset(last_bet, '\0', 10);

                while(*temp != ' ')//Ѱ�ҵ�һ���ո�
                    temp++;
                temp = temp + 1;//������һ���ո�
                for(i = 0;*temp != ' '; temp++,i++)
                {
                    last_jetton[i] = *temp;
                }
                temp = temp + 1;//�����ڶ����ո�
                while(*temp != ' ')//Ѱ�ҵ������ո�
                    temp++;
                temp = temp + 1;//�����������ո�
                for(i = 0;*temp != ' '; temp++,i++)
                {
                    last_bet[i] = *temp;
                }
                temp = NULL;
            }



            /*�˴���������ľ��߳��� ���Ź����淵�ط�������*/
            //���������������������Ļ�Ӧ
            //���������Ƶĳ���
			if(inquire_num_players - fold_num == 1)	//������Ҽ�ȥ���Ƶ��������һ��˵�������˶����ƣ���ʱ�Լ������ע��òʳؽ��
			{
				memset(buf, 0, BUF_SIZE);
				strcat(buf, "raise \n");
				write(clnt_sock, buf, strlen(buf) + 1);
				fold_num = 0;//�������㣬ӭ����һ��ѯ����Ϣ
			}
			else
			{
				//���������Ƶĳ���
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
				//���������Ƶĳ���
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
				//���������Ƶĳ���
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

				//���������Ƶĳ���
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

				fold_num = 0;//�������㣬ӭ����һ��ѯ����Ϣ
				inquire_num_players = 0;//ѯ����Ϣ���ܺ��ܵ���ң��������Ƶ���ң�
			}

        }
        //��һ�ֽ���
        if( NULL != strstr(readbuf, "pot-win/ \n") )
        {
		
        }
        //��һ�ֽ���
		
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
