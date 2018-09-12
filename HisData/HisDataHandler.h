#ifndef _HIS_DATA_HANDLER_H
#define _HIS_DATA_HANDLER_H

#include <fstream>
#include <iostream>
#include "GessDate.h"
#include "Workthread.h"
#include "BroadcastPacket.h"
#include "SamplerPacket.h"

/*
市场类别定义：
各位含义表示如下：
15		   12		8					0
|			|	  	  |					|
| 金融分类	|市场分类 |	交易品种分类	|
*/
typedef short HSMarketDataType;			  // 市场分类数据类型
/*金融大类*/
#define STOCK_MARKET			 0X1000   // 股票
#	define SH_BOURSE			 0x0100   // 上海
#	define SZ_BOURSE			 0x0200   // 深圳
#	define SYSBK_BOURSE			 0x0400   // 系统板块
#	define USERDEF_BOURSE		 0x0800   // 自定义（自选股或者自定义板块）
#			define KIND_INDEX		 0x0000   // 指数 
#			define KIND_STOCKA		 0x0001   // A股 
#			define KIND_STOCKB		 0x0002   // B股 
#			define KIND_BOND		 0x0003   // 债券
#			define KIND_FUND		 0x0004   // 基金
#			define KIND_THREEBOAD	 0x0005   // 三板
#			define KIND_SMALLSTOCK	 0x0006   // 中小盘股
#			define KIND_PLACE		 0x0007	  // 配售
#			define KIND_LOF			 0x0008	  // LOF
#			define KIND_ETF			 0x0009   // ETF
#			define KIND_QuanZhen	 0x000A   // 权证

#			define KIND_OtherIndex	 0x000E   // 第三方行情分类，如:中信指数

#			define SC_Others		 0x000F   // 其他 0x09
#			define KIND_USERDEFINE	 0x0010   // 自定义指数

// 港股市场
#define HK_MARKET				 0x2000 // 港股分类
#	define HK_BOURSE			 0x0100 // 主板市场
#	define	GE_BOURSE			 0x0200 // 创业板市场(Growth Enterprise Market)
#	define	INDEX_BOURSE		 0x0300	// 指数市场
#	define	NX_BOURSE		     0x0400	// 牛熊证
#		define HK_KIND_INDEX			 0x0000   // 港指
#		define HK_KIND_FUTURES_INDEX	 0x0001   // 期指
//#		define	KIND_Option				 0x0002	  // 港股期权

#	define SYSBK_BOURSE			 0x0400 // 港股板块(H股指数成份股，讯捷指数成份股）。
#	define USERDEF_BOURSE		 0x0800 // 自定义（自选股或者自定义板块）
#			define HK_KIND_BOND		 0x0000   // 债券
#			define HK_KIND_MulFund	 0x0001   // 一揽子认股证
#			define HK_KIND_FUND		 0x0002   // 基金
#			define KIND_WARRANTS	 0x0003   // 认股证
#			define KIND_JR			 0x0004   // 金融
#			define KIND_ZH			 0x0005   // 综合
#			define KIND_DC			 0x0006   // 地产
#			define KIND_LY			 0x0007   // 旅游
#			define KIND_GY			 0x0008   // 工业
#			define KIND_GG			 0x0009   // 公用
#			define KIND_QT			 0x000A   // 其它

/*期货大类*/
#define FUTURES_MARKET			 0x4000 // 期货
#		define DALIAN_BOURSE		 0x0100	// 大连
#				define KIND_BEAN		 0x0001	// 连豆
#				define KIND_YUMI		 0x0002	// 大连玉米
#				define KIND_SHIT		 0x0003	// 大宗食糖
#				define KIND_DZGY		 0x0004	// 大宗工业1
#				define KIND_DZGY2		 0x0005	// 大宗工业2
#				define KIND_DOUYOU		 0x0006	// 连豆油
#				define KIND_JYX			 0x0007	// 聚乙烯
#				define KIND_ZTY			 0x0008	// 棕榈油

#		define SHANGHAI_BOURSE	  0x0200	// 上海
#				define KIND_METAL		 0x0001	// 上海金属
#				define KIND_RUBBER		 0x0002	// 上海橡胶
#				define KIND_FUEL		 0x0003	// 上海燃油
//#				define KIND_GUZHI		 0x0004	// 股指期货
#				define KIND_QHGOLD		 0x0005	// 上海黄金

#		define ZHENGZHOU_BOURSE	  0x0300	// 郑州
#				define KIND_XIAOM		 0x0001	// 郑州小麦
#				define KIND_MIANH		 0x0002	// 郑州棉花
#				define KIND_BAITANG		 0x0003	// 郑州白糖
#				define KIND_PTA			 0x0004	// 郑州PTA
#				define KIND_CZY			 0x0005	// 菜籽油

#		define HUANGJIN_BOURSE	  0x0400		// 黄金交易所
#				define KIND_GOLD		 0x0001	// 上海黄金

#		define GUZHI_BOURSE		  0x0500		// 股指期货
#				define KIND_GUZHI		 0x0001	// 股指期货

#		define SELF_BOURSE		  0x0600	// 自定义数据

#		define DZGT_BOURSE		  0x0610	// 大宗钢铁数据

