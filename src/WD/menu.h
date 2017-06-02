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

/* ----------------------------------------------------- */
/* Menu Commands struct                                  */
/* ----------------------------------------------------- */
#if 0
  hialan: MENU ���c�j�P�W�ϥ� desc �� key �h�L��
          �b desc ���i�H���w���槹�o�ӫ��O�����U�@�ӿﶵ
          �H�U�O desc ���ϥΪk:

          /* next/key/description*/
          
          �Ҧp:
           "RNew           [�\\Ū�s�H]",
           "RRead          [�H��C��]",
           
         �h�� n �|����\Ū�s�H, �� r �|����H��C��
         ����槹�\Ū�s�H��, ��з|�۰ʸ���H��C��
#endif

void domenu();
/* ----------------------------------------------------- */
/* The set user menu for administrator                   */
/* ----------------------------------------------------- */


static struct one_key adminuser[] = {
'1',  null_menu,	   	PERM_ADMIN,	"11\033[1;31m�`�N:�T��f�ֵ��U��!!\033[m",0,
'2',  null_menu,    	PERM_ADMIN,	"22\033[1;31m�`�N:�T������ק��]\033[m",0,
'u',  "SO/admin.so:m_user",	PERM_ACCOUNTS,	"UUser          [�ϥΪ̸��]",1,
'm',  "SO/register.so:m_newuser",
                        PERM_ACCOUNTS,	"MMake User     [�s�W�ϥΪ�]",1,
'f',  "SO/admin.so:search_key_user",
                	PERM_ACCOUNTS,	"FFind User     [�j�M�ϥΪ�]",1,  
'r',  "SO/admin.so:m_register",   	
                        PERM_ACCOUNTS,	"RRegister      [�f�ֵ��U��]",1,
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
  win_formchange();
}

static struct one_key adminlist[] = {
'H',  hialan_test, 0, "HHialan test", 0,
'U',  m_user_admin, PERM_ACCOUNTS,	"UUser Menu     [�ϥΪ̳]�w]",0,
'N',  "SO/admin.so:m_newbrd",
                PERM_BOARD,     "NNew Board     [�}�ҷs�ݪO]",1,
'S',  "SO/admin.so:m_board",
                PERM_BOARD,     "SSet Board     [ �]�w�ݪO ]",1,
'X',  XFile,        PERM_SYSOP,     "XXfile         [�ק�t����]",0,
'C',  "SO/admin.so:reload_cache",
                PERM_SYSOP,     "CCache Reload  [ ��s���A ]",1,
'G',  "SO/admin.so:adm_givegold", 
                PERM_SYSOP,     "GGive $$       [ �o����� ]",1,

#ifdef HAVE_MAILCLEAN
//'M',  "SO/admin.so:m_mclean, PERM_BBSADM, "MMail Clean    �M�z�ϥΪ̭ӤH�H�c",1,
#endif
'M',  "SO/admin.so:x_reg",
                PERM_ACCOUNTS,  "MMerge         [�f�֭ײz��]",1,
'\0', NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* class menu                                            */
/* ----------------------------------------------------- */

int board(), local_board(), Boards(), ReadSelect() ,
    New(),Favor(),favor_edit(),good_board(),voteboard();

static struct one_key classlist[] = {
'V',  voteboard, PERM_BASIC,"VVoteBoard    [�ݪO�s�p�t��]",0,
'C',      board, 0,         "CClass        [���������ݪO]",0,
'N',        New, 0,         "NNew          [�Ҧ��ݪO�C��]",0,
'L',local_board, 0,         "LLocal        [�����ݪO�C��]",0,
'G', good_board, 0,         "GGoodBoard    [  �u�}�ݪO  ]",0,
'B',      Favor, PERM_BASIC,"BBoardFavor   [�ڪ��̷R�ݪO]",0,
'F', favor_edit, PERM_BASIC,"FFavorEdit    [�s��ڪ��̷R]",0,
'S', ReadSelect, 0,         "SSelect       [  ��ܬݪO  ]",0,
0,NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* Ptt money menu                                        */
/* ----------------------------------------------------- */

static struct one_key finlist[] = {
'0',  "SO/buy.so:bank",     0,      "00Bank           \033[1;36m�����Ȧ�\033[m",1,
'1',  "SO/song.so:ordersong",0,     "11OrderSong      \033[1;33m�����I�q��\033[m     $5g/��",1,
'2',  "SO/buy.so:p_cloak",  0,      "22Cloak          �{������/�{��  $2g/��     (�{���K�O)",1,
'3',  "SO/buy.so:p_from",   0,      "33From           �ק�G�m       $5g/��",1,
'4',  "SO/buy.so:p_exmail", 0,      "44BuyMailbox     �ʶR�H�c�W��   $100g/��",1,
'5',  "SO/buy.so:p_spmail", 0,	"55VenMailbox     ��X�H�c�W��   $80g/��",1,
'6',  "SO/buy.so:p_fcloak", 0,      "66UltimaCloak    �׷������j�k   $500g      �i�ä[����",1,
'7',  "SO/buy.so:p_ffrom",  0,      "77PlaceBook      �G�m�ק��_��   $1000g     User�W���F�i��G�m",1,
'8',  "SO/buy.so:p_ulmail", 0,      "88NoLimit        �H�c�L�W��     $100000g   �H�c�W���L����",1,
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

'L',  t_users,      0,              "LList          [�u�W�W��]",0,
'P',  t_pager,      PERM_BASIC,     "PPager         [�������A]",0,
'I',  t_idle,       0,              "IIdle          [��w�ù�]",0,
'Q',  t_query,      0,              "QQueryUser     [�d��User]",0,
/*
'T',  t_talk,       PERM_PAGE,      "TTalk          [��H���]",0,
 */
'C',  "SO/chat.so:t_chat",PERM_CHAT,"CChatRoom      [�s�u���]",1,
'D',  t_display,    0,              "DDisplay       [���y�^�U]",0,
'W',  t_bmw,        PERM_PAGE,      "WWrite         [���y�^�U]", 0,
'X',  XFile,        PERM_XFILE,     "XXfile         [�ק�t����]",0,
0, NULL, 0, NULL,0};

/*-------------------------------------------------------*/
/* powerbook menu                                        */
/* ----------------------------------------------------- */

int my_gem();

static struct one_key powerlist[] = {
'G',  my_gem,              0,       "GGem           [�ڪ����]",0,
'-',  null_menu,           0,	"------ ������ �\\�� ------",0,
'P',  "SO/pnote.so:p_read",0,       "PPhone Answer  [ť���d��]",1,
'C',  "SO/pnote.so:p_call",0,       "CCall phone    [�e�X�d��]",1,
'R',  "SO/pnote.so:p_edithint",0,   "RRecord        [���w���]",1,

0, NULL, 0, NULL,0};

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
int u_info(), u_cloak(), u_list(), u_habit(),
    PowerBook(), ListMain(), change_lightbar();

#ifdef REG_MAGICKEY
int u_verify();
#endif

static struct one_key userlist[] = {
'P',  PowerBook,    PERM_BASIC,	 "PPowerBook     [�U�Τ�U]",0,
'I',  u_info,       PERM_BASIC,  "IInfo          [�ק���]",0,
'H',  u_habit,      0, 	         "HHabit         [�ߦn�]�w]",0,
'L',  ListMain,     PERM_LOGINOK,"LList          [�]�w�W��]",0, 
'F',  u_editfile,   PERM_LOGINOK,"FFileEdit      [�ӤH�ɮ�]",0,
'C',  u_cloak,      PERM_CLOAK,  "CCloak         [���αK�k]",0,

#ifdef REG_FORM
'R', "SO/register.so:u_register",PERM_POST,"RRegister      [����U��]",1,
#endif

#ifdef REG_MAGICKEY
'V',  u_verify, 	PERM_BASIC,	"VVerify        [����U�X]",0,
#endif

'U',  u_list,       PERM_BASIC,     "UUsers         [���U�W��]",0,
  
'B', change_lightbar,         0,	"BBar Change    [�����C��]",0,
0, NULL, 0, NULL,0};


/* ----------------------------------------------------- */
/* service menu                                          */
/* ----------------------------------------------------- */
int note();

static struct one_key servicelist[] = {
'F',  finance,      PERM_LOGINOK,   "FFinance       [�ӫ~�j��]",0,
  
'V',  "SO/vote.so:all_vote",
                PERM_LOGINOK,   "VVote          [�벼����]",1,
'N',  note,         PERM_LOGINOK,   "NNote          [�g�d���O]",0,
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
'N',  m_new,        PERM_READMAIL,  "RNew           [�\\Ū�s�H]",0,
'R',  m_read,       PERM_READMAIL,  "RRead          [�H��C��]",0,
'S',  m_send,       PERM_BASIC,     "SSend          [�����H�H]",0,
'M',  mail_list,    PERM_BASIC,     "MMailist       [�s�ձH�H]",0,
'I',  m_internet,   PERM_INTERNET,  "IInternet      [�����l��]",0,
'F',  setforward,   PERM_LOGINOK,   "FForward       [���H��H]",0,
'O',  m_sysop,      0,              "OOp Mail       [�ԴA����]",0,
'Z',  mail_mbox,    PERM_INTERNET,  "ZZip           [���]���]",0,
'A',  mail_all,     PERM_SYSOP,     "AAll           [�t�γq�i]",0,
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
'A',  Announce,     0,              "AAnnounce      [�Ѧa���]",0,
'B',  BOARD,        0,              "BBoard         [�ݪO�\\��]",0,
'C',  board,        0,              "CClass         [�����ݪO]",0,
'F',  Favor,        PERM_BASIC,     "FFavor         [�ڪ��̷R]",0,
'M',  Mail,         PERM_BASIC,     "MMail          [�֭��ֻ�]",0,
'T',  Talk,         0,              "TTalk          [�ͤѻ��a]",0,
'U',  User,         0,              "UUser          [�ӤH�u��]",0,
'S',  Service,      PERM_BASIC,     "SService       [�U�تA��]",0,
'0',  admin,        PERM_ADMIN,     "00Admin        [�t�κ޲z]",0,
'G',  Goodbye,      0,              "GGoodbye       [���t�d��]",0,
0, NULL, 0, NULL,0};
/* INDENT ON */
