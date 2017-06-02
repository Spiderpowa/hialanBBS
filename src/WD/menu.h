/*-------------------------------------------------------*/
/* menu.h       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu                                         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

// wildcat : ����u�Ϊ��ſ�� :p
int
null_menu()
{
  pressanykey("�o�O�@�Ӫſ�� :p ");
  return 0;
}

void domenu();
/* ----------------------------------------------------- */
/* The set user menu for administrator                   */
/* ----------------------------------------------------- */


static struct one_key adminuser[] = {
'1',  null_menu,	PERM_ADMIN,	"1\033[1;31m�`�N:�T��f�ֵ��U��!!\033[m",0,
'2',  null_menu,    	PERM_ADMIN,	"2\033[1;31m�`�N:�T������ק��]\033[m",0,
'U',  "SO/admin.so:m_user",
			PERM_ACCOUNTS,  "User          [�ϥΪ̸��]",1,
'M',  "SO/register.so:m_newuser",
                        PERM_ACCOUNTS,	"Make User     [�s�W�ϥΪ�]",1,
'F',  "SO/admin.so:search_key_user",
                	PERM_ACCOUNTS,	"Find User     [�j�M�ϥΪ�]",1,  
'R',  "SO/admin.so:m_register",   	
                        PERM_ACCOUNTS,	"Register      [�f�ֵ��U��]",1,
'\0', NULL, 0, NULL, 0};

int
m_user_admin()
{
  domenu(ADMIN, "�]�w�ϥΪ�", 'U',adminuser);
  return 0;
}

/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */

int XFile();

#ifdef  HAVE_MAILCLEAN
int m_mclean();
#endif

int hialan_test()
{
  edit_loginplan();
}

