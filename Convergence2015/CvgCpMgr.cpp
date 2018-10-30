/******************************************************************************

��    Ȩ:��������������ϵͳ���޹�˾.
ģ������:OfferingMgr.cpp
������	:��ΰ
��������:2008.07.22
��    ��:1.0				
ģ������:�������̻��������ص���ģ��
��Ҫ����:Init(...)��ʼ������
         Finish() ��������
         Run()�����̺߳���
�޸ļ�¼:
******************************************************************************/

#include <iostream>
#include "Logger.h"
#include "ConfigImpl.h"
#include "CvgCpMgr.h"
#include "HisDataHandler.h"
#include "ServiceHandler.h"
#include "DeliverMgr.h"
#include "IfSvAgent.h"
#include "ProtocolConnectPoint.h"
#include "ProcessInterfaceZC.h"
#include "ProcessInterfaceZS.h"
#include "ProcessInterfaceH1C.h"
#include "ProcessInterfaceH2C.h"
#include "ProcessInterfaceKC.h"
#include "ProcessInterfaceCmd.h"
#include "LinePacket.h"
#include "GessTimerMgrPosix.h"
#include "AbsTimerMgrWin32.h"
#include "NetMgrModule.h"
#include <sstream>
#include <iomanip>

// added by Jerry Lee, 2010-12-21, ֧�ַ������ݵ��������
#include "SendCommand.h"

//Դ�ӿ�+������ ����·�����ñ�
CCvgCpMgr::IfRouterCfg CCvgCpMgr::m_tblIfRouterCfg[] = 
{
	//from EnumKeyIfZC1
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC1,     "00000006"},
	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC1,     "00000006"},

	{0,    EnumKeyServiceHandler1,		EnumKeyIfZC1,     "80000003"},

	//���ĳɹ���Ҫ����
	{0,    	EnumKeySvAgent,				EnumKeyIfZC1,     "80000003"},

	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler1,		EnumKeyIfZC1,     gc_sDefaultCmdID},

	//��Ϊ���ж�,�ϼ��ڵ�ļ�ز�ѯ���� ֻ���ܴ��ϼ�cvg�յ���SV_REQ����
	{0,		   EnumKeySvAgent,			EnumKeyIfZC1,     "00000011"},
	//��Ϊ���ж�,�ϼ��ڵ�ļ�ز�ѯӦ�� Ӧ������ת��
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "80000011"},
	//��Ϊ���ж�,��ظ澯�¼� �¼�����ת��
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "00000012"},
	//��Ϊ���ж�,�����¼�����ת��
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "00000013"},
	//��Ϊ���ж�,�˶��¼�����ת��
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "00000014"},
	//��Ϊ���ж�,��ز�ѯ���¼��ڵ�ת�� ��������ת��
	{0,		   EnumKeyIfZS,				EnumKeySvAgent,   "00000011"},

	//�¼��ڵ����ļ�����ѯӦ��SV_RSP
	{0,		   EnumKeySvAgent,			EnumKeyIfZS,		"80000011"},
	//�¼��ڵ����ļ���¼��澯SV_NTF
	{0,		   EnumKeySvAgent,			EnumKeyIfZS,		"00000012"},
	//�¼��ڵ����Ķ���֪ͨ
	{0,		   EnumKeySvAgent,			EnumKeyIfZS,		"00000013"},

	//from EnumKeyServiceHandler1
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC1,				EnumKeyServiceHandler1,     gc_sDefaultCmdID},

	//from EnumKeyIfZC2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC2,     "00000006"},
	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC2,     "00000006"},
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler2,		EnumKeyIfZC2,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC2,				EnumKeyServiceHandler2,     gc_sDefaultCmdID},



	//from EnumKeyIfZC3
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC3,     "00000006"},
	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC3,     "00000006"},
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler3,		EnumKeyIfZC3,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler3
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC3,				EnumKeyServiceHandler3,     gc_sDefaultCmdID},



	//from EnumKeyIfZC4
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC4,     "00000006"},
	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC4,     "00000006"},
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler4,		EnumKeyIfZC4,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler4
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC4,				EnumKeyServiceHandler4,     gc_sDefaultCmdID},





	//from EnumKeyIfZC5
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC5,     "00000006"},

	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC5,     "00000006"},
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler5,		EnumKeyIfZC5,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler5
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC5,				EnumKeyServiceHandler5,     gc_sDefaultCmdID},






	//from EnumKeyIfZC6
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC6,     "00000006"},

	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC6,     "00000006"},
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler6,		EnumKeyIfZC6,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler6
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC6,				EnumKeyServiceHandler6,     gc_sDefaultCmdID},






	//from EnumKeyIfZC7
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC7,     "00000006"},

	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC7,     "00000006"},
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler7,		EnumKeyIfZC7,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler7
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC7,				EnumKeyServiceHandler7,     gc_sDefaultCmdID},
	//from EnumKeyIfZC8
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC8,     "00000006"},

	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC8,     "00000006"},
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler8,		EnumKeyIfZC8,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler8
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC8,				EnumKeyServiceHandler8,     gc_sDefaultCmdID},




	//from EnumKeyDeliverMgr
	//EnumKeyServiceHandler1 To EnumKeyIfZC1 ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZS,				EnumKeyDeliverMgr,     gc_sDefaultCmdID},


	//from EnumKeyIfZS
	//EnumKeyIfZS To EnumKeyDeliverMgr 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////	
	{0,    EnumKeyDeliverMgr,			EnumKeyIfZS,     "00000003"},
	{0,    EnumKeyDeliverMgr,			EnumKeyIfZS,     "00000004"},

	//����/�˶�
	{0,    EnumKeySvAgent,				EnumKeyIfZS,     "00000003"},
	{0,    EnumKeySvAgent,				EnumKeyIfZS,     "00000004"},


	//EnumKeyIfZC1 To EnumKeyTranslator ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	//{0,		EnumKeyIfZC1,			EnumKeyIfZS,     "00000005"},
	{0,		EnumKeyServiceHandlerSvr,	EnumKeyIfZS,     gc_sDefaultCmdID},

	//from EnumKeyIfZS
	//EnumKeyIfZC1 To EnumKeyTranslator ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyIfZS,		EnumKeyServiceHandlerSvr,     gc_sDefaultCmdID},
	{0,		   EnumKeyIfZC1,	EnumKeyServiceHandlerSvr,     "00000005"},

	//from H1	
	//H1 To NetMgrModule
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeySvAgent,			EnumKeyIfH1,            "1921"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1922"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1923"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1924"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,		    "1925"}, 


	//from NetMgrModule	
	//NetMgrModule To H1 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		//
	{0,    EnumKeySvAgent,				EnumNetMagModule,       "1921"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1922"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1923"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1924"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1925"}, 


	//NetMgrModule To H2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From					CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onEventNotify"},   //�¼��㲥�౨��
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onAlarmNotify"},   //�澯�㲥�౨��
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onNodeMibTblChg"}, //��¼�仯�㲥����

	//from IFCMD
	//IFCMD To default ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyCmdHandler,		EnumKeyIfCmd,    			 gc_sDefaultCmdID},

    // added by Jerry Lee, 2010-12-21, �յ���ʷ���������Ĵ�������
    {0,	   EnumKeyDeliverMgr,		EnumKeyIfZC1,     "00000007"},
    {0,	   EnumKeyDeliverMgr,		EnumKeyIfZC2,     "00000007"},
	{0,	   EnumKeyDeliverMgr,		EnumKeyIfZC3,     "00000007"},//kenny 2013-10-10,�������ýڵ�
	{0,	   EnumKeyDeliverMgr,		EnumKeyIfZC4,     "00000007"},

    // added by Jerry Lee, 2011-1-17, �յ�tick���������Ĵ�������
    {0,	   EnumKeyDeliverMgr,		EnumKeyIfZC1,     "00000008"},
    {0,	   EnumKeyDeliverMgr,		EnumKeyIfZC2,     "00000008"},
	{0,	   EnumKeyDeliverMgr,		EnumKeyIfZC3,     "00000008"},//kenny 2013-10-10,�������ýڵ�
	{0,	   EnumKeyDeliverMgr,		EnumKeyIfZC4,     "00000008"},

    // added by Jerry Lee, 2011-2-24, �յ�info���������Ĵ�������
    {0,	   EnumKeyDeliverMgr,		EnumKeyIfZC1,     "00000009"},
    {0,	   EnumKeyDeliverMgr,		EnumKeyIfZC2,     "00000009"},
	{0,	   EnumKeyDeliverMgr,		EnumKeyIfZC3,     "00000009"},//kenny 2013-10-10,�������ýڵ�
	{0,	   EnumKeyDeliverMgr,		EnumKeyIfZC4,     "00000009"}
};


//Telnet or Console CommandLine ��Ӧ���������ñ�
CCvgCpMgr::CmdLine2Api CCvgCpMgr::m_CmdLine2Api[] = 
{
	//������			��д			�������ָ��					˵��
	{"quit",			"q",			&CCvgCpMgr::OnCmdLineQuit,			"quit the system"},
	{"buf",				"b",			&CCvgCpMgr::OnCmdLineBuffer,		"list the quotation buffer"},
	{"mem",				"m",			&CCvgCpMgr::OnCmdLineMem,			"show mem bytes"},
	{"evt",				"e",			&CCvgCpMgr::OnCmdLineEvtTest,		"test evt notify"},
	{"info",			"i",			&CCvgCpMgr::OnCmdLineSysInfo,		"show SysInfo"},
	{"que",			   "que",			&CCvgCpMgr::OnCmdLineQue,			"for que"},
	{"?",				"",				&CCvgCpMgr::OnCmdLineHelp,			"for help"},
	{"help",			"h",			&CCvgCpMgr::OnCmdLineHelp,			"for help"},	
    // added by Jerry Lee, 2010-12-21
    {"send",            "s",            &CCvgCpMgr::OnCmdLineSend,          "send history data"},
	
	{"settime",        "st",            &CCvgCpMgr::OnCmdLineSetTime,      "set time"},
	// added by Ben, 2011-05-29
	{"quitnode",        "qt",           &CCvgCpMgr::OnCmdLineQuitNode,      "Quit specifies node"}

};


CCvgCpMgr::CCvgCpMgr()
:m_sProcName("Convergence")
,m_pCpInterfaceCmd(0)
,m_pCpInterfaceH1(0)
,m_pCpInterfaceH2(0)
,m_pCpInterfaceZS(0)
,m_pCpInterfaceZC1(0)
,m_pCpInterfaceZC2(0)
,m_pCpInterfaceZC3(0)
,m_pCpInterfaceZC4(0)
,m_pCpInterfaceZC5(0)
,m_pCpInterfaceZC6(0)
,m_pCpInterfaceZC7(0)
,m_pCpInterfaceZC8(0)
,m_pHisDataHandler(0)
,m_pServiceHandler1(0)
,m_pServiceHandler2(0)
,m_pServiceHandler3(0)
,m_pServiceHandler4(0)
,m_pServiceHandler5(0)
,m_pServiceHandler6(0)
,m_pServiceHandler7(0)
,m_pServiceHandler8(0)
,m_pServiceHandlerSvr(0)
,m_pDeliverMgr(0)
,m_pNetMagModule(0)
,m_pSvAgent(0)
,m_pGessTimerMgr(0)
,m_uiNodeID(0)
,m_uiNodeType(0)
,m_bStop(false)
,m_cStoreType(0)
{
	m_pConfig = new CConfigImpl();

	for (int i = 0; i < EnumKeyUnknown; i++)
	{
		m_nConNumIf[i] = 0;
	}
}

CCvgCpMgr::~CCvgCpMgr(void)
{
	m_deqTelnets.clear();
	m_vResetTime.clear();
}

//�ͻ���Э�����ӵ����ӳɹ���ص�
int CCvgCpMgr::OnConnect(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	m_csConNum.Lock();
	if (0 == nFlag)
	{
		m_nConNumIf[ulKey]++;
	}
	m_csConNum.Unlock();
	return 0;
}

//�����Э�����ӵ���յ����Ӻ�ص�
int CCvgCpMgr::OnAccept(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;
	
	return 0;
}

int CCvgCpMgr::OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	return 0;
}

int CCvgCpMgr::OnClose(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;


	bool blNeedLogout = false;
	m_csConNum.Lock();
	if (m_nConNumIf[ulKey] > 0)
		m_nConNumIf[ulKey]--;

	m_csConNum.Unlock();
	return 0;
}
//��ʼ��·�ɱ�
int CCvgCpMgr::InitRouterTbl()
{
	//���ñ�
	int nSize = sizeof(m_tblIfRouterCfg)/sizeof(IfRouterCfg);
	//����·�����ñ��ʼ���ڴ�·�ɱ�
	for ( int m = 0; m < nSize; m++ )
	{
		unsigned long ulRow = m_tblIfRouterCfg[m].ulIfFrom;
		m_tblIfRouter[ulRow].ulIfFrom = m_tblIfRouterCfg[m].ulIfFrom;
		string sCmdID = m_tblIfRouterCfg[m].sCmdID;

		switch(m_tblIfRouterCfg[m].ulIfTo)
		{
		case EnumKeyDeliverMgr:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pDeliverMgr));
			break;
		case EnumKeyHisDataHandler:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pHisDataHandler));
			break;
		case EnumKeyIfZS:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZS));
			break;
		case EnumKeyServiceHandlerSvr:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandlerSvr));
			break;
		case EnumKeyServiceHandler1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler1));
			break;
		case EnumKeyServiceHandler2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler2));
			break;
		case EnumKeyServiceHandler3:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler3));
			break;
		case EnumKeyServiceHandler4:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler4));
			break;
		case EnumKeyServiceHandler5:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler5));
			break;
		case EnumKeyServiceHandler6:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler6));
			break;
		case EnumKeyServiceHandler7:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler7));
			break;
		case EnumKeyServiceHandler8:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler8));
			break;
		case EnumKeyIfZC1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC1));
			break;
		case EnumKeyIfZC2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC2));
			break;
		case EnumKeyIfZC3:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC3));
			break;
		case EnumKeyIfZC4:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC4));
			break;
		case EnumKeyIfZC5:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC5));
			break;
		case EnumKeyIfZC6:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC6));
			break;
		case EnumKeyIfZC7:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC7));
			break;
		case EnumKeyIfZC8:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC8));
			break;
		case EnumKeyIfH1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceH1));
			break;
		case EnumKeyIfH2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceH2));
			break;
		case EnumKeyIfCmd:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceCmd));
			break;
		case EnumKeyCmdHandler:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, &m_oCpCmdHandler));
			break;
		case EnumNetMagModule:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pNetMagModule));
			break;
		case EnumKeySvAgent:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pSvAgent));
			break;
		default:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, 0));
			break;
		}
	}

	return 0;
}

