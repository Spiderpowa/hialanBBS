/*-------------------------------------------------------*/
/* msg.c        ( WD_hialan BBS    Ver 1.00 )            */
/*-------------------------------------------------------*/
/* target : 水球訊息                                     */
/* create : 2003/01/21                                   */
/* update : 2003/01/21                                   */
/* change : hialan					 */
/*-------------------------------------------------------*/

#include "bbs.h"

char last_return_msg[128] = " 你還沒有丟過水球呦 !!";  /*最後一句水球回顧 by hialan*/
char watermode=0;
char no_oldmsg=0,oldmsg_count=0;            /* pointer */
msgque oldmsg[MAX_REVIEW];   /* 丟過去的水球 */

extern struct UTMPFILE *utmpshm;

int cmpuids(int ,user_info *);
int cmppids(pid_t, user_info *);


void
do_aloha(char *hello)
{
   int fd;
   PAL pal;
   char genbuf[200];

   sethomefile(genbuf, cuser.userid, FN_ALOHA);
   if ((fd = open(genbuf, O_RDONLY)) > 0)
   {
      user_info *uentp;
      int tuid;   
      
      sprintf(genbuf + 1, hello);
      *genbuf = 1;
      while (read(fd, &pal, sizeof(pal)) == sizeof(pal)) 
      {
         if ( (tuid = searchuser(pal.userid))  && tuid != usernum &&
             (uentp = (user_info *) search_ulistn(cmpuids, tuid, 1)) &&
             ((uentp->userlevel & PERM_SYSOP) || ((!currutmp->invisible || 
           uentp->userlevel & PERM_SEECLOAK) && !(is_rejected(uentp) & 1))))    
            my_write(uentp->pid, genbuf);
      }
      close(fd);
   }
}

void
show_last_call_in()
{
   char buf[200];
   
   sprintf(buf, "[1m[33;41m[[37m%s[33m][m[34;47m %s [m",
      currutmp->msgs[0].last_userid,
      currutmp->msgs[0].last_call_in);

   move(b_lines, 0);
   clrtoeol();
   refresh();
   outs(buf);
}

