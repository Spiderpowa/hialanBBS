/*-------------------------------------------------------*/
/* main.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : BBS main/login/top-menu routines             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#define _MAIN_C_
#include "bbs.h"
#define WELCOME_TITLE "etc/Welcomes/Welcome_title/"

jmp_buf byebye;

static uschar enter_uflag;

#ifdef SHOW_IDLE_TIME
char fromhost[STRLEN - 20] = "\0";
char tty_name[20] = "\0";
#else
char fromhost[STRLEN] = "\0";
#endif
char remoteusername[40];

void check_register();
int mbbsd = 0;

/* ----------------------------------------------------- */
/* 離開 BBS 程式                                         */
/* ----------------------------------------------------- */

void
log_usies(mode, msg)
  char *mode, *msg;
{
  char buf[512], data[512];
  time_t now;
  
  time(&now);
  if (!msg)
  {
    sprintf(data, "Stay: %d (%s)", 
      (now - login_start_time) / 60, cuser.username);
    msg = data;
  }
  sprintf(buf, "%s %s %-13s%s", Etime(&now), mode, cuser.userid, msg);
  f_cat(FN_USIES, buf);
}
                          

static void
setflags(mask, value)
  int mask, value;
{
  if (value)
    cuser.uflag |= mask;
  else
    cuser.uflag &= ~mask;
}


void
u_exit(mode)
  char *mode;
{
  extern void auto_backup();    /* 編輯器自動備份 */
  userec xuser;
  int diff = (time(0) - login_start_time) / 60;

  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
      auto_backup();
  setflags(PAGER_FLAG, currutmp->pager != 1);
  setflags(CLOAK_FLAG, currutmp->invisible);
  xuser.pager = currutmp->pager;	 /* 記錄pager狀態, add by wisely */
  xuser.invisible = currutmp->invisible; /* 紀錄隱形狀態 by wildcat */
  xuser.totaltime += time(0) - update_time;
  xuser.numposts = cuser.numposts;
  xuser.feeling[4] = '\0';
//  xuser.pagernum[6] = '\0';

  if (!HAS_PERM(PERM_DENYPOST) && !currutmp->invisible)
  {
    char buf[256];
    time_t now;
    
    time(&now);
    sprintf(buf,"<<下站通知>> -- 我走囉！ - %s",Etime(&now));
    do_aloha(buf);
  }

  purge_utmp(currutmp);
  if(!diff && cuser.numlogins > 1 && strcmp(cuser.userid,STR_GUEST))
    xuser.numlogins = --cuser.numlogins; /* Leeym 上站停留時間限制式 */
  substitute_record(fn_passwd, &xuser, sizeof(userec), usernum);
  log_usies(mode, NULL);
}


void
system_abort()
{
  if (currmode)
    u_exit("ABORT");
  printf("謝謝光臨, 記得常來喔 !\n");
  sleep(1);
  exit(0);
}


void
abort_bbs()
{
  if (currmode)
    u_exit("AXXED");
  exit(0);
}


void
leave_bbs()
{
   reset_tty();
}


/* ----------------------------------------------------- */
/* 登錄 BBS 程式                                         */
/* ----------------------------------------------------- */


int
dosearchuser(userid)
  char *userid;
{
  if (usernum = getuser(userid))
    memcpy(&cuser, &xuser, sizeof(cuser));
  else
    memset(&cuser, 0, sizeof(cuser));
  return usernum;
}


static void
talk_request()
{
#ifdef  LINUX
  /*
   * Linux 下連續 page 對方兩次就可以把對方踢出去： 這是由於某些系統一 nal
   * 進來就會將 signal handler 設定為內定的 handler, 不幸的是 default 是將程
   * erminate. 解決方法是每次 signal 進來就重設 signal handler
   */

  signal(SIGUSR1, talk_request);
#endif
  bell();
  if (currutmp->msgcount) {
     char buf[512];
     time_t now = time(0);

     sprintf(buf, "[1;33;42m[[37m%s[33m][1;34;47m [%s] %s [0m",
        (currutmp->destuip)->userid,  my_ctime(&now),
        (currutmp->sig == 2) ? "重要消息廣播！(請Ctrl-U,l查看熱訊記錄)" : "呼叫、呼叫，聽到請回答");
     move(0, 0);
     clrtoeol();
     outs(buf);
     refresh();
  }
  else {
     uschar mode0 = currutmp->mode;
     char c0 = currutmp->chatid[0];
     screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
     currutmp->mode = 0;
     currutmp->chatid[0] = 1;
     vs_save(screen);
     talkreply();
     vs_restore(screen);
     currutmp->mode = mode0;
     currutmp->chatid[0] = c0;
   }
}

extern msgque oldmsg[MAX_REVIEW];   /* 丟過去的水球 */
extern char   no_oldmsg,oldmsg_count;            /* pointer */
extern char   watermode;

static void
write_request()
{
  time_t now;
  extern char watermode;
/*  Half-hour remind  */
  if(*currmsg) {
    outmsg(currmsg);
    refresh();
    bell();
    *currmsg = 0;
    return;
  }

  time(&now);

#ifdef  LINUX
  signal(SIGUSR2, write_request);
#endif

  update_data();
  ++cuser.receivemsg;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  bell();
  show_last_call_in();
  memcpy(&oldmsg[no_oldmsg],&currutmp->msgs[0],sizeof(msgque));
  ++no_oldmsg;
  no_oldmsg %= MAX_REVIEW;
  if(oldmsg_count < MAX_REVIEW) oldmsg_count++;
  if(watermode)
  {
    if(watermode<oldmsg_count) watermode++;
    t_display_new(0);
  }
  refresh();
  currutmp->msgcount = 0;
}

static void
multi_user_check()
{
  register user_info *ui;
  register pid_t pid;
  int cmpuids();

  if (HAS_PERM(PERM_SYSOP))
    return;         /* wildcat:站長,跟okid裡的人不限制 */

  if (cuser.userlevel)
  {
    if (!(ui = (user_info *) search_ulist(cmpuids, usernum)))
      return;                   /* user isn't logged in */

    pid = ui->pid;
    if (!pid || (kill(pid, 0) == -1))
      return;                   /* stale entry in utmp file */

//    if (getans("您想刪除其他重複的 login (Y/N)嗎？[Y] ") != 'n')
    if(getans2(b_lines, 0, "您想刪除其他重複的 login 嗎? ", 0, 2, 'y') != 'n')
    {
      kill(pid, SIGHUP);
      log_usies("KICK ", cuser.username);
    }
    else
    {
      int nums = MULTI_NUMS;
      if(HAS_PERM(PERM_BM))
        nums += 2;
      if (count_multi() >= nums)
        system_abort();
    }
  } 
  else  /* guest的話 */
  {
    if (count_multi() > 32)
    {
      pressanykey("抱歉，目前已有太多 guest, 請稍後再試。");
      oflush();
      exit(1);
    }
  }
}

/* --------- */
/* bad login */
/* --------- */

static char str_badlogin[] = "logins.bad";


static void
logattempt(uid, type)
  char *uid;
  char type;                    /* '-' login failure   ' ' success */
{
  char fname[40];
  char genbuf[512];

  sprintf(genbuf, "%c%-12s[%s] %s@%s\n", type, uid,
    Etime(&login_start_time), remoteusername, fromhost);
  f_cat(str_badlogin, genbuf);
  
  if (type == '-')
  {
    sprintf(genbuf, "[%s] %s\n", Etime(&login_start_time), fromhost);
    sethomefile(fname, uid, str_badlogin);

    f_cat(fname, genbuf);
  }
}

#ifdef BSD44
static int
over_load()
{
  double cpu_load[3];

  getloadavg(cpu_load, 3);

  if(cpu_load[0] > MAX_CPULOAD)
  {
    pressanykey("目前系統負荷 %f 過高,請稍後再來",cpu_load[0]);
    return 1;
  }  
  return 0;
}
#endif

static void login_query()
{
  char uid[IDLEN + 1], passbuf[PASSLEN];
  int attempts;
  char genbuf[512];
  extern struct UTMPFILE *utmpshm;

/* by Excalibur(大魔王) is84006@cis.nctu.edu.tw
   避免hacker開一堆窗停在登入畫面佔資源 */
  signal(SIGALRM, abort_bbs);

  resolve_utmp();
  attempts = utmpshm->number;
//  term_init_m2("vt100");
  
  initscr();
  clear();

/*
  {
    move(10,0);
    outs("       硬碟更新中 , 預計需半天時間 , 請大家耐心等待!!");
    pressanykey("換新硬碟才不會爆掉 :p");
    oflush();
    sleep(1);
    exit(1);
  }  
*/

#ifdef RANDLOGIN
  /*亂數換title  by hialan*/
/*  srand(time(0));
  randlogin = rand()%5 + '0';
  sprintf(rbuf,"%s%c",WELCOME_TITLE,randlogin);
  show_file(rbuf,0,17,ONLY_COLOR);
*/
  time(&i);
  i=i%5;
  sprintf(rbuf,"%s%d",WELCOME_TITLE,i);
  show_file(rbuf,0,17,ONLY_COLOR);
#else
  show_file("etc/Welcome_title",0,15,ONLY_COLOR);
#endif

//  move(18,0);
//  counter(BBSHOME"/log/counter/上站人次","光臨本站",1);
  
  show_file("etc/Welcome_news",19,5,ONLY_COLOR);
  
  if (attempts >= MAXACTIVE)
  {
    pressanykey("目前站上人數已達上限，請您稍後再來。");
    oflush();
    sleep(1);
    exit(1);
  }
#ifdef BSD44
  if(over_load()) 
  {
    oflush();
    sleep(1);
    exit(1); 
  }
#endif
  attempts = 0;
  while (1)
  {
    alarm(LOGIN_TIMEOUT);
    if (attempts++ >= LOGINATTEMPTS)
    {
      pressanykey("錯誤太多次,掰掰~~~~~");
      oflush();
      exit(1);
    }
    getdata(21, 40, "Athenaeum Login: ",uid, IDLEN + 1 , DOECHO,0);
    if (ci_strcmp(uid, str_new) == 0)
    {
#ifdef LOGINASNEW
      DL_func("SO/register.so:va_new_register", 0);
//      new_register(0);
      break;
#else
      pressanykey("\033[1;31m本系統目前無法以 new 註冊, 請用 guest 進入\033[1m");
      continue;
#endif
    }
    else if (uid[0] == '\0' || !dosearchuser(uid))
    {
      pressanykey(err_uid);
    }
    else if (strcmp(uid, STR_GUEST))
    {
      getdata(21, 40, "Password: ", passbuf, PASSLEN, PASS,0); /*(21,40 ychia*/
      passbuf[8] = '\0';
      if (!chkpasswd(cuser.passwd, passbuf))
      {
        logattempt(cuser.userid, '-');
        pressanykey(ERR_PASSWD);
      }
      else
      {
        /* SYSOP gets all permission bits */

        if (!ci_strcmp(cuser.userid, str_sysop))
          cuser.userlevel = ~0;
        if (0 && HAS_PERM(PERM_SYSOP) && !strncmp(getenv("RFC931"), "?@", 2)) {
           logattempt(cuser.userid, '*');
           outs("站長請由 trusted host 進入");
           continue;
        }
        else {
           logattempt(cuser.userid, ' ');
           break;
        }
      }
    }
    else
    {                           /* guest */
      cuser.userlevel = 0;
      cuser.uflag = COLOR_FLAG | PAGER_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
      break;
    }
  }

  signal(SIGALRM, SIG_IGN);

  multi_user_check();
  sethomepath(genbuf, cuser.userid);
  mkdir(genbuf, 0755);
  srand(time(0) ^ getpid() ^ (getpid() << 10));
  srandom(time(0) ^ getpid() ^ (getpid() << 10));
}



/*
woju
*/
add_distinct(char* fname, char* line)
{
   FILE *fp;
   int n = 0;

   if (fp = fopen(fname, "a+")) {
      char buffer[80];
      char tmpname[100];
      FILE *fptmp;

      strcpy(tmpname, fname);
      strcat(tmpname, "_tmp");
      if (!(fptmp = fopen(tmpname, "w"))) {
         fclose(fp);
         return;
      }

      rewind(fp);
      while (fgets(buffer, 80, fp)) {
         char* p = buffer + strlen(buffer) - 1;

         if (p[-1] == '\n' || p[-1] == '\r')
            p[-1] = 0;
         if (!strcmp(buffer, line))
            break;
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
      }

      if (feof(fp))
         fprintf(fptmp, "%s%c#1\n", line, 0);
      else {
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n + 1);
         while (fgets(buffer, 80, fp)) {
            sscanf(buffer + strlen(buffer) + 2, "%d", &n);
            fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
         }
      }
      fclose(fp);
      fclose(fptmp);
      unlink(fname);
      rename(tmpname, fname);
   }
}


del_distinct(char* fname, char* line)
{
   FILE *fp;
   int n = 0;

   if (fp = fopen(fname, "r")) {
      char buffer[80];
      char tmpname[100];
      FILE *fptmp;

      strcpy(tmpname, fname);
      strcat(tmpname, "_tmp");
      if (!(fptmp = fopen(tmpname, "w"))) {
         fclose(fp);
         return;
      }

      rewind(fp);
      while (fgets(buffer, 80, fp)) {
         char* p = buffer + strlen(buffer) - 1;

         if (p[-1] == '\n' || p[-1] == '\r')
            p[-1] = 0;
         if (!strcmp(buffer, line))
            break;
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
      }

      if (!feof(fp))
         while (fgets(buffer, 80, fp)) {
            sscanf(buffer + strlen(buffer) + 2, "%d", &n);
            fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
         }
      fclose(fp);
      fclose(fptmp);
      unlink(fname);
      rename(tmpname, fname);
   }
}

#ifdef WHERE
/* Jaky and Ptt*/
int where (char *from)
{
   extern struct FROMCACHE *fcache;
   register int i,count,j;

   resolve_fcache();
   for (j=0;j<fcache->top;j++)
   {
      char *token=strtok(fcache->domain[j],"&");
      i=0;count=0;
      while(token)
      {
         if (strstr(from,token)) count++;
         token=strtok(NULL, "&");
         i++;
      }
      if (i==count) break;
   }
   if (i!=count) return 0;
   return j;
}
#endif

void
check_BM()      /* Ptt 自動取下離職板主權力 */
{
  int i;
  boardheader *bhdr;
  extern boardheader *bcache;
  extern int numboards;

  resolve_boards();
  cuser.userlevel &= ~PERM_BM;
  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
      userid_is_BM(cuser.userid, bhdr->BM);
}

void
setup_utmp(mode)
  int mode;
{
  user_info uinfo;
  char buf[80];

  memset(&uinfo, 0, sizeof(uinfo));
  uinfo.pid = currpid = getpid();
  uinfo.uid = usernum;
  uinfo.mode = currstat = mode;
  uinfo.msgcount = 0;
  if(cuser.userlevel & PERM_BM) check_BM(); /* Ptt 自動取下離職板主權力 */
  uinfo.userlevel = cuser.userlevel;

  strcpy(uinfo.userid, cuser.userid);
  strcpy(uinfo.realname, cuser.realname);
  strcpy(uinfo.username, cuser.username);
  strcpy(uinfo.feeling, cuser.feeling);
  strncpy(uinfo.from, fromhost, 23);
  uinfo.invisible = cuser.invisible;
  uinfo.pager     = cuser.pager;

/* Ptt WHERE*/

#ifdef WHERE
  uinfo.from_alias = where(fromhost);
#else
  uinfo.from_alias = 0;
#endif
  sethomefile(buf, cuser.userid, "remoteuser");
  add_distinct(buf, getenv("RFC931"));

#ifdef SHOW_IDLE_TIME
  strcpy(uinfo.tty, tty_name);
#endif

  if (enter_uflag & CLOAK_FLAG)
      uinfo.invisible = YEA;
  
  getnewutmpent(&uinfo);
  friend_load();
}

