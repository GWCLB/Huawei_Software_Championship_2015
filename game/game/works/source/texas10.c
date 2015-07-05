#include"texasv30g.h"


char *deck="23456789TJQKA";//一副牌的值和花色
char *suit="CDHS";
int deal[7];
int dealtmp[7]={0}; 
char card[50] = {'\0'}; //传递给我的牌

extern int my_blind;
extern int fold_num;//累计统计fold的队友数
extern int check_num;//累计check的人数
extern int raise_num;
extern int all_num;
extern int inquire_num_players;
extern int game_round_num;

int bet;
int pot;
int blind;


 int pork_max(int *deal,int n,int m)//为了计算公共牌值最大的
 {
     int max=deal[n]>>2;
     int j;
     for(j=n+1;j<=m;++j)
         if((deal[j]>>2)>max)
             max=(deal[j]>>2);
     return max;
 }
 int suit_num(int *deal,int n,int m )//判断m-n+1张牌的花色
 {
     int five=0;
     int four=0;
     int three=0;
     int two=0;
     int count_suit_sub[4]={0};
     int j;
     for(j=n;j<=m;++j)
         ++count_suit_sub[deal[j]&3];
     for(j=0;j<4;++j)
     {
         if(5==count_suit_sub[j])
             five=1;
         else if(4==count_suit_sub[j])
             four=1;
         else if(3==count_suit_sub[j])
             three=1;
         else if(2==count_suit_sub[j])
             two=1;
     }
     if(five)
         return 5;
     else if(four)
         return 4;
     else if(three)
         return 3;
     else if(two)
         return 2;
     else
     return 1;//五张公共牌不会出现1的情况
 }

  int pork_same_num(int *deal,int n,int m)//判断m-n+1张牌的牌型
 {

     int count_sub[13]={0};
     int four=0;
     int three=0;
     int two=0;
     int straight=0;
     int j;
     int i;
     for(j=n;j<=m;++j)
         ++count_sub[deal[j]>>2];
     if(m-n+1>=5)
     {
         for(j=0;j<=8;++j)
         //A2345也算顺子
         if((count_sub[j]==1&&count_sub[j+1]==1&&count_sub[j+2]==1&&count_sub[j+3]==1&&count_sub[j+4]==1)||(count_sub[0]==1&&count_sub[1]==1&&count_sub[2]==1&&count_sub[3]==1&&count_sub[12]==1))
                straight=1;//顺子
      }
     for(i=0;i<=12;++i)
         {
             if(4==count_sub[i])
                 four=1;
             else if(3==count_sub[i])
                 three =1;
             else if(2==count_sub[i])
                 ++two;
        }
     if(four)
         return 6;
     else if(three&&two)
         return 5;
     else if(straight)
         return 4;
     else if(three)
         return 3;
     else if(2==two)
         return 2;
     else if(two)
         return 1;
     else
         return 0;
 }

  int judgemypork(int pork_num)
  {
      
      int pairs_num=0;//对牌数
      int three_num=0;//三条数2.11
      int four_num=0;//四条数0.024
      int flush_num=0;//同花数0.197
      int straight_num=0;//顺子数0.39
      int count[13]={0};//手牌中每种点数的重复次数

      int count_suit[4]={0}; //手牌中每种花色的重复次数

	  
      //***************************************************我是分割线

  if(2==pork_num)//当前有两张牌
  {
    int j,i;
    int *p_deal=deal;
	int round_rate=game_round_num/5;

   for(i=0;i<2 ;++i)
   deal[i]=((strchr(deck,card[2*i+1])-deck)<<2)+(strchr(suit,card[2*i])-suit);



   for(j=0;j<2;j++)
    ++count[p_deal[j]>>2];//利用计数器来记录每张牌重复的个数

   for(j=12;j>=0;--j) //记录了对牌对应count的下标 j
     if(2==count[j])
     {
          pairs_num=1;
          break;
     }

		 if(pairs_num&&j>=9||((deal[0]>>2)==12&&(deal[1]>>2)==11)||((deal[1]>>2)==12&&(deal[0]>>2)==11))//两张牌是对牌且牌值大于JJ QQ KK AA AK (1)
        {
		 if(((deal[0]>>2)==(deal[1]>>2))&&(deal[0]>=44))//KK AA
			 return raise2;
		 else if(bet<=7*blind)
			 return raise;
		 else if(bet<=(14*blind+19))
			 return call;
		 else
			 return fold;

        }
	 
		 else if(pairs_num) //66 77 88 99 TT (2,3 4 5) 改成1-10
     {
		 
		  if(j>=6)//对子大于6
		  {
			  if(bet<=3*blind+19)
				  return raise;
			  else if(bet<=7*blind+19)
				  return call;
			  else
				  return fold;
		  }
		 else if(bet<=6*blind+19)
			 return check;
         else
             return fold;		 
     }

		else if((deal[0]>>2)>=8&&(deal[1]>>2)>=8)// AQ AJ AT KQ KJ KT QJ QT JT(3 4 5)
	 {
		 if(bet<=3*blind+19)
			 return raise;
		 else if(bet<=7*blind+19)
			 return call;
         else
             return fold;
	 }

     	
		else if(((deal[0]&3)==(deal[1]&3))&&((deal[0]>=28)&&(deal[1]>=28)))//9Ts 9Js 9Qs TJs TQs TKs TAs JQs JKs JAs QKs QAs  
			 {
				 //if(((deal[0]>>2)==7&&(deal[1]>>2)==11)||((deal[1]>>2)==7&&(deal[0]>>2)==11))//9Ks不要
					// return fold;
				// else
				 if(bet<=4*blind+19)
					return raise;
				 else if(bet<=6*blind+19)
					 return call;
				 else
					 return fold;

			 }

		else if(((deal[0]&3)==(deal[1]&3))&&(deal[0]>=48||deal[1]>=48))//As+9s--As+2s 
				 {
					if(bet<=4*blind+19)
						return call;
					else
						return fold;
				}

		else if((deal[0]&3)==(deal[1]&3))		//同花色	 
			 {
				 if((deal[0]>>2)>=6&&(deal[1]>>2)>=6)
					{
						if(bet<=3*blind+19)
							return raise;
						else if(bet<=7*blind+19)
							return check;
						else
							return fold;
					}

				else
				 {
					 if(round_rate&1)
					 {
						if(bet<=4*blind+19)
						 return call;
						else
						 return fold;
					 }
					 else
						 return fold;
				 }
				 				
			 }
			

			 else
			 {
				 if(my_blind&&!raise_num&&!all_num)
				 {
					 if(round_rate&1)
						 return call;
					 else
						 return fold;
				 }
				 else if(my_blind)
				{
					if(round_rate&1)
					{
						if(bet<=4*blind+19)
							return call;
						else
							return fold;
					}
					else
						return fold;

				 }				
				 else
					 return fold;
			 }			 				 
   
   }
    //**********************************************************************我是分割线

   if(5==pork_num)
   {
       int j,i;
    int *p_deal=deal;
	int round_rate=game_round_num/10;

    for(i=0;i<5;++i)
         deal[i]=((strchr(deck,card[2*i+1])-deck)<<2)+(strchr(suit,card[2*i])-suit);


       for(j=0;j<5;++j)//五张牌花色计数
           ++count_suit[p_deal[j]&3];

       for(j=0;j<5;j++)//五张牌点数计数
         ++count[p_deal[j]>>2];

       for(j=0;j<4;++j)
       {
        if(5==count_suit[j])
             flush_num=1;//同花
       }

       for(j=0;j<=8;++j)
           //A2345也算顺子
           if((count[j]==1&&count[j+1]==1&&count[j+2]==1&&count[j+3]==1&&count[j+4]==1)||(count[0]==1&&count[1]==1&&count[2]==1&&count[3]==1&&count[12]==1))
                  straight_num=1;//顺子

       for(j=12;j>=0;--j)
       {
         if(count[j]==2)
               ++pairs_num;
         else if(3==count[j])
             three_num=1;
         else if(4==count[j])
             four_num=1;

       }
         if(flush_num&&straight_num)//同花顺
                   return raise2;

          else if(four_num)//五张时只有一人能构成四条
               return raise2 ;

          else if(three_num&&pairs_num==1)
           {
               
                   return raise2;
               
           }

           else if(flush_num)
                   return raise2;		  
           
           else  if(straight_num)
                   return raise2;

           else if(three_num)//三条几率挺高的，需要分情况讨论
           {
               if((deal[0]>>2)==(deal[1]>>2))
               {
                   if(bet<=8*blind)
                   return raise2 ;
                   else
                   return call;
               }
               else if((deal[2]>>2==deal[3]>>2)&&(deal[2]>>2==deal[4]>>2))//三条在公共牌
               {
				   if(raise_num||all_num)
                  {
					  if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }

					else if(bet<=10*blind)
						return call;
					else                 
						return fold;
				   }
				   else
					   return call;
               }
               else//三条中一对在公共牌,对方可能  听同花  听顺子 葫芦
               {
				   if(raise_num||all_num)
                  {
					  if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }
					  else if(bet<=12*blind)
						return call;
					  else
						return fold;
				   }
				   else
					   return raise2;
               }

           }

             else if(pairs_num==2)//两对  0.95
           {
			   
				if(bet>=8*blind)
				 return  check;
			   else
				 return  raise2;


           }

		   //*****************************对子
			else if(pairs_num==1&&(deal[0]>>2)==(deal[1]>>2)&&(deal[0]>=36))//JJ QQ KK AA
			 {
				   if(raise_num||all_num)
				   {
					   if(bet>=6*blind+19)
					      return check;
					   else
						  return raise;
				   }					   
				   else
					   return raise;

			 }

            else if(pairs_num==1&&((deal[0]>>2)==(deal[1]>>2)))//小对牌在底牌中
           {
			   int max_number=pork_max(deal,2,4);

			   if(raise_num||all_num)
			  { 
				 if(bet<=10*blind)
				 {
				   if(deal[0]>max_number)
						return call;
				   else
					   return check;
				 }
			    else
                   return fold;
			  }

			   else
				{
					 if(deal[0]>max_number)
						return raise;
					 else
					   return check;
					   
			    }

           }
			
           else if(pairs_num==1)
           { 
			    int pork_same_number=pork_same_num(deal,2,4);
				int suit_number=suit_num(deal,0,4);

			   if(pork_same_number)//在公共牌中
			   {
				   if(raise_num||all_num)
				   {
					    if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					    {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					    }

						else if(bet<=8*blind+19)				   																   
						{
							if(suit_number==4)							
								return call;

							else 
								 return check;
					    }
					   else
						   return fold;				   
				   }

				   else
					   return call;

			   }

			 


			   else if(!pork_same_number)//底牌与公共牌组成对子
			   { 
				   if(raise_num||all_num)
				   {
					   if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }

					   else if(bet<=10*blind)
							return call;

					else													
						 return fold;
					   

				   }
				   else
					   return check;
			   }
				
			}

			//*************************************************高牌
           else
			{
				if(raise_num||all_num)
				{
					 if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=6*blind+19)
							 return raise;
					      else if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }

					 else if(bet<=9*blind+19)
					{
						int suit_number=suit_num(deal,0,4);
						int max_number=pork_max(deal,2,4);

					  if((deal[0]>>2)>max_number||(deal[1]>>2)>max_number)
						return call;
					
					  else if(suit_number==4)
						return check;

					  else if(bet<=8*blind+19)
						return check;

					  else
						return fold;
					}  
				  else 
					return fold;
			  }

				else//无加注的对手
				{
					int max_number=pork_max(deal,2,4);
					int suit_number=suit_num(deal,0,4);

					if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }
					else if((deal[0]>>2)>max_number||(deal[1]>>2)>max_number)
					{
						if(round_rate&1)
							return raise;
						else
							return call;

					}
					else if(suit_number==4)
						return call;
					else
					{
						if(round_rate&1)
							return call;
						else
							return fold;
					}
					
					
				}
		   }                  	      
		 
   }
   //*************************************************************************我就是分割线，对六张牌的决策
  if(6==pork_num)
   {
        int j,i;
		int *p_deal=deal;
		int round_rate=game_round_num/10;

       for(i=0;i<6 ;++i)
         deal[i]=((strchr(deck,card[2*i+1])-deck)<<2)+(strchr(suit,card[2*i])-suit);


       for(j=0;j<6;++j)//6张牌花色计数
           ++count_suit[p_deal[j]&3];

       for(j=0;j<6;j++)//6张牌点数计数
         ++count[p_deal[j]>>2];

       for(j=0;j<4;++j)
       {
        if(5<=count_suit[j])
             flush_num=1;//同花
       }

       for(j=0;j<=8;++j)
           //A2345也算顺子
           if((count[j]&&count[j+1]&&count[j+2]&&count[j+3]&&count[j+4]||count[0]==1&&count[1]==1)&&(count[2]==1&&count[3]==1&&count[12]==1))
                  straight_num=1;//顺子

       for(j=12;j>=0;--j)
       {
         if(2==count[j])
             ++pairs_num;
         else if(3==count[j])
             ++three_num;
         else if(4==count[j])
             four_num=1;

       }
         if(flush_num&&straight_num)//同花顺
                return raise2;

          else if(four_num)//四条复杂，分以下情况
          {
              int pork_same_number=pork_same_num(deal,2,5);//公共牌牌型

              if(pork_same_number)//底牌和公共牌各两张，对方也可能有四条，比如我是33，对方44，公共为3344
               return raise;

              else if(3==pork_same_number )//三张一样的在公共牌中，其他人绝不可能组成四条，但在发第七张牌的时候或许能成为同花顺，不过几率几乎为0
                return raise2;

              else if(6==pork_same_number &&((deal[0]>>2)==12||(deal[1]>>2)==12))//四条出现在公共牌中若我有A，此剧必胜！
                 return raise2;
              else//大家都是四条，但是你手里没有A，弃牌吧
                  return fold;
         }

          else if(three_num&&pairs_num==1||three_num==2)//有葫芦时，也要分情况
           {
                int pork_same_number=pork_same_num(deal,2,5);//公共牌中点数相同的牌的数目
                if(three_num==1)
                {
                   if((deal[0]>>2)==(deal[1]>>2))//底牌相等
					{
						if(3==pork_same_number)//公共牌有三条,对方有大葫芦和四条概率相当大呢22 3334
							                    
								return call;                       
							
						else//22 2334
							                     
								return call;							
					}
					else//23 3324对方现在除了同花顺不可能有比葫芦更大的
                    return raise2;
               }
                else
					{       
						if(raise_num)            
							return call;                   
						else
							return raise;
					 }
          }

		  //****************************同花
           else if(flush_num)
           {
			  
               int pork_same_number=pork_same_num(deal,2,5);//公共牌中点数相同的牌的数目

               if((deal[0]&3)==(deal[1]&3))
               {
                   if(!pork_same_number )//公共牌没有点数相同的牌
                    return raise;

                   else if(bet<=8*blind)
					   return raise;
				   else
                    return call;
               }
               else//公共牌为3c5c7c8c，对手可能为同花顺
                   return raise;
          }

		  //***************************顺子
          else if(straight_num)
                  {
					  int pork_same_number=pork_same_num(deal,2,5);//有点赌了，因为可能是23 4567 对手可能是大顺子，而且当23 4566时可能是葫芦或者四条，赌了！！！
					  if(raise_num||all_num)
						  return call;
					  else
						  return raise;
					}


		   //***************************三条分下面情况
          else if(three_num)
           {
			   int pork_same_number=pork_same_num(deal,2,5);

               if((deal [0]>>2)==(deal[1]>>2))//2张同点数的底牌和一张公共组成三条22 2456 0.77
               {
				   if(raise_num||all_num)
					  {
						   if((deal[0]>>2)>=9)//JJ QQ KK AA
							   return raise;
					  

						else if(bet<=10*blind+19)
						  return call;

						else
						  return fold;
					  }
					  else
						  return raise;
                 
               }

			    else if(3==pork_same_number)//三条在公共牌,对方可能是  葫芦  四条
               {
				   if(raise_num||all_num)
				   {
						int max_number=pork_max(deal,2,5);

						if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }
				   else if(bet<=10*blind+19)
						{
							if((deal[0]>>2)>=max_number||(deal[1]>>2)>=max_number)
								return call;
							else
								return check;
						}
                        else                 
                            return fold;

				   }
				   else
					   return call;
               }
               else                       //23 2247 0.9         对手可能是  顺子  同花  葫芦          
				{
					if(raise_num||all_num)
					{
						if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }
						else if(bet<=10*blind+19)
							return call;
						else
							return fold;
					}
					else
						return raise;

				}
           }

		  //**************************有两对,六张牌有可能出现三对
             else if(pairs_num>=2)
           {
              int pork_same_number=pork_same_num(deal,2,5);//公共牌中点数相同的牌的数目

              if((deal [0]>>2)==(deal[1]>>2))//底牌相等
				{
                  if(2==pork_same_number)//22 3344 对手很可能有  葫芦  三条  四条
				  {
					  if(raise_num||all_num)
					  {
						  if((deal[0]>>2)>=9)//JJ QQ KK AA
							   return call;
					  
						  else if(bet<=10*blind+19)
						  return call;

						else
						  return fold;
					  }
					  else
						  return call;

				  }

                  else //22 3345 对手可能有   三条  顺子  同花   葫芦  四条，最后或许能葫芦，可以弃牌0.67
				  {
					   if(raise_num||all_num)
					  {
						 if((deal[0]>>2)>=9)//JJ QQ KK AA
							   return call;

						else if(bet<=10*blind+19)
						  return call;

						else
						  return fold;
					  }
					  else
						  return raise;
					  
					  
				  }				                     
				}

               else//底牌不等
               {
                    if(pork_same_number)//23 2344  23 3466对手可能是 三条  顺子  同花  葫芦  四条，最后或许能葫芦，可以弃牌0.75                        
					{
						if(raise_num||all_num)
					  {
						  if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
						{
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
						}
						else if(bet<=10*blind+19)
						  return call;

						else
						  return fold;
					  }

					  else
						  return raise;
						
					}

                    else//23 2345对手可能是  三条  顺子  同花0.68
                    {                      
						if(raise_num||all_num)
					  {
						  if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
						{
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
						}

						else if(bet<=10*blind+19)
						  return call;

						else
						  return fold;
					  }
					  else
						  return raise;                 
                    }
              }
           }

		  //***************************************************一对
            else if(pairs_num==1)
           {
			   int max_number=pork_max(deal,2,5);
               int pork_same_number=pork_same_num(deal,2,5);

                if(pork_same_number)//对子在公共牌中  对方可能是 两对 三条 顺子 同花等
                   {
					   if(raise_num||all_num)
					   {
						   
						   int suit_number=suit_num(deal,0,5);

						   if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK 
						{
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
						}

						   else if(suit_number==4)
							{
								if(bet<=10*blind+19)					     
									return call;
								else
									return fold;
							}

						   else if(bet<=10*blind+19)
								return check;

						   else 
								return fold;
							
					     }

						 else 
						   return call;
					   
					  
				   }
				else if(pairs_num==1&&(deal[0]>>2)==(deal[1]>>2)&&(deal[0]>=36))//大对子
			   {
				   if(raise_num||all_num)
					   return check;
				   else
					   return call;

			   }

                else //对子在手中    对方可能是 两对 三条 顺子 同花等
                {
					if(raise_num||all_num)
					{
						int suit_number=suit_num(deal,0,5);

						if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK 
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }
						else if(suit_number==4)
						{
							if(bet<=10*blind+19)
							   return call;
							else
								return fold;
						}
						else if(((deal[0]>>2)>=max_number)||((deal[0]>>2)>=max_number))
						 {
							if(bet<=10*blind+19)
							   return call;
						  else
						       return fold;
						 }

						else 
						  {						
							if(bet<=10*blind)
								return check;
							else
								return fold;                  
						  }
					}

					else
						return raise;
                }
           }

		   //*************************************高牌
           else
		   {
			   if(raise_num||all_num)
				{
					if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }
					else if(bet<=10*blind+19)
					{
						int suit_number=suit_num(deal,0,5);
						int max_number=pork_max(deal,2,5);

						if((deal[0]>>2)>max_number||(deal[1]>>2)>max_number)
							return call;

						else if(suit_number==4)
							return check;

						else if(bet<=10*blind+18)
							return check;

						else 
							return fold;
					}

					else
						return fold;
			   }

               else 
				{
						int suit_number=suit_num(deal,0,5);
						int max_number=pork_max(deal,2,5);
						if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }

						else if((deal[0]>>2)>max_number||(deal[1]>>2)>max_number)
							return raise;

						else if(4==suit_number)
							return call;

						else 
							return call;						
							
			   }
			}
			
   }
   
    //**************************************************分割线！下面是对七张牌的决策
   if(7==pork_num)
   {
        int j,i;
		int *p_deal=deal;
       for(i=0;i<7;++i)
         deal[i]=((strchr(deck,card[2*i+1])-deck)<<2)+(strchr(suit,card[2*i])-suit);


       for(j=0;j<7;++j)//7张牌花色计数
           ++count_suit[p_deal[j]&3];

       for(j=0;j<7;j++)//7张牌点数计数
         ++count[p_deal[j]>>2];

       for(j=0;j<4;++j)
       {
        if(5<=count_suit[j])
             flush_num=1;//同花
       }

       for(j=0;j<=8;++j)
           if(count[j]&&count[j+1]&&count[j+2]&&count[j+3]&&count[j+4]||count[0]==1&&count[1]==1&&count[2]==1&&count[3]==1&&count[12]==1)//A2345也算顺子
                  straight_num=1;//顺子


       for(j=12;j>=0;--j)
       {
         if(count[j]==2)
               ++pairs_num;
         else if(3==count[j])
             ++three_num;
         else if(4==count[j])
             four_num=1;

       }
           if(flush_num&&straight_num)//同花顺，大家都是同花顺，对方是更大同花顺几率几乎为0啊~
                return raise2;


          else if(four_num)//四条复杂，分以下情况
          {
              int pork_same_number=pork_same_num(deal,2,6);//公共牌中可以组成的牌型

              if(pork_same_number)//底牌和公共牌各两张，对方也可能有四条，比如我是33，对方44，公共为33445；对方可能组成同花顺如33456 +78
                    return raise;

              else if(3==pork_same_number )//三张一样的在公共牌中
                  {
                    if((deal[0]>>2)==(deal[1]>>2))//底牌相等，说明公共牌有一对和底牌相等的底牌为33，公共为44433，对方可能组成4444的四条；
                        return raise;

                    else//底牌不同牌型为34 33355，对方可能为5555；对方可能组成同花顺
                        return raise;
                    }
              else if(6==pork_same_number &&((deal[0]>>2)==12||(deal[1]>>2)==12))//四条出现在公共牌中若我有A，此局必胜；对方无法组成同花顺
                  return raise2;

              else//大家都是四条，但是你手里没有A，弃牌吧
			  {
				  if(raise_num||all_num)
						return fold;
				  else
					  return call;
			  }
         }

          else if((three_num&&pairs_num)||(three_num==2))//葫芦复杂，分以下情况
           {
               int pork_same_number=pork_same_num(deal,2,6);//公共牌可以组成的牌型
               if(three_num==1)
			   {
				//公共牌葫芦，对方很可能有葫芦四条，但是绝不可能有同花顺
					if(5==pork_same_number)
					{
						 if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK 
						{
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
						}
							 
						 else
							return fold;
					 }
               //公共牌是三条，对方可能有葫芦,四条或者同花顺
					 else if(3==pork_same_number)
						{                   
							 return call;
						 }
               //公共牌是两对对方可能有葫芦，四条或者同花顺，这里deal[0],和deal[1]对比也应该加进去
					else if(2==pork_same_number)
					{                    
                       return call;
					 }
               //公共牌是一对，对方可能有葫芦，四条或者同花顺，这里deal[0],和deal[1]对比也应该加进去
					 else
						 return raise2;
               }
               else
               {
                   
                  return call;
                  
               }
           }

           else if(flush_num)//同花
           {
               int suit_number=suit_num(deal,2,6);
               //同花出现在公共牌中，大家都是同花（公共牌点数各不相同），则对方最多只能是更大的同花同花顺
               if(5==suit_number)
               {
                   //底牌花色至少一张与公共牌相同并且牌点数为A,k
                   if(((deal[0]&3)==(deal[3]&3)&&(deal[0]>>2)>=11)||(((deal[1]&3)==(deal[3]&3))&&((deal[1]>>2)>=11)))
                       return raise2;
                   else
                        return fold;

               }
               //底牌中花色不同，说明公共牌四张花色相同
               else if((p_deal[0]&3)!=(p_deal[1]&3))
               {
                  int pork_same_number=pork_same_num(deal,2,6);//公共牌可以组成的牌型
                  if(!pork_same_number)//若公共牌为高牌，则除非对方是同花顺（若公共牌差一两张可以组成同花顺，可以赌一次，若只是四张同花不顺，肯定要跟的）
                      return raise2;

                  else//公共牌有一对，对方可能有同花顺或者葫芦                  
                      return call;
                  
               }
               //底牌中花色相同
               else if((p_deal[0]&3)==(p_deal[1]&3))
               {
                    int pork_same_number=pork_same_num(deal,2,6);//公共牌可以组成的牌型
                    if(!pork_same_number)//若公共牌为高牌
                        return raise2;

                    else if(1==pork_same_number)//若公共牌有一对
                        return raise;

                    else if(2==pork_same_number)//若公共牌有2对,对方有葫芦四条几率非常高
                    {
						if(bet<=15*blind)
							return raise2;

						else                       
                            return call;//**********
                        
                    }

                    else if(3==pork_same_number)//若公共牌有3条，对方有葫芦四条几率非常高
					{
						if(bet<=15*blind)
							return raise;

						else
                            return call;//****************
					}
               }
           }

           else  if(straight_num)//顺子
           {
               int pork_same_number=pork_same_num(deal,2,6);//公共牌可以组成的牌型
               int suit_number=suit_num(deal,2,6);//公共牌花色数
               int max_number=pork_max(deal,2,6);//公共牌点数最大值

               if(4==pork_same_number)//顺子在公共牌中，对方可能有更大的顺子同花或者同花顺，对手不可能组成葫芦和四条
               {
                   if(2==suit_number)//公共牌中只有两张花色相同的牌，不可能组成同花顺
                   {
					   if(((deal[0]>>2)-max_number==1)||((deal[1]>>2)-max_number==1))
                           return raise2;
                       else
					   {
						   if(bet<=14*blind+19)
							   return check;
						   else
								return fold;
					   }
				   }
                   else //公共牌中只有3张以上花色相同的牌
                       {
						    if(bet<=14*blind+19)
							   return call;
						   else
								return fold;
						   
						}//这里加入随机跟或者弃牌较好
               }
               else if(!pork_same_number)//公共牌组成高牌
                 {
					int suit_number=suit_num(deal,2,6);
					if(raise_num||all_num)
				   {
					   
	                   if(suit_number>=3)//对手可能同花
			              return check;                  
					   else 
				          return call;          
				   }

				   else
					   return raise2;
					

			   }
               else if(1==pork_same_number)//公共牌有一对，对可能有同花，葫芦，四条，同花顺23446
                   
				   {
					   if(raise_num||all_num)
					   {
						   if(bet<=14*blind+19)
								return call;//随机弃牌0.97
						   else
							   return fold;
					   }
					  
					   else
						   return raise2;
					}

               else if(2==pork_same_number||3==pork_same_number)
                  {
					   if(raise_num)
					   {
						   if(bet<=14*blind+19)
								return call;//随机弃牌0.97
						   else
							   return fold;
					   }
					   else if(all_num)
					   {
						   if(bet<=14*blind+19)
							   return call;
						   else
								return fold;
					   }
					   else
						   return raise2;//23434;23444 0.6 0.8
			   }
           }

		   //**************************************三条
           else if(three_num)
           {
               int pork_same_number=pork_same_num(deal,2,6);
               if(3==pork_same_number)//三条出现在公共牌中   对方可能为 顺子 同花 葫芦 四条                
				 {
					 if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }
					 else if(raise_num||all_num)
						return fold;
					else 
						 return call;
			     }

               else if(1==pork_same_number)//公共牌为一对,对方 顺子  同花  葫芦 23 22456
               {
                      
                   //int suit_number=suit_num(deal,2,6);
				   if(raise_num||all_num)
				   {
					   if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }

					   else if(bet<=12*blind+19)	                  
			              return call;

					   else 
				          return fold;          
				   }

				   else
					   return raise;
                      
               }
               else if(0==pork_same_number)
			   {
				    int suit_number=suit_num(deal,2,6);
					 if(pairs_num==1&&(deal[0]>>2)==(deal[1]>>2)&&(deal[0]>=36))//大对子
					{
					 if(raise_num||all_num)
					   return call;
					 else
					   return raise;

					}
					if(raise_num||all_num)
				   {
					   if(bet<=14*blind+12)
							return call;                  
						else
							return fold;					   	                   
				   }

				   else
					   return raise;
                   
			   }

           }

		   //********************************************两对
             else if(pairs_num>=2)
           {
               int pork_same_number=pork_same_num(deal,2,6);

               if(2==pork_same_number)//公共牌是两对  对方 可能为三条 顺子 同花 葫芦 四条等
			   {
				   if(pairs_num==1&&(deal[0]>>2)==(deal[1]>>2)&&(deal[0]>=36))//大对子
					{
					 if(raise_num||all_num)
					   return call;
					 else
					   return raise;

					}
				   else if(raise_num||all_num)
				   {
					   if(bet<=12*blind+19)
						return call;
					   else
						return fold;
				   }
				   else
					   return call;
			   }

               else if(pork_same_number)//公共牌为一对  23 23446  23 34457   对方 可能为三条 顺子 同花 葫芦 四条等
               {

				   if(raise_num||all_num)
				   {
					   if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					  {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					  }
					   else  if(bet<=12*blind+19)
						return call;
					   else
						return fold;
				   }
				   else
					   return call;
                        
                   
               }

               else if(!pork_same_number)//公共牌是高牌 23 23457  可能为三条 顺子 同花
               {
				   int suit_number=suit_num(deal,2,6);

				   if(suit_number>=3)//对手可能是同花
				   {
					   if(raise_num||all_num)						
						{
							if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
						   {
						     if(bet<=(14*blind+19))
							   return call;
						     else
							   return fold;
						    }
							else if(bet<=12*blind+19)
								return call;
							else
								return fold;
						 }	

					   else
							return call;
				   }

				   else if(raise_num||all_num)//对手可能是顺子  三条
					   {
						   if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					      {
						    if(bet<=(14*blind+19))
							  return call;
						    else
							  return fold;
					      }
						   else  if(bet<12*blind+19)
							   return call;
						   else
							   return fold;
						}

				   else                                   
                      return raise;
               }
           }

		   //******************************************一对
            else if(pairs_num==1)
			{
				int max_number=pork_max(deal,2,6);//公共牌点数最大值
				int pork_same_number=pork_same_num(deal,2,6);
				int suit_number=suit_num(deal,2,6);

				
				 if(pork_same_number)
				{
					if(raise_num||all_num)
					{
						if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
						{
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
						 }
							return fold;
					}
					else
				        return call;
				}
				else if(((deal[0]>>2)>=max_number)||((deal[1]>>2)>=max_number))
				{
					if(raise_num||all_num)
					{
						if(pairs_num==1&&(deal[0]>>2)==(deal[1]>>2)&&(deal[0]>=36))//大对子
					
					         return call;
					
						else if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					   {
						  if(bet<=(14*blind+19))
							return call;
						  else
							return fold;
					    }

						else if(bet<=12*blind+19)
							return call;
						else
							return fold;
					}
					else
						return raise;
				}

				else
					{
						if(raise_num||all_num)	
						{
							if(pairs_num==1&&(deal[0]>>2)==(deal[1]>>2)&&(deal[0]>=36))//大对子
							{
							 if(raise_num||all_num)
								 return call;
							 else
							  return raise;

							}
							else if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
							{
							 if(bet<=(14*blind+19))
								return call;
							else
								return fold;
							}
							else if(bet<=12*blind+19)
								return call;
							else
								return fold;
						}
						else
							return call;
					}

			}

           else//既然到了这步，说明上面大家都没什好牌，若对方check，则跟
		   {
			   int max_number=pork_max(deal,2,6);

			   if(raise_num||all_num)				   
				{ 
					 if(((deal[0]>>2)==11)&&((deal[1]>>2)==12)||((deal[0]>>2)==12)&&((deal[1]>>2)==11))//AK
					 {
						if(bet<=(14*blind+19))
							return call;
						else
							return fold;
					}

					else if(bet<=12*blind+19)
					{
					 if((deal[0]>>2)>=max_number||(deal[1]>>2)>=max_number)
						return call;
					 else
						return check;
					}
					else
						return fold;
				}
			   else
				 {
					 if((deal[0]>>2)>=max_number||(deal[1]>>2)>=max_number)
						return raise;

					 else
						return call;
					 
				}
			}
		}
		
 }//函数末尾
 