/*-------------------------------------------------------*/
/* util/bhttpd.c        ( NCKU CivilEngineer LeeymBBS)   */
/*-------------------------------------------------------*/
/* target : BBS's HTTP interface                         */
/* create : 97/04/07                                     */
/* update : 97/04/08                                     */
/*-------------------------------------------------------*/
/* author : leeym@mail.civil.ncku.edu.tw                 */
/*-------------------------------------------------------*/
/* syntax : bhttpd                                       */
/*-------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bbs.h"
#include "type.h"
/*
#include "stuff.c"
#include "cache.c"
*/

#define PORT 8080

#define PROFILE_NUM     50
#define PROFILE_SIZE    100

/*nekobe HTML Tag define
        0:enable
        1:disable       */
#define ENABLE_TAG      1

#define ACCESSLOG       BBSHOME"/log/bhttpd-access.log"
#define ERRORLOG        BBSHOME"/log/bhttpd-error.log"

#define COUNTERDIR      BBSHOME"/log/counter/"

#define BBSIMG          "wdbbs.net/image/title-b.jpg"
#define BBSIMG_S        "wdbbs.net/image/title-s.jpg"
#define BBSBACK         "wdbbs.net/image/bg.jpg"

int ansi_on, ansi_blink, ansi_hlight;

#define field_count 5
#define MAX_WORDS       1024

#define WHITE   "#ffffff"
#define GRAY    "#c0c0c0"
#define BLACK   "#000000"

#define RED     "#a00000"
#define LRED    "#ff0000"

#define GREEN   "#007000"
#define LGREEN  "#00ff00"

#define DBLUE    "#000070"
#define BLUE    "#0000a0"
#define LBLUE   "#0000ff"

#define YELLOW  "#a0a000"
#define LYELLOW "#ffff00"

#define PURPLE  "#a000a0"
#define LPURPLE "#ff00ff"

#define CRAY    "#00a0a0"
#define LCRAY   "#00ffff"

/*-------------------------------------------------------*/
/* .UTMP cache                                           */
/*-------------------------------------------------------*/

struct UTMPFILE *utmpshm;
userec uec;
user_info *ushm_head, *ushm_tail;

struct BCACHE *brdshm;
boardheader allbrd[MAXBOARD];
boardheader *bcache;
int includetext = 0, header = 1, readline = 0;
char protocol[20] = "";

struct user_info *uentp;
char field_str[ field_count ][ 128 ];
int  field_lst_no [ field_count ];
int  user_num =0 ,hide_num = 0;
// wildcat
int  auth_ok = 0;
userec cuser,xuser;
char *FROM,*TITLE; 
char ARTICLE[65536]; 

int  field_lst_size [ field_count ] = {
   12, 20, 24, 15,  8
};

char *field_name[] = {
    "代號",
    "暱稱",
    "故鄉",
    "動態",
    "發呆",
    NULL
};

/* Gene's include */

char *fn_board=".BOARDS";

char *
Cdate(clock)
  time_t *clock;
{
  static char foo[22];
  struct tm *mytm = localtime(clock);

  strftime(foo, 22, "%D %T %a", mytm);
  return (foo);
}

int
dashl(fname)
  char *fname;
{
  struct stat st;

  return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}


dashf(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}


int
dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}



int numboards = -1;

int
ci_strcmp(s1, s2)
  register char *s1, *s2;
{
  register int c1, c2, diff;

  do
  {
    c1 = *s1++;
    c2 = *s2++;
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 32;
    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 32;
    if (diff = c1 - c2)
      return (diff);
  } while (c1);
  return 0;
}

void
resolve_boards()
{
  if (brdshm == NULL)
  {
    brdshm = shm_new(BRDSHM_KEY, sizeof(*brdshm));
    if (brdshm->touchtime == 0)
      brdshm->touchtime = 1;
    bcache = brdshm->bcache;
  }

  while (brdshm->uptime < brdshm->touchtime)
     reload_bcache();
  numboards = brdshm->number;
}

boardheader *
getbcache(bname)
  char *bname;
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
    /* if (Ben_Perm(bhdr)) */
    if (!ci_strcmp(bname, bhdr->brdname))
      return bhdr;
  return NULL;
}

void
resolve_utmp()
{
  if (utmpshm == NULL)
  {
    utmpshm = shm_new(UTMPSHM_KEY, sizeof(*utmpshm));
    if (utmpshm->uptime == 0)
      utmpshm->uptime = utmpshm->number = 1;
  }
}

reload_bcache()
{
   if (brdshm->busystate)
   {
     sleep(1);
   }
   else
   {
     int fd;

     brdshm->busystate = 1;

     if ((fd = open(fn_board, O_RDONLY)) > 0)
     {
       brdshm->number = read(fd, bcache, MAXBOARD * sizeof(boardheader))
         / sizeof(boardheader);
       close(fd);
     }

     /* 等所有 boards 資料更新後再設定 uptime */

     brdshm->uptime = brdshm->touchtime;
     /*log_usies("CACHE", "reload bcache");*/
     brdshm->busystate = 0;
   }
}

/* Gene's include end */

int
counter_(fname)
  char *fname;
{
  char buf[80];
  FILE *fn;
  int num=0;

  sprintf(buf, "%s%s", COUNTERDIR, fname);
  if (fn=fopen(buf, "r"))
  {
    fscanf(fn, "%d", &num);
    fclose(fn);
  }
  num++;
  if (fn=fopen(buf, "w"))
  {
    fprintf(fn, "%d", num);
    fclose(fn);
  }
  return num;
}

print_head()
{
    int i, size;
    printf("<center><TD bgcolor=%s>No.\n", LBLUE);
    for (i = 0; i < field_count; i++) {
        size  = field_lst_size[ i ];
        printf("<TD bgcolor=%s>%-*.*s ", BLUE, size, size, field_name[i]);
    }
    printf("<TR>\n");
}


print_record()
{
    int i, size;

    for (i = 0 ; i < field_count; i++) {
        size  = field_lst_size[ i ];
        if(i==0)

        printf("<TD bgcolor=%s align=center>%-d<td bgcolor=%s>"
        "<font size=4><A HREF=http:/~%s/>%-*.*s</A></font>\n",
        CRAY, user_num+1, GREEN,
        field_str[i], size, size, field_str[i]);

/*nekobe*/
       else if(i==1)
        printf("<TD><A HREF=mailto:%s.bbs@wdbbs.net>"
               "%s</A>",field_str[i-1],field_str[i]);

       else
/*
        printf("<TD bgcolor=%s>%-*.*s ", BLACK, size, size,
        field_str[i][0] ? field_str[i] : "<pre>");
*/
        printf("<TD>%-*.*s ", size, size,
        field_str[i][0] ? field_str[i] : "<pre>");
    }
    printf("<TR>\n");
}


char *
idle_str(lastact)
  time_t lastact;
{
  static char hh_mm_ss[8];
  struct stat buf;
  time_t now = time(0);
  time_t diff = now - lastact;

  if (diff)
    sprintf(hh_mm_ss, "%2d:%02d\'%02d\"", diff / 3600, diff / 60 , diff % 60);
  else
    strcpy(hh_mm_ss, "<pre>");

  return hh_mm_ss;
}


dump_record(serial_no, p)
int serial_no;
struct user_info *p;
{
    int i = 0;

    sprintf( field_str[i++], "%s", p->userid );
    sprintf( field_str[i++], "%s", p->username );
    sprintf( field_str[i++], "%s", p->from );
    sprintf( field_str[i++], "%s", ModeTypeTable[p->mode] );
    sprintf( field_str[i++], "%s", idle_str(p->lastact));
}

int
chkmail(userid)
char *userid;
{
    FILE *mail;
    char buf[80];
    struct stat st;
    fileheader fh;

    sprintf(buf, BBSHOME"/home/%s/.DIR", userid);
    stat(buf, &st);

    if( access(buf,F_OK) || !st.st_size) /* .DIR 不存在或為 0 size */
        return 0;
    else
     if( (mail=fopen(buf,"r")) == NULL){
         perror("fopen:.DIR");
         exit(1);
     }
     else
     while( fread(&fh,sizeof(fileheader),1,mail) > 0);
     if(fh.filemode & FILE_READ)
         return 0;
     else
         return 1;
}


int
board_cmp(a, b)
  boardheader *a, *b;
{
  int bcmp;
  char modea[8], modeb[8];
  strncpy(modea, a->title, 6);
  strncpy(modeb, b->title, 6);
  modea[6] = modeb[6] = '\0';
  bcmp = strcmp(modea, modeb);
  if (!bcmp)
    bcmp = strcasecmp(a->brdname, b->brdname);
  return bcmp;
}


int
get_num_records(fpath, size)
  char *fpath;
{
  struct stat st;
  if (stat(fpath, &st) == -1)
    return 0;
  return (st.st_size / size);
}

fileheader *
getfhdr(char *fdir, int no)
{
  int fd;
  int size = sizeof(fileheader), i = 0;
  char buf[size];
  fileheader *fhdr = (fileheader *) malloc(sizeof(fileheader));
  if (fd = open(fdir, O_RDONLY))
  {
    lseek(fd, size * (no - 1), SEEK_SET);
    if (read(fd, buf, size) == size)
    {
      close(fd);
      fhdr = (fileheader *) buf;
      return (fhdr);
    }
    else
    {
      close(fd);
      return NULL;
    }
  }
  return NULL;
}


int
getindex(fpath, fname, size)
  char *fpath;
  char *fname;
  int size;
{
  int fd, now=0;
  fileheader fhdr;

  if ((fd = open(fpath, O_RDONLY, 0)) != -1)
  {
    while((read(fd, &fhdr, size) == size)){
      now++;
      if(!strcmp(fhdr.filename, fname)){
        close(fd);
        return now;
      }
    }
    close(fd);
  }
  return 0;
}

void
accesslog(method, mesg)
  char *method, *mesg;
{
  FILE *fp;
  char genbuf[200];

  if (fp = fopen(ACCESSLOG, "a"))
  {
    time_t now = time(0);

    fprintf(fp, "client - - [%s +0800] \"%s %s %s\" 200 - \n",
      Cdate(&now), method, mesg, protocol);
    fclose(fp);
  }
}

void
errorlog(method, mesg)
  char *method, *mesg;
{
  FILE *fp;
  char genbuf[200];

  if (fp = fopen(ERRORLOG, "a"))
  {
    time_t now = time(0);

    fprintf(fp, "[%s +0800] access to %s fail for client, reason: %s does not exist - %s\n",
      Cdate(&now), method, mesg);
    fclose(fp);
  }
}


/* ----------------------------------------------------- */
/* HTML output sub finction                              */
/* ----------------------------------------------------- */

html_head(char *str, int refresh, int blackground, int firstpage)
{
  time_t now = time(0);
  printf("<HTML><style>A{text-decoration:none}</style>"
  "<TITLE>%s BBS 站 -- %s</TITLE>"
  "<BODY bgcolor=%s text=%s link=%s vlink=%s alink=%s>\n",
    BOARDNAME, str, BLACK, WHITE, LCRAY, GRAY, LRED);
  if (refresh)

    printf("<head><META HTTP-EQUIV=\"Refresh\" CONTENT=60></HEAD>");
  /*nekobe*/
  if (firstpage)
  {
  printf("<CENTER><IMG SRC=http://%s BORDER=0></CENTER>", BBSIMG);
  printf("<TD><CENTER><font size=6><i><b>%s</b></i></font>\n",BOARDNAME);
  }
  else
  printf("<CENTER><TABLE BORDER=0><TR>
  <TD><a href=http://%s/~bbs><IMG SRC=http://%s BORDER=0>BBS系統</a></TD></TR></TABLE>\n",
          MYHOSTNAME,BBSIMG);

  printf("<br>");
  /*
  printf("[ BBS |\n");
  printf(" <a href=http://ftp.civil.ncku.edu.tw/>FTP</a> |\n");
  printf(" <a href=http://mailing.civil.ncku.edu.tw/>Mailing</a> |\n");
  printf(" <a href=http://mrtg.civil.ncku.edu.tw/>MRTG</a> |\n");
  printf(" <a href=http://news.civil.ncku.edu.tw/>News</a> |\n");
  printf(" <a href=http://ns.civil.ncku.edu.tw/>NS</a> |\n");
  printf(" <a href=http://proxy.civil.ncku.edu.tw/>Proxy</a> |\n");
  printf(" <a href=http://www.civil.ncku.edu.tw/wel.html>WWW</a> ]<p>\n");
  printf("<img src=http://www.civil.ncku.edu.tw/pic/cvlhr2.gif>");
  */
  printf("<img src=http://newage.stat.tku.edu.tw/pic/cvlhr2.gif>");
  printf("<p>最後更新時間 %s<p>\n", ctime(&now));
  printf("[ <a href=/man/>精華公佈欄</a> |\n");
  printf(" <a href=/boards/>佈告討論區</a> |\n");
  printf(" <a href=/user/>線上使用者</a> ]\n");
  printf("<form method=\"post\">\n");
  printf("<font size=2>\n<input type=button value=\"回上一頁\"");
  printf("onclick=\"history.go( -1 );return true;\"> </font>\n");
  printf("\n</form>\n</p>\n");
}

html_error(char *url, char *thing)
{
  char buf[512];
  sprintf(buf, "錯誤的 URL: %s", url);
  html_head(buf, 0, 0, 0);
  printf("<h1>找不到 %s<br>請檢查檔案路徑名稱</h1>\n", url);
  errorlog(url, thing);
}

void tag8859_1( char **tag, char c )
{
/*   switch( c ) */
   {
/*
   case '\"': strcpy( *tag, "&quot " );   *tag+=6;break;
   case '<':  strcpy( *tag, "&lt " );     *tag+=4;break;
   case '>':  strcpy( *tag, "&gt " );     *tag+=4;break;
   case '&':  strcpy( *tag, "&amp " );    *tag+=5;break;
   default:
*/
      **tag=c; (*tag)++;
   }
}

int getansi( char **ap, int *attr, char *cmd )
{
   char cattr[100], *cap;

   cap = cattr; *cap=0;
   if( **ap == 0 ) return EOF;

   while( **ap>='0' && **ap<='9' )
   {
      *cap=**ap; cap++; (*ap)++;
   }
   if( cap==cattr ) return 0;

   *cap=0;
   sscanf( cattr, "%d", attr );
   if( **ap == 0 ) return 1;

   *cmd=**ap; (*ap)++;
   if( **ap == 0 ) return 2;
   return 3;
}

void add_href( char *l )
{
   char tag[MAX_WORDS], url[MAX_WORDS];
   char *ap, *tp, *u;
   int found;

   ap=l; tp=tag;

   while( *ap!=0 )
   {
      found=0;

      strncpy( url, ap, 7 ); url[7]=0;
      if (!strcasecmp(url,"http://") && (ENABLE_TAG==1))
        found=1;

      strncpy( url, ap, 6 ); url[6]=0;
      if (!strcmp(url,"ftp://") && (ENABLE_TAG==1))
        found=1;

      strncpy( url, ap, 7 ); url[7]=0;
      if (!strcmp(url,"file://") && (ENABLE_TAG==1))
        found=1;

      strncpy( url, ap, 9 ); url[9]=0;
      if (!strcmp(url,"gopher://") && (ENABLE_TAG==1))
        found=1;

      strncpy( url, ap, 9 ); url[9]=0;
      if (!strcmp(url,"telnet://") && (ENABLE_TAG==1))
        found=1;

      strncpy( url, ap, 7 ); url[7]=0;
/*nekobe*/
      if (!strcmp(url,"mailto:") && (ENABLE_TAG==1))
        found=1;

      if( found )
      {
         for( u=url; *ap!=0; *u++=*ap++ )
         {
            if( *ap==' ' ) break;
            if( *ap=='\"' ) break;
            if( *ap=='&' ) break;
            if( (unsigned char)*ap>=127 ) break;
         }
         *u=0;

         sprintf( tp, "<A HREF=\"%s\">%s</A>", url, url );
         tp+=(strlen(tp));
      }
      else /* Not URL */
      {
         *tp++=*ap++;
      }
   }

   *tp = 0;
   strcpy( l, tag );
}

void tagcolor( char **tag, int attr )
{
   switch( attr ) /* === filter no-used ansi control === */
   {
   case 0: case 5: case 1:
   case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
      break;

   default:
      return;
   }


   if( attr==5 )
   {
      if( ansi_blink==0 )
      { ansi_blink=1; strcpy( *tag, "<blink>" ); *tag+=7; }
      return;
   }

   if( ansi_blink ) { strcpy( *tag, "</blink>" ); *tag+=8; }

   if( ansi_on )
   {
      strcpy( *tag, "</FONT>" ); *tag+=7;
      ansi_on=0;
   }

   switch( attr )
   {
   case 0:  ansi_blink=0; return;
   case 1:  ansi_hlight=1; break;

   case 30:
        sprintf(*tag, "<FONT color=%s>", ansi_hlight ? GRAY : BLACK);
        *tag+=20;
        break;
   case 31:
        sprintf(*tag, "<FONT color=%s>", ansi_hlight ? LRED : RED);
        *tag+=20;
        break;
   case 32:
        sprintf(*tag, "<FONT color=%s>", ansi_hlight ? LGREEN : GREEN);
        *tag+=20;
        break;
   case 33:
        sprintf(*tag, "<FONT color=%s>", ansi_hlight ? LYELLOW : YELLOW);
        *tag+=20;
        break;
   case 34:
        sprintf(*tag, "<FONT color=%s>", ansi_hlight ? LBLUE : BLUE);
        *tag+=20;
        break;
   case 35:
        sprintf(*tag, "<FONT color=%s>", ansi_hlight ? LPURPLE : PURPLE);
        *tag+=20;
        break;
   case 36:
        sprintf(*tag, "<FONT color=%s>", ansi_hlight ? LCRAY : CRAY);
        *tag+=20;
        break;
   case 37:
        sprintf(*tag, "<FONT color=%s>", ansi_hlight ? WHITE : GRAY);
        *tag+=20;
        break;

   default:
      if( ansi_blink ) { strcpy( *tag, "<blink>" ); *tag+=7; }
      return;
   }

   if (ansi_blink) {
        strcpy( *tag, "<blink>" );
        *tag+=7;
   }

   ansi_on=1;
}

void ansi2tag( char *l )
{
   char tag[MAX_WORDS], esc[3];
   char *ap = l, *tp = tag, cmd;
   int  attr, num, nextansi=0;

   esc[2]=0;
   while(*ap)
   {
      esc[0]=ap[0];esc[1]=ap[1];
      if( !strcmp(esc,"[") || nextansi )
      {
         if( nextansi ) nextansi=0; else ap+=2;
         num=getansi( &ap, &attr, &cmd );
         if( num==EOF )
         {
            /* end-of-string */
            break;
         }
         else if( num==0 )
         {
            /* ANSI+noint? eat the word */
            ap++;
         }
         else if( num==1 )
         {
            /* ANSI+int+EOL go next line */
            break;
         }
         else if( num==2 )
         {
            /* ANSI+attr+cmd+EOL, set color and go next line */
            if( cmd=='m') tagcolor( &tp, attr );
            break;
         }
         else if( num==3 )
         {
            /* ANSI+attr+cmd+str, set color.. */
            tagcolor( &tp, attr );
            if( cmd==';') nextansi=1;
         }
      }
      else
      {
         tag8859_1( &tp, *ap ); ap++;
      }
   }
   *tp = 0;
   strcpy( l, tag );
}


/* ----------------------------------------------------- */
/* BOARD                                                 */
/* ----------------------------------------------------- */

// wildcat patch : 可以判斷 WD's CLASS & GROUP 看板
html_boardlist(char *boardtitle , int mode)
{
  FILE *fp;
  int i, count, inf;
/*nekobe*/
  int i_real;
  char prefix[6]="\0";

  html_head("看版列表", 0, 0, 0);

  printf("已有 %d 人次來看過", counter_("board"));
  printf("<table><tr><td bgcolor=%s width=50>編號</td><td bgcolor=%s width=75>看版</td>"
    "<td bgcolor=%s width=400>中文說明</td><td bgcolor=%s width=75>版主</td></tr>",DBLUE,DBLUE,DBLUE,DBLUE);

  i = count = 0;
/*nekobe*/
  i_real=0;
#if 1
  inf = open(BBSHOME "/" FN_BOARD, O_RDONLY);
  if (inf == -1)
  {
    printf("error open file\n");
    exit(1);
  }

  /* read in all boards */

  i = 0;
  memset(allbrd, 0, MAXBOARD * sizeof(boardheader));
  while (read(inf, &allbrd[i], sizeof(boardheader)) == sizeof(boardheader))
  {
    if (allbrd[i].brdname[0])
    {
      i++;
    }
  }
  close(inf);
  
  /* sort them by name */
  count = i;
  qsort(allbrd, count, sizeof(boardheader), board_cmp);

    /* write out the target file */
// wildcat :專門秀出 CLASS 用
  if(mode == 1)
  {
    for (i = 0; i < count; i++)
    {
      if (allbrd[i].brdattr & BRD_CLASS)
      {
      printf("<tr><td>%d</td><td><a href=/boards/%s/>%s</a></td><td>%s</td>"
        "<td>%s</td></tr>\n", i_real+1, allbrd[i].brdname, allbrd[i].brdname,
        allbrd[i].title, allbrd[i].BM);
        i_real++;
      }
    }
  }
// 所有看板
  else if(mode == 2)
  {
    for (i = 0; i < count; i++)
    {
      if (((!(allbrd[i].brdattr & BRD_POSTMASK) 
        && allbrd[i].level <=  PERM_LOGINOK) || (allbrd[i].brdattr & BRD_POSTMASK)) 
        && !(allbrd[i].brdattr & BRD_HIDE) && !(allbrd[i].brdattr & BRD_CLASS)
        && !(allbrd[i].brdattr & BRD_GROUPBOARD))
      {
      printf("<tr><td>%d</td><td><a href=/boards/%s/>%s</a></td><td>%s</td>"
        "<td>%s</td></tr>\n", i_real+1, allbrd[i].brdname, allbrd[i].brdname,
        allbrd[i].title, allbrd[i].BM);
        i_real++;
      }
    }
  }
// wildcat : 子看板或是普通看板
  else
  {
    for (i = 0; i < count; i++)
    {
      // wildcat : 抓 prefix
      strncpy(prefix,allbrd[i].title,4);
      prefix[4] ='\0';
      // wildcat : 為什麼子看板還會有 level ??
      if(strstr(allbrd[i].title,"Σ")) allbrd[i].level=0;
      if (((!(allbrd[i].brdattr & BRD_POSTMASK) && allbrd[i].level <=  PERM_LOGINOK)
      || (allbrd[i].brdattr & BRD_POSTMASK))
      && !(allbrd[i].brdattr & BRD_HIDE) && strstr(boardtitle+7,prefix))
                                            // wildcat : 記得要 + 7 ,不然會
                                            // 連子看板的 prefix 一起抓進去
      {
      printf("<tr><td>%d</td><td><a href=/boards/%s/>%s</a></td><td>%s</td>"
        "<td>%s</td></tr>\n", i_real+1, allbrd[i].brdname, allbrd[i].brdname,
        allbrd[i].title, allbrd[i].BM);
        i_real++;
      }
    }
  }
#endif
  printf("</table>");
}

html_board(char *board)
{
  char buf[80], brdname[20];
  boardheader *bhdr;

  strcpy(buf, board);
  strcpy(brdname, strtok(buf, "/\n\0"));
  bhdr = getbcache(brdname);
  if (bhdr == NULL)
  {
    sprintf(buf, "/boards/%s", board);
    html_error(buf, "board");
  }
  else if ( bhdr->brdattr & BRD_CLASS || bhdr->brdattr & BRD_GROUPBOARD)
    html_boardlist(bhdr->title,0);
  else if (((!(bhdr->brdattr & BRD_POSTMASK) && bhdr->level <=  PERM_LOGINOK)
      || (bhdr->brdattr & BRD_POSTMASK)) && !(bhdr->brdattr & BRD_HIDE))
  {
    if (strchr(board, '/') && strchr(strchr(board, '/') + 1, '/'))
    {
      sprintf(buf, "看版: %s", bhdr->brdname);
      html_head(buf, 0, 0, 0);

      sprintf(buf, "board-%s", bhdr->brdname);
      printf("看版 %s 有 %d 人次來看過", bhdr->brdname, counter_(buf));

      html_articlelist(board);
    }
    else
    {
      int i;
      sprintf(buf, "%s/boards/%s/.DIR", BBSHOME, board);
      i = get_num_records(buf, sizeof(fileheader));
      printf("<head><META HTTP-EQUIV=Refresh "
            "CONTENT=3;URL=http:/boards/%s/%d/></HEAD>",
            bhdr->brdname, i);//get_num_records(buf, sizeof(fileheader)));
      html_head("進版畫面", 0, 0, 1);
      brd_note(board,i);
    }
  }
  else
  {
    html_head("看版限制閱\讀", 0, 0, 0);
    printf("看版 %s 限制閱\讀<p>\n", brdname);
  }
}

/* ----------------------------------------------------- */
/* ARTICLE                                               */
/* ----------------------------------------------------- */

html_articlebar(char *board, int no)
{
  char buf[80], flastname[80], fnextname[80];
  int total;
  sprintf(buf, "%s/boards/%s/.DIR", BBSHOME, board);
  total = get_num_records(buf, sizeof(fileheader));

  if (no <= 1)
  {
    strcpy(fnextname, getfhdr(buf, no+1)->filename);
    printf("<table CELLSPACING=0 CELLPADDING=4 BORDER=0 width=80%%><tr>"
      "<td width=20%% align=center bgcolor=%s>"
//      "<a href=mailto:%s.board@%s>發表文章</a></td>"
      "<a href=/.post/%s>發表文章</a></td>"
      "<td width=20%% align=center bgcolor=%s>上一篇文章</td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%s>下一篇文章</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%d/>文章列表</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/man/boards/%s/>本版精華區</a></td></tr></table><p>\n",
      DBLUE, board, DBLUE, DBLUE, board, fnextname,
//      DBLUE, board, no < 100 ? 100 : no + 100, DBLUE, board);
// wildcat : 一百篇太長?!
      DBLUE, board, no < 20 ? 20 : no + 20, DBLUE, board);
  }
  else if (no == total)
  {
    strcpy(flastname, getfhdr(buf, no-1)->filename);
    printf("<table CELLSPACING=0 CELLPADDING=4 BORDER=0 width=80%%><tr>"
      "<td width=20%% align=center bgcolor=%s>"
//      "<a href=mailto:%s.board@%s>發表文章</a></td>"
      "<a href=/.post/%s>發表文章</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%s>上一篇文章</a></td>"
      "<td width=20%% align=center bgcolor=%s>下一篇文章</td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%d/>文章列表</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/man/boards/%s/>本版精華區</a></td></tr></table><p>\n",
      DBLUE, board, DBLUE, board, flastname, DBLUE,
      DBLUE, board, no, DBLUE, board);
  }
  else
  {
    strcpy(flastname, getfhdr(buf, no-1)->filename);
    strcpy(fnextname, getfhdr(buf, no+1)->filename);
    printf("<table CELLSPACING=0 CELLPADDING=4 BORDER=0 width=80%%><tr>"
      "<td width=20%% align=center bgcolor=%s>"
//      "<a href=mailto:%s.board@%s>發表文章</a></td>"
      "<a href=/.post/%s>發表文章</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%s>上一篇文章</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%s>下一篇文章</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%d/>文章列表</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/man/boards/%s/>本版精華區</a></td></tr></table><p>\n",
      DBLUE, board, DBLUE, board, flastname, DBLUE, board, fnextname,
// wildcat : 一百篇太長?!
//      DBLUE, board, no < total - 100 ? (total - 100) : total, DBLUE, board);
      DBLUE, board, no < total - 20 ? (total - 20) : total, DBLUE, board);
  }
}

html_articlelistbar(char *board, int last)
{
  char buf[80];
  int total;
  sprintf(buf, "%s/boards/%s/.DIR", BBSHOME, board);
  total = get_num_records(buf, sizeof(fileheader));

// wildcat : 一百篇太長?!
//  if (total < 100)
  if (total < 20)
    printf("<table CELLSPACING=0 CELLPADDING=4 BORDER=0 width=80%%><tr>"
      "<td width=20%% align=center bgcolor=%s>"
//      "<a href=mailto:%s.board@%s>發表文章</a></td>"
      "<a href=/.post/%s>發表文章</a></td>"
// wildcat : 一百篇太長?!
      "<td width=20%% align=center bgcolor=%s>上二十篇</td>"
      "<td width=20%% align=center bgcolor=%s>下二十篇</td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/>看版列表</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/man/boards/%s/>本版精華區</a></td></tr></table><p>\n",
      DBLUE, board, DBLUE, DBLUE, DBLUE, DBLUE, board);
  else if (last < 20)
    printf("<table CELLSPACING=0 CELLPADDING=4 BORDER=0 width=80%%><tr>"
      "<td width=20%% align=center bgcolor=%s>"
      "<a href=/.post/%s>發表文章</a></td>"
//      "<a href=mailto:%s.board@%s>發表文章</a></td>"
      "<td width=20%% align=center bgcolor=%s>上二十篇</td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%d/>下二十篇</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/>看版列表</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/man/boards/%s/>本版精華區</a></td></tr></table><p>\n",
      DBLUE, board, DBLUE,
      DBLUE, board, last + 20 > total ? total : last + 20, DBLUE, DBLUE, board);
  else if (last == total)
    printf("<table CELLSPACING=0 CELLPADDING=4 BORDER=0 width=80%%><tr>"
      "<td width=20%% align=center bgcolor=%s>"
      "<a href=/.post/%s>發表文章</a></td>"
//      "<a href=mailto:%s.board@%s>發表文章</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%d/>上二十篇</a></td>"
      "<td width=20%% align=center bgcolor=%s>下二十篇</td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/>看版列表</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/man/boards/%s/>本版精華區</a></td></tr></table><p>\n",
      DBLUE, board, DBLUE, board, last > 20 ? last - 20 : 0, DBLUE, DBLUE, DBLUE, board);
  else
    printf("<table CELLSPACING=0 CELLPADDING=4 BORDER=0 width=80%%><tr>"
      "<td width=20%% align=center bgcolor=%s>"
      "<a href=/.post/%s>發表文章</a></td>"
//      "<a href=mailto:%s.board@%s>發表文章</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%d/>上二十篇</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/%s/%d/>下二十篇</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/boards/>看版列表</a></td>"
      "<td width=20%% align=center bgcolor=%s><a href=/man/boards/%s/>本版精華區</a></td></tr></table><p>\n",
      DBLUE, board, DBLUE, board, last > 20 ? last - 20 : 0,
      DBLUE, board, last + 20 > total ? total : last + 20, DBLUE, DBLUE, board);
}

html_articlelist(char *str)
{
  FILE *fp;
  int i, first, last;
  char board[20], buf[80];
  fileheader fhdr;

  i = first = last = 0;
  strcpy(board, strtok(str, "/\n"));
  last = atoi(strtok(NULL, "/\n"));
// wildcat : 一百篇太長?!
  first = last > 20 ? last - 20 : 0;
  sprintf(buf, "%s/boards/%s/.DIR", BBSHOME, board);
  if (fp = fopen(buf, "r"))
  {
    html_articlelistbar(board, last);
    printf("<table><tr><td bgcolor=%s width=50>編號</td><td bgcolor=%s width=50>日期</td>"
        "<td bgcolor=%s width=100>作者</td><td bgcolor=%s width=400>標題</td></tr>\n",DBLUE,DBLUE,DBLUE,DBLUE);
    while(fread(&fhdr, sizeof(fhdr), 1, fp) && ++i)
    {
      if (i < first)
        continue;
      else if (i > last)
        break;
      printf("<tr><td>%d</td><td>%s</td><td>%s</td><td>"
        "<a href=/boards/%s/%s>%s</td></tr>\n",
        i, fhdr.date, fhdr.owner, board, fhdr.filename, fhdr.title);
    }
    printf("</table>");
    html_articlelistbar(board, last);
  fclose(fp);
  }
}


char *
html_word(char *word)
{
  char *ans = (char *) malloc(sizeof(char) * 512), buf[512];
  int i, j, k, ansi;
  ansi2tag(word);
  add_href(word);
  i = j = k =0;
  buf[0] = '\0';

  sprintf(buf,": %s\n",word);  
  strcat(ARTICLE,buf);

  if (*word == ':' || *word == '>')
  {
    sprintf(buf, "<font color=%s>%s</font>", CRAY, word);
    includetext = 1;
  }
  else if (!strncmp(word, "※", 2))
  {
    sprintf(buf, "<font color=%s>%s</font>", LGREEN, word);
    includetext = 1;
  }
  else if (!strncmp(word, "==>", 3) || word[0] == ':' || word[0] == '>')
  {
    sprintf(buf, "<font color=%s>%s</font>", CRAY, word);
    includetext = 1;
  }
  else if (includetext)
  {
    sprintf(buf, "<font color=%s>%s</font>", GRAY, word);
    includetext = 0;
  }
  else if (!strcmp(word, "--\n"))
    sprintf(buf, "</blink></u></font><font color=%s>%s", GRAY, word);
  else
    sprintf(buf, "%s", word);
  ans = buf;
  return (char *) (ans);
}

void
showuser(uec)
  userec *uec;
{
  int i=0 , user_num;
  FILE *plans;
  char modestr[40], buf[600], *ptr;
  char *cnum[10]={"一","二","三","四","五","六","七","八","九","十"};
  user_info *unow, *utail;

  printf("</center><table>");
  printf("<tr><td><font size=4 color=%s>%s</font><font size=4 color=%s>"
        " <a href=mailto:%s.bbs@%s>%s </a>(%s) "
        "共上站 %d 次，發表過 %d 篇文章</td></tr>\n",
  (uec->userlevel & PERM_LOGINOK) ? LRED : CRAY,
  (uec->userlevel & PERM_LOGINOK) ? "㊣" : "？", WHITE,
  uec->userid, MYHOSTNAME,
  uec->userid, uec->username, uec->numlogins, uec->numposts);
  ptr=uec->lasthost;
  if( strchr(ptr,'@') ){
      ptr=index(uec->lasthost,'@')+1;
  }
  printf("<tr><td>最近(%s)從[%s]上站</td></tr>\n",
        Cdate(&(uec->lastlogin)), ptr);

  strcpy(modestr,"不在站上");

  ushm_head = utmpshm->uinfo;
  ushm_tail = ushm_head + USHM_SIZE;

  unow = ushm_head;
  utail = ushm_tail;

  do
  {
    if (!strcmp(uec->userid, unow->userid))
    {
      if (!PERM_HIDE(unow))
         {
            strcpy(modestr, ModeTypeTable[unow->mode]);

          if(!unow->mode){
            if (unow->chatid[0] == 1)
              strcpy(modestr,"回應呼叫");
            if (unow->chatid[0] == 2)
              sprintf(modestr,"中%s顆水球", cnum[unow->msgcount-1]);
            else if (unow->chatid[0] == 3)
              strcpy(modestr,"水球準備中");
            else
              sprintf(modestr, "發呆: %s", unow->destuid == 6 ?
                unow->chatid : IdleTypeTable[(0 <= unow->destuid &&
                unow->destuid < 6) ? unow->destuid : 0]);
              }

          if(unow->mode == LOGIN && uec->numlogins == 0)
            strcpy(modestr,"新伙伴註冊中");
         }
      break;
    }
  } while (++unow <= ushm_tail);

  sprintf(buf, BBSHOME"/home/%s/.forward", uec->userid);

  printf("<tr><td><font color=%s>[目前動態：%s]</font><font color=%s> %s</td></tr>\n",
    LRED, modestr, BLACK, dashf(buf) ? "信件自動轉寄" :
      (chkmail(uec->userid) ? "有新進信件還沒看":"所有信件都看過了"));

  sprintf(buf, BBSHOME"/WWW/user/%s.html", uec->userid);
  if(!dashf(buf))
    sprintf(buf, BBSHOME"/home/%s/plans", uec->userid);
  if( !(i=access(buf, F_OK)) ){
      printf("<tr><td>%s 的名片:</td></tr></table><pre>\n", uec->userid);
      if( (plans=fopen(buf,"r")) == NULL ){
          perror("fopen:plans");
          exit(1);
      }
      while (/*i++ < MAXQUERYLINES && */ fgets(buf, 512, plans))
      {
        char word[1000];
        sprintf(word, "%s", html_word(buf));
        printf(word);
      }
      printf("</font></pre><table>");
      fclose(plans);
  }
  else
      printf("<tr><td>%s 目前沒有名片</td></tr>\n", uec->userid);

  sprintf(buf, BBSHOME"/home/%s/note.ans", uec->userid);
  if( !(i=access(buf, F_OK)) ){
      printf("<tr><td><a href=/~%s/note>看 %s 的留言版</a></td></tr>\n"
      , uec->userid, uec->userid);
     printf("</font>");
  }
  else
      printf("<tr><td>%s 目前沒有留言版</td></tr>\n", uec->userid);

  sprintf(buf, BBSHOME"/home/%s/.profile", uec->userid);
  if (!(i=access(buf, F_OK)) )
  {
      printf("<tr><td><a href=/~%s/profile>看 %s 的基本資料</a></td></tr>\n"
       , uec->userid, uec->userid);
  }
  else
    printf("<tr><td>%s 目前沒有基本資料</td></tr>\n", uec->userid);

  printf("</table>");
}


html_note(char *userid)
{
  char buf[600];
  FILE *plans;

    sprintf(buf, "%s的留言本", userid);
    html_head(buf, 0, 1, 0);

  sprintf(buf, BBSHOME"/home/%s/note.ans", userid);

  printf("<a href=#write>寫留言給%s</a></center><pre>\n", userid, userid);

  if( (plans=fopen(buf,"r")) == NULL ){
          printf("</pre>原本沒有言本<p>\n");/*
          perror("fopen:plans");*/
      }
  else
  {
      while (fgets(buf, 512, plans))
      {
        char word[1000];
        sprintf(word, "%s", html_word(buf));
        printf(word);
      }
     printf("</font></pre>");
     fclose(plans);
  }

  printf("<hr>");
  printf("<center><font size=6><b><a name=write>寫留言給%s</b></font></a></center>", userid);
  printf("<form action=http://%s/~%s/cgi-bin/note.cgi method=post>\n",
          MYHOSTNAME, BBSUSER);
  printf("<input type=hidden name=name value=%s><p>\n", userid);
  printf("代  號: <input type=text name=myname size=12 maxlength=12><p>\n");
  printf("第一行: <input type=text name=line1 size=40 maxlength=80><p>\n");
  printf("第二行: <input type=text name=line2 size=40 maxlength=80><p>\n");
  printf("第三行: <input type=text name=line3 size=40 maxlength=80><p>\n");
  printf("<input type=submit name=option value=送出>     ");
  printf("<input type=reset name=name value=清除>\n");
  printf("</form>");
}

int
html_profile(char *userid)
{
  FILE *pf;
  char buffer[300], fnb[200];
  char proname[PROFILE_NUM][PROFILE_SIZE];
  char prodata[PROFILE_NUM][PROFILE_SIZE];
  int i, j, size, fix, still;
  char *p;

  sprintf(buffer, "%s的基本資料", userid);
  html_head(buffer, 1, 0, 0);

  printf("<center><font size=6>使用者 %s 的基本資料</font><br>", userid);
  printf("[ <a href=/~%s/> %s 的狀態</a> ", userid, userid);
  printf(" | <a href=/~%s/note> %s 的留言本 </a> ", userid, userid);
  printf(" ] <br><br>");


  sprintf(fnb, BBSHOME"/home/%s/.profile", userid);

  if (pf=fopen(fnb, "r"))
  {
    while (fgets(buffer, 256, pf))
    {
      p=(char *)buffer;
      i++;
      j=0;
      while(buffer[j] != '|') j++;
      strncpy(proname[i], buffer, j);
      j=j+2;
      if (strlen(buffer)>j+1)
      {
        p+=j;
        if (strstr(p, "\n")) p=strtok(p, "\n");
        strcpy(prodata[i], p);
      }
    }
    fclose(pf);
  }
  size=i;

  printf("<table border=1>\n");
/*nekobe*/
  printf("<tr><td rowspan=5>%s</td><td>%s</td><td>%s</td>", prodata[6],  proname[1], prodata[1]);
  for (i=2; i<=5; i++)
  {
        printf("<tr><td>%s</td><td>%s　</td></tr>\n", proname[i], prodata[i]);
  }
  for (i=7; i<=size; i++)
  {
  printf("<tr><td>%s</td><td colspan=2>%s　</td></tr>\n", proname[i], prodata[i]);
  }

/*  for (i=1; i<=size; i++)
  {
    printf("<tr><td>%s<td>%s</tr>\n", proname[i], prodata[i]);
  }
*/
  printf("</table>\n");
}


html_mainnote()
{
  char buf[600];
  FILE *plans;

  sprintf(buf, BBSHOME"/note.ans");

  printf("<p><a href=/write>寫留言</a></center><pre>\n");

      if( (plans=fopen(buf,"r")) == NULL ){
          printf("</font></pre>");
          perror("fopen:plans");
          exit(1);
      }
      while (fgets(buf, 512, plans))
      {
        char word[1000];
        sprintf(word, "%s", html_word(buf));
        printf(word);
      }
     printf("</font></pre>");
     fclose(plans);
}

brd_note(char *board,int postnum)
{
  char buf[600];
  FILE *plans;

  printf("</center><p><pre>\n");

  sprintf(buf, BBSHOME"/boards/%s/notes", board);

      if( (plans=fopen(buf,"r")) == NULL ){
          return;
      }
      while (fgets(buf, 512, plans))
      {
        char word[1000];
        sprintf(word, "%s", html_word(buf));
        printf(word);
      }
     printf("</font></pre><center>");
     //wildcat : 怕 user 不知道要等多久,可以直接進入
     printf("<br><a href=/boards/%s%d/>進入 %s 板</a><br>",board,postnum,board);
     fclose(plans);
}


html_boardarticle(char *fpath)
{
  FILE *fp;
  char buf[512], fdir[512], fname[20], brdname[20], title[TTLEN];
  fileheader *fhdr;
  int no, i;

  sprintf(buf, "%s%s", BBSHOME, fpath);
  if (fp = fopen(buf, "r"))
  {
    sprintf(fdir, "%s%s", BBSHOME, fpath);
    strcpy(fname, strrchr(fpath, '/') + 1);
    strcpy(buf, fpath + 8);
    strcpy(brdname, strtok(buf, "/"));
    strcpy(strrchr(fdir, '/') + 1, ".DIR");
    no = getindex(fdir, fname, sizeof(fileheader));
    strcpy(title, getfhdr(fdir, no)->title);
    sprintf(buf, "文章: %s", title);
    html_head(buf, 0, 1, 0);
    html_articlebar(brdname, no);
    printf("</center><pre><font size=3>");
    while(fgets(buf, 512, fp))
    {
      char word[512];
      sprintf(word, "%s", html_word(buf));
      printf(word);
    }
    printf("</font></pre><center>");
    printf("<a href=/.reply%s>回覆文章</a>",fpath);
    html_articlebar(brdname, no);
  }
  else
    html_error(fpath, "article");
}

html_manarticle(char *fpath)
{
  FILE *fp;
  char buf[512], fdir[512], fname[20], title[TTLEN];
  fileheader *fhdr;
  int no, i;

  sprintf(buf, "%s%s", BBSHOME, fpath);

  if (dashd(buf))
  {
    if (buf[strlen(buf+1)]=='/')
    {
      buf[strlen(buf)-1]='\0';
      i = readlink(buf, fdir, 250);
      fdir[i]='/';
      fdir[i+1]='\0';
      html_man(fdir+10);
      return 0;
    }
  }

  sprintf(buf, "%s%s", BBSHOME, fpath);
  if (fp = fopen(buf, "r"))
  {
    sprintf(fdir, "%s%s", BBSHOME, fpath);
    strcpy(fname, strrchr(fpath, '/') + 1);
    strcpy(buf, fpath + 8);
    strcpy(strrchr(fdir, '/') + 1, ".DIR");
    no = getindex(fdir, fname, sizeof(fileheader));
    strcpy(title, getfhdr(fdir, no)->title);
    sprintf(buf, "文章: %s", title);
    html_head(buf, 0, 1, 0);
    printf("</center><pre><font size=4>");
    while(fgets(buf, 512, fp))
    {
      char word[1024];
      sprintf(word, "%s", html_word(buf));
      printf(word);
    }
    printf("</font></pre><center>");
  }
  else
    html_error(fpath, "article");
}


html_userlist()
{
  int i;

  html_head("線上使用者列表", 1, 0, 0);

  printf("已有 %d 人次來看過 , ", counter_("user"));

  printf(" 我是 %s ",auth_ok ? cuser.userid : "guest");

  printf("<TABLE border=1>");
  print_head();

  for (i = user_num = hide_num = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->userid[0] && !PERM_HIDE(uentp) && !uentp->invisible)
    {
      dump_record(i, uentp);
      print_record();
      user_num++;
    }
    else if (PERM_HIDE(uentp) || uentp->invisible)
      hide_num++;
  }
  printf("<TD COLSPAN=6 bgcolor=%s>"
        "<CENTER>目前站上有 %d 人 , 隱形人 %d 人"
        "</CENTER></FONT></TABLE>\n", DBLUE, user_num+hide_num,hide_num);
}

html_tail()
{
  printf("<p><center>[ <A href=\"telnet://%s\">",MYHOSTNAME);
  printf("登入 BBS 站 |</a>\n");
  /*
  printf(" <A href=http://mrtg.civil.ncku.edu.tw/bbs/bbs.civil.ncku.edu.tw.html>");
  printf("本站線上使用者統計列表 |</a>\n");
  */
  printf(" <A href=mailto:wildcat.bbs@%s>",MYHOSTNAME);
  printf("留下您寶貴的意見給 wildcat</a> ]<HR>");

  printf("</CENTER></BODY></HTML>\n");
}

html_query(char *userid)
{
  char buf[80];
  FILE *pwd;

  if((pwd=fopen(BBSHOME"/.PASSWDS","r")) == NULL){
     printf("無法開啟密碼檔<p>\n");
     return;
  }

  resolve_utmp();

  while(fread(&uec, sizeof(userec), 1, pwd) > 0 )
  {
      if(ci_strcmp(uec.userid, userid))
      {
        continue;
      }
      else
      {
        sprintf(buf, "查詢使用者 %s", userid);
        html_head(buf, 1, 0, 0);
        sprintf(buf, "user-%s", userid);
        printf("使用者 %s 有 %d 人次來查過", userid, counter_(buf));

        showuser(&uec);
      }
      return;
  }
  sprintf(buf, "/~%s/", userid);
  html_error(buf, "user");
}

html_man(char *man)
{
  fileheader fhdr;
  char fdir[1024], fname[1024];
  FILE *fp;
  int i = 0;

  sprintf(fdir, "精華區: %s", man);
  html_head(fdir, 0, 0, 0);
  printf("已有 %d 人次來看過", counter_("man"));

  sprintf(fdir,"%s/%s/.DIR", BBSHOME, man);
  if (fp = fopen(fdir, "r")) {
    printf("<center><table>\n");
//    printf("<td align=left bgcolor=%s width=50>類型</td>", DBLUE);
    printf("<td align=left bgcolor=%s width=50>編號</td>", DBLUE);
    printf("<td align=left bgcolor=%s width=50>日期</td>", DBLUE);
    printf("<td align=left bgcolor=%s width=100>編選</td>", DBLUE);
    printf("<td align=left bgcolor=%s width=400>標題</td></tr>", DBLUE);
    while(fread(&fhdr, sizeof(fhdr), 1, fp) && ++i) 
    {
      if (!(fhdr.filemode & FILE_REFUSE))
      {	/* shakalaca.990816: 有限制的就不要送出去了 */
        sprintf(fname,"%s/%s/%s", BBSHOME, man, fhdr.filename);
        printf("<tr>");
/*        printf("<td><img src=http://wdbbs.net/icons/%s></td>",
          dashl(fname) ? "link.gif" : dashd(fname) ? "folder.gif" : 
          dashf(fname) ? "text.gif" : "unknown.gif"); */
        printf("<td>%d</td>", i);
        printf("<td>%s</td>", fhdr.date);
        printf("<td>%s</td>", fhdr.owner);
        printf("<td><a href=/%s%s%s>", man, fhdr.filename, dashd(fname) ? "/" : "");
        printf("%s</a></td>", fhdr.title);
        printf("</tr>\n");
      }
    }
    printf("</center></table>\n");
  }
}

pro_file()
{
  FILE *fp;
  char buf[512]; 

  sprintf(buf, BBSHOME"/etc/today");
  fp=fopen(buf, "r");
  if( !access(buf, F_OK)){
      printf("<font size=3><pre>");
      while (fgets(buf, 512, fp))
      {
        char word[1000];
        sprintf(word, "%s", html_word(buf));
        printf(word);
      }
      printf("</font></pre><table>");
      fclose(fp);
  }
}

mainpage()
{
  FILE *fp;
  char buf[512];

  sprintf(buf, "%s/WWW/index.html", BBSHOME);
  fp=fopen(buf, "r");

  while (fgets(buf, 512, fp))
  {
    printf(buf);
  }
  fclose(fp);
}

html_post(char *brdid)
{
  char buf[600];

  if(!brdid[0]) html_error("/.post/","post");
  else
  {
    sprintf(buf, "在 %s 板發表文章", brdid);
    html_head(buf, 0, 1, 0);

    printf("<hr>");
    printf("<center><font size=6><b><a name=write>在 %s 板發表文章</b></font></a>", brdid);
    printf("<table border=0 >");
    printf("<form action=http://%s/cgi-bin/cgiwrap/%s/post.cgi method=post>\n",
          MYHOSTNAME, BBSUSER);
    printf("<tr><td>帳  號:</td><td> <input type=text name=id size=12 maxlength=12></td></tr>\n");
    printf("<tr><td>密  碼:</td><td> <input type=password name=passwd size=12 maxlength=12></td></tr>\n");
    printf("<tr><td>看  板:</td><td> %s<input type=hidden name=board value=%s ></td></tr>\n",brdid,brdid);
    printf("<tr><td>標  題:</td><td> <input type=text name=subject size=40 maxlength=80></td></tr>\n");
    printf("<tr><td>文  章:</td><td> <textarea name=article cols=60 rows=10 wrap=virtual></textarea></td></tr></table>\n");
    printf("<input type=submit name=option value=送出>  ");
    printf("<input type=reset name=name value=清除>\n");
    printf("</form>");
  }
}

html_reply(char *brdid,char *fpath)
{
  char buf[600],*title;
  FILE *fp;

  if(!brdid[0]) html_error("/.reply/","post");
  else
  {
    sprintf(buf, "在 %s 板回覆文章", brdid);
    html_head(buf, 0, 1, 0);
    sprintf(buf,"%s%s",BBSHOME,fpath); 
    if (fp = fopen(buf, "r"))
    {
      while(fgets(buf, 512, fp))
      {
        char word[512];
        sprintf(word, ": %s", buf);
        strcat(ARTICLE,word); 
      }
    }
    printf("<hr>");
    printf("<center><font size=6><b><a name=write>在 %s 板回覆文章</b></font></a>", brdid);
    printf("<table border=0>");
    printf("<form action=http://%s/cgi-bin/cgiwrap/%s/post.cgi method=post>\n",
          MYHOSTNAME, BBSUSER);
    printf("<tr><td>帳  號:</td><td> <input type=text name=id size=12 maxlength=12></td></tr>\n");
    printf("<tr><td>密  碼:</td><td> <input type=password name=passwd size=12 maxlength=12></td></tr>\n");
    printf("<tr><td>看  板:</td><td> %s<input type=hidden name=board value=%s></td></tr>\n",brdid,brdid);
    printf("<tr><td>標  題:</td><td> <input type=text name=subject size=40 maxlength=80 value=Re:></td></tr>\n");
    printf("<tr><td>文  章:</td><td> <textarea name=article cols=70 rows=30 wrap=virtual>%s</textarea></td></tr></table>\n",ARTICLE);
    printf("<input type=submit name=option value=送出>  ");
    printf("<input type=reset name=name value=清除>\n");
    printf("</form>");
  }
}


/* ----------------------------------------------------- */
/* Main program                                          */
/* ----------------------------------------------------- */

main(argc, argv, envp)
  int argc;
  char *argv[];
  char *envp[];
{
  char method[10], buf[512];
  int done=0;

  resolve_utmp();

  if (fscanf(stdin, "%s %s %s", method, buf, protocol))
  {
    accesslog(method, buf);
    if (!strcmp(buf, "/user/"))
      html_userlist();
    else if (!strncmp(buf, "/.post/",7))
      html_post(buf+7);
    else if (!strncmp(buf, "/.reply/",8))
    {
      char *bid;
      strcpy(bid,buf+15);
      strtok(bid,"/");
      html_reply(bid,buf+7);
    }
    else if (!strncmp(buf, "/man/", 5))
    {
       if (strstr(buf, "/M.") || strstr(buf,"/S.") || strstr(buf, "/G."))
         html_manarticle(buf); 
       else
         html_man(buf + 1);
    }
    else if (!strncmp(buf, "/boards/", 7))
    {
      if (!strcmp(buf, "/boards/"))
// wildcat : 秀分類看板 !
        html_boardlist("看板",1);
      else if (!strcmp(buf, "/boards/all/"))
// wildcat : 秀所有看板 !
        html_boardlist("看板",2);
      else if (strstr(buf, "/M."))
        html_boardarticle(buf);
      else
        html_board(buf+8);
    }
    else if (!strncmp(buf, "/~", 2))
    {
      if (strstr(buf, "note"))
      {
        html_note(strtok(buf +2, "/\n"));
        done=1;
      }
      if (strstr(buf, "profile"))
      {
        html_profile(strtok(buf +2, "/\n"));
        done=1;
      }
      if ((buf[2])&& !(done))
      {
        html_query(strtok(buf + 2, "/\n"));
        done=1;
      }
      if (!(done))
        printf("<head><META HTTP-EQUIV=\"Refresh\" CONTENT=0;URL=/user/></HEAD>");

    }
    else if (!strcmp(buf, "/"))
    {
      html_head("主選單", 0, 0, 1);
      printf("已有 %d 人次來看過", counter_("main"));
      printf("<a href=http://%s/~%s/>本站介紹</a> | ",
             MYHOSTNAME, BBSUSER);
      printf("<a href=/note>看留言板</a> | <a href=/profile>看排行榜</a> ]");

      /* for(i=0; argv[i]!= (char *) 0; i++)
        printf("%s\n", argv[i]);*/
      mainpage();
    }
    else if (!strcmp(buf, "/note"))
    {
      html_head("留言版", 0, 1, 0);
      printf("已有 %d 人次來看過", counter_("note"));
      html_mainnote();
    }
    else
    if (!strcmp(buf, "/profile"))
    {
      html_head("本站站友", 0, 0, 1);
      printf("已有 %d 人次來看過", counter_("profile"));
      pro_file();
    }
    else if (!strcmp(buf, "/countuser"))
    {
      int i;
      for (i = user_num = hide_num = 0; i < USHM_SIZE; i++)
      {
        uentp = &(utmpshm->uinfo[i]);
        if (uentp->userid[0] && !PERM_HIDE(uentp) && !uentp->invisible)
        {
          user_num++;
        }
        else if (PERM_HIDE(uentp) || uentp->invisible)
          hide_num++;
      }
      printf("%d\n\n", user_num+hide_num);
    }
    else
      html_error(buf, "unknown");
  }
  html_tail();
}