/*外盘大类*/
#define WP_MARKET				 ((HSMarketDataType)0x5000) // 外盘
#		define WP_INDEX				0x0100	// 国际指数 // 不用了
#		define WP_LME				0x0200	// LME		// 不用了
#			define WP_LME_CLT			0x0210 //"场内铜";
#			define WP_LME_CLL			0x0220 //"场内铝";
#			define WP_LME_CLM			0x0230 //"场内镍";
#			define WP_LME_CLQ			0x0240 //"场内铅";
#			define WP_LME_CLX			0x0250 //"场内锌";
#			define WP_LME_CWT			0x0260 //"场外铝";
#			define WP_LME_CW			0x0270 //"场外";
#			define WP_LME_SUB			0x0000

#			define WP_CBOT				0x0300	// CBOT			
#			define WP_NYMEX	 			0x0400	// NYMEX	 	
#			define WP_NYMEX_YY			0x0000	//"原油";
#			define WP_NYMEX_RY			0x0001	//"燃油";
#			define WP_NYMEX_QY			0x0002	//"汽油";

#			define WP_COMEX	 			0x0500	// COMEX	 	
#			define WP_TOCOM	 			0x0600	// TOCOM	 	
#			define WP_IPE				0x0700	// IPE			
#			define WP_NYBOT				0x0800	// NYBOT		
#			define WP_NOBLE_METAL		0x0900	// 贵金属	
#			  define WP_NOBLE_METAL_XH	0x0000  //"现货";
#			  define WP_NOBLE_METAL_HJ	0x0001  //"黄金";
#			  define WP_NOBLE_METAL_BY	0x0002  //"白银";

#			define WP_FUTURES_INDEX		0x0a00	// 期指
#			define WP_SICOM				0x0b00	// SICOM		
#			define WP_LIBOR				0x0c00	// LIBOR		
#			define WP_NYSE				0x0d00	// NYSE
#			define WP_CEC				0x0e00	// CEC


#			define WP_Other_TZTHuanjin	0x0F10	// 黄金期货数据生成,第三方数据
#			define WP_Other_JinKaiXun	0x0F20	// 金凯讯的数据
#			define WP_JKX               WP_Other_JinKaiXun
#			define WP_XJP               0x0F30	// 新加坡数据


#			define WP_INDEX_AZ	 		0x0110 //"澳洲";
#			define WP_INDEX_OZ	 		0x0120 //"欧洲";
#			define WP_INDEX_MZ	 		0x0130 //"美洲";
#			define WP_INDEX_TG	 		0x0140 //"泰国";
#			define WP_INDEX_YL	 		0x0150 //"印尼";
#			define WP_INDEX_RH	 		0x0160 //"日韩";
#			define WP_INDEX_XHP 		0x0170 //"新加坡";
#			define WP_INDEX_FLB 		0x0180 //"菲律宾";
#			define WP_INDEX_CCN 		0x0190 //"中国大陆";
#			define WP_INDEX_TW  		0x01a0 //"中国台湾";
#			define WP_INDEX_MLX 		0x01b0 //"马来西亚";
#			define WP_INDEX_SUB 		0x0000 


/*外汇大类*/
#define FOREIGN_MARKET			 ((HSMarketDataType)0x8000) // 外汇
#	define WH_BASE_RATE			0x0100	// 基本汇率
#	define WH_ACROSS_RATE		0x0200	// 交叉汇率
#		define FX_TYPE_AU 			0x0000 // AU	澳元
#		define FX_TYPE_CA 			0x0001 // CA	加元
#		define FX_TYPE_CN 			0x0002 // CN	人民币
#		define FX_TYPE_DM 			0x0003 // DM	马克
#		define FX_TYPE_ER 			0x0004 // ER	欧元	 
#		define FX_TYPE_HK 			0x0005 // HK	港币
#		define FX_TYPE_SF 			0x0006 // SF	瑞士 
#		define FX_TYPE_UK 			0x0007 // UK	英镑
#		define FX_TYPE_YN 			0x0008 // YN	日元

#	define WH_FUTURES_RATE			0x0300  // 期汇

/*外汇大类*/
#define HJ_MARKET			 ((HSMarketDataType)0x6000) // 黄金
#	define HJ_SH_CURR			0x0100	// 上海现货
#	define HJ_SH_QH		        0x0200	// 上海期货
#	define HJ_WORLD	        0x0300	// 国际市场
#	define HJ_OTHER	        0x0400	// 其它市场

class CHisDataCpMgr;
class CHisDataHandler : public CWorkThread
{
public:
	CHisDataHandler(CHisDataCpMgr* p);
	~CHisDataHandler(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int Enque(QUOTATION& stQuotation);

	void OpenHisdataFile();
	string HandleCmdLine(const string& sCmd, const vector<string>& vecPara);
private:
	void WriteTick(const QUOTATION& stQuotation);
private:
	CHisDataCpMgr*     m_pProviderCpMgr;
	CConfig*		m_pCfg;
	
	vector<string>			m_vTickInst;	
	map<string, ofstream*>	m_mapOfsTick;
	//ofstream		m_ofsTick;

	//Test
	ofstream		m_ofsQuotation;
	string			m_sFilePathAbs;
	int				m_nTest;
	unsigned int	m_uiPkts;

	std::deque<QUOTATION> m_deqQuotation;
	CCondMutex	m_deqCondMutex;

	CGessDate		m_oDateLast;
	//
	map<std::string, QUOTATION> m_mapQuotation;

	int ThreadEntry();
	int End();

};

#endif