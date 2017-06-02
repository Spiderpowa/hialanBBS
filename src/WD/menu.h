/*-------------------------------------------------------*/
/* menu.h       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu                                         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
      
/* ----------------------------------------------------- */
/* Menu Commands struct                                  */
/* ----------------------------------------------------- */

struct MENU
{
  void *cmdfunc;
//  int (*cmdfunc) ();
  usint level;
  char *desc;                   /* next/key/description */
  int mode;
};
typedef struct MENU MENU;

void domenu(int, char *, int, MENU *);
/* ----------------------------------------------------- */
/* The set user menu for administrator                   */
/* ----------------------------------------------------- */

int admin_please();

static struct MENU adminuser[] = {
  admin_please,	   	PERM_ADMIN,	"11\033[1;31m�`�N:�T��f�ֵ��U��!!\033[m",0,
  admin_please,    	PERM_ADMIN,	"22\033[1;31m�`�N:�T������ק��]\033[m",0,
  "SO/admin.so:m_user",	PERM_ACCOUNTS,	"UUser          [�ϥΪ̸��]",1,
  "SO/register.so:m_newuser",
                        PERM_ACCOUNTS,	"MMake User     [�s�W�ϥΪ�]",1,
  "SO/admin.so:search_key_user",
                	PERM_ACCOUNTS,	"FFind User     [�j�M�ϥΪ�]",1,  
  "SO/admin.so:m_register",   	
                        PERM_ACCOUNTS,	"RRegister      [�f�ֵ��U��]",1,
NULL, 0, NULL, 0};

int
m_user_admin()
{
  domenu(ADMIN, "�]�w�ϥΪ�", 'U',adminuser);
  return 0;
}

int
admin_please()
{
  return 0;
}

/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */

int XFile();

#ifdef  HAVE_MAILCLEAN
int m_mclean();
#endif

static struct MENU adminlist[] = {
  m_user_admin, PERM_ACCOUNTS,	"UUser Menu     [�ϥΪ̳]�w]",0,
  "SO/admin.so:m_newbrd",
                PERM_BOARD,     "NNew Board     [�}�ҷs�ݪO]",1,
  "SO/admin.so:m_board",
                PERM_BOARD,     "SSet Board     [ �]�w�ݪO ]",1,
  XFile,        PERM_SYSOP,     "XXfile         [�ק�t����]",0,
  "SO/admin.so:reload_cache",
                PERM_SYSOP,     "CCache Reload  [ ��s���A ]",1,
  "SO/admin.so:adm_givegold", 
                PERM_SYSOP,     "GGive $$       [ �o����� ]",1,

/*
  m_mclean, PERM_BBSADM, "MMail Clean    �M�z�ϥΪ̭ӤH�H�c",0,
#endif */
  "SO/admin.so:x_reg",
                PERM_ACCOUNTS,  "MMerge         [�f�֭ײz��]",1,
  
NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* class menu                                            */
/* ----------------------------------------------------- */

int board(), local_board(), Boards(), ReadSelect() ,
    New(),Favor(),favor_edit(),good_board(),voteboard();

static struct MENU classlist[] = {
voteboard, PERM_BASIC,"VVoteBoard    [�ݪO�s�p�t��]",0,
   board, 0,          "CClass        [���������ݪO]",0,
   New, 0,            "NNew          [�Ҧ��ݪO�C��]",0,
   local_board,0,     "LLocal        [�����ݪO�C��]",0,
   good_board,0,      "GGoodBoard    [  �u�}�ݪO  ]",0,
   Favor,PERM_BASIC,  "BBoardFavor   [�ڪ��̷R�ݪO]",0,
favor_edit,PERM_BASIC,"FFavorEdit    [�s��ڪ��̷R]",0,
   ReadSelect, 0,     "SSelect       [  ��ܬݪO  ]",0,
   NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* Ptt money menu                                        */
/* ----------------------------------------------------- */

static struct MENU finlist[] = {
  "SO/buy.so:bank",     0,      "00Bank           \033[1;36m�����Ȧ�\033[m",1,
  "SO/song.so:ordersong",0,     "11OrderSong      \033[1;33m�����I�q��\033[m     $5g/��",1,
  "SO/buy.so:p_cloak",  0,      "22Cloak          �{������/�{��  $2g/��     (�{���K�O)",1,
  "SO/buy.so:p_from",   0,      "33From           �ק�G�m       $5g/��",1,
  "SO/buy.so:p_exmail", 0,      "44BuyMailbox     �ʶR�H�c�W��   $100g/��",1,
  "SO/buy.so:p_spmail", 0,	"55VenMailbox     ��X�H�c�W��   $80g/��",1,
  "SO/buy.so:p_fcloak", 0,      "66UltimaCloak    �׷������j�k   $500g      �i�ä[����",1,
  "SO/buy.so:p_ffrom",  0,      "77PlaceBook      �G�m�ק��_��   $1000g     User�W���F�i��G�m",1,
  "SO/buy.so:p_ulmail", 0,      "88NoLimit        �H�c�L�W��     $100000g   �H�c�W���L����",1,
NULL, 0, NULL,0};

int
finance()
{
  domenu(FINANCE, "���Ĥ���", '0', finlist);
  return 0;
}

/* ----------------------------------------------------- */
/* Talk menu                                             */
/* ----------------------------------------------------- */

int t_users(), t_idle(), t_query(), t_pager();
// t_chat(), t_list(), t_talk();
/* Thor: for ask last call-in message */
int t_display();
int t_bmw();

static struct MENU talklist[] = {

  t_users,      0,              "LList          [�u�W�W��]",0,
  t_pager,      PERM_BASIC,     "PPager         [�������A]",0,
  t_idle,       0,              "IIdle          [��w�ù�]",0,
  t_query,      0,              "QQueryUser     [�d��User]",0,
/*
  t_talk,       PERM_PAGE,      "TTalk          [��H���]",0,
 */
  "SO/chat.so:t_chat",PERM_CHAT,"CChatRoom      [�s�u���]",1,
  t_display,    0,              "DDisplay       [�ѭ��^�U]",0,
  t_bmw,        PERM_PAGE,      "WWrite         [���n�^�U]", 0,
  XFile,        PERM_XFILE,     "XXfile         [�ק�t����]",0,
NULL, 0, NULL,0};

/*-------------------------------------------------------*/
/* powerbook menu                                        */
/* ----------------------------------------------------- */

int null_menu(),my_gem();

static struct MENU powerlist[] = {
  my_gem,              0,       "GGem           [�ڪ����]",0,
  null_menu,           0,	"------ ������ �\\�� ------",0,
  "SO/pnote.so:p_read",0,       "PPhone Answer  [ť���d��]",1,
  "SO/pnote.so:p_call",0,       "CCall phone    [�e�X�d��]",1,
  "SO/pnote.so:p_edithint",0,   "RRecord        [���w���]",1,

NULL, 0, NULL,0};

int
PowerBook()
{
  domenu(POWERBOOK, "�U�Τ�U", 'G', powerlist);
  return 0;
}


/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

extern int u_editfile();
int u_info(), u_cloak(), u_list(), u_habit(), PowerBook(), ListMain(), change_lightbar();

#ifdef REG_MAGICKEY
int u_verify();
#endif

#ifdef POSTNOTIFY
int re_m_postnotify();
#endif

static struct MENU userlist[] = {
  PowerBook,	PERM_BASIC,	"PPowerBook     [�U�Τ�U]",0,
  u_info,       PERM_BASIC,     "IInfo          [�ק���]",0,
  u_habit,      0, 	        "HHabit         [�ߦn�]�w]",0,
  ListMain,     PERM_LOGINOK,   "LList          [�]�w�W��]",0, 

#ifdef POSTNOTIFY
  re_m_postnotify,PERM_BASIC,   "PPostNotify    [�f�֤峹�q��]",0,
#endif

  u_editfile,   PERM_LOGINOK,   "FFileEdit      [�ӤH�ɮ�]",0,
  u_cloak,      PERM_CLOAK,     "CCloak         [���αK�k]",0,

#ifdef REG_FORM
  //u_register,   PERM_POST,      "RRegister      [����U��]",0,
"SO/register.so:u_register",PERM_POST,"RRegister      [����U��]",1,
#endif

#ifdef REG_MAGICKEY
  u_verify, 	PERM_BASIC,	"VVerify        [����U�X]",0,
#endif

  u_list,       PERM_BASIC,     "UUsers         [���U�W��]",0,
  
change_lightbar,         0,	"BBar Change    [�����C��]",0,
NULL, 0, NULL,0};

int note();

static struct MENU servicelist[] = {
  finance,      PERM_LOGINOK,   "FFinance       [�ӫ~�j��]",0,
  
  "SO/vote.so:all_vote",
                PERM_LOGINOK,   "VVote          [�벼����]",1,
  note,         PERM_LOGINOK,   "NNote          [�g�d���O]",0,
  NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* mail menu                                             */
/* ----------------------------------------------------- */
int m_new(), m_read(), m_send(),m_sysop(),mail_mbox(),mail_all(),
    setforward(),mail_list();

#ifdef INTERNET_PRIVATE_EMAIL
int m_internet();
#endif

static struct MENU maillist[] = {
  m_new,        PERM_READMAIL,  "RNew           [�\\Ū�s�H]",0,
  m_read,       PERM_READMAIL,  "RRead          [�H��C��]",0,
  m_send,       PERM_BASIC,     "SSend          [�����H�H]",0,
  mail_list,    PERM_BASIC,     "MMailist       [�s�ձH�H]",0,
  m_internet,   PERM_INTERNET,  "IInternet      [�����l��]",0,
  setforward,   PERM_LOGINOK,   "FForward       [���H��H]",0,
  m_sysop,      0,              "OOp Mail       [�ԴA����]",0,
  mail_mbox,    PERM_INTERNET,  "ZZip           [���]���]",0,
  mail_all,     PERM_SYSOP,     "AAll           [�t�γq�i]",0,
NULL, 0, NULL,0};



/* ----------------------------------------------------- */
/* main menu                                             */
/* ----------------------------------------------------- */

static int
admin()
{
  domenu(ADMIN, "�����Ѥj", 'X', adminlist);
  return 0;
}

static int
BOARD()
{
  domenu(CLASS, "�ݪO�C��", 'G', classlist);
  return 0;
}

static int
Mail()
{
  domenu(MAIL, "�l����", 'R', maillist);
  return 0;
}

int
static Talk()
{
  domenu(TMENU, "��ѿ��", 'L', talklist);
  return 0;
}

static int
User()
{
  domenu(UMENU, "�ӤH�]�w", 'H', userlist);
  return 0;
}


static int
Service()
{
  domenu(PMENU, "�U�تA��", 'F', servicelist);
  return 0;
}


int Announce(), Boards(), Goodbye(),Log(),board(),Favor();


struct MENU cmdlist[] = {
  Announce,     0,              "AAnnounce      [�Ѧa���]",0,
  BOARD,        0,              "BBoard         [�ݪO�\\��]",0,
  board,        0,              "CClass         [�����ݪO]",0,
  Favor,        PERM_BASIC,     "FFavor         [�ڪ��̷R]",0,
  Mail,         PERM_BASIC,     "MMail          [�֭��ֻ�]",0,
  Talk,         0,              "TTalk          [�ͤѻ��a]",0,
  User,         0,              "UUser          [�ӤH�u��]",0,
/*
  Log,          0,              "HHistory       [���v�y��]",0,
*/ 
  Service,      PERM_BASIC,     "SService       [�U�تA��]",0,
  admin,        PERM_ADMIN,     "00Admin        [�t�κ޲z]",0,
  Goodbye,      0,              "GGoodbye       [���t�d��]",0,
NULL, 0, NULL,0};
/* INDENT ON */
