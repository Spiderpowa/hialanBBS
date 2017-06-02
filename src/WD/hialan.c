/*-------------------*/
/*  hialan's script  */
/*-------------------*/

#include "bbs.h"

//#define HIALAN_DEBUG

int
change_bp(y, title, desc)
  char desc[3][80];
  char *title;
  int y;
{
  int i, ch;
  char barcolor[50];
  char *bgcolor="\033[0;30;46m";
  
  move(y, 0);
  clrtoeol();
  prints("   \033[0m\033[30m\033[47m    %-53s%-16s\033[m", title, "�� q �䵲��");

  y++;
  for(i = 0;i < 3;i++)
  {
    move(i+y,0);
    clrtoeol();
    prints("     %s%-71s\033[m", bgcolor, desc[i]);
  }

  get_lightbar_color(barcolor);
  i = 0;
  while(1)
  {
    move(i+y,0);
    clrtoeol();
    prints("   ��%s%-71s\033[m", barcolor, desc[i]);
    
    ch = igetkey();
    
    move(i+y,0);
    clrtoeol();
    prints("     %s%-71s\033[m", bgcolor, desc[i]);
   
    switch(ch)
    {
      case KEY_UP:
        i--;
        if(i < 0) i = 2;
        break;
        
      case KEY_DOWN:
        i++;
        if(i > 2) i = 0;
        break;
      
      case '\r':
      
        do
        {
          getdata(i+y, 0, "     ", desc[i], 71, DOECHO, desc[i]);
 
          move(i+y,0);
          clrtoeol();
          prints("     %s%-71s\033[m", bgcolor, desc[i]);

          i++;
          if(i > 2) i = 0;
        }while(desc[i][0] == '\0' && i != 0);
      
        break;
      
      case 'Q':
      case 'q':
        return 0;
    }
  }
  pressanykey(NULL);
}

int 
word_bigsmall(ch)
  int ch;
{
  if(ch >= 'a' && ch <= 'z')
    ch = (ch - 'a') + 'A';
                                                                                
  return ch;
}
                                                                                
/*  getans2 �ǭȸ���  by hialan 2002/4/14
                                                                                
line    --> �ĴX�C
col     --> �ĴX�Ӧr
prompt  --> ����
s       --> �ﶵ�榡
  �Ҧp:
    char *test[2] = {"yY.�O","nN.�_"};
  �Ĥ@�Ӥp�gy�O�ֳt��
  �ĤG�ӥH��Ψ����
many    --> ���X�ӿﶵ
def     --> �w�]��
  �P�W��:
    getans2(b_line,0,"", test, 2, 'y');
                                                                                
*/
                                                                                
char
getans2(line, col, prompt, s, many, def)
  int line,col,many;
  char *prompt, **s, def;
{
  int point = 0;        /*����*/
  int len = strlen(prompt);
  int i;
  int off_set = 0;
  int ch;
  char bar_color[50];
  char buf2[128];
  char *p;      /*�T��������*/
  char *choose_yesno[3]={"yY.�O","nN.�_","qQ.����"};
                                                                                
  /*�w�]��*/
  off_set = offset_count(prompt);
                                                                                
  if(!s) s = choose_yesno;
                                                                               
  for(i = 0;i < many;i++)
  {
    p = s[i];
    if(def == *p)
    {
      point = i;
      break;
    }
  }
  
  /* ��� getdata */                                                                              
  if(!HAVE_HABIT(HABIT_LIGHTBAR))
  {
    char tmp[128],tmp2[2];
                                                                                
    buf2[0] = '\0';
    tmp2[0] = word_bigsmall(*(s[point]));
    tmp2[1] = '\0';
                                                                                
    strcpy(tmp,prompt);
                                                                                
    for(i = 0; i < many; i++)
    {
      p = s[i];
      strcat(tmp, p+1);
      strcat(tmp, " ");
    }
                                                                                
    sprintf(tmp,"%s[%c] ", tmp, tmp2[0]);
    getdata(line, col, tmp, buf2, 4, LCECHO, 0);
                                                                                
    if(*buf2 == '\0')
      return *tmp2;
    else
      return *buf2;
  }
                                                                                
  get_lightbar_color(bar_color);
                                                                                
  /*�M�ſù�*/
  if (prompt)
  {
    move(line, col);
    clrtoeol();
    outs(prompt);
  }
  else
    clrtoeol();
                                                                                
  do
  {
    for(i = 0;i < many;i++)
    {
      char buf[128];
                                                                                
      p = s[i];
      if(i == point)
        sprintf(buf,"\033[1m[\033[m%s%s\033[0;1m]\033[m",bar_color,p+1);
      else
        sprintf(buf," %s ",p+1);
                                                                                
      if(i == 0)
        sprintf(buf2,"%s",buf);
      else
        strcat(buf2,buf);
    }
    move(line, (col+len) - off_set);
    outs(buf2);
    move(b_lines,0);
                                                                                
    ch = igetkey();
    switch(ch)
    {
      case KEY_TAB:
      case KEY_DOWN:
      case KEY_RIGHT:
        point++;
        if(point >= many)
          point = 0;
        break;
      case KEY_UP:
      case KEY_LEFT:
        point--;
        if(point < 0)
          point = many - 1;
        break;
                                                                                
      default:
        ch = word_bigsmall(ch);
        for(i = 0;i < many;i++)
        {
          p = s[i];
          if(ch == word_bigsmall(*p))
            point = i;
        }
        break;
    }
  }while(ch != '\r');
                                                                                
  return *(s[point]);
                                                                                
  /*  �H�W����
      p = s[point];
      return p[0];*/
}