static struct one_key adminlist[] = {
'H',  hialan_test, 		0, "Hialan test", 0,
'U',  m_user_admin, PERM_ACCOUNTS, "User Menu     [�ϥΪ̳]�w]",0,
'N',  "SO/admin.so:m_newbrd",
                       PERM_BOARD, "New Board     [�}�ҷs�ݪO]",1,
'S',  "SO/admin.so:m_board",
               		PERM_BOARD,"Set Board     [ �]�w�ݪO ]",1,
'X',  XFile,        	PERM_SYSOP,"Xfile         [�ק�t����]",0,
'C',  "SO/admin.so:reload_cache",
                	PERM_SYSOP,"Cache Reload  [ ��s���A ]",1,
'G',  "SO/admin.so:adm_givegold", 
                	PERM_SYSOP,"Give $$       [ �o����� ]",1,
#ifdef HAVE_MAILCLEAN
//'M',  "SO/admin.so:m_mclean, PERM_BBSADM, "Mail Clean    �M�z�ϥΪ̭ӤH�H�c",1,
#endif
'M',  "SO/admin.so:x_reg",
                     PERM_ACCOUNTS,"Merge         [�f�֭ײz��]",1,
'\0', NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* class menu                                            */
/* ----------------------------------------------------- */

int board(), local_board(), Boards(), ReadSelect() ,
    New(),Favor(),favor_edit(),good_board(),voteboard();

static struct one_key classlist[] = {
'V',  voteboard, PERM_BASIC,"VoteBoard    [�ݪO�s�p�t��]",0,
'C',      board, 	  0,"Class        [���������ݪO]",0,
'N',        New, 	  0,"New          [�Ҧ��ݪO�C��]",0,
'L',local_board, 	  0,"Local        [�����ݪO�C��]",0,
'G', good_board, 	  0,"GoodBoard    [  �u�}�ݪO  ]",0,
'B',      Favor, PERM_BASIC,"BoardFavor   [�ڪ��̷R�ݪO]",0,
'F', favor_edit, PERM_BASIC,"FavorEdit    [�s��ڪ��̷R]",0,
'S', ReadSelect, 	  0,"Select       [  ��ܬݪO  ]",0,
0,NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* Ptt money menu                                        */
/* ----------------------------------------------------- */

static struct one_key finlist[] = {
'0',  "SO/buy.so:bank",     0,      "0Bank           \033[1;36m�����Ȧ�\033[m",1,
'1',  "SO/song.so:ordersong",0,     "1OrderSong      \033[1;33m�����I�q��\033[m     $5g/��",1,
'2',  "SO/buy.so:p_cloak",  0,      "2Cloak          �{������/�{��  $2g/��     (�{���K�O)",1,
'3',  "SO/buy.so:p_from",   0,      "3From           �ק�G�m       $5g/��",1,
'4',  "SO/buy.so:p_exmail", 0,      "4BuyMailbox     �ʶR�H�c�W��   $100g/��",1,
'5',  "SO/buy.so:p_spmail", 0,      "5VenMailbox     ��X�H�c�W��   $80g/��",1,
'6',  "SO/buy.so:p_fcloak", 0,      "6UltimaCloak    �׷������j�k   $500g      �i�ä[����",1,
'7',  "SO/buy.so:p_ffrom",  0,      "7PlaceBook      �G�m�ק��_��   $1000g     User�W���F�i��G�m",1,
'8',  "SO/buy.so:p_ulmail", 0,      "8NoLimit        �H�c�L�W��     $100000g   �H�c�W���L����",1,
0, NULL, 0, NULL,0};

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
// t_talk();
/* Thor: for ask last call-in message */
int t_display();
int t_bmw();

static struct one_key talklist[] = {

'L',  t_users,      0,              "List          [�u�W�W��]",0,
'P',  t_pager,      PERM_BASIC,     "Pager         [�������A]",0,
'I',  t_idle,       0,              "Idle          [��w�ù�]",0,
'Q',  t_query,      0,              "QueryUser     [�d��User]",0,
/*
'T',  t_talk,       PERM_PAGE,      "Talk          [��H���]",0,
 */
'C',  "SO/chat.so:t_chat",PERM_CHAT,"ChatRoom      [�s�u���]",1,
'D',  t_display,    0,              "Display       [���y�^�U]",0,
'W',  t_bmw,        PERM_PAGE,      "Write         [���y�^�U]", 0,
'X',  XFile,        PERM_XFILE,     "Xfile         [�ק�t����]",0,
0, NULL, 0, NULL,0};

/*-------------------------------------------------------*/
/* powerbook menu                                        */
/* ----------------------------------------------------- */

int my_gem();

static struct one_key powerlist[] = {
'G',  my_gem,              0,       "Gem           [�ڪ����]",0,
'-',  null_menu,           0,	    "----- ������ �\\�� -----",0,
'P',  "SO/pnote.so:p_read",0,       "Phone Answer  [ť���d��]",1,
'C',  "SO/pnote.so:p_call",0,       "Call phone    [�e�X�d��]",1,
'R',  "SO/pnote.so:p_edithint",0,   "Record        [���w���]",1,

0, NULL, 0, NULL,0};

int
PowerBook()
{
  domenu(POWERBOOK, "�U�Τ�U", 'G', powerlist);
  return 0;
}
/* ----------------------------------------------------- */
/* Habit menu                                            */
/* ----------------------------------------------------- */
int u_habit(), change_lightbar(), win_formchange(), edit_loginplan();
static struct one_key userhabit[] = {
'H', u_habit,		     0,  "Habit         [�ߦn�]�w]",0,
'B', change_lightbar,        0,	 "Bar Change    [�����C��]",0,
'W', win_formchange,	     0,  "Win Form      [�����~�[]",0,
'L', edit_loginplan,PERM_BASIC,  "Login Plan    [�W���]�w]",0,
0, NULL, 0, NULL,0};

static int
UserHabit()
{
  domenu(UMENU, "�ߦn�]�w", 'H', userhabit);
  return 0;
}
/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

extern int u_editfile();
int u_info(), u_cloak(), u_list(), 
    PowerBook(), ListMain();

#ifdef REG_MAGICKEY
int u_verify();
#endif

static struct one_key userlist[] = {
'P',  PowerBook,    PERM_BASIC,	 "PowerBook     [�U�Τ�U]",0,
'I',  u_info,       PERM_BASIC,  "Info          [�ק���]",0,
'H',  UserHabit,    0, 	         "Habit         [�ߦn�]�w]",0,
'L',  ListMain,     PERM_LOGINOK,"List          [�]�w�W��]",0, 
'F',  u_editfile,   PERM_LOGINOK,"FileEdit      [�ӤH�ɮ�]",0,
'C',  u_cloak,      PERM_CLOAK,  "Cloak         [���αK�k]",0,

#ifdef REG_FORM
'R', "SO/register.so:u_register",PERM_POST,"Register      [����U��]",1,
#endif

#ifdef REG_MAGICKEY
'V',  u_verify,	    PERM_BASIC,  "Verify        [����U�X]",0,
#endif
'U',  u_list,       PERM_BASIC,  "Users         [���U�W��]",0,
0, NULL, 0, NULL,0};


/* ----------------------------------------------------- */
/* service menu                                          */
/* ----------------------------------------------------- */
int note();

static struct one_key servicelist[] = {
'F',  finance,      PERM_LOGINOK,   "Finance       [�ӫ~�j��]",0,
  
'V',  "SO/vote.so:all_vote",
                    PERM_LOGINOK,   "Vote          [�벼����]",1,
'N',  note,         PERM_LOGINOK,   "Note          [�g�d���O]",0,
0,  NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* mail menu                                             */
/* ----------------------------------------------------- */
int m_new(), m_read(), m_send(),m_sysop(),mail_mbox(),mail_all(),
    setforward(),mail_list();

#ifdef INTERNET_PRIVATE_EMAIL
int m_internet();
#endif

static struct one_key maillist[] = {
'N',  m_new,        PERM_READMAIL,  "New           [�\\Ū�s�H]",0,
'R',  m_read,       PERM_READMAIL,  "Read          [�H��C��]",0,
'S',  m_send,       PERM_BASIC,     "Send          [�����H�H]",0,
'M',  mail_list,    PERM_BASIC,     "Mailist       [�s�ձH�H]",0,
'I',  m_internet,   PERM_INTERNET,  "Internet      [�����l��]",0,
'F',  setforward,   PERM_LOGINOK,   "Forward       [���H��H]",0,
'O',  m_sysop,      0,              "Op Mail       [�ԴA����]",0,
'Z',  mail_mbox,    PERM_INTERNET,  "Zip           [���]���]",0,
'A',  mail_all,     PERM_SYSOP,     "All           [�t�γq�i]",0,
0, NULL, 0, NULL,0};



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


int Announce(), Boards(), Goodbye(), board(), Favor();


struct one_key cmdlist[] = {
'A',  Announce,     0,              "Announce      [�Ѧa���]",0,
'B',  BOARD,        0,              "Board         [�ݪO�\\��]",0,
'C',  board,        0,              "Class         [�����ݪO]",0,
'F',  Favor,        PERM_BASIC,     "Favor         [�ڪ��̷R]",0,
'M',  Mail,         PERM_BASIC,     "Mail          [�֭��ֻ�]",0,
'T',  Talk,         0,              "Talk          [�ͤѻ��a]",0,
'U',  User,         0,              "User          [�ӤH�u��]",0,
'S',  Service,      PERM_BASIC,     "Service       [�U�تA��]",0,
'0',  admin,        PERM_ADMIN,     "0Admin        [�t�κ޲z]",0,
'G',  Goodbye,      0,              "Goodbye       [���t�d��]",0,
0, NULL, 0, NULL,0};
/* INDENT ON */