int
my_write(pid, hint)
  pid_t pid;
  char *hint;
{
  int len, a;
  int currstat0 = currstat;  
  char msg[80], genbuf[200];
  char c0 = currutmp->chatid[0];  
  FILE *fp;
  struct tm *ptime;
  time_t now;
  user_info *uin ;
  uschar mode0 = currutmp->mode;

  if(watermode > 0)
  {
     a = (no_oldmsg - watermode + MAX_REVIEW )%MAX_REVIEW;
     uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
  }
  else
     uin = (user_info*)search_ulist(cmppids, pid);

  if (( !oldmsg_count || !isprint2(*hint)) && !uin )
  {
     pressanykey("糟糕! 對方已落跑了(不在站上)! ~>_<~");
     watermode = -1;
     return 0;
  }

  currutmp->mode = 0;
  currutmp->chatid[0] = 3;
  currstat = XMODE;


  time(&now);
  ptime = localtime(&now);

  if (isprint2(*hint))
  {
    if (!(len = getdata(0, 0, hint, msg, 65, DOECHO,0))) 
    {
      pressanykey("算了! 放你一馬...");
      currutmp->chatid[0] = c0;
      currutmp->mode = mode0;
      currstat = currstat0;
      watermode = -1;
      return 0;
  }
/* Ptt */
    if(watermode > 0)
    {
      a = (no_oldmsg - watermode + MAX_REVIEW )%MAX_REVIEW;
      uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
    }

    strip_ansi(msg,msg,0);
    if (!uin  || !*uin->userid) {
       pressanykey("糟糕! 對方已落跑了(不在站上)! ~>_<~");
       currutmp->chatid[0] = c0;
       currutmp->mode = mode0;
       currstat = currstat0;
       watermode = -1;
       return 0;
    }

    watermode = -1;
    sprintf(genbuf, "丟%s水球:%.40s....? ", uin->userid, msg);
    if (getans2(0, 0, genbuf, 0, 2, 'y') == 'n') 
    {
      currutmp->chatid[0] = c0;
      currutmp->mode = mode0;
      currstat = currstat0;
      return 0;
    }
    if (!uin || !*uin->userid) 
    {
       pressanykey("糟糕! 對方已落跑了(不在站上)! ~>_<~");
       currutmp->chatid[0] = c0;
       currutmp->mode = mode0;
       currstat = currstat0;
       return 0;
    }
  }
  else 
  {
     strcpy(msg, hint + 1);
     strip_ansi(msg,msg,0);
     len = strlen(msg);
     watermode = -1;
  }
   now = time(0);
   if (*hint != 1) 
   {
      sethomefile(genbuf, uin->userid, fn_writelog);
      if (fp = fopen(genbuf, "a")) 
      {
        fprintf(fp, COLOR2"[1;33;41m【[37m %s [33m】[1;47;34m %s %s [0m[%s]\n",
          cuser.userid, (*hint == 2) ? "[1;33;42m廣播" : "", msg, Cdatelite(&now));
        fclose(fp);
      }
      sethomefile(genbuf, cuser.userid, fn_writelog);
      if (fp = fopen(genbuf, "a")) 
      {
        fprintf(fp, "To %s: %s [%s]\n", uin->userid, msg, Cdatelite(&now));
        fclose(fp);
        update_data();
        ++cuser.sendmsg;
        substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
      }
/* itoc.011104: for BMW */
      {
        fileheader bmw;
        time_t now = time(0);
        struct tm *ptime = localtime(&now);
        
        sprintf(bmw.date, "%02d:%02d", ptime->tm_hour, ptime->tm_min);
        strcpy(bmw.title, msg);
                                                                                
        bmw.savemode = 1;             /* 對方是接收端 */
        strcpy(bmw.owner, cuser.userid);
        sethomefile(genbuf, uin->userid, FN_BMW);
        rec_add(genbuf, &bmw, sizeof(fileheader));
                                                                                
        bmw.savemode = 0;             /* 我是傳送端 */
        strcpy(bmw.owner, uin->userid);
        sethomefile(genbuf, cuser.userid, FN_BMW);
        rec_add(genbuf, &bmw, sizeof(fileheader));
      }
      
/* hialan.020713 for 最後一句話水球回顧*/
      sprintf(last_return_msg, "\033[m 給 %s \033[1;33;44m %s \033[m", uin->userid, msg);
   }
   if (*hint == 2 && uin->msgcount) 
   {
      uin->destuip = currutmp;
      uin->sig = 2;
      kill(uin->pid, SIGUSR1);
   }
   else if (*hint != 1 && !HAS_PERM(PERM_SYSOP) && ( uin->pager == 3
       || uin->pager == 2 || (uin->pager == 4 && !(is_friend(uin) & 2)) ))
      pressanykey("糟糕! 對方防水了!");
   else {
//      if (uin->msgcount < MAXMSGS) 
      {
         uschar pager0 = uin->pager;
         uin->msgcount=0;
         uin->pager = 2;
         uin->msgs[uin->msgcount].last_pid = currpid;
         strcpy(uin->msgs[uin->msgcount].last_userid, currutmp->userid);
         strcpy(uin->msgs[uin->msgcount++].last_call_in, msg);
         uin->pager = pager0;
      }
      if (uin->msgcount  == 1 && kill(uin->pid, SIGUSR2) == -1 && *hint != 1)
         pressanykey("糟糕! 沒打中! ~>_<~");
      else if (uin->msgcount == 1 && *hint != 1)
         outz("[1m[44m水球丟過去了! *^o^Y[m");
   }
//   clrtoeol();
//   refresh();

   currutmp->chatid[0] = c0;
   currutmp->mode = mode0;
   currstat = currstat0;
   return 1;
}

static char t_display_new_flag =0;

int
t_display_new(int b_f_flag)
{
   int i, j;  /* j:已經下站的使用者*/
   char buf[256];
   user_info *uin;

   if(t_display_new_flag) return;

   else t_display_new_flag = 1;

   if(oldmsg_count && watermode > 0)
     {
         clrchyiuan(1, oldmsg_count + 5);
         move(1,0);
         clrtoeol();
         outs(
" [1;34m───────[37m水[34m─[37m球[34m─[37m回[34m─[37m顧[34m─────────"COLOR1" [Ctrl-R]往下切換 [34;40m────── [m");
         for(i=0 ; i < oldmsg_count ;i++)
                {
                 int a = (no_oldmsg - i - 1 + MAX_REVIEW )%MAX_REVIEW;
                 
                 uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
                 move(i+2,0);
                 clrtoeol();
                 if(i == 0) j = 0;
                 if(watermode-1 == i)
                 {
                   if(!uin)
                   {
                     if (!b_f_flag)
                       watermode = (watermode + oldmsg_count)%oldmsg_count+1;
                     else
                       watermode = (watermode+2*oldmsg_count-2)%oldmsg_count+1;
                     j++;
                     if (j != oldmsg_count) i = -1;
                   }
                   else
                     sprintf(buf,"> \033[1m[37;45m%s [33;44m%s[m",
                         oldmsg[a].last_userid,oldmsg[a].last_call_in);
		 }
		 else
		 {
                   if (!uin) j++;
                   sprintf(buf,"%s %s \033[1m[33;44m%s[m,%d",
                         (!uin) ? "[31mX":"[m ",
                         oldmsg[a].last_userid,oldmsg[a].last_call_in,a);
                 }
                 outs(buf);  /*用 prints 會造成使用者被踢 hialan.020717*/
               }
	  refresh();
	  move(i+2,0);
	  outs(
" [1;34m──────────────────────────────────────[m ");	 
          move(i+3,0);
          outs(last_return_msg);
          move(i+4,0);
          clrtoeol();
          outs(
" [1;34m───────────────────────"COLOR1" [Ctrl-T]往上切換 [40;34m──────[m ");
     }
  t_display_new_flag =0;
  
  return j;
}