static void
user_login()
{
  char genbuf[512];
  struct tm *ptime,*tmp;
  time_t now = time(0);
  int a;

  extern struct FROMCACHE *fcache;
  extern struct UTMPFILE *utmpshm;
  extern int fcache_semid;
  
  log_usies("ENTER", getenv("RFC931")/* fromhost */);

  /* ------------------------ */
  /* 初始化 uinfo、flag、mode */
  /* ------------------------ */

  setup_utmp(LOGIN);
  currmode = MODE_STARTED;
  enter_uflag = cuser.uflag;

/* get local time */
  tmp = localtime(&cuser.lastlogin);

  update_data(); //wildcat: update user data
/*Ptt check 同時上線人數 */
  resolve_fcache();
  resolve_utmp();

  if((a=utmpshm->number)>fcache->max_user)
    {
      sem_init(FROMSEM_KEY,&fcache_semid);
      sem_lock(SEM_ENTER,fcache_semid);
      fcache->max_user = a;
      fcache->max_time = now;
      sem_lock(SEM_LEAVE,fcache_semid);
    }

#ifdef  INITIAL_SETUP
  if (!getbnum(DEFAULT_BOARD))
  {
    strcpy(currboard, "尚未選定");
  }
  else
#endif

  {
    brc_initial(DEFAULT_BOARD);
    set_board();
  }

  /* ------------ */
  /* 畫面處理開始 */
  /* ------------ */

  if (!(HAS_PERM(PERM_SYSOP) && HAS_PERM(PERM_DENYPOST)))
  {
    char buf[256];
    time_t now;
    
    time(&now);
    sprintf(buf,"<<上站通知>> -- 我來囉！ - %s",Etime(&now));
    do_aloha(buf);
  }

/*
Ptt
*/

  time(&now);
  ptime = localtime(&now);

  if((cuser.day == ptime->tm_mday) && (cuser.month == (ptime->tm_mon + 1)))
  {
    currutmp->birth  = 1;
  }
  else
  {
    more("etc/Welcome_login", NA);
    currutmp->birth = 0 ;
  }
/* wildcat : 搬家通知用
  if(belong(BBSHOME"/etc/oldip",fromhost))
  {
    more(BBSHOME"/etc/removal");
    abort_bbs();
  }    
*/
  if (cuser.userlevel)          /* not guest */
  {
    move(t_lines - 3, 0);
    prints("      歡迎您第 [1;33m%d[0;37m 度拜訪本站，\
上次您是從 [1;33m%s[0;37m 連往本站，\n\
     我記得那天是 [1;33m%s[0;37m。\n",
      ++cuser.numlogins, cuser.lasthost,
      Etime(&cuser.lastlogin));
    pressanykey(NULL);

/* Ptt */
    ptime = localtime(&cuser.lastlogin);
    if(currutmp->birth == 1 
      && (ptime->tm_mday != cuser.day || ptime->tm_mon + 1 != cuser.month))
    {
      more("etc/Welcome_birth",YEA);
      brc_initial("Greeting");
      set_board();
      do_post();
    }
    sethomefile(genbuf, cuser.userid, str_badlogin);
    if (more(genbuf, NA) != -1)
    {
//      if (getans("您要刪除以上錯誤嘗試的記錄嗎(Y/N)?[Y]") != 'n')
        if(getans2(b_lines, 0, "您要刪除以上錯誤嘗試的記錄嗎? ", 0, 2, 'y') != 'n')
        unlink(genbuf);
    }

    check_register();
    strncpy(cuser.lasthost, fromhost, 24);
    substitute_record(fn_passwd, &cuser, sizeof(cuser), usernum);
    cuser.lasthost[23] = '\0';
    restore_backup();
  }

/* 板主提高信箱上限 */
  if(HAS_PERM(PERM_BM) && cuser.exmailbox < 100)
    cuser.exmailbox = 100;
  else if (!strcmp(cuser.userid, STR_GUEST))
  {
    char *nick[10] = {"麵包", "美女圖", "FreeBSD", "DBT目錄", "mp3",
                      "女王頭", "病毒", "童年", "石像", "386 CPU"
                     };
    char *name[10] = {"卡殺米亞", "白鳥澤麗子", "風與塵埃小站", "狂x或保x捷", 
                      "Wu Bai & China Blue","伊力沙白", "C-brain", 
                      "時間", "我", "Intel inside" 
                      };
    int sex[10] = {6, 4, 7, 7, 2, 6, 0, 7, 7, 0};

    int i = rand() % 10;
    sprintf(cuser.username, "被風化的%s", nick[i]);
    sprintf(currutmp->username, cuser.username);
    sprintf(cuser.realname, name[i]);
    sprintf(currutmp->realname, cuser.realname);
    strcpy(cuser.cursor, ">>");
    cuser.sex = sex[i];
    cuser.silvermoney = 300;
    cuser.habit = HABIT_GUEST;	/* guest's habit */
    currutmp->pager = 2;
    pressanykey(NULL);
  }

  if (bad_user(cuser.userid)) {
     sprintf(currutmp->username, "BAD_USER");
     cuser.userlevel = 0;
     cuser.uflag = COLOR_FLAG | PAGER_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
  }
  if (!PERM_HIDE(currutmp))
     cuser.lastlogin = login_start_time;
  substitute_record(fn_passwd, &cuser, sizeof(cuser), usernum);

   if(cuser.numlogins < 2)
   {
     more("etc/newuser",YEA);
     HELP();
     pressanykey("唷?!新來的唷?自我介紹一下吧!");
     if(brc_initial("Hi_All"))
     {
       set_board();
       do_post();
     }
   }
/*   
  if(HAS_HABIT(HABIT_NOTE))
    more("note.ans",YEA);
  if(HAS_HABIT(HABIT_SEELOG))
    Announce();
  if(!HAS_HABIT(HABIT_ALREADYSET) && cuser.userlevel)
    u_habit();
*/
  login_plan();
}

void
check_max_online()
{
  FILE *fp;
  int maxonline=0;
  time_t now = time(NULL);
  struct tm *ptime;

  ptime = localtime(&now);

  if(fp = fopen(".maxonline", "r"))
  {
    fscanf(fp, "%d", &maxonline);
    fclose(fp);
  }

  if ((count_ulist() > maxonline) && (fp = fopen(".maxonline", "w")))
  {
    fprintf(fp, "%d", count_ulist());
    fclose(fp);
  }
  if(fp = fopen(".maxtoday", "r"))
  {
    fscanf(fp, "%d", &maxonline);
    if (count_ulist() > maxonline){
      fclose(fp);
      fp = fopen(".maxtoday", "w");
      fprintf(fp, "%d", count_ulist());
    }
    fclose(fp);
  }
}

main(argc, argv)
  int argc;
  char **argv;
{
  extern struct commands cmdlist[];
  extern char currmaildir[32];

  /* ----------- */
  /* system init */
  /* ----------- */

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);  
  currmode = 0;
  srand(login_start_time = time(0));
  update_time = login_start_time;

  if (argc > 1)
  {
    strcpy(fromhost, argv[1]);

#ifdef SHOW_IDLE_TIME
    if (argc > 2)
      strcpy(tty_name, argv[2]);

#endif

    if (argc > 3)
       strcpy(remoteusername, argv[3]);
  }
/*
woju
*/
{
   char cmd[80] = "?@";

   if (!getenv("RFC931"))
      setenv("RFC931", strcat(cmd, fromhost), 1);
}

  atexit(leave_bbs);

  signal(SIGHUP, abort_bbs);
  signal(SIGBUS, abort_bbs);
  signal(SIGSEGV, abort_bbs);
#ifdef SIGSYS
  signal(SIGSYS, abort_bbs);
#endif
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, SIG_IGN);

  signal(SIGURG, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);

  signal(SIGUSR1, talk_request);
  signal(SIGUSR2, write_request);

  if (setjmp(byebye))
    abort_bbs();
  nice(3);  /* lower priority */

  init_tty();
  login_query();
  user_login();
/* wildcat : 修復中
  if(!HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("整修中 , 禁止站長以外 user 登入");
    abort_bbs();
  }
*/
  check_max_online();

  sethomedir(currmaildir, cuser.userid);
  if (HAVE_PERM(PERM_SYSOP | PERM_BM))
  {
    DL_func("SO/vote.so:b_closepolls");
  }
  if (!HAVE_HABIT(HABIT_COLOR))
    showansi = 0;
#ifdef DOTIMEOUT
  init_alarm();
#else
  signal(SIGALRM, SIG_IGN);
#endif
  while (chkmailbox())
     m_read();
#ifdef HAVE_GAME
  waste_money();
#endif

  force_board("Announce");
  {
    char buf[80];
    setbfile(buf, "VIP", FN_LIST);
    if(belong_list(buf,cuser.userid))
      force_board("VIP");
    setbfile(buf, "WD_plan", FN_LIST);
    if(belong_list(buf,cuser.userid))
      force_board("WD_plan");
  }
  if(HAS_PERM(PERM_SYSOP))
    force_board("Working");
//  force_board("Boards");

  if (HAS_PERM(PERM_ACCOUNTS) && dashf(fn_register))
    DL_func("SO/admin.so:m_register");
//    m_register();             
  domenu(MMENU, "主功\能表", (chkmail(0) ? 'M' : 'B'), cmdlist);
  return;
}