//Convergence���ӵ��������ʼ��
int CCvgCpMgr::Init(const string& sProcName)
{
	m_sProcName = sProcName;

	cout << "���������ļ�..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + "Convergence";//m_sProcName;
	sCfgFilename = sCfgFilename + ".cfg";
	if (m_pConfig->Load(sCfgFilename) != 0)
	{
		cout << "���������ļ�[" << sCfgFilename << "]ʧ��!" << endl;
		msleep(3);
		return -1;
	}

	cout << "��ʼ����־..." << endl;

	// ��ʼ����־
	if (CLogger::Instance()->Initial(m_pConfig->GetProperties("logger")) != 0)
	{
		cout << "Init Log [" << m_sProcName << "] failure!" << endl;
		msleep(3);
		return -1;
	}

	cout << "������־..." << endl;

	// ������־
	if (CLogger::Instance()->Start() != 0)
	{
		cout << "Log start failure!" << endl;
		msleep(3);
		return -1;
	}

	string sTmp = "";
	if (0 == m_pConfig->GetProperty("node_id",sTmp))
		m_uiNodeID = FromString<unsigned int>(sTmp);

	if (0 == m_pConfig->GetProperty("node_type",sTmp))
		m_uiNodeType = FromString<unsigned int>(sTmp);

    // added by Jerry Lee, 2010-12-21, �õ���ʷ����·��
    if (0 == m_pConfig->GetProperty("hisdata",sTmp))
        m_strHisDataPath = trim(sTmp);

    // added by Jerry Lee, 2011-1-17, �õ�tick���ݱ���·��
    if (0 == m_pConfig->GetProperty("tickdata",sTmp))
        m_strTickDataPath = trim(sTmp);
    
    // added by Jerry Lee, 2011-2-14, ��Ѷ�������ݱ���·�� 
    if (0 == m_pConfig->GetProperty("infodata",sTmp))
        m_strInfoIndexPath = trim(sTmp);

    // added by Jerry Lee, 2011-2-26, ��Ѷ�������ݱ���·�� 
    if (0 == m_pConfig->GetProperty("infocontent",sTmp))
        m_strContentPath = trim(sTmp);


	// added by Jerry Lee, 2011-2-14, ��Ѷ�������ݱ���·�� 
	if (0 == m_pConfig->GetProperty("storeType",sTmp))
		m_cStoreType = atoi(trim(sTmp).c_str());


	// added by ZYB, 2012-6-11, ��Ѷ���ݿ� ODBC
	if (0 == m_pConfig->GetProperty("infodb_ODBC",sTmp))
		m_szInfoODBC = sTmp;
	

	CRLog(E_NOTICE,"[%s]�����...",sProcName.c_str());

	//
	char szFileName[_MAX_PATH];
	::GetModuleFileName(0,szFileName, _MAX_PATH);
	sTmp = szFileName;
	sTmp = strutils::LeftSubRight(sTmp, '.');

	sTmp = strutils::SubRight(sTmp, '\\');

	m_oMemShareAlive.Bind(E_PROCESS_APP);
	if (FALSE == m_oMemShareAlive.Create(sTmp.c_str()))
	{
		CRLog(E_ERROR, "m_oMemShareAlive.Create fail");
		return -1;
	}
	unsigned int uiProcessID = ::GetCurrentProcessId();
	m_oMemShareAlive.IamAlive(uiProcessID);
	m_oMemShareAlive.SetNodeID(m_uiNodeID);

	//
	CAbsTimerMgrWin32::Instance()->Init();

	//������ʱ��������
	m_pGessTimerMgr = CGessTimerMgrImp::Instance();
	m_pGessTimerMgr->Init(2);
	
	int nInterval = 4;
	string sInterval("4");
	if (0 == m_pConfig->GetProperty("hello_interval",sInterval))
	{
		nInterval = FromString<int>(sInterval);
	}
	if (nInterval > 10)
		nInterval = 10;
	if (nInterval < 2)
		nInterval = 2;

	m_oIfkTimer.Bind(this);
	m_pGessTimerMgr->CreateTimer(&m_oIfkTimer,nInterval,"KHello");

	// added by zyb ͬ��ʱ�䶨ʱ�� Ĭ��ÿ��2������һ��ʱ��ͬ��
	int nSyncInterval = 2;
	string sSyncInterval("2");
	if (0 == m_pConfig->GetProperty("sync_interval",sSyncInterval))
	{
		nSyncInterval = FromString<int>(sSyncInterval);
	}

	m_bSync_time = false;
	string sSync_time("0");
	if (0 == m_pConfig->GetProperty("sync_time",sSync_time))
	{
		if (sSync_time == "1")
			m_bSync_time = true;
	}

	if (m_bSync_time == true)
	{ // ����ʱ��ͬ����ʱ��
		m_oSyncTimer.Bind(this);
		m_pGessTimerMgr->CreateTimer(&m_oSyncTimer,nSyncInterval, "SyncTime");
	}

	m_bAcceptSyncTime = true;
	string sAcceptSyncTime("1");
	if (0 == m_pConfig->GetProperty("accept_sync_time",sAcceptSyncTime))
	{
		if (sAcceptSyncTime == "0")
			m_bAcceptSyncTime = false;
	}
	// end add


	//��ʱ��������
	string sResetTimes = "";
	if (0 == m_pConfig->GetProperty("reset_time",sResetTimes))
	{
		bool blPara = false;
		vector<string> vWeekDayTm = explodeQuoted(",",sResetTimes);
		if (vWeekDayTm.size() == 2)
		{
			int nWeekDay = strutils::FromString<int>(vWeekDayTm[0]);
			if (nWeekDay >= 0 && nWeekDay <= 6)
			{
				CGessTime oTm;
				if (oTm.FromString(trim(vWeekDayTm[1])))
				{
					m_oResetTimer.Bind(this);
					CAbsTimerMgrWin32::Instance()->CreateWeekTimer(&m_oResetTimer,nWeekDay, oTm, "reset_timer_key");
					blPara = true;
				}
			}
		}
		else if (vWeekDayTm.size() == 1)
		{
			CGessTime oTm;
			if (oTm.FromString(trim(vWeekDayTm[0])))
			{
				m_oResetTimer.Bind(this);
				CAbsTimerMgrWin32::Instance()->CreateDayTimer(&m_oResetTimer, oTm, "reset_timer_key");
				blPara = true;
			}
		}
		
		if (!blPara)
		{
			 CRLog(E_APPINFO,"%s", "�Զ�����ʱ������ó���");
		}
	}

	CRLog(E_NOTICE,"��ʼ�����ܴ���");
	string sTblPrefix = "cvg";
	CConfig *pCfgNetMagModule = m_pConfig->GetProperties(gc_sCfgNetMagModule);
	if (0 != pCfgNetMagModule)
	{
		if (0 == pCfgNetMagModule->GetProperty("tbl_prefix",sTmp))
			sTblPrefix = sTmp;
	}
	m_pNetMagModule = new CNetMgrModule();
	CNetMgr::Instance()->NmInit(m_pNetMagModule,sTblPrefix);
	

	//��ʼ�����ܴ���ģ��
	m_pNetMagModule->Bind(this,EnumNetMagModule);
	m_pNetMagModule->Init(pCfgNetMagModule);

	CConfig *pCfgHisData;
	pCfgHisData = m_pConfig->GetProperties(gc_sCfgHisData);
	if (0 != pCfgHisData && !pCfgHisData->IsEmpty())
	{
	}
	else
	{
		pCfgHisData = m_pConfig;
	}
	CRLog(E_NOTICE,"��ʼ�����ӵ�HisDataHandler");
	m_pHisDataHandler = new CHisDataHandler();
	m_pHisDataHandler->Bind(this,EnumKeyHisDataHandler);
	m_pHisDataHandler->Init(pCfgHisData);

	CConfig *pCfgServiceHandler;
	pCfgServiceHandler = m_pConfig->GetProperties(gc_sCfgService);
	if (0 != pCfgServiceHandler && !pCfgServiceHandler->IsEmpty())
	{
	}
	else
	{
		pCfgServiceHandler = m_pConfig;
	}
	
	CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandlerSvr");
	m_pServiceHandlerSvr = new CServiceHandler();
	m_pServiceHandlerSvr->Bind(this,EnumKeyServiceHandlerSvr);
	m_pServiceHandlerSvr->Init(pCfgServiceHandler);
	
	
	CConfig *pCfgZS;
	pCfgZS = m_pConfig->GetProperties(gc_sCfgIfZS);
	if (0 != pCfgZS && !pCfgZS->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZS");
		m_pCpInterfaceZS = new CProtocolCpSvr<CProcessInterfaceZS>();
		m_pCpInterfaceZS->Bind(this,EnumKeyIfZS);
		m_pCpInterfaceZS->Init(pCfgZS);
	}


	CConfig *pCfgDeliverMgr;
	pCfgDeliverMgr = m_pConfig->GetProperties(gc_sCfgDeliver);
	if (0 != pCfgDeliverMgr && !pCfgDeliverMgr->IsEmpty())
	{
	}
	else
	{
		pCfgDeliverMgr = m_pConfig;
	}
	CRLog(E_NOTICE,"��ʼ�����ӵ�DeliverMgr");
	m_pDeliverMgr = new CDeliverMgr();
	m_pDeliverMgr->Bind(this,EnumKeyDeliverMgr);
	m_pDeliverMgr->Init(pCfgDeliverMgr);

	CConfig *pCfgZC1;
	pCfgZC1 = m_pConfig->GetProperties(gc_sCfgIfZC1);
	if (0 != pCfgZC1 && !pCfgZC1->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC1");
		m_pCpInterfaceZC1 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC1->Bind(this,EnumKeyIfZC1);
		m_pCpInterfaceZC1->Init(pCfgZC1);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler1");
		m_pServiceHandler1 = new CServiceHandler();
		m_pServiceHandler1->Bind(this,EnumKeyServiceHandler1);
		m_pServiceHandler1->Init(pCfgServiceHandler);
	}

	CConfig *pCfgZC2;
	pCfgZC2 = m_pConfig->GetProperties(gc_sCfgIfZC2);
	if (0 != pCfgZC2 && !pCfgZC2->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC2");
		m_pCpInterfaceZC2 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC2->Bind(this,EnumKeyIfZC2);
		m_pCpInterfaceZC2->Init(pCfgZC2);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler2");
		m_pServiceHandler2 = new CServiceHandler();
		m_pServiceHandler2->Bind(this,EnumKeyServiceHandler2);
		m_pServiceHandler2->Init(pCfgServiceHandler);
	}

	CConfig *pCfgZC3;
	pCfgZC3 = m_pConfig->GetProperties(gc_sCfgIfZC3);
	if (0 != pCfgZC3 && !pCfgZC3->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC3");
		m_pCpInterfaceZC3 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC3->Bind(this,EnumKeyIfZC3);
		m_pCpInterfaceZC3->Init(pCfgZC3);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler3");
		m_pServiceHandler3 = new CServiceHandler();
		m_pServiceHandler3->Bind(this,EnumKeyServiceHandler3);
		m_pServiceHandler3->Init(pCfgServiceHandler);
	}

	
	CConfig *pCfgZC4;
	pCfgZC4 = m_pConfig->GetProperties(gc_sCfgIfZC4);
	if (0 != pCfgZC4 && !pCfgZC4->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC4");
		m_pCpInterfaceZC4 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC4->Bind(this,EnumKeyIfZC4);
		m_pCpInterfaceZC4->Init(pCfgZC4);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler4");
		m_pServiceHandler4 = new CServiceHandler();
		m_pServiceHandler4->Bind(this,EnumKeyServiceHandler4);
		m_pServiceHandler4->Init(pCfgServiceHandler);
	}

	CConfig *pCfgZC5;
	pCfgZC5 = m_pConfig->GetProperties(gc_sCfgIfZC5);
	if (0 != pCfgZC5 && !pCfgZC5->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC5");
		m_pCpInterfaceZC5 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC5->Bind(this,EnumKeyIfZC5);
		m_pCpInterfaceZC5->Init(pCfgZC5);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler5");
		m_pServiceHandler5 = new CServiceHandler();
		m_pServiceHandler5->Bind(this,EnumKeyServiceHandler5);
		m_pServiceHandler5->Init(pCfgServiceHandler);
	}

	CConfig *pCfgZC6;
	pCfgZC6 = m_pConfig->GetProperties(gc_sCfgIfZC6);
	if (0 != pCfgZC6 && !pCfgZC6->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC6");
		m_pCpInterfaceZC6 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC6->Bind(this,EnumKeyIfZC6);
		m_pCpInterfaceZC6->Init(pCfgZC6);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler6");
		m_pServiceHandler6 = new CServiceHandler();
		m_pServiceHandler6->Bind(this,EnumKeyServiceHandler6);
		m_pServiceHandler6->Init(pCfgServiceHandler);
	}


	CConfig *pCfgZC7;
	pCfgZC7 = m_pConfig->GetProperties(gc_sCfgIfZC7);
	if (0 != pCfgZC7 && !pCfgZC7->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC7");
		m_pCpInterfaceZC7 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC7->Bind(this,EnumKeyIfZC7);
		m_pCpInterfaceZC7->Init(pCfgZC7);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler7");
		m_pServiceHandler7 = new CServiceHandler();
		m_pServiceHandler7->Bind(this,EnumKeyServiceHandler7);
		m_pServiceHandler7->Init(pCfgServiceHandler);
	}

	CConfig *pCfgZC8;
	pCfgZC8 = m_pConfig->GetProperties(gc_sCfgIfZC8);
	if (0 != pCfgZC8 && !pCfgZC8->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC8");
		m_pCpInterfaceZC8 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC8->Bind(this,EnumKeyIfZC8);
		m_pCpInterfaceZC8->Init(pCfgZC8);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler8");
		m_pServiceHandler8 = new CServiceHandler();
		m_pServiceHandler8->Bind(this,EnumKeyServiceHandler8);
		m_pServiceHandler8->Init(pCfgServiceHandler);
	}

	//H1�ӿں�H2�ӿ�
	CConfig *pCfgH1;
	pCfgH1 = m_pConfig->GetProperties(gc_sCfgIfH1);
	if (0 != pCfgH1 && !pCfgH1->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�H1");
		m_pCpInterfaceH1 = new CProtocolCpCli<CProcessInterfaceH1C>();
		m_pCpInterfaceH1->Bind(this,EnumKeyIfH1);
		m_pCpInterfaceH1->Init(pCfgH1);
	}
	
	CConfig *pCfgH2;
	pCfgH2 = m_pConfig->GetProperties(gc_sCfgIfH2);
	if (0 != pCfgH2 && !pCfgH2->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�H2");
		m_pCpInterfaceH2 = new CProtocolCpCli<CProcessInterfaceH2C>();
		m_pCpInterfaceH2->Bind(this,EnumKeyIfH2);
		m_pCpInterfaceH2->Init(pCfgH2);
	}
	
	CConfig *pCfgSvAgent;
	pCfgSvAgent = m_pConfig->GetProperties(gc_sCfgSvAgent);
	if (0 == pCfgSvAgent || pCfgSvAgent->IsEmpty())
	{
		pCfgSvAgent = m_pConfig;
	}
	m_pSvAgent = new CIfSvAgent();
	m_pSvAgent->Bind(this,EnumKeySvAgent);
	m_pSvAgent->Init(pCfgSvAgent);
	m_pSvAgent->SetObj(m_pNetMagModule, m_pCpInterfaceH1, m_pCpInterfaceH2);

	//��ʼ������·�ɱ�
	InitRouterTbl();
	return 0;
}

//�����ӵ�����
int CCvgCpMgr::Start()
{
	m_pGessTimerMgr->Start();
	CAbsTimerMgrWin32::Instance()->Start();

	if (0 != m_pSvAgent)
	{
		//CRLog(E_NOTICE,"����SvAgentģ��");
		m_pSvAgent->Start();
	}

	//������������ģ��
	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"�������ܴ���ģ��");
		m_pNetMagModule->Start();
	}

	if (0 != m_pDeliverMgr)
	{
		CRLog(E_NOTICE,"����DeliverMgr");
		m_pDeliverMgr->Start();
	}

	if (0 != m_pHisDataHandler)
	{
		CRLog(E_NOTICE,"����HisDataHandler");
		m_pHisDataHandler->Start();
	}

	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"����ServiceHandler1");
		m_pServiceHandler1->Start();
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"����ServiceHandler2");
		m_pServiceHandler2->Start();
	}

	if (0 != m_pServiceHandler3)
	{
		CRLog(E_NOTICE,"����ServiceHandler3");
		m_pServiceHandler3->Start();
	}

	if (0 != m_pServiceHandler4)
	{
		CRLog(E_NOTICE,"����ServiceHandler4");
		m_pServiceHandler4->Start();
	}

	if (0 != m_pServiceHandler5)
	{
		CRLog(E_NOTICE,"����ServiceHandler5");
		m_pServiceHandler5->Start();
	}

	if (0 != m_pServiceHandler6)
	{
		CRLog(E_NOTICE,"����ServiceHandler6");
		m_pServiceHandler6->Start();
	}

	if (0 != m_pServiceHandler7)
	{
		CRLog(E_NOTICE,"����ServiceHandler7");
		m_pServiceHandler7->Start();
	}

	if (0 != m_pServiceHandler8)
	{
		CRLog(E_NOTICE,"����ServiceHandler8");
		m_pServiceHandler8->Start();
	}

	if (0 != m_pServiceHandlerSvr)
	{
		CRLog(E_NOTICE,"����ServiceHandlerSvr");
		m_pServiceHandlerSvr->Start();
	}
	

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC1");
		m_pCpInterfaceZC1->Start();
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC2");
		m_pCpInterfaceZC2->Start();
	}

	if (0 != m_pCpInterfaceZC3)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC3");
		m_pCpInterfaceZC3->Start();
	}

	if (0 != m_pCpInterfaceZC4)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC4");
		m_pCpInterfaceZC4->Start();
	}

	if (0 != m_pCpInterfaceZC5)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC5");
		m_pCpInterfaceZC5->Start();
	}

	if (0 != m_pCpInterfaceZC6)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC6");
		m_pCpInterfaceZC6->Start();
	}

	if (0 != m_pCpInterfaceZC7)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC7");
		m_pCpInterfaceZC7->Start();
	}

	if (0 != m_pCpInterfaceZC8)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC8");
		m_pCpInterfaceZC8->Start();
	}

	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"�������ӵ�ZS");
		m_pCpInterfaceZS->Start();
	}

	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"�������ӵ�H1");
		m_pCpInterfaceH1->Start();
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"�������ӵ�H2");
		m_pCpInterfaceH2->Start();
	}


    // added by Jerry Lee, 2011-2-14, ������Ѷ������
    CRLog(E_NOTICE,"������Ѷ����ģ��");
	
	//���ݴ洢��ʽ��ѡ�����Ǹ��߳�
	if (m_cStoreType == 0)
	{
		m_InfoPubr.Start(m_strInfoIndexPath.c_str(), m_strContentPath.c_str(),
			m_pDeliverMgr);
	}
	else if (m_cStoreType == 1)
	{
		m_InfoPubr.Start(m_pDeliverMgr, m_szInfoODBC);
	}  

	return 0;
}

//ֹͣ�����ӵ�
void CCvgCpMgr::Stop()
{
	//ֹͣ��ʱ��������
	CRLog(E_NOTICE,"ֹͣ��ʱ��������");
	m_pGessTimerMgr->Stop();
	CAbsTimerMgrWin32::Instance()->Stop();

	if (0 != m_pDeliverMgr)
	{
		CRLog(E_NOTICE,"ֹͣDeliverMgr");
		m_pDeliverMgr->Stop();
	}

	if (0 != m_pHisDataHandler)
	{
		CRLog(E_NOTICE,"ֹͣHisDataHandler");
		m_pHisDataHandler->Stop();
	}

	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler1");
		m_pServiceHandler1->Stop();
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler2");
		m_pServiceHandler2->Stop();
	}

	if (0 != m_pServiceHandler3)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler3");
		m_pServiceHandler3->Stop();
	}

	if (0 != m_pServiceHandler4)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler4");
		m_pServiceHandler4->Stop();
	}

	if (0 != m_pServiceHandler5)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler5");
		m_pServiceHandler5->Stop();
	}

	if (0 != m_pServiceHandler6)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler6");
		m_pServiceHandler6->Stop();
	}

	if (0 != m_pServiceHandler7)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler7");
		m_pServiceHandler7->Stop();
	}

	if (0 != m_pServiceHandler8)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler8");
		m_pServiceHandler8->Stop();
	}

	if (0 != m_pServiceHandlerSvr)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandlerSvr");
		m_pServiceHandlerSvr->Stop();
	}

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC1");
		m_pCpInterfaceZC1->Stop();
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC2");
		m_pCpInterfaceZC2->Stop();
	}

	if (0 != m_pCpInterfaceZC3)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC3");
		m_pCpInterfaceZC3->Stop();
	}

	if (0 != m_pCpInterfaceZC4)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC4");
		m_pCpInterfaceZC4->Stop();
	}

	if (0 != m_pCpInterfaceZC5)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC5");
		m_pCpInterfaceZC5->Stop();
	}

	if (0 != m_pCpInterfaceZC6)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC6");
		m_pCpInterfaceZC6->Stop();
	}

	if (0 != m_pCpInterfaceZC7)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC7");
		m_pCpInterfaceZC7->Stop();
	}

	if (0 != m_pCpInterfaceZC8)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC8");
		m_pCpInterfaceZC8->Stop();
	}

	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZS");
		m_pCpInterfaceZS->Stop();
	}
	
	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�H1");
		m_pCpInterfaceH1->Stop();
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�H2");
		m_pCpInterfaceH2->Stop();
	}


	//ֹͣ��������ģ��
	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"ֹͣ���ܴ���ģ��");
		m_pNetMagModule->Stop();
	}

	if (0 != m_pSvAgent)
	{
		m_pSvAgent->Stop();
	}

    // added by Jerry Lee, 2011-2-14, ֹͣ��Ѷ������
    CRLog(E_NOTICE,"ֹͣ��Ѷ����ģ��");
    m_InfoPubr.Stop();

}

//��������
void CCvgCpMgr::Finish()
{
	m_pGessTimerMgr->Finish();
	m_pGessTimerMgr=0;
	CAbsTimerMgrWin32::Instance()->Finish();

	if (0 != m_pDeliverMgr)
	{
		CRLog(E_NOTICE,"����DeliverMgr");
		m_pDeliverMgr->Finish();
		delete m_pDeliverMgr;
		m_pDeliverMgr = 0;
	}

	if (0 != m_pHisDataHandler)
	{
		CRLog(E_NOTICE,"����HisDataHandler");
		m_pHisDataHandler->Finish();
		delete m_pHisDataHandler;
		m_pHisDataHandler = 0;
	}

	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"����ServiceHandler1");
		m_pServiceHandler1->Finish();
		delete m_pServiceHandler1;
		m_pServiceHandler1 = 0;
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"����ServiceHandler2");
		m_pServiceHandler2->Finish();
		delete m_pServiceHandler2;
		m_pServiceHandler2 = 0;
	}

	if (0 != m_pServiceHandler3)
	{
		CRLog(E_NOTICE,"����ServiceHandler3");
		m_pServiceHandler3->Finish();
		delete m_pServiceHandler3;
		m_pServiceHandler3 = 0;
	}

	if (0 != m_pServiceHandler4)
	{
		CRLog(E_NOTICE,"����ServiceHandler4");
		m_pServiceHandler4->Finish();
		delete m_pServiceHandler4;
		m_pServiceHandler4 = 0;
	}

	if (0 != m_pServiceHandler5)
	{
		CRLog(E_NOTICE,"����ServiceHandler5");
		m_pServiceHandler5->Finish();
		delete m_pServiceHandler5;
		m_pServiceHandler5 = 0;
	}

	if (0 != m_pServiceHandler6)
	{
		CRLog(E_NOTICE,"����ServiceHandler6");
		m_pServiceHandler6->Finish();
		delete m_pServiceHandler6;
		m_pServiceHandler6 = 0;
	}

	if (0 != m_pServiceHandler7)
	{
		CRLog(E_NOTICE,"����ServiceHandler7");
		m_pServiceHandler7->Finish();
		delete m_pServiceHandler7;
		m_pServiceHandler7 = 0;
	}

	if (0 != m_pServiceHandler8)
	{
		CRLog(E_NOTICE,"����ServiceHandler8");
		m_pServiceHandler8->Finish();
		delete m_pServiceHandler8;
		m_pServiceHandler8 = 0;
	}

	
	if (0 != m_pServiceHandlerSvr)
	{
		CRLog(E_NOTICE,"����ServiceHandlerSvr");
		m_pServiceHandlerSvr->Finish();
		delete m_pServiceHandlerSvr;
		m_pServiceHandlerSvr = 0;
	}

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC1");
		m_pCpInterfaceZC1->Finish();
		delete m_pCpInterfaceZC1;
		m_pCpInterfaceZC1 = 0;
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC2");
		m_pCpInterfaceZC2->Finish();
		delete m_pCpInterfaceZC2;
		m_pCpInterfaceZC2 = 0;
	}

	if (0 != m_pCpInterfaceZC3)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC3");
		m_pCpInterfaceZC3->Finish();
		delete m_pCpInterfaceZC3;
		m_pCpInterfaceZC3 = 0;
	}

	if (0 != m_pCpInterfaceZC4)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC4");
		m_pCpInterfaceZC4->Finish();
		delete m_pCpInterfaceZC4;
		m_pCpInterfaceZC4 = 0;
	}


	if (0 != m_pCpInterfaceZC5)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC5");
		m_pCpInterfaceZC5->Finish();
		delete m_pCpInterfaceZC5;
		m_pCpInterfaceZC5 = 0;
	}

	if (0 != m_pCpInterfaceZC6)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC6");
		m_pCpInterfaceZC6->Finish();
		delete m_pCpInterfaceZC6;
		m_pCpInterfaceZC6 = 0;
	}


	if (0 != m_pCpInterfaceZC7)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC7");
		m_pCpInterfaceZC7->Finish();
		delete m_pCpInterfaceZC7;
		m_pCpInterfaceZC7 = 0;
	}

	if (0 != m_pCpInterfaceZC8)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC8");
		m_pCpInterfaceZC8->Finish();
		delete m_pCpInterfaceZC8;
		m_pCpInterfaceZC8 = 0;
	}


	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"�������ӵ�ZS");
		m_pCpInterfaceZS->Finish();	
		delete m_pCpInterfaceZS;
		m_pCpInterfaceZS = 0;
	}
	
	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"�������ӵ�H1");
		m_pCpInterfaceH1->Finish();
		delete m_pCpInterfaceH1;
		m_pCpInterfaceH1 = 0;
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"�������ӵ�H2");
		m_pCpInterfaceH2->Finish();	
		delete m_pCpInterfaceH2;
		m_pCpInterfaceH2 = 0;
	}
	

	//
	m_oCpCmdHandler.Finish();

	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"�������ܴ���ģ��");
		m_pNetMagModule->Finish();
		delete m_pNetMagModule;
		m_pNetMagModule=0;
	}

	if (0 != m_pSvAgent)
	{
		m_pSvAgent->Finish();
		delete m_pSvAgent;
		m_pSvAgent = 0;
	}
	
	CLogger::Instance()->Finish();
	delete m_pConfig;

	CNetMgr::Instance()->NmFinish();
	
	//
	m_oMemShareAlive.UnMap();
}

//�����߳�
int CCvgCpMgr::StartMe()
{
	//CRLog(E_NOTICE,"������ʱ������ⶨʱ��");
	//m_oResetTimer.Bind(this);
	//m_pGessTimerMgr->CreateTimer(&m_oResetTimer,4,"reset_timer");

	//telnet ���Ĵ������ӵ��
	m_oCpCmdHandler.Bind(this,EnumKeyCmdHandler);
	//������־�ص������
	m_oNetLogHost.Bind(this);
	//������־�����̰߳�
	m_oNetLogThread.Bind(this);
	//�����д����̰߳�
	m_oCmdLineThread.Bind(this);


	//
	m_oCpCmdHandler.Start();

	//telnet ���������ӵ��ʼ������
	CConfig *pCfgCmd = m_pConfig->GetProperties(gc_sCfgIfCmd);
	if (0 != pCfgCmd && !pCfgCmd->IsEmpty())
	{
		m_pCpInterfaceCmd = new CProtocolCpSvr<CProcessInterfaceCmd>();
		CRLog(E_NOTICE,"��ʼ�����ӵ�Telnet Cmd");
		m_pCpInterfaceCmd->Init(pCfgCmd);
		m_pCpInterfaceCmd->Bind(this,EnumKeyIfCmd);

		CRLog(E_NOTICE,"�������ӵ�Telnet Cmd");
		m_pCpInterfaceCmd->Start();
	}

	string sCmdFlag("0");
	int nCmdFlag = 0;
	if (0 == m_pConfig->GetProperty("cmdline",sCmdFlag))
	{
		nCmdFlag = FromString<int>(sCmdFlag);
	}
	if (1 == nCmdFlag)
	{
		//���������д����߳�
		m_oCmdLineThread.BeginThread();
	}

	//����������־�����߳�
	m_oNetLogThread.BeginThread();
	return 0;
}

//ֹͣ�߳�
void CCvgCpMgr::StopMe()
{
	//CRLog(E_NOTICE,"ֹͣ��ʱ������ⶨʱ��");
	//m_pGessTimerMgr->DestroyTimer(&m_oResetTimer,"reset_timer");

	m_oNetLogThread.EndThread();
	//m_oCmdLineThread.EndThread();

	//
	m_oCpCmdHandler.Stop();
	m_oCpCmdHandler.Finish();

	if (0 != m_pCpInterfaceCmd)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�Telnet Cmd");
		m_pCpInterfaceCmd->Stop();

		CRLog(E_NOTICE,"�������ӵ�Telnet Cmd");
		m_pCpInterfaceCmd->Finish();
		delete m_pCpInterfaceCmd;
		m_pCpInterfaceCmd = 0;
	}
}

int CCvgCpMgr::HandleCmdSpecial(const string& sCmdID, CPacket &pkt)
{
	int nRtn = -1;
	try
	{
		// added by Ben 2011-05-29 �����˳���������
		if (strutils::ToHexString<unsigned int>(YL_QUITMSG) == sCmdID)
		{
			CSamplerPacket &sPkt = dynamic_cast<CSamplerPacket &>( pkt);
			unsigned int uiNodeidDest = 0;
			CMessageImpl& msg = dynamic_cast<CMessageImpl&>(sPkt.GetMsg());
			msg.GetField(MSG_BODY_NODEID, uiNodeidDest);

			if (uiNodeidDest == m_uiNodeID)
			{
				m_bStop = true;
				m_deqCondMutex.Signal();
			}
			else
			{
				map<unsigned int,SUB_CONTEXT> mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();
				if( !mapSubscri.empty())
				{
					for(map<unsigned int,SUB_CONTEXT>::iterator it = mapSubscri.begin();it != mapSubscri.end();++it)
					{

						msg.SetField(MSG_NODE_ID,it->second.nodeId);
						if (m_pCpInterfaceZS != 0)
							m_pCpInterfaceZS->SendPacket(pkt);
					}
				}
			}
			nRtn = 0;
		}
		else if (strutils::ToHexString<unsigned int>(YL_SYNC_TIME) == sCmdID)
		{
			CSamplerPacket &sPkt = dynamic_cast<CSamplerPacket &>( pkt);
			unsigned int uiNodeidDest = 0;
			unsigned int uiNodeid = 0;
			CMessageImpl& msg = dynamic_cast<CMessageImpl&>(sPkt.GetMsg());
			msg.GetField(MSG_NODE_ID, uiNodeid);
			msg.GetField(MSG_BODY_NODEID, uiNodeidDest);

			unsigned int uiDate = 0;
			unsigned int uiTime = 0;
			msg.GetField(MSG_DATE, uiDate);
			msg.GetField(MSG_TIME, uiTime);

			if (uiNodeid == m_uiNodeID)
			{
				if (uiNodeidDest == m_uiNodeID)
				{
					if (m_bAcceptSyncTime)
					{
						SYSTEMTIME st;
						st.wYear = uiDate / 100000;
						st.wMonth = (uiDate % 100000) / 1000;
						st.wDay = (uiDate % 1000) / 10;
						st.wDayOfWeek = uiDate % 10;

						st.wHour = uiTime / 10000000;
						st.wMinute = (uiTime % 10000000) / 100000;
						st.wSecond = (uiTime % 100000) / 1000;
						st.wMilliseconds = uiTime % 1000;

						SYSTEMTIME stLocal;
						GetLocalTime(&stLocal);

						string sDate = ToString<unsigned int>(uiDate);
						string sTime = ToString<unsigned int>(uiTime);

						if (st.wYear != stLocal.wYear || st.wMonth != stLocal.wMonth ||
							st.wDay != stLocal.wDay ||	st.wDayOfWeek != stLocal.wDayOfWeek ||
							st.wHour != stLocal.wHour || st.wMinute != stLocal.wMinute ||
							abs(st.wSecond - stLocal.wSecond) > 2 )
						{
							string sEvtContent = "����ʱ��ͬ���� ";

							sEvtContent = sEvtContent + sDate + " ";
							sEvtContent = sEvtContent + sTime;
							CRLog(E_NOTICE,sEvtContent.c_str());
							NotifyEvent(sEvtContent);
							::SetLocalTime(&st);

						}
						else
						{
							string sEvtContent = "����ʱ��ͬ���� ";

							sEvtContent = sEvtContent + sDate + " ";
							sEvtContent = sEvtContent + sTime;
							CRLog(E_NOTICE,sEvtContent.c_str());
							NotifyEvent(sEvtContent);
						}
					}
				}

				if (m_bSync_time == false) // �����������¶�ʱ������Բ������·�ʱ��
				{
					// modified by zyb 20120110  ���������Ӷ�ʱ
					map<unsigned int,SUB_CONTEXT> mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();
					if( !mapSubscri.empty())
					{
						for(map<unsigned int,SUB_CONTEXT>::iterator it = mapSubscri.begin();it != mapSubscri.end();++it)
						{
							// added by zyb ���˽��еĻ�۽�����
							//if (it->second.nodeId == 100300 || it->second.nodeId == 100301 || it->second.nodeId == 100310)
							//	continue;
							// end addd

							CSamplerPacket oPacketTime(YL_SYNC_TIME);
							CMessage &  syncmsg = oPacketTime.GetMsg();
							syncmsg.SetField(MSG_SEQ_ID, static_cast<unsigned int>(0));
							syncmsg.SetField(MSG_NODE_ID, it->second.nodeId);
							syncmsg.SetField(MSG_BODY_NODEID, it->second.nodeId);
							syncmsg.SetField(MSG_DATE, uiDate);
							syncmsg.SetField(MSG_TIME, uiTime);

							if (m_pCpInterfaceZS != 0)
							{
								m_pCpInterfaceZS->SendPacket(oPacketTime);

								string sEvtContent = "����ͬ��ʱ�䣺Translate-settime ";
								sEvtContent = sEvtContent + ToString<unsigned int>(it->second.nodeId) + " ";
								sEvtContent = sEvtContent + ToString<unsigned int>(it->second.nodeId);
								//CRLog(E_NOTICE,sEvtContent.c_str());
								//NotifyEvent(sEvtContent);
							}
						}
					}
				}
			}

			nRtn = 0;
		}
		// end add
		return nRtn;
	}
	catch(...)
	{
		return -1;
	}
}

//����ת������ ����ֵ-2��ʾ��·��
int CCvgCpMgr::Forward(CPacket &pkt,const unsigned long& ulKey)
{
	std::string sCmdID = "";
	try
	{	
		int nRtn = -2;
		assert(EnumKeyUnknown > ulKey);
		if (EnumKeyUnknown <= ulKey)
			return -1;

		if (m_bStop)
			return 0;

		sCmdID = pkt.GetCmdID();

		//���������
		if (0 == HandleCmdSpecial(sCmdID, pkt))
			return 0;
		
		bool blFound = false;
		MMAP_IT it;
		RANGE_CP range = m_tblIfRouter[ulKey].mmapCmds.equal_range(sCmdID);
		for (it = range.first; it != range.second; ++it)
		{
			if (0 != (*it).second)
			{
				nRtn = (*it).second->SendPacket(pkt);
			}
			blFound = true;
		}

		//kenny  2013-10-2  ������Ѷ
		/*if (!blFound)
		{
			CRLog(E_ERROR,"��ʼ��ӡ·��:From KEY:%u,ID:%s,·�ɸ���Ϊ:%d!", ulKey, sCmdID.c_str(),m_tblIfRouter[ulKey].mmapCmds.size());
			MMAP_IT it = m_tblIfRouter[ulKey].mmapCmds.begin();
			while (it != m_tblIfRouter[ulKey].mmapCmds.end())
			{
					CRLog(E_ERROR,"·���б�:%s!", (*it).first.c_str());
					it++;
			}
			CRLog(E_ERROR,"������ӡ·��:From KEY:%u,ID:%s!", ulKey, sCmdID.c_str());
		}*/
		

		if (!blFound)
		{
			it = m_tblIfRouter[ulKey].mmapCmds.find(gc_sDefaultCmdID);
			if (it != m_tblIfRouter[ulKey].mmapCmds.end())
			{
				if (0 != (*it).second)
				{
					nRtn = (*it).second->SendPacket(pkt);
				}
			}
		}
		return nRtn;
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"from %u,%s packet error!", ulKey, sCmdID.c_str());
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"from %u,%s exception:%s", ulKey, sCmdID.c_str(), e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"from %u,%s Unknown exception", ulKey, sCmdID.c_str());
		return -1;
	}
}

//�������̺߳��� �����߳��˳�������˳�
int CCvgCpMgr::Run()
{
	try
	{
		while ( !m_bStop )
		{
			m_deqCondMutex.Lock();
			while(!m_bStop)
				m_deqCondMutex.Wait();
			
			if (m_bStop)
			{
				m_deqCondMutex.Unlock();
				break;
			}		

		}

		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}

//CmdLine����ƥ�䴦��
string CCvgCpMgr::OnCmd(const string& sCmdLine, const vector<string>& vecPara)
{
	try
	{
		std::string sCmdID = trim(sCmdLine);
	
		int nSize = sizeof(m_CmdLine2Api)/sizeof(CmdLine2Api);
		for ( int i = 0 ; i < nSize ; i++ )
		{
			if ( m_CmdLine2Api[i].sCmdName == sCmdID || (sCmdID != "" && m_CmdLine2Api[i].sCmdAbbr == sCmdID) )
			{
				if (m_CmdLine2Api[i].pMemberFunc == 0)
					break;

				return (this->*(m_CmdLine2Api[i].pMemberFunc))(sCmdLine, vecPara);				
			}
		}
		
		string sRtn("");
		if (sCmdID != "")
		{
			sRtn += "err command!\r\n";
		}
		sRtn += "Cvg->";
		return sRtn;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		string sRtn = "\r\nCvg->";
		return sRtn;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		string sRtn = "\r\nCvg->";
		return sRtn;
	}
}

//telnet�ն������
int CCvgCpMgr::OnPacketCmd(CPacket& pkt)
{
	try
	{
		CPacketLineReq & pktLine = dynamic_cast<CPacketLineReq&>(pkt);
		string sRouteKey = pktLine.RouteKey();
		string sCmd = trim(pktLine.GetCmdID());

		vector<string> vecPara;
		vecPara.clear();
		pktLine.GetPara(vecPara);
		
		string sRsp("");
		if ("q" == sCmd || "quit" == sCmd)
		{
			sRsp += "Cvg->";
		}
		else if (sCmd == "debug")
		{
			string sPara("");
			if (vecPara.size() > 0)
				sPara = vecPara[0];
			
			if (trim(sPara) == "on")
			{
				m_csTelnets.Lock();
				m_deqTelnets[sRouteKey] = sRouteKey;
				if (m_deqTelnets.size() == 1)
					CLogger::Instance()->RegisterNetLog(&m_oNetLogHost);
				m_csTelnets.Unlock();

				sRsp += "Cvg->";
			}
			else if (trim(sPara) == "off")
			{
				map<string,string>::iterator itTel;	
				m_csTelnets.Lock();
				itTel = m_deqTelnets.find(sRouteKey);
				if (itTel != m_deqTelnets.end())
					m_deqTelnets.erase( itTel);

				if (m_deqTelnets.size() == 0)
					CLogger::Instance()->UnRegisterNetLog(&m_oNetLogHost);
				m_csTelnets.Unlock();

				sRsp += "Cvg->";
			}
			else
			{
				sRsp = "Parameter err!";
				sRsp += "\r\n";
				sRsp += "Cvg->";
			}
		}
		else
		{
			sRsp = OnCmd(sCmd, vecPara);
		}

		CPacketLineRsp pktRsp(sRsp);
		pktRsp.AddRouteKey(sRouteKey);
		return m_pCpInterfaceCmd->SendPacket(pktRsp);
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"packet error!");
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"Unknown exception!");
		return -1;
	}
}

//��־��Ϣ���̶߳���
void CCvgCpMgr::OnNetLogMsg(const string& sMsg)
{
	m_oNetLogThread.Enque(sMsg);
}

//��־����
int CCvgCpMgr::HandleNetLogMsg(const string & sNetLogMsg)
{
	string sMsg = sNetLogMsg;
	sMsg += "\r\n";
	map<string,string>::iterator itTel;	
	m_csTelnets.Lock();
	for (itTel = m_deqTelnets.begin(); itTel != m_deqTelnets.end(); ++itTel)
	{
		CPacketLineRsp pktRsp(sMsg);
		pktRsp.AddRouteKey((*itTel).first);
		if (0 != m_pCpInterfaceCmd->SendPacket(pktRsp))
		{
			string sKey = (*itTel).first;
			m_deqTelnets.erase(itTel);
			if (0 == m_deqTelnets.size())
			{
				cout << "UnRegisterNetLog(" << sKey << ")!" << endl;
				CLogger::Instance()->UnRegisterNetLog(&m_oNetLogHost);
			}

			break;
		}
	}
	m_csTelnets.Unlock();

	return 0;
}

//�������̴߳����� ������
int CCvgCpMgr::HandleCmdLine(string& sIn)
{
	try
	{
		char cIn = 0;
		cin.get(cIn);
		if (cIn == '\n')
		{				
			//CRLog(E_CRITICAL,"exceptionOnCmd:%s", "startCmd1");
			string sCmd("");

			sIn = trim(sIn);
			vector<string> vPara;
			vPara = explodeQuoted(" ", sIn);

			if (vPara.size() > 1)
			{
				sCmd = vPara[0];
				vPara.erase(vPara.begin());
			}
			else
			{
				sCmd = sIn;
				vPara.clear();
			}


			string sOut = OnCmd(sCmd, vPara);
			cout << sOut.c_str();
			sIn.clear();
		}
		else
		{
			sIn.append(1,cIn);
		}

		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}

//K�ӿ�������ʱ���ص��ӿ�
int CCvgCpMgr::OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan)
{
	m_oMemShareAlive.IamAlive();
	if (m_oMemShareAlive.IsIQuitCmd())
	{
		//�¼�֪ͨ
		string sEvtContent = "��ʱʱ�䵽,�Ժ�ʼ����!";

		CRLog(E_NOTICE,sEvtContent.c_str());
		NotifyEvent(sEvtContent);

		m_bStop = true;
		m_deqCondMutex.Signal();
		return -1;
	}
	return 0;
}


//��ʱ������ʱ���ص��ӿ�
int CCvgCpMgr::OnResetTimeout(const string& sTmKey)
{
	//�¼�֪ͨ
	string sEvtContent = "��ʱʱ�䵽,�Ժ�ʼ����!";

	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	m_bStop = true;
	m_deqCondMutex.Signal();
	return 0;
}

//ʱ��ͬ����ʱ���ص��ӿ�
int CCvgCpMgr::OnSyncTime(const string& sTmKey,unsigned long& ulTmSpan)
{
	string sCmd = "settime";
	vector<string> vecPara;
	vecPara.push_back("");
	vecPara.push_back("*");
	

	auto & mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();
	if( !mapSubscri.empty())
	{
		for(auto it = mapSubscri.begin();it != mapSubscri.end();++it)
		{
			vecPara[0] = ToString<unsigned int>(it->second.nodeId);
			vecPara[1] = ToString<unsigned int>(it->second.nodeId);
			OnCmdLineSetTime(sCmd, vecPara);

			//string sEvtContent = "��ʱͬ��ʱ�䣺settime ";
			//sEvtContent = sEvtContent + vecPara[0] + " ";
			//sEvtContent = sEvtContent + vecPara[1];
			//CRLog(E_NOTICE,sEvtContent.c_str());
			//NotifyEvent(sEvtContent);
		}
	}
	return 0;
}


//������Ϣ
string CCvgCpMgr::OnCmdLineHelp(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn("");
	int nSize = sizeof(m_CmdLine2Api)/sizeof(CmdLine2Api);
	for ( int i = 0 ; i < (nSize - 2); i++ ) // modiedified by Ben 2011-05-29 ��ֹquitnode�������ʾ
	{
		sRtn += m_CmdLine2Api[i].sCmdName;
		if (m_CmdLine2Api[i].sCmdAbbr != "")
		{
			sRtn += "/";
			sRtn += m_CmdLine2Api[i].sCmdAbbr;
		}
		sRtn += ":";
		sRtn += m_CmdLine2Api[i].sHelp;
		sRtn += "\r\n";
	}
	sRtn += "Cvg->";

	return sRtn;
}

//quit�����
string CCvgCpMgr::OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara)
{
	//�¼�֪ͨ
	string sEvtContent = "�����з���Quit�˳�ָ��,��Լ3����˳�!";
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);
		
	CancelOrder();
	msleep(3);

	m_bStop = true;
	m_deqCondMutex.Signal();

	string sRtn = "Cvg->";
	return sRtn;
}

//����������ʾ
string CCvgCpMgr::OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn;
	string sInstID;

	if (vecPara.size() >  0)
	{
		sInstID = vecPara[0];
	}

	map<string,QUOTATION> mapQuotation;
	CMemData::Instance()->GetQuotationTbl().GetRecs(mapQuotation);
	if (sInstID.empty())
	{
		for (auto it = mapQuotation.begin(); it != mapQuotation.end(); ++it)
		{
			sRtn += ToHexString<unsigned int>((*it).second.m_CodeInfo.m_usMarketType);
			sRtn += "\t";
			sRtn += (*it).second.m_CodeInfo.m_acCode;
			sRtn += "\t";
			sRtn += (*it).second.m_CodeInfo.m_acName;
			sRtn += "\t";
			sRtn += ToString<unsigned int>((*it).second.m_CodeInfo.m_uiUnit);
			sRtn += "\r\n";
		}
		sRtn += "Total ";
		sRtn += ToString<unsigned int>(mapQuotation.size());
		sRtn += "\r\n";
	}
	else
	{
		for (auto it = mapQuotation.begin(); it != mapQuotation.end(); ++it)
		{
			string sTmpID = (*it).second.m_CodeInfo.m_acCode;
			if (sTmpID == sInstID)
			{
				sRtn += ToHexString<unsigned int>((*it).second.m_CodeInfo.m_usMarketType);
				sRtn += "\t";
				sRtn += (*it).second.m_CodeInfo.m_acCode;
				sRtn += "\t";
				sRtn += (*it).second.m_CodeInfo.m_acName;
				sRtn += "\t";
				sRtn += ToString<unsigned int>((*it).second.m_CodeInfo.m_uiUnit);
				sRtn += "\r\n";
				
				sRtn += "m_uiSeqNo:";        
				sRtn += ToString<unsigned int>((*it).second.m_uiSeqNo);
				sRtn += "\r\n";
				sRtn += "m_uiDate:";            
				sRtn += ToString<unsigned int>((*it).second.m_uiDate);
				sRtn += "\r\n";
				sRtn += "m_uiTime:";            
				sRtn += ToString<unsigned int>((*it).second.m_uiTime);
				sRtn += "\r\n";
				sRtn += "m_uilastClose:";    
				sRtn += ToString<unsigned int>((*it).second.m_uilastClose);
				sRtn += "\r\n";
				sRtn += "m_uiLastSettle:"; 
				sRtn += ToString<unsigned int>((*it).second.m_uiLastSettle);
				sRtn += "\r\n";
				sRtn += "m_uiSettle:";   
				sRtn += ToString<unsigned int>((*it).second.m_uiSettle);   
				sRtn += "\r\n";
				sRtn += "m_uiOpenPrice:";    
				sRtn += ToString<unsigned int>((*it).second.m_uiOpenPrice);
				sRtn += "\r\n";
				sRtn += "m_uiHigh:";            
				sRtn += ToString<unsigned int>((*it).second.m_uiHigh);
				sRtn += "\r\n";
				sRtn += "m_uiLow:";             
				sRtn += ToString<unsigned int>((*it).second.m_uiLow);
				sRtn += "\r\n";
				sRtn += "m_uiClose:";           
				sRtn += ToString<unsigned int>((*it).second.m_uiClose);
				sRtn += "\r\n";
				sRtn += "m_uiHighLimit:";       
				sRtn += ToString<unsigned int>((*it).second.m_uiHighLimit);
				sRtn += "\r\n";
				sRtn += "m_uiLowLimit:";	       
				sRtn += ToString<unsigned int>((*it).second.m_uiLowLimit);
				sRtn += "\r\n";
				sRtn += "m_uiLast:";            
				sRtn += ToString<unsigned int>((*it).second.m_uiLast);
				sRtn += "\r\n";
				sRtn += "m_uiAverage:";        
				sRtn += ToString<unsigned int>((*it).second.m_uiAverage);
				sRtn += "\r\n";
				sRtn += "m_uiVolume:";          
				sRtn += ToString<unsigned int>((*it).second.m_uiVolume);
				sRtn += "\r\n";
				sRtn += "m_uiTurnOver:";        
				sRtn += ToString<unsigned int>((*it).second.m_uiTurnOver);
				sRtn += "\r\n";
				sRtn += "m_uiChiCangLiang:";    
				sRtn += ToString<unsigned int>((*it).second.m_uiChiCangLiang);
				sRtn += "\r\n";
				sRtn += "m_uiLastChiCangLiang:";
				sRtn += ToString<unsigned int>((*it).second.m_uiLastChiCangLiang);
				for (int nIndex = 0; nIndex < 5; nIndex++)
				{
					sRtn += "\r\n";
					sRtn += "Bid" + ToString<int>(nIndex+1) + ":";
					sRtn += ToString<unsigned int>((*it).second.m_Bid[nIndex].m_uiPrice);
					sRtn += "\t";
					sRtn += "Bidlot" + ToString<int>(nIndex+1) + ":";
					sRtn += ToString<unsigned int>((*it).second.m_Bid[nIndex].m_uiVol);

					sRtn += "\r\n";
					sRtn += "Ask" + ToString<int>(nIndex+1) + ":";
					sRtn += ToString<unsigned int>((*it).second.m_Ask[nIndex].m_uiPrice);
					sRtn += "\t";
					sRtn += "Asklot" + ToString<int>(nIndex+1) + ":";
					sRtn += ToString<unsigned int>((*it).second.m_Ask[nIndex].m_uiVol);
				}
				sRtn += "\r\n";
				sRtn += "\r\n";
			}
		}
	}
	return sRtn;
}

string CCvgCpMgr::OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn = CSelectorIo::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CSelectorListen::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CGessTimerMgrImp::Instance()->ToString();
	sRtn += "Cvg->";
	return sRtn;
}

//��ʾ�ڴ�
string CCvgCpMgr::OnCmdLineMem(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Cvg->\r\n";

	bool blFalg = true;
	unsigned long ulMemAddr = 0x00;
	unsigned long ulLen = 16;
	std::stringstream ss1;
	if (vecPara.size() == 2)
	{
		ss1 << hex << vecPara[0];
		ss1 >> ulMemAddr;
		ss1 << hex << vecPara[1];
		ss1 >> ulLen;
		if (ulLen > 1024)
			ulLen = 1024;
	}
	else if (vecPara.size() == 1)
	{
		ss1 << vecPara[0];
		ss1 >> ulMemAddr;
	}
	else
	{
		blFalg = false;
	}

	
	if (blFalg)
	{
		try
		{
			std::stringstream ss2;
			const unsigned char * pMemAddr = reinterpret_cast<const unsigned char*>(ulMemAddr);
			for (unsigned long ulIndex = 0; ulIndex < ulLen; ulIndex++,pMemAddr++)
			{
				unsigned int uiVal = static_cast<unsigned int>(*pMemAddr);			
				if (ulIndex != 0 && ulIndex % 4 == 0)
				{
					ss2 << " ";
				}
				ss2 << setfill('0') << setw(2) << hex << uppercase << uiVal;
			}
			sRtn = ss2.str();
		}
		catch(...)
		{
			CRLog(E_ERROR,"��ȡ��ַ������Χ");
		}
	}

	return sRtn;
}

// K�ӿ�[onRecvQuit] ҵ����Ӧ�����
int CCvgCpMgr::OnRecvQuit(CIpcPacket& pkt)
{
	//�¼�֪ͨ
	string sEvtContent = "Convergence���յ��˳�ָ��,��Լ3����˳�!";
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);
	//msleep(3);

	CancelOrder();
	msleep(1);

	m_bStop = true;
	m_deqCondMutex.Signal();

	return 0;
}

int CCvgCpMgr::HandleConsolMsg(unsigned int uiMsg)
{
	
	//�¼�֪ͨ
	string sEvtContent = "";
	switch (uiMsg)
	{
	case CTRL_CLOSE_EVENT:
		sEvtContent = "Convergence����̨���ڱ�ǿ�ƹر�,�����˳�Ӧ��!";
		break;
	case CTRL_SHUTDOWN_EVENT:
		sEvtContent = "Convergence������ػ�,�����˳�Ӧ��!";
		break;
	default:
		return 0;
	}
	
	CancelOrder();

	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	m_bStop = true;
	m_deqCondMutex.Signal();
	return 0;
}

void CCvgCpMgr::NotifyEvent(const string& sEvt)
{
	//�¼�֪ͨ
	CEventSimple e;
	e.m_nEvtCategory = 0;
	e.m_nEvtType = 0;
	e.m_nGrade = 1;
	e.m_sDateTime = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");		
	e.m_sEvtContent = sEvt;
	CNetMgr::Instance()->OnEvtNotify(e);
}

string CCvgCpMgr::OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Cvg->";
		
	//�¼�֪ͨ
	if (vecPara.size() == 0)
	{
		string sEvtContent = "�¼�����!";
		NotifyEvent(sEvtContent);
		return sRtn;
	}

	string sType = "1";
	string sNodeID = "1308";
	if (vecPara.size() >= 1)
	{
		sNodeID = vecPara[0];
	}
	if (vecPara.size() >= 2)
	{
		sType = vecPara[1];
	}

	CTradePacket oPkt;
	HEADER_REQ stHeaderReq;
	QueryItemReq stBodyReq;
	memset(&stHeaderReq, 0x00, sizeof(stHeaderReq));
	strcpy(stHeaderReq.seq_no, "12345678");
	strcpy(stHeaderReq.msg_type, "1");
	strcpy(stHeaderReq.msg_flag, "1");
	strcpy(stHeaderReq.term_type, "13");
	strcpy(stHeaderReq.user_type, "1");
	strcpy(stHeaderReq.user_id, "admin");
	
	strcpy(stHeaderReq.exch_code, "1922");
	if (sType == "1")
	{
		strcpy(stHeaderReq.exch_code, "1922");
	}
	else if (sType == "2")
	{
		strcpy(stHeaderReq.exch_code, "1923");
	}
	else if (sType == "3")
	{
		strcpy(stHeaderReq.exch_code, "1924");
	}
	oPkt.SetHeader(stHeaderReq);

	stBodyReq.host_id  = "13";
	stBodyReq.oper_flag = 1;
	stBodyReq.node_id = sNodeID;

	CPacketStructTradeNm::Struct2Packet(stBodyReq, oPkt);
	Forward(oPkt, EnumKeyIfH1);
	return sRtn;
}

//���г���
string CCvgCpMgr::OnCmdLineQue(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Cvg->";
	
	return sRtn;
}

int CCvgCpMgr::CancelOrder()
{
	CMessageImpl oMsgUnsub;
	
	unsigned int uiVal = 0xFFFFFFFF;
	string sItems;
	sItems.assign((char*)(&uiVal),sizeof(unsigned int));
	oMsgUnsub.SetBinaryField(MSG_SUBSCRIP_RECS,sItems);

	CSamplerPacket oPktUnsub(oMsgUnsub,YL_UNSUBSCRIP);
	if (0 != m_pCpInterfaceZC1)
	{
		m_pCpInterfaceZC1->SendPacket(oPktUnsub);
	}

	if (0 != m_pCpInterfaceZC2)
	{
		m_pCpInterfaceZC2->SendPacket(oPktUnsub);		
	}

	return 0;
}

/*
���������ʽ��send [nodeid] [markettype] [code] [begin time] [end time] [cc] [cc] ... [cc]
nodeid: ��󳤶�Ϊ�����ַ�������һ�����������
markettype: ���ȹ̶�Ϊ�����ַ�����ʾ�г����ͣ�������ο�MARKETINFOS��marketAdj�Ķ��壬
code: ��󳤶�Ϊ�����ַ�������Ʒ�ִ���, 
begin time: yyyymmddhhnn, yyyy��ʾ���, mm��ʾ�·�, dd��ʾ��, hh��ʾСʱ��nn��ʾ���� 
end time: yyyymmddhhnn����ʽ������ͬ
cc: ���ַ������ƣ���ʾ�����ߡ����ߵ����ͣ������ж������Ϳ���ѡ��

��ʱֻ֧��ÿ�η���һ��Ʒ�ֵ�����
*/
string CCvgCpMgr::OnCmdLineSend(const string& sCmd, const vector<string>& vecPara)
{
    int n = 0;

    string ret = "send";

    for (vector<string>::const_iterator it = vecPara.begin(); it != vecPara.end(); it++)
    {
        ret += " ";
        ret += *it;
    }

    CSendCommand cmd(m_pDeliverMgr, m_strHisDataPath, m_strTickDataPath,
        m_strInfoIndexPath, m_strContentPath);

    cmd.Execute(vecPara);

    return ret;
}




//����������ʾ
string CCvgCpMgr::OnCmdLineQuitNode(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn;
	string sNodeIDHead;
	string sNodeIDDestination;

	if (vecPara.size() <  2)
	{
		sRtn = "��������ȷ����������յĽڵ�ź��˳��Ľڵ�š�";
		return sRtn;
	}


	sNodeIDHead = vecPara[0];
	sNodeIDDestination = vecPara[1];

	CSamplerPacket oPacketQuit(YL_QUITMSG);
	CMessage &  msg = oPacketQuit.GetMsg();

	unsigned int uiNodeID = FromString<unsigned int>(sNodeIDHead);
	unsigned int uiNodeIDDest = FromString<unsigned int>(sNodeIDDestination);


	msg.SetField(MSG_SEQ_ID, static_cast<unsigned int>(0));
	msg.SetField(MSG_NODE_ID, uiNodeID);
	msg.SetField(MSG_BODY_NODEID, uiNodeIDDest);
	if (0 != m_pCpInterfaceZS)
	{
		m_pCpInterfaceZS->SendPacket(oPacketQuit);
	}

	sRtn = "Send Quit msg OK!";
	return sRtn;
}

string CCvgCpMgr::OnCmdLineSetTime(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn;
	string sNodeIDHead;
	string sNodeIDDestination;
	if (vecPara.size() <  2)
	{
		sRtn = "��������ȷ����������յĽڵ�ź�Ŀ�Ľڵ�š�";
		return sRtn;
	}

	sNodeIDHead = vecPara[0];
	sNodeIDDestination = vecPara[1];
	unsigned int uiNodeID = FromString<unsigned int>(sNodeIDHead);
	unsigned int uiNodeIDDest = FromString<unsigned int>(sNodeIDDestination);

	SYSTEMTIME st;
	::GetLocalTime(&st);
	unsigned int uiDate = st.wYear * 100000 + st.wMonth * 1000 + st.wDay * 10 + st.wDayOfWeek;
	unsigned int uiTime = st.wHour * 10000000 + st.wMinute * 100000 + st.wSecond * 1000 + st.wMilliseconds;
	
	CSamplerPacket oPacketTime(YL_SYNC_TIME);
	CMessage &  msg = oPacketTime.GetMsg();
	msg.SetField(MSG_SEQ_ID, static_cast<unsigned int>(0));
	msg.SetField(MSG_NODE_ID, uiNodeID);
	msg.SetField(MSG_BODY_NODEID, uiNodeIDDest);
	msg.SetField(MSG_DATE, uiDate);
	msg.SetField(MSG_TIME, uiTime);
	if (0 != m_pCpInterfaceZS)
	{
		m_pCpInterfaceZS->SendPacket(oPacketTime);
	}
	return sRtn;
}