int
talk_mail2user()
{
  char fname[128];

  sethomefile(fname, cuser.userid, fn_writelog);
  mail2user(cuser.userid, "熱線\033[37;41m記錄\033[m", fname, FILE_READ);
  unlink(fname);
  
  /* itoc.011104: delete BMW */
  sethomefile(fname, cuser.userid, FN_BMW);
  unlink(fname);
  
  return 0;
}

int
t_display()
{
  char genbuf[64];

  sethomefile(genbuf, cuser.userid, fn_writelog);

  if (more(genbuf, YEA) != -1)
  {
    char *choose[3] = {"cC)清除","mM)移至備忘錄","rR)保留"};
    
    /* add by hialan 20020519  水球容量小於200k */
       
       char fpath[80];
       struct stat st;
       sethomefile(fpath,cuser.userid,"writelog");
       if (stat(fpath, &st) == 0 && st.st_size > 200000)
       {
         pressanykey("你的水球佔用容量:%d byte !!",st.st_size);
         pressanykey("水球保留的容量不得超過200K!系統自動轉存到信箱！");
         
         talk_mail2user();
       }
    /* add end */
       else
       {
         switch (getans2(b_lines, 0, "", choose, 3, 'r'))
         {
         case 'm':
           talk_mail2user();
           break; 
           
         case 'c':
           unlink(genbuf);
          
           /* itoc.011104: delete BMW */
           sethomefile(genbuf, cuser.userid, FN_BMW);
           unlink(genbuf);

         default:
           break;
         }
       }
      return RC_FULL;
  }
  return RC_NONE;
}



/* itoc.011104: for BMW */
int
bmw_refresh()
{
  return RC_FULL;
}

extern int write_msg();  //bbs.c

static struct one_key bmwlist_key[]={
'w' , write_msg, PERM_LOGINOK, "丟水球",0,
's' , bmw_refresh,          0, "更新畫面",0,
'\0', NULL, 0, NULL,0};

static void bmwtitle()
{
  char buf[256];
  
  sprintf(buf,"%s [線上 %d 人]",BOARDNAME, count_ulist());
  showtitle("水球回顧", buf);

  prints("[↑/↓]上下 [PgUp/PgDn]上下頁 [Home/End]首尾 [←][q]離開\n");
  prints("\033[1;37;46m 編號 代 號       心  情  故  事"
         "                                         時間 \033[m\n");
}

static void bmw_lightbar(int num, fileheader *bmw, int row, char *barcolor)
{
  move(row, 0);
  clrtoeol();
  prints("%4d %s%-13s%s%-55.55s\033[m%s",
       num, (barcolor) ? barcolor : (bmw->savemode) ? "\033[33m" : "\033[m", 
       bmw->owner, (bmw->savemode) ? "\033[0;33m" : "\033[m", 
       bmw->title, bmw->date);
}

                                                                                
int t_bmw()
{
  char fname[80];
  char *choose[3] = {"cC)清除","mM)移至備忘錄","rR)保留"};  
  
  sethomefile(fname, cuser.userid, FN_BMW);
  if(dashf(fname))
  {
    i_read(SEEWATER, fname, bmwtitle, bmw_lightbar, bmwlist_key, NULL);
  
    switch (getans2(b_lines, 0, "", choose, 3, 'r'))
    {
      case 'm':
        talk_mail2user();
        break;            
    
      case 'c':
        unlink(fname);
           
        /* itoc.011104: delete BMW */
        sethomefile(fname, cuser.userid, fn_writelog);
        unlink(fname);

      default:
        break;
    }
  }
  else
    pressanykey("您尚未收到水球喔!!");
  
  return RC_FULL;
}
