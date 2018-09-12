/*******************************************************************************
* Copyright (c)2003, 讯捷软件有限公司
* All rights reserved.
*
* 文件名称：HsStruct.h
* 文件标识：通讯基本结构
* 摘    要：通讯基本结构
*
* 当前版本：dfx2003
* 作    者：讯捷王工
* 完成日期：2003-07
*
* 备    注：
*
* 修改记录：
*
*	日	  期：2004年2月26日
*	作	  者：讯捷王工
*	修改内容：加入了新的股票分类KIND_PLACE及新的市场分类STOCK_WHILOM_NAME_MARKET
*
*	日	  期：2004年2月27日
*	作	  者：讯捷王工
*	修改内容：改StockTick结构说明，其中的原期货的m_nChiCangLiang字段可用来存放深交所股票的单笔成交数数据
*			  改StockOtherData结构说明，其中原期货的m_lKaiCang字段可用来存放深交所股票的单笔成交数数据
*******************************************************************************/
#ifndef HS_STRUCT_H
#define HS_STRUCT_H

#include <windows.h>
#include "time.h"
#include "computer_intel_convert.h"


/* 
注意：HS_SUPPORT_UNIX 当前头文件是否使用于服务器端
*/
//#define HS_SUPPORT_UNIX

#ifndef HS_SUPPORT_UNIX
#pragma	pack(1)
#else
#endif
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


// 内部分类，给股票曾用名用
#define STOCK_WHILOM_NAME_MARKET ((HSMarketDataType)0xF000)


//#define	MakeMarket(x)			((HSMarketDataType)((x) & 0xF000))
//#define MakeMainMarket(x)		((HSMarketDataType)((x) & 0xFFF0))
//#define	MakeMidMarket(x)		((HSMarketDataType)((x) & 0x0F00)) // 分类第二位

static int	MakeMarket(HSMarketDataType x)
{
	return ((HSMarketDataType)((x) & 0xF000));
}
static int  MakeMainMarket(HSMarketDataType x)
{
	return ((HSMarketDataType)((x) & 0xFFF0));
}
static int	MakeMidMarket(HSMarketDataType x)
{
	return ((HSMarketDataType)((x) & 0x0F00)); // 分类第二位
}


//#define MakeSubMarket(x)		((HSMarketDataType)((x) & 0x000F))
static int MakeSubMarket(HSMarketDataType x)
{
	return ((HSMarketDataType)((x) & 0x000F));
}

//#define MakeHexSubMarket(x)		( (HSMarketDataType)((x) & 0x000F) )
//#define MakeSubMarketPos(x)		( ((MakeHexSubMarket(x) / 16) * 10) + (MakeHexSubMarket(x) % 16) )

static int MakeHexSubMarket(HSMarketDataType x)
{
	return ( (HSMarketDataType)((x) & 0x000F) );
}

static int MakeSubMarketPos(HSMarketDataType x)
{
	return ( ((MakeHexSubMarket(x) / 16) * 10) + (MakeHexSubMarket(x) % 16) );
}

// 是否为国内股票指数
//#define MakeIndexMarket(x)		( (MakeMarket(x) == STOCK_MARKET) && (MakeSubMarket(x) == KIND_INDEX))
static int MakeIndexMarket(HSMarketDataType x)
{
	return ( (MakeMarket(x) == STOCK_MARKET) &&
			 (MakeMidMarket(x) != 0) &&
		     ((MakeSubMarket(x) == KIND_INDEX) ||
			 (MakeSubMarket(x) == KIND_OtherIndex)));
}

static int MakeIndexSelf(HSMarketDataType x)
{
	return ( (MakeMarket(x) == STOCK_MARKET) &&
			 (MakeMidMarket(x) != 0) &&
		     ((MakeSubMarket(x) == KIND_OtherIndex)));
}

// 是否为基金
static int MakeFundMarket(HSMarketDataType x)
{
	return ( (MakeMarket(x) == STOCK_MARKET) &&
			 (MakeMidMarket(x) != 0) &&
		     (MakeSubMarket(x) == KIND_FUND) );
}

// 是否为港股指数
//#define MakeHKIndex(x)          ( MakeMainMarket(x) == (HK_MARKET | INDEX_BOURSE) )
static int MakeHKIndex(HSMarketDataType x)
{
	return ( MakeMainMarket(x) == (HK_MARKET | INDEX_BOURSE) );
}

// 是否为股指期货
static int MakeGZIndex(HSMarketDataType x)
{
	return ( (MakeMainMarket(x) == (FUTURES_MARKET | GUZHI_BOURSE)) &&
		     (MakeSubMarket(x) == KIND_GUZHI) );
}

// 是否为黄金期货
static int MakeGoldIndex(HSMarketDataType x)
{
	return ( (MakeMainMarket(x) == (FUTURES_MARKET | HUANGJIN_BOURSE)) &&
		     (MakeSubMarket(x) == KIND_GOLD) );
}
//yangdl 2008.03.06 特殊处理价格
// 是否为需要特殊处理价格的分类
static int MakeNegativeIndex(HSMarketDataType x)
{
	return ( (MakeMainMarket(x) == (FOREIGN_MARKET | WH_FUTURES_RATE)) &&
		     ( (MakeSubMarket(x) == FX_TYPE_AU)  || (MakeSubMarket(x) == FX_TYPE_YN)));
}

//#define MakeWPIndex(x)          ( (MakeMainMarket(x) >= (WP_MARKET | WP_INDEX_AZ)) && (MakeMainMarket(x) <= (WP_MARKET | WP_INDEX_MLX)) )
//#define MakeWP_LME(x)           ( (MakeMainMarket(x) >= (WP_MARKET | WP_LME_CLT))  && (MakeMainMarket(x) <= (WP_MARKET | WP_LME_CW)) )
static int MakeWPIndex(HSMarketDataType x)
{
	return ( (MakeMainMarket(x) >= (WP_MARKET | WP_INDEX_AZ)) && (MakeMainMarket(x) <= (WP_MARKET | WP_INDEX_MLX)) );
}

static int MakeWP_LME(HSMarketDataType x)
{
	return ( (MakeMainMarket(x) >= (WP_MARKET | WP_LME_CLT))  && (MakeMainMarket(x) <= (WP_MARKET | WP_LME_CW)) );
}

//#define	WhoMarket(x,y)			( MakeMarket(x) == MakeMarket(y) )
static int	WhoMarket(HSMarketDataType x,HSMarketDataType y)
{
	return ( MakeMarket(x) == MakeMarket(y) );
}

// 是否为国内股票ETF
//#define MakeETF(x)				( (MakeMarket(x) == STOCK_MARKET) && (MakeSubMarket(x) == KIND_ETF))
static int MakeETF(HSMarketDataType x)
{
	return ( (MakeMarket(x) == STOCK_MARKET) && (MakeSubMarket(x) == KIND_ETF));
}

#define SH_Bourse	((HSMarketDataType)(STOCK_MARKET|SH_BOURSE))
#define SZ_Bourse	((HSMarketDataType)(STOCK_MARKET|SZ_BOURSE))

#define ToDataType(x) ((HSMarketDataType)(x))

#ifndef HS_SUPPORT_UNIX
extern UINT HashKey(LPCTSTR key,int nKeyCount,const int nHashTableSize); // ref #include "stockmanager.h"
#endif /*HS_SUPPORT_UNIX*/

#ifdef Support_XHX_Dll 
   #define Negative(x,y)  ( y ? x&0x0FFFFFFF : x )
   #define NegativeFlag(x,y) ( y ? ( ( x&0xF0000000 ) ? 1 : 0 ) : 0  ) 
#else
   #define Negative(x,y)   x
   #define NegativeFlag(x,y) 0
#endif

struct NegativeValue
{
	DWORD m_lValue:28;
	DWORD m_lFlag:4;
};

struct NegativeData
{
	union
	{
		DWORD		  m_keyValue;		
		NegativeValue m_Value;
	};
};
// use
// NegativeData data;
// data.m_Value.m_lValue = 123; // 实际数值
// data.m_Value.m_lFlag  = 1;   // 符号 0、正号；1、负号

#define PYJC_MAX_LENGTH		16			// 拼音长度
#define STOCK_NAME_SIZE		16			// 股票名称长度

// 股票代码结构
struct CodeInfo
{
	HSMarketDataType	m_cCodeType;	// 证券类型
	char				m_cCode[6];		// 证券代码

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_cCodeType = YlsShortIntelToComputer(m_cCodeType);
		}
		else
		{
			m_cCodeType = YlsShortComputerToIntel(m_cCodeType);
		}
#endif
	}



};

//股票详细信息
struct StockUserInfo
{
	CodeInfo	m_ciStockCode;		//股票代码结构
	char		m_cStockName[STOCK_NAME_SIZE];	//股票名称
	char		m_cStockPYJC[PYJC_MAX_LENGTH];	//拼音简称
	long		m_lPrevClose;		//昨收
	unsigned long		m_l5DayVol;	//五日量


};


//yangdl 2007.09.24 快速下单
struct StockKsXdInfo
{
	BYTE m_nXd;
    StockUserInfo m_StockUser;
	float m_nPrice;
	DWORD m_nAmount;
};

//证券信息
struct HSTypeTime
{
	short	m_nOpenTime;	// 前开市时间
	short	m_nCloseTime;	// 前闭市时间

	short   GetDist() { return (m_nCloseTime - m_nOpenTime); }

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_nOpenTime = YlsShortIntelToComputer(m_nOpenTime);
			m_nCloseTime = YlsShortIntelToComputer(m_nCloseTime);
		}
		else
		{
			m_nOpenTime = YlsShortComputerToIntel(m_nOpenTime);
			m_nCloseTime = YlsShortComputerToIntel(m_nCloseTime);
		}
#endif
	}
};

struct  HSTypeTime_Unoin
{
	short	m_nAheadOpenTime;	// 前开市时间
	short	m_nAheadCloseTime;	// 前闭市时间
	short	m_nAfterOpenTime;	// 后开市时间
	short	m_nAfterCloseTime;	// 后闭市时间

	HSTypeTime	m_nTimes[9];	// 新加入区段边界,两边界为-1时，为无效区段

	HSTypeTime	m_nPriceDecimal;   // 小数位, < 0

	// 第一个无效区段后所有区段全为无效。
};

// 股票分类名称
struct StockTypeName 
{
	char m_szName[20];	// 股票分类名称
};

struct StockType
{
	StockTypeName m_stTypeName;	// 对应分类的名称

	short   m_nStockType;		// 证券类型
	short   m_nTotal;			// 证券总数
	short   m_nOffset;			// 偏移量
	short   m_nPriceUnit;		// 价格单位
	short   m_nTotalTime;		// 总开市时间（分钟）
	short   m_nCurTime;			// 现在时间（分钟）

	union
	{
		HSTypeTime		 m_nNewTimes[11];
		HSTypeTime_Unoin m_union;
	};

#ifndef HS_SUPPORT_UNIX
	StockType()
	{
		memset(this,0,sizeof(StockType));
	}
#endif

	BOOL IsInitTime(int nTimer,int nDist = 10)
	{
		return ( nTimer >= (m_nNewTimes->m_nOpenTime - nDist) && nTimer < m_nNewTimes->m_nOpenTime );
	}

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_nStockType = YlsShortIntelToComputer(m_nStockType);
			m_nTotal = YlsShortIntelToComputer(m_nTotal);
			m_nOffset = YlsShortIntelToComputer(m_nOffset);
			m_nPriceUnit = YlsShortIntelToComputer(m_nPriceUnit);
			m_nTotalTime = YlsShortIntelToComputer(m_nTotalTime);
			m_nCurTime = YlsShortIntelToComputer(m_nCurTime);
		}
		else
		{
			m_nStockType = YlsShortComputerToIntel(m_nStockType);
			m_nTotal = YlsShortComputerToIntel(m_nTotal);
			m_nOffset = YlsShortComputerToIntel(m_nOffset);
			m_nPriceUnit = YlsShortComputerToIntel(m_nPriceUnit);
			m_nTotalTime = YlsShortComputerToIntel(m_nTotalTime);
			m_nCurTime = YlsShortComputerToIntel(m_nCurTime);
		}

		for( int i = 0; i < 12; i++ )
		{
			m_nNewTimes[i].To(cIntelToComputer);
		}
#endif
	}
};

#define STOCKTYPE_COUNT 60
// 市场信息结构(客户端本地使用)
struct BourseInfo
{
	StockTypeName	m_stGroupName;	// 市场名称(对应市场类别)
	short			m_nMarketType;	// 市场类别(最高俩位)

	unsigned int	m_dwCRC;		// CRC校验码（市场）

	long			m_lDate;		// 今日日期（19971230）

	//char  m_cVersion;			    /* 版本号，= 'A' 表示本版本，其他	为老版本*/	
	//short m_nSize;				// 本结构的长度

	short  m_cType;				// 有效的证券类型个数

	StockType		 m_stNewType[STOCKTYPE_COUNT];	// 证券信息
	StockTypeName	 m_stTypeName[STOCKTYPE_COUNT]; // 对应的名称
	int				 m_nMenuID[STOCKTYPE_COUNT];    // 对应的菜单id

#ifndef HS_SUPPORT_UNIX
	BourseInfo()
	{
		memset(this, 0, sizeof(BourseInfo));
		//m_cVersion = 'A';
		//m_nSize    = sizeof(BourseInfo);
	}
#endif
};






// 市场信息结构
struct CommBourseInfo
{
	StockTypeName	m_stTypeName;	// 市场名称(对应市场类别)
	short			m_nMarketType;	// 市场类别(最高俩位)

	short			m_cCount;		// 有效的证券类型个数

	long			m_lDate;		// 今日日期（19971230）
	unsigned int	m_dwCRC;		// CRC校验码（分类）

	StockType		m_stNewType[1];	// 证券信息	

#ifndef HS_SUPPORT_UNIX
	CommBourseInfo()
	{
		memset(this, 0, sizeof(CommBourseInfo));
	}
#endif

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_nMarketType = YlsShortIntelToComputer(m_nMarketType);
			m_cCount = YlsShortIntelToComputer(m_cCount);
			m_lDate = YlsIntIntelToComputer(m_lDate);
			m_dwCRC = YlsIntIntelToComputer(m_dwCRC);

			for(int i = 0; i < m_cCount; i++ )
			{
				m_stNewType[i].To(cIntelToComputer);
			}
		}
		else
		{
			for(int i = 0; i < m_cCount; i++ )
			{
				m_stNewType[i].To(cIntelToComputer);
			}

			m_nMarketType = YlsShortComputerToIntel(m_nMarketType);
			m_cCount = YlsShortComputerToIntel(m_cCount);
			m_lDate = YlsIntComputerToIntel(m_lDate);
			m_dwCRC = YlsIntComputerToIntel(m_dwCRC);
		}		
#endif
	}
};

// 服务器证券简短信息
struct ServerCompare	
{
	HSMarketDataType	m_cBourse;			// 证券分类类型
	short				m_nAlignment;    	// 为了4字节对齐而添加的字段
	unsigned int		m_dwCRC;			// CRC校验码

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_cBourse = YlsShortIntelToComputer(m_cBourse);
			m_nAlignment = YlsShortIntelToComputer(m_nAlignment);
			m_dwCRC = YlsIntIntelToComputer(m_dwCRC);
		}
		else
		{
			m_cBourse = YlsShortComputerToIntel(m_cBourse);
			m_nAlignment = YlsShortComputerToIntel(m_nAlignment);
			m_dwCRC = YlsIntComputerToIntel(m_dwCRC);
		}
#endif
	}
};

// 计算数据
struct SeverCalculateData
{
	//CodeInfo m_sCodeInfo;
	HSMarketDataType	m_cCodeType;	// 证券类型
	char				m_cCode[6];		// 证券代码

	float				m_fUpPrice;     // 涨停板价
	float				m_fDownPrice;   // 跌停板价

	void To( char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_cCodeType = YlsShortComputerToIntel(m_cCodeType);

			m_fUpPrice = YlsFloatComputerToIntel(m_fUpPrice);
			m_fDownPrice = YlsFloatComputerToIntel(m_fDownPrice);
		}
		else
		{
			m_cCodeType = YlsShortIntelToComputer(m_cCodeType);

			m_fUpPrice = YlsFloatComputerToIntel(m_fUpPrice);
			m_fDownPrice = YlsFloatComputerToIntel(m_fDownPrice);
		}
#endif
	}
};


// Level 2 扩展的静态数据
struct LevelStatic
{
	SeverCalculateData m_calc;			// 计算的数据

	float m_fPrice;						// 发行價

	float m_fShareFaceValue;			// 每股面值

	float m_fOutstandingShares;			// 总发行量
	float m_fPublicFloatShareQuantity;	// 流通股数

	long  MaturityDate;					// 到期/交割日
	long  CouponPaymentDate;			// 付息日
	long  LastInterestPaymentDate;		// 上一次理论付息日
	long  NextInterestPaymentDate;		// 下一次理论付息日

	float ConversionPrice;				// 行权/转股价格
	float m_fCVRatio;					// 行权比例
	long  ConversionPeriod;				// 行权时间

	float InterestRate;					// 当年利率
	float m_fCouponRate;				// 票面利率
	float ContractMultiplier;			// 债券折合成回购标准券的比例
	long  InterestAccrualDate;			// 起息日
	long  Residualmaturity;			    // 剩余时间
	float AccruedInterestAmt;			// 应计利息
	
	float RoundLot;						// 交易单位

	int	  NoSecurityAltID;				// 备选证券代码个数

	char  SecurityAltID[16];			// 备选证券代码
	char  SecurityAltIDSource[16];		// 备选证券代码源
	char  SecurityIDSource[16];			// 证券代码源
	char  SecurityDesc[16];				// 证券描述

	char  CFICode[4];					// 证券类别
	char  SecurityExchange[4];			// 交易所代码
	char  SecuritySubType[4];			// 证券子类别

	char  m_szIndustryClassification[4];// 行业种类
	char  Currency[4];					// 币种

	char  SecurityTradingStatus[4];		// 交易状态
	char  m_szCorporateAction[4];		// 除权除息标志

	char  CouponRate[4];

	char  BondType[4];					// 债券类型
	char  American_European;			// 美式/欧式
	char  CallOrPut;					// Call/Put 标志
	char  Underlying[12];				// 基础证券,为基础证券代码
	char  Issuer[16];					// 发行机构

	char  m_szResever[10];				// 保留

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN

		m_calc.To(cIntelToComputer);

		if( cIntelToComputer )
		{					
			m_fPrice = YlsFloatComputerToIntel(m_fPrice);			

			m_fShareFaceValue = YlsFloatComputerToIntel(m_fShareFaceValue);			
			m_fOutstandingShares = YlsFloatComputerToIntel(m_fOutstandingShares);
			m_fPublicFloatShareQuantity = YlsFloatComputerToIntel(m_fPublicFloatShareQuantity);
			ConversionPrice = YlsFloatComputerToIntel(ConversionPrice);
			m_fCVRatio = YlsFloatComputerToIntel(m_fCVRatio);
			InterestRate = YlsFloatComputerToIntel(InterestRate);
			m_fCouponRate = YlsFloatComputerToIntel(m_fCouponRate);
			ContractMultiplier = YlsFloatComputerToIntel(ContractMultiplier);
			AccruedInterestAmt = YlsFloatComputerToIntel(AccruedInterestAmt);
			RoundLot = YlsFloatComputerToIntel(RoundLot);

			MaturityDate = YlsIntComputerToIntel(MaturityDate);
			CouponPaymentDate = YlsIntComputerToIntel(CouponPaymentDate);

			LastInterestPaymentDate = YlsIntComputerToIntel(LastInterestPaymentDate);
			NextInterestPaymentDate = YlsIntComputerToIntel(NextInterestPaymentDate);
			ConversionPeriod = YlsIntComputerToIntel(ConversionPeriod);

			InterestAccrualDate = YlsIntComputerToIntel(InterestAccrualDate);
			Residualmaturity = YlsIntComputerToIntel(Residualmaturity);
			NoSecurityAltID = YlsIntComputerToIntel(NoSecurityAltID);
		}
		else
		{	
			m_fPrice = YlsFloatComputerToIntel(m_fPrice);			

			m_fShareFaceValue = YlsFloatComputerToIntel(m_fShareFaceValue);			
			m_fOutstandingShares = YlsFloatComputerToIntel(m_fOutstandingShares);
			m_fPublicFloatShareQuantity = YlsFloatComputerToIntel(m_fPublicFloatShareQuantity);
			ConversionPrice = YlsFloatComputerToIntel(ConversionPrice);
			m_fCVRatio = YlsFloatComputerToIntel(m_fCVRatio);
			InterestRate = YlsFloatComputerToIntel(InterestRate);
			m_fCouponRate = YlsFloatComputerToIntel(m_fCouponRate);
			ContractMultiplier = YlsFloatComputerToIntel(ContractMultiplier);
			AccruedInterestAmt = YlsFloatComputerToIntel(AccruedInterestAmt);
			RoundLot = YlsFloatComputerToIntel(RoundLot);

			MaturityDate = YlsIntIntelToComputer(MaturityDate);
			CouponPaymentDate = YlsIntIntelToComputer(CouponPaymentDate);

			LastInterestPaymentDate = YlsIntIntelToComputer(LastInterestPaymentDate);
			NextInterestPaymentDate = YlsIntIntelToComputer(NextInterestPaymentDate);
			ConversionPeriod = YlsIntIntelToComputer(ConversionPeriod);

			InterestAccrualDate = YlsIntIntelToComputer(InterestAccrualDate);
			Residualmaturity = YlsIntIntelToComputer(Residualmaturity);
			NoSecurityAltID = YlsIntIntelToComputer(NoSecurityAltID);
		}		
#endif
	}
};

// Level 2 扩展的行情数据
struct LevelRealTime 
{
	long				m_lBuyPrice1;			// 买一价
	unsigned long		m_lBuyCount1;			// 买一量
	long				m_lBuyPrice2;			// 买二价
	unsigned long		m_lBuyCount2;			// 买二量
	long				m_lBuyPrice3;			// 买三价
	unsigned long		m_lBuyCount3;			// 买三量
	long				m_lBuyPrice4;			// 买四价
	unsigned long		m_lBuyCount4;			// 买四量
	long				m_lBuyPrice5;			// 买五价
	unsigned long		m_lBuyCount5;			// 买五量

	long				m_lSellPrice1;			// 卖一价
	unsigned long		m_lSellCount1;			// 卖一量
	long				m_lSellPrice2;			// 卖二价
	unsigned long		m_lSellCount2;			// 卖二量
	long				m_lSellPrice3;			// 卖三价
	unsigned long		m_lSellCount3;			// 卖三量
	long				m_lSellPrice4;			// 卖四价
	unsigned long		m_lSellCount4;			// 卖四量
	long				m_lSellPrice5;			// 卖五价
	unsigned long		m_lSellCount5;			// 卖五量

	float				m_fBuyTotal;			// 委托买入总量
	float				WeightedAvgBidPx;			// 加权平均委买价格
	float				AltWeightedAvgBidPx;

	float				m_fSellTotal;			// 委托卖出总量
	float				WeightedAvgOfferPx;			// 加权平均委卖价格
	float				AltWeightedAvgOfferPx;

	unsigned long       m_lTickCount;			// 成交笔数

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{		
			m_lBuyPrice1 = YlsIntIntelToComputer(m_lBuyPrice1);			// 买一价
			m_lBuyCount1 = YlsIntIntelToComputer(m_lBuyCount1);			// 买一量
			m_lBuyPrice2 = YlsIntIntelToComputer(m_lBuyPrice2);			// 买二价
			m_lBuyCount2 = YlsIntIntelToComputer(m_lBuyCount2);			// 买二量
			m_lBuyPrice3 = YlsIntIntelToComputer(m_lBuyPrice3);			// 买三价
			m_lBuyCount3 = YlsIntIntelToComputer(m_lBuyCount3);			// 买三量
			m_lBuyPrice4 = YlsIntIntelToComputer(m_lBuyPrice4);			// 买四价
			m_lBuyCount4 = YlsIntIntelToComputer(m_lBuyCount4);			// 买四量
			m_lBuyPrice5 = YlsIntIntelToComputer(m_lBuyPrice5);			// 买五价
			m_lBuyCount5 = YlsIntIntelToComputer(m_lBuyCount5);			// 买五量

			m_lSellPrice1 = YlsIntIntelToComputer(m_lSellPrice1);			// 卖一价
			m_lSellCount1 = YlsIntIntelToComputer(m_lSellCount1);			// 卖一量
			m_lSellPrice2 = YlsIntIntelToComputer(m_lSellPrice2);			// 卖二价
			m_lSellCount2 = YlsIntIntelToComputer(m_lSellCount2);			// 卖二量
			m_lSellPrice3 = YlsIntIntelToComputer(m_lSellPrice3);			// 卖三价
			m_lSellCount3 = YlsIntIntelToComputer(m_lSellCount3);			// 卖三量
			m_lSellPrice4 = YlsIntIntelToComputer(m_lSellPrice4);			// 卖四价
			m_lSellCount4 = YlsIntIntelToComputer(m_lSellCount4);			// 卖四量
			m_lSellPrice5 = YlsIntIntelToComputer(m_lSellPrice5);			// 卖五价
			m_lSellCount5 = YlsIntIntelToComputer(m_lSellCount5);			// 卖五量

			m_fBuyTotal = YlsFloatComputerToIntel(m_fBuyTotal);			//	委托买入总量
			WeightedAvgBidPx = YlsFloatComputerToIntel(WeightedAvgBidPx);
			AltWeightedAvgBidPx = YlsFloatComputerToIntel(AltWeightedAvgBidPx);
			m_fSellTotal = YlsFloatComputerToIntel(m_fSellTotal);
			WeightedAvgOfferPx = YlsFloatComputerToIntel(WeightedAvgOfferPx);
			AltWeightedAvgOfferPx = YlsFloatComputerToIntel(AltWeightedAvgOfferPx);

			m_lTickCount = YlsIntIntelToComputer(m_lTickCount);			
		}
		else
		{
			m_lBuyPrice1 = YlsIntComputerToIntel(m_lBuyPrice1);			// 买一价
			m_lBuyCount1 = YlsIntComputerToIntel(m_lBuyCount1);			// 买一量
			m_lBuyPrice2 = YlsIntComputerToIntel(m_lBuyPrice2);			// 买二价
			m_lBuyCount2 = YlsIntComputerToIntel(m_lBuyCount2);			// 买二量
			m_lBuyPrice3 = YlsIntComputerToIntel(m_lBuyPrice3);			// 买三价
			m_lBuyCount3 = YlsIntComputerToIntel(m_lBuyCount3);			// 买三量
			m_lBuyPrice4 = YlsIntComputerToIntel(m_lBuyPrice4);			// 买四价
			m_lBuyCount4 = YlsIntComputerToIntel(m_lBuyCount4);			// 买四量
			m_lBuyPrice5 = YlsIntComputerToIntel(m_lBuyPrice5);			// 买五价
			m_lBuyCount5 = YlsIntComputerToIntel(m_lBuyCount5);			// 买五量

			m_lSellPrice1 = YlsIntComputerToIntel(m_lSellPrice1);			// 卖一价
			m_lSellCount1 = YlsIntComputerToIntel(m_lSellCount1);			// 卖一量
			m_lSellPrice2 = YlsIntComputerToIntel(m_lSellPrice2);			// 卖二价
			m_lSellCount2 = YlsIntComputerToIntel(m_lSellCount2);			// 卖二量
			m_lSellPrice3 = YlsIntComputerToIntel(m_lSellPrice3);			// 卖三价
			m_lSellCount3 = YlsIntComputerToIntel(m_lSellCount3);			// 卖三量
			m_lSellPrice4 = YlsIntComputerToIntel(m_lSellPrice4);			// 卖四价
			m_lSellCount4 = YlsIntComputerToIntel(m_lSellCount4);			// 卖四量
			m_lSellPrice5 = YlsIntComputerToIntel(m_lSellPrice5);			// 卖五价
			m_lSellCount5 = YlsIntComputerToIntel(m_lSellCount5);			// 卖五量

			m_fBuyTotal = YlsFloatComputerToIntel(m_fBuyTotal);			//	委托买入总量
			WeightedAvgBidPx = YlsFloatComputerToIntel(WeightedAvgBidPx);
			AltWeightedAvgBidPx = YlsFloatComputerToIntel(AltWeightedAvgBidPx);
			m_fSellTotal = YlsFloatComputerToIntel(m_fSellTotal);
			WeightedAvgOfferPx = YlsFloatComputerToIntel(WeightedAvgOfferPx);
			AltWeightedAvgOfferPx = YlsFloatComputerToIntel(AltWeightedAvgOfferPx);

			m_lTickCount = YlsIntIntelToComputer(m_lTickCount);			
		}		
#endif
	}
};

// 实时数据
struct HSStockRealTime 
{
	long				m_lOpen;         		// 今开盘
	long				m_lMaxPrice;     		// 最高价
	long				m_lMinPrice;     		// 最低价
	long				m_lNewPrice;     		// 最新价
	unsigned long		m_lTotal;				// 成交量(单位:股)
	float				m_fAvgPrice;			// 成交金额

	long				m_lBuyPrice1;			// 买一价
	unsigned long		m_lBuyCount1;			// 买一量
	long				m_lBuyPrice2;			// 买二价
	unsigned long		m_lBuyCount2;			// 买二量
	long				m_lBuyPrice3;			// 买三价
	unsigned long		m_lBuyCount3;			// 买三量
	long				m_lBuyPrice4;			// 买四价
	unsigned long		m_lBuyCount4;			// 买四量
	long				m_lBuyPrice5;			// 买五价
	unsigned long		m_lBuyCount5;			// 买五量

	long				m_lSellPrice1;			// 卖一价
	unsigned long		m_lSellCount1;			// 卖一量
	long				m_lSellPrice2;			// 卖二价
	unsigned long		m_lSellCount2;			// 卖二量
	long				m_lSellPrice3;			// 卖三价
	unsigned long		m_lSellCount3;			// 卖三量
	long				m_lSellPrice4;			// 卖四价
	unsigned long		m_lSellCount4;			// 卖四量
	long				m_lSellPrice5;			// 卖五价
	unsigned long		m_lSellCount5;			// 卖五量

	long				m_nHand;				// 每手股数	(是否可放入代码表中？？？？）
	long				m_lNationalDebtRatio;	// 国债利率,基金净值

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{		
			m_lOpen = YlsIntIntelToComputer(m_lOpen);         		// 今开盘
			m_lMaxPrice = YlsIntIntelToComputer(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntIntelToComputer(m_lMinPrice);     		// 最低价
			m_lNewPrice = YlsIntIntelToComputer(m_lNewPrice);     		// 最新价
			m_lTotal = YlsIntIntelToComputer(m_lTotal);				// 成交量(单位:股)
			m_fAvgPrice = YlsFloatComputerToIntel(m_fAvgPrice);			// 成交金额

			m_lBuyPrice1 = YlsIntIntelToComputer(m_lBuyPrice1);			// 买一价
			m_lBuyCount1 = YlsIntIntelToComputer(m_lBuyCount1);			// 买一量
			m_lBuyPrice2 = YlsIntIntelToComputer(m_lBuyPrice2);			// 买二价
			m_lBuyCount2 = YlsIntIntelToComputer(m_lBuyCount2);			// 买二量
			m_lBuyPrice3 = YlsIntIntelToComputer(m_lBuyPrice3);			// 买三价
			m_lBuyCount3 = YlsIntIntelToComputer(m_lBuyCount3);			// 买三量
			m_lBuyPrice4 = YlsIntIntelToComputer(m_lBuyPrice4);			// 买四价
			m_lBuyCount4 = YlsIntIntelToComputer(m_lBuyCount4);			// 买四量
			m_lBuyPrice5 = YlsIntIntelToComputer(m_lBuyPrice5);			// 买五价
			m_lBuyCount5 = YlsIntIntelToComputer(m_lBuyCount5);			// 买五量

			m_lSellPrice1 = YlsIntIntelToComputer(m_lSellPrice1);			// 卖一价
			m_lSellCount1 = YlsIntIntelToComputer(m_lSellCount1);			// 卖一量
			m_lSellPrice2 = YlsIntIntelToComputer(m_lSellPrice2);			// 卖二价
			m_lSellCount2 = YlsIntIntelToComputer(m_lSellCount2);			// 卖二量
			m_lSellPrice3 = YlsIntIntelToComputer(m_lSellPrice3);			// 卖三价
			m_lSellCount3 = YlsIntIntelToComputer(m_lSellCount3);			// 卖三量
			m_lSellPrice4 = YlsIntIntelToComputer(m_lSellPrice4);			// 卖四价
			m_lSellCount4 = YlsIntIntelToComputer(m_lSellCount4);			// 卖四量
			m_lSellPrice5 = YlsIntIntelToComputer(m_lSellPrice5);			// 卖五价
			m_lSellCount5 = YlsIntIntelToComputer(m_lSellCount5);			// 卖五量

			m_nHand = YlsIntIntelToComputer(m_nHand);					// 每手股数	(是否可放入代码表中？？？？）
			m_lNationalDebtRatio = YlsIntIntelToComputer(m_lNationalDebtRatio);	// 国债利率,基金净值
		}
		else
		{
			m_lOpen = YlsIntComputerToIntel(m_lOpen);         		// 今开盘
			m_lMaxPrice = YlsIntComputerToIntel(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntComputerToIntel(m_lMinPrice);     		// 最低价
			m_lNewPrice = YlsIntComputerToIntel(m_lNewPrice);     		// 最新价
			m_lTotal = YlsIntComputerToIntel(m_lTotal);				// 成交量(单位:股)
			m_fAvgPrice = YlsFloatComputerToIntel(m_fAvgPrice);			// 成交金额

			m_lBuyPrice1 = YlsIntComputerToIntel(m_lBuyPrice1);			// 买一价
			m_lBuyCount1 = YlsIntComputerToIntel(m_lBuyCount1);			// 买一量
			m_lBuyPrice2 = YlsIntComputerToIntel(m_lBuyPrice2);			// 买二价
			m_lBuyCount2 = YlsIntComputerToIntel(m_lBuyCount2);			// 买二量
			m_lBuyPrice3 = YlsIntComputerToIntel(m_lBuyPrice3);			// 买三价
			m_lBuyCount3 = YlsIntComputerToIntel(m_lBuyCount3);			// 买三量
			m_lBuyPrice4 = YlsIntComputerToIntel(m_lBuyPrice4);			// 买四价
			m_lBuyCount4 = YlsIntComputerToIntel(m_lBuyCount4);			// 买四量
			m_lBuyPrice5 = YlsIntComputerToIntel(m_lBuyPrice5);			// 买五价
			m_lBuyCount5 = YlsIntComputerToIntel(m_lBuyCount5);			// 买五量

			m_lSellPrice1 = YlsIntComputerToIntel(m_lSellPrice1);			// 卖一价
			m_lSellCount1 = YlsIntComputerToIntel(m_lSellCount1);			// 卖一量
			m_lSellPrice2 = YlsIntComputerToIntel(m_lSellPrice2);			// 卖二价
			m_lSellCount2 = YlsIntComputerToIntel(m_lSellCount2);			// 卖二量
			m_lSellPrice3 = YlsIntComputerToIntel(m_lSellPrice3);			// 卖三价
			m_lSellCount3 = YlsIntComputerToIntel(m_lSellCount3);			// 卖三量
			m_lSellPrice4 = YlsIntComputerToIntel(m_lSellPrice4);			// 卖四价
			m_lSellCount4 = YlsIntComputerToIntel(m_lSellCount4);			// 卖四量
			m_lSellPrice5 = YlsIntComputerToIntel(m_lSellPrice5);			// 卖五价
			m_lSellCount5 = YlsIntComputerToIntel(m_lSellCount5);			// 卖五量

			m_nHand = YlsIntComputerToIntel(m_nHand);					// 每手股数	(是否可放入代码表中？？？？）
			m_lNationalDebtRatio = YlsIntComputerToIntel(m_lNationalDebtRatio);	// 国债利率,基金净值
		}		
#endif
	}
};  
// 原28*4 = 112

// 指标类实时数据
struct HSIndexRealTime  
{
	long		m_lOpen;				// 今开盘
	long		m_lMaxPrice;			// 最高价
	long		m_lMinPrice;			// 最低价
	long		m_lNewPrice;			// 最新价
	unsigned long		m_lTotal;				// 成交量
	float		m_fAvgPrice;			// 成交金额

	short		m_nRiseCount;			// 上涨家数
	short		m_nFallCount;			// 下跌家数
	long		m_nTotalStock1;			/* 对于综合指数：所有股票 - 指数
										对于分类指数：本类股票总数 */
	unsigned long		m_lBuyCount;			// 委买数
	unsigned long		m_lSellCount;			// 委卖数
	short		m_nType;				// 指数种类：0-综合指数 1-A股 2-B股
	short		m_nLead;            	// 领先指标
	short		m_nRiseTrend;       	// 上涨趋势
	short		m_nFallTrend;       	// 下跌趋势
	short		m_nNo2[5];				// 保留
	short		m_nTotalStock2;			/* 对于综合指数：A股 + B股 
										对于分类指数：0 */
	long		m_lADL;					// ADL 指标
	long		m_lNo3[3];				// 保留
	long		m_nHand;				// 每手股数	

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		int i;

		if( cIntelToComputer )
		{
			m_lOpen = YlsIntIntelToComputer(m_lOpen);         		// 今开盘
			m_lMaxPrice = YlsIntIntelToComputer(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntIntelToComputer(m_lMinPrice);     		// 最低价
			m_lNewPrice = YlsIntIntelToComputer(m_lNewPrice);     		// 最新价
			m_lTotal = YlsIntIntelToComputer(m_lTotal);				// 成交量(单位:股)
			m_fAvgPrice = YlsFloatComputerToIntel(m_fAvgPrice);			// 成交金额

			m_nRiseCount = YlsShortIntelToComputer(m_nRiseCount);			// 上涨家数
			m_nFallCount = YlsShortIntelToComputer(m_nFallCount);			// 下跌家数
			m_nTotalStock1 = YlsIntIntelToComputer(m_nTotalStock1);			/* 对于综合指数：所有股票 - 指数
																			对于分类指数：本类股票总数 */
			m_lBuyCount = YlsIntIntelToComputer(m_lBuyCount);			    // 委买数
			m_lSellCount = YlsIntIntelToComputer(m_lSellCount);			// 委卖数
			m_nType = YlsShortIntelToComputer(m_nType);				// 指数种类：0-综合指数 1-A股 2-B股
			m_nLead = YlsShortIntelToComputer(m_nLead);            	// 领先指标
			m_nRiseTrend = YlsShortIntelToComputer(m_nRiseTrend);       	// 上涨趋势
			m_nFallTrend = YlsShortIntelToComputer(m_nFallTrend);       	// 下跌趋势
			for( i = 0; i < 5; i++ )
				m_nNo2[5] = YlsShortIntelToComputer(m_nNo2[5]);			// 保留
			m_nTotalStock2 = YlsShortIntelToComputer(m_nTotalStock2);		/* 对于综合指数：A股 + B股 
																			对于分类指数：0 */
			m_lADL = YlsIntIntelToComputer(m_lADL);					// ADL 指标
			for( i = 0; i < 3; i++ )
				m_lNo3[i] = YlsIntIntelToComputer(m_lNo3[i]);			// 保留
			m_nHand = YlsIntIntelToComputer(m_nHand);					// 每手股数
		}
		else
		{
			m_lOpen = YlsIntComputerToIntel(m_lOpen);         		// 今开盘
			m_lMaxPrice = YlsIntComputerToIntel(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntComputerToIntel(m_lMinPrice);     		// 最低价
			m_lNewPrice = YlsIntComputerToIntel(m_lNewPrice);     		// 最新价
			m_lTotal = YlsIntComputerToIntel(m_lTotal);				// 成交量(单位:股)
			m_fAvgPrice = YlsFloatComputerToIntel(m_fAvgPrice);			// 成交金额

			m_nRiseCount = YlsShortComputerToIntel(m_nRiseCount);			// 上涨家数
			m_nFallCount = YlsShortComputerToIntel(m_nFallCount);			// 下跌家数
			m_nTotalStock1 = YlsIntComputerToIntel(m_nTotalStock1);			/* 对于综合指数：所有股票 - 指数
																			对于分类指数：本类股票总数 */
			m_lBuyCount = YlsIntComputerToIntel(m_lBuyCount);			    // 委买数
			m_lSellCount = YlsIntComputerToIntel(m_lSellCount);			// 委卖数
			m_nType = YlsShortComputerToIntel(m_nType);				// 指数种类：0-综合指数 1-A股 2-B股
			m_nLead = YlsShortComputerToIntel(m_nLead);            	// 领先指标
			m_nRiseTrend = YlsShortComputerToIntel(m_nRiseTrend);       	// 上涨趋势
			m_nFallTrend = YlsShortComputerToIntel(m_nFallTrend);       	// 下跌趋势
			for( i = 0; i < 5; i++ )
				m_nNo2[5] = YlsShortComputerToIntel(m_nNo2[5]);			// 保留
			m_nTotalStock2 = YlsShortComputerToIntel(m_nTotalStock2);		/* 对于综合指数：A股 + B股 
																			对于分类指数：0 */
			m_lADL = YlsIntComputerToIntel(m_lADL);					// ADL 指标
			for( i = 0; i < 3; i++ )
				m_lNo3[i] = YlsIntComputerToIntel(m_lNo3[i]);			// 保留
			m_nHand = YlsIntComputerToIntel(m_nHand);					// 每手股数
		}		
#endif
	}
};  

// 港股实时 29*4 = 116 或  26*4 = 104（价变成short)
struct HSHKStockRealTime 
{
	long		m_lOpen;         		// 今开盘
	long		m_lMaxPrice;     		// 最高价
	long		m_lMinPrice;     		// 最低价
	long		m_lNewPrice;     		// 最新价

	unsigned long		m_lTotal;				// 成交量（股）	
	float		m_fAvgPrice;			// 成交金额(元)

	long		m_lBuyPrice;			// 买价
	long		m_lSellPrice;			// 卖价
	union
	{
		long		m_lYield;		// 周息率 股票相关
		long		m_lOverFlowPrice;	// 溢价% 认股证相关
		// 认购证的溢价＝（认股证现价×兑换比率＋行使价－相关资产现价）/相关资产现价×100
		// 认沽证的溢价＝（认股证现价×兑换比率－行使价＋相关资产现价）/相关资产现价×100

	};

	long		m_lBuyCount1;			// 买一量
	long		m_lBuyCount2;			// 买二量
	long		m_lBuyCount3;			// 买三量
	long		m_lBuyCount4;			// 买四量
	long		m_lBuyCount5;			// 买五量

	long		m_lSellCount1;			// 卖一量
	long		m_lSellCount2;			// 卖二量
	long		m_lSellCount3;			// 卖三量
	long		m_lSellCount4;			// 卖四量
	long		m_lSellCount5;			// 卖五量

	unsigned short		m_lSellOrder1;	// 卖一盘数
	unsigned short		m_lSellOrder2;	// 卖二盘数
	unsigned short		m_lSellOrder3;	// 卖三盘数
	unsigned short		m_lSellOrder4;	// 卖四盘数
	unsigned short		m_lSellOrder5;	// 卖五盘数

	unsigned short		m_lBuyOrder1;	// 买一盘数
	unsigned short		m_lBuyOrder2;	// 买二盘数
	unsigned short		m_lBuyOrder3;	// 买三盘数
	unsigned short		m_lBuyOrder4;	// 买四盘数
	unsigned short		m_lBuyOrder5;	// 买五盘数

	long		m_lIEP;					// 参考平衡价
	long		m_lIEV;					// 参考平衡量

	// 主推分笔当前成交对盘类型？？？？
	long		m_lMatchType;			// 对盘分类

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_lOpen = YlsIntIntelToComputer(m_lOpen);         		// 今开盘
			m_lMaxPrice = YlsIntIntelToComputer(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntIntelToComputer(m_lMinPrice);     		// 最低价
			m_lNewPrice = YlsIntIntelToComputer(m_lNewPrice);     		// 最新价
			m_lTotal = YlsIntIntelToComputer(m_lTotal);				// 成交量(单位:股)
			m_fAvgPrice = YlsFloatComputerToIntel(m_fAvgPrice);			// 成交金额

			m_lBuyPrice = YlsIntIntelToComputer(m_lBuyPrice);			// 买价
			m_lSellPrice = YlsIntIntelToComputer(m_lSellPrice);			// 卖价
			m_lYield = YlsIntIntelToComputer(m_lYield);		// 周息率 股票相关

			m_lBuyCount1 = YlsIntIntelToComputer(m_lBuyCount1);			// 买一量
			m_lBuyCount2 = YlsIntIntelToComputer(m_lBuyCount2);			// 买二量
			m_lBuyCount3 = YlsIntIntelToComputer(m_lBuyCount3);			// 买三量
			m_lBuyCount4 = YlsIntIntelToComputer(m_lBuyCount4);			// 买四量
			m_lBuyCount5 = YlsIntIntelToComputer(m_lBuyCount5);			// 买五量

			m_lSellCount1 = YlsIntIntelToComputer(m_lSellCount1);			// 卖一量
			m_lSellCount2 = YlsIntIntelToComputer(m_lSellCount2);			// 卖二量
			m_lSellCount3 = YlsIntIntelToComputer(m_lSellCount3);			// 卖三量
			m_lSellCount4 = YlsIntIntelToComputer(m_lSellCount4);			// 卖四量
			m_lSellCount5 = YlsIntIntelToComputer(m_lSellCount5);			// 卖五量

			m_lSellOrder1 = YlsShortIntelToComputer(m_lSellOrder1);	// 卖一盘数
			m_lSellOrder2 = YlsShortIntelToComputer(m_lSellOrder2);	// 卖二盘数
			m_lSellOrder3 = YlsShortIntelToComputer(m_lSellOrder3);	// 卖三盘数
			m_lSellOrder4 = YlsShortIntelToComputer(m_lSellOrder4);	// 卖四盘数
			m_lSellOrder5 = YlsShortIntelToComputer(m_lSellOrder5);	// 卖五盘数

			m_lBuyOrder1 = YlsShortIntelToComputer(m_lBuyOrder1);	// 买一盘数
			m_lBuyOrder2 = YlsShortIntelToComputer(m_lBuyOrder2);	// 买二盘数
			m_lBuyOrder3 = YlsShortIntelToComputer(m_lBuyOrder3);	// 买三盘数
			m_lBuyOrder4 = YlsShortIntelToComputer(m_lBuyOrder4);	// 买四盘数
			m_lBuyOrder5 = YlsShortIntelToComputer(m_lBuyOrder5);	// 买五盘数

			m_lIEP = YlsIntIntelToComputer(m_lIEP);					// 参考平衡价
			m_lIEV = YlsIntIntelToComputer(m_lIEV);					// 参考平衡量

			// 主推分笔当前成交对盘类型？？？？
			m_lMatchType = YlsIntIntelToComputer(m_lMatchType);			// 对盘分类
		}
		else
		{
			m_lOpen = YlsIntComputerToIntel(m_lOpen);         		// 今开盘
			m_lMaxPrice = YlsIntComputerToIntel(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntComputerToIntel(m_lMinPrice);     		// 最低价
			m_lNewPrice = YlsIntComputerToIntel(m_lNewPrice);     		// 最新价
			m_lTotal = YlsIntComputerToIntel(m_lTotal);				// 成交量(单位:股)
			m_fAvgPrice = YlsFloatComputerToIntel(m_fAvgPrice);			// 成交金额

			m_lBuyPrice = YlsIntComputerToIntel(m_lBuyPrice);			// 买价
			m_lSellPrice = YlsIntComputerToIntel(m_lSellPrice);			// 卖价
			m_lYield = YlsIntComputerToIntel(m_lYield);		// 周息率 股票相关

			m_lBuyCount1 = YlsIntComputerToIntel(m_lBuyCount1);			// 买一量
			m_lBuyCount2 = YlsIntComputerToIntel(m_lBuyCount2);			// 买二量
			m_lBuyCount3 = YlsIntComputerToIntel(m_lBuyCount3);			// 买三量
			m_lBuyCount4 = YlsIntComputerToIntel(m_lBuyCount4);			// 买四量
			m_lBuyCount5 = YlsIntComputerToIntel(m_lBuyCount5);			// 买五量

			m_lSellCount1 = YlsIntComputerToIntel(m_lSellCount1);			// 卖一量
			m_lSellCount2 = YlsIntComputerToIntel(m_lSellCount2);			// 卖二量
			m_lSellCount3 = YlsIntComputerToIntel(m_lSellCount3);			// 卖三量
			m_lSellCount4 = YlsIntComputerToIntel(m_lSellCount4);			// 卖四量
			m_lSellCount5 = YlsIntComputerToIntel(m_lSellCount5);			// 卖五量

			m_lSellOrder1 = YlsShortComputerToIntel(m_lSellOrder1);	// 卖一盘数
			m_lSellOrder2 = YlsShortComputerToIntel(m_lSellOrder2);	// 卖二盘数
			m_lSellOrder3 = YlsShortComputerToIntel(m_lSellOrder3);	// 卖三盘数
			m_lSellOrder4 = YlsShortComputerToIntel(m_lSellOrder4);	// 卖四盘数
			m_lSellOrder5 = YlsShortComputerToIntel(m_lSellOrder5);	// 卖五盘数

			m_lBuyOrder1 = YlsShortComputerToIntel(m_lBuyOrder1);	// 买一盘数
			m_lBuyOrder2 = YlsShortComputerToIntel(m_lBuyOrder2);	// 买二盘数
			m_lBuyOrder3 = YlsShortComputerToIntel(m_lBuyOrder3);	// 买三盘数
			m_lBuyOrder4 = YlsShortComputerToIntel(m_lBuyOrder4);	// 买四盘数
			m_lBuyOrder5 = YlsShortComputerToIntel(m_lBuyOrder5);	// 买五盘数

			m_lIEP = YlsIntComputerToIntel(m_lIEP);					// 参考平衡价
			m_lIEV = YlsIntComputerToIntel(m_lIEV);					// 参考平衡量

			// 主推分笔当前成交对盘类型？？？？
			m_lMatchType = YlsIntComputerToIntel(m_lMatchType);			// 对盘分类
		}		
#endif
	}
}; 

// 港股期权
struct HSQQStockRealTime 
{
	long				m_Month;				// 年月日,19990101

	char				m_Symbol[10];			// 简称
	char				m_OptionType;			// 期权种类：’P’(卖出)；‘C’（买入）

	float				m_StrikePrice;		    // 行使价

	float				m_lOpen;         		// 今开盘
	float				m_lMaxPrice;     		// 最高价
	float				m_lMinPrice;     		// 最低价
	float				m_lNewPrice;     		// 最新价

	float				m_lTotal;				// 成交量（股）	
	float				m_fAvgPrice;			// 成交金额(元)

	float				m_lBuyPrice;			// 买价
	float				m_lSellPrice;			// 卖价

	float				m_BidVol;				// 买入量
	float				m_AskVol;				// 卖出量

	float				m_YClose;				// 前日收盘价

	char				m_cReserve[20];			// 保留
};

/*
// 期货实时数据
struct HSQHRealTime 
{
long		m_lOpen;         	// 今开盘
long		m_lMaxPrice;     	// 最高价
long		m_lMinPrice;     	// 最低价
long		m_lNewPrice;     	// 最新价

unsigned long		m_lTotal;		   	// 成交量(单位:合约单位)
long				m_lChiCangLiang;    // 持仓量(单位:合约单位)

long		m_lBuyPrice1;		// 买一价
long		m_lBuyCount1;		// 买一量
long		m_lSellPrice1;		// 卖一价
long		m_lSellCount1;		// 卖一量

long		m_lPreJieSuanPrice; // 昨结算价

union
{
struct
{
long				m_lBuyPrice2;			// 买二价
unsigned short		m_lBuyCount2;			// 买二量
long				m_lBuyPrice3;			// 买三价
unsigned short		m_lBuyCount3;			// 买三量
long				m_lBuyPrice4;			// 买四价
unsigned short		m_lBuyCount4;			// 买四量
long				m_lBuyPrice5;			// 买五价
unsigned short		m_lBuyCount5;			// 买五量

long				m_lSellPrice2;			// 卖二价
unsigned short		m_lSellCount2;			// 卖二量
long				m_lSellPrice3;			// 卖三价
unsigned short		m_lSellCount3;			// 卖三量
long				m_lSellPrice4;			// 卖四价
unsigned short		m_lSellCount4;			// 卖四量
long				m_lSellPrice5;			// 卖五价
unsigned short		m_lSellCount5;			// 卖五量
};

struct
{
long		m_lJieSuanPrice;    // 现结算价
long		m_lCurrentCLOSE;	// 今收盘
long		m_lHIS_HIGH;		// 史最高
long		m_lHIS_LOW;	 		// 史最低
long		m_lUPPER_LIM;		// 涨停板
long		m_lLOWER_LIM;		// 跌停板

long 		m_lLongPositionOpen;	// 多头开(单位:合约单位)
long 		m_lLongPositionFlat;	// 多头平(单位:合约单位)
long 		m_lNominalOpen;			// 空头开(单位:合约单位)	
long 		m_lNominalFlat;			// 空头平(单位:合约单位)

long		m_lPreClose;			// 前天收盘????
};
};

long		m_nHand;				// 每手股数
long 		m_lPreCloseChiCang;		// 昨持仓量(单位:合约单位)
};
*/

// 期货、外盘实时数据
struct HSQHRealTime 
{
	long		m_lOpen;         	// 今开盘
	long		m_lMaxPrice;     	// 最高价
	long		m_lMinPrice;     	// 最低价
	long		m_lNewPrice;     	// 最新价

	unsigned long		m_lTotal;		   	// 成交量(单位:合约单位)
	long				m_lChiCangLiang;    // 持仓量(单位:合约单位)

	long		m_lPreJieSuanPrice; // 昨结算价

	//long		m_nHand;				// 每手股数
	//long 		m_lPreCloseChiCang;		// 昨持仓量(单位:合约单位)


	long		m_lJieSuanPrice;    // 现结算价
	long		m_lCurrentCLOSE;	// 今收盘
	long		m_lHIS_HIGH;		// 史最高
	long		m_lHIS_LOW;	 		// 史最低
	long		m_lUPPER_LIM;		// 涨停板
	long		m_lLOWER_LIM;		// 跌停板

	long		m_nHand;				// 每手股数
	long 		m_lPreCloseChiCang;		// 昨持仓量(单位:合约单位)

	long 		m_lLongPositionOpen;	// 多头开(单位:合约单位)
	long 		m_lLongPositionFlat;	// 多头平(单位:合约单位)
	long 		m_lNominalOpen;			// 空头开(单位:合约单位)	
	long 		m_lNominalFlat;			// 空头平(单位:合约单位)

	long		m_lPreClose;			// 前天收盘????
	long        m_lAvgPrice;            // 成交金额  added by Ben 20100506

	// modified by Ben 20100506
	long				m_lBuyPrice1;			// 买一价
	unsigned long		m_lBuyCount1;			// 买一量
	long				m_lBuyPrice2;			// 买二价
	unsigned long		m_lBuyCount2;			// 买二量
	long				m_lBuyPrice3;			// 买三价
	unsigned long		m_lBuyCount3;			// 买三量
	long				m_lBuyPrice4;			// 买四价
	unsigned long		m_lBuyCount4;			// 买四量
	long				m_lBuyPrice5;			// 买五价
	unsigned long		m_lBuyCount5;			// 买五量

	long				m_lSellPrice1;			// 卖一价
	unsigned long		m_lSellCount1;			// 卖一量
	long				m_lSellPrice2;			// 卖二价
	unsigned long		m_lSellCount2;			// 卖二量
	long				m_lSellPrice3;			// 卖三价
	unsigned long		m_lSellCount3;			// 卖三量
	long				m_lSellPrice4;			// 卖四价
	unsigned long		m_lSellCount4;			// 卖四量
	long				m_lSellPrice5;			// 卖五价
	unsigned long		m_lSellCount5;			// 卖五量


	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_lOpen = YlsIntIntelToComputer(m_lOpen);         		// 今开盘
			m_lMaxPrice = YlsIntIntelToComputer(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntIntelToComputer(m_lMinPrice);     		// 最低价
			m_lNewPrice = YlsIntIntelToComputer(m_lNewPrice);     		// 最新价
			m_lTotal = YlsIntIntelToComputer(m_lTotal);				// 成交量(单位:股)
			m_lChiCangLiang = YlsIntComputerToIntel(m_lChiCangLiang);			// 成交金额

			m_lPreJieSuanPrice = YlsIntComputerToIntel(m_lPreJieSuanPrice); // 昨结算价

			//long		m_nHand;				// 每手股数
			//long 		m_lPreCloseChiCang;		// 昨持仓量(单位:合约单位)

			m_lJieSuanPrice = YlsIntComputerToIntel(m_lJieSuanPrice);    // 现结算价
			m_lCurrentCLOSE = YlsIntComputerToIntel(m_lCurrentCLOSE);	// 今收盘
			m_lHIS_HIGH = YlsIntComputerToIntel(m_lHIS_HIGH);		// 史最高
			m_lHIS_LOW = YlsIntComputerToIntel(m_lHIS_LOW);	 		// 史最低
			m_lUPPER_LIM = YlsIntComputerToIntel(m_lUPPER_LIM);		// 涨停板
			m_lLOWER_LIM = YlsIntComputerToIntel(m_lLOWER_LIM);		// 跌停板

			m_nHand = YlsIntComputerToIntel(m_nHand);				// 每手股数
			m_lPreCloseChiCang = YlsIntIntelToComputer(m_lPreCloseChiCang);		// 昨持仓量(单位:合约单位)

			m_lLongPositionOpen = YlsIntIntelToComputer(m_lLongPositionOpen);	// 多头开(单位:合约单位)
			m_lLongPositionFlat = YlsIntIntelToComputer(m_lLongPositionFlat);	// 多头平(单位:合约单位)
			m_lNominalOpen = YlsIntIntelToComputer(m_lNominalOpen);			// 空头开(单位:合约单位)	
			m_lNominalFlat = YlsIntIntelToComputer(m_lNominalFlat);			// 空头平(单位:合约单位)

			m_lPreClose = YlsIntIntelToComputer(m_lPreClose);			// 前天收盘????
			m_lAvgPrice = YlsIntIntelToComputer(m_lAvgPrice);



			m_lBuyPrice1 = YlsIntIntelToComputer(m_lBuyPrice1);			// 买一价
			m_lBuyCount1 = YlsIntIntelToComputer(m_lBuyCount1);			// 买一量
			m_lBuyPrice2 = YlsIntIntelToComputer(m_lBuyPrice2);			// 买二价
			m_lBuyCount2 = YlsIntIntelToComputer(m_lBuyCount2);			// 买二量
			m_lBuyPrice3 = YlsIntIntelToComputer(m_lBuyPrice3);			// 买三价
			m_lBuyCount3 = YlsIntIntelToComputer(m_lBuyCount3);			// 买三量
			m_lBuyPrice4 = YlsIntIntelToComputer(m_lBuyPrice4);			// 买四价
			m_lBuyCount4 = YlsIntIntelToComputer(m_lBuyCount4);			// 买四量
			m_lBuyPrice5 = YlsIntIntelToComputer(m_lBuyPrice5);			// 买五价
			m_lBuyCount5 = YlsIntIntelToComputer(m_lBuyCount5);			// 买五量

			m_lSellPrice1 = YlsIntIntelToComputer(m_lSellPrice1);			// 卖一价
			m_lSellCount1 = YlsIntIntelToComputer(m_lSellCount1);			// 卖一量
			m_lSellPrice2 = YlsIntIntelToComputer(m_lSellPrice2);			// 卖二价
			m_lSellCount2 = YlsIntIntelToComputer(m_lSellCount2);			// 卖二量
			m_lSellPrice3 = YlsIntIntelToComputer(m_lSellPrice3);			// 卖三价
			m_lSellCount3 = YlsIntIntelToComputer(m_lSellCount3);			// 卖三量
			m_lSellPrice4 = YlsIntIntelToComputer(m_lSellPrice4);			// 卖四价
			m_lSellCount4 = YlsIntIntelToComputer(m_lSellCount4);			// 卖四量
			m_lSellPrice5 = YlsIntIntelToComputer(m_lSellPrice5);			// 卖五价
			m_lSellCount5 = YlsIntIntelToComputer(m_lSellCount5);			// 卖五量

		}
		else
		{
			m_lOpen = YlsIntComputerToIntel(m_lOpen);         		// 今开盘
			m_lMaxPrice = YlsIntComputerToIntel(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntComputerToIntel(m_lMinPrice);     		// 最低价
			m_lNewPrice = YlsIntComputerToIntel(m_lNewPrice);     		// 最新价
			m_lTotal = YlsIntComputerToIntel(m_lTotal);				// 成交量(单位:股)
			m_lChiCangLiang = YlsIntComputerToIntel(m_lChiCangLiang);			// 成交金额

			m_lPreJieSuanPrice = YlsIntComputerToIntel(m_lPreJieSuanPrice); // 昨结算价

			//long		m_nHand;				// 每手股数
			//long 		m_lPreCloseChiCang;		// 昨持仓量(单位:合约单位)

			m_lJieSuanPrice = YlsIntComputerToIntel(m_lJieSuanPrice);    // 现结算价
			m_lCurrentCLOSE = YlsIntComputerToIntel(m_lCurrentCLOSE);	// 今收盘
			m_lHIS_HIGH = YlsIntComputerToIntel(m_lHIS_HIGH);		// 史最高
			m_lHIS_LOW = YlsIntComputerToIntel(m_lHIS_LOW);	 		// 史最低
			m_lUPPER_LIM = YlsIntComputerToIntel(m_lUPPER_LIM);		// 涨停板
			m_lLOWER_LIM = YlsIntComputerToIntel(m_lLOWER_LIM);		// 跌停板

			m_nHand = YlsIntComputerToIntel(m_nHand);				// 每手股数
			m_lPreCloseChiCang = YlsIntComputerToIntel(m_lPreCloseChiCang);		// 昨持仓量(单位:合约单位)

			m_lLongPositionOpen = YlsIntComputerToIntel(m_lLongPositionOpen);	// 多头开(单位:合约单位)
			m_lLongPositionFlat = YlsIntComputerToIntel(m_lLongPositionFlat);	// 多头平(单位:合约单位)
			m_lNominalOpen = YlsIntComputerToIntel(m_lNominalOpen);			// 空头开(单位:合约单位)	
			m_lNominalFlat = YlsIntComputerToIntel(m_lNominalFlat);			// 空头平(单位:合约单位)

			m_lPreClose = YlsIntComputerToIntel(m_lPreClose);			// 前天收盘????
			m_lAvgPrice = YlsIntComputerToIntel(m_lAvgPrice);			// 前天收盘????


			m_lBuyPrice1 = YlsIntComputerToIntel(m_lBuyPrice1);			// 买一价
			m_lBuyCount1 = YlsIntComputerToIntel(m_lBuyCount1);			// 买一量
			m_lBuyPrice2 = YlsIntComputerToIntel(m_lBuyPrice2);			// 买二价
			m_lBuyCount2 = YlsIntComputerToIntel(m_lBuyCount2);			// 买二量
			m_lBuyPrice3 = YlsIntComputerToIntel(m_lBuyPrice3);			// 买三价
			m_lBuyCount3 = YlsIntComputerToIntel(m_lBuyCount3);			// 买三量
			m_lBuyPrice4 = YlsIntComputerToIntel(m_lBuyPrice4);			// 买四价
			m_lBuyCount4 = YlsIntComputerToIntel(m_lBuyCount4);			// 买四量
			m_lBuyPrice5 = YlsIntComputerToIntel(m_lBuyPrice5);			// 买五价
			m_lBuyCount5 = YlsIntComputerToIntel(m_lBuyCount5);			// 买五量

			m_lSellPrice1 = YlsIntComputerToIntel(m_lSellPrice1);			// 卖一价
			m_lSellCount1 = YlsIntComputerToIntel(m_lSellCount1);			// 卖一量
			m_lSellPrice2 = YlsIntComputerToIntel(m_lSellPrice2);			// 卖二价
			m_lSellCount2 = YlsIntComputerToIntel(m_lSellCount2);			// 卖二量
			m_lSellPrice3 = YlsIntComputerToIntel(m_lSellPrice3);			// 卖三价
			m_lSellCount3 = YlsIntComputerToIntel(m_lSellCount3);			// 卖三量
			m_lSellPrice4 = YlsIntComputerToIntel(m_lSellPrice4);			// 卖四价
			m_lSellCount4 = YlsIntComputerToIntel(m_lSellCount4);			// 卖四量
			m_lSellPrice5 = YlsIntComputerToIntel(m_lSellPrice5);			// 卖五价
			m_lSellCount5 = YlsIntComputerToIntel(m_lSellCount5);			// 卖五量

		}		
#endif
	}
};


// 外汇中心债券
struct HSWHCenterRealTime
{	
	long		m_lOpen;         			// 开盘价	->开盘价
	long		m_lMaxPrice;     			// 最高价	->最高价
	long		m_lMinPrice;     			// 最低价	->最低价
	long		m_lNewPrice;     			// 最新价	->最新价
	unsigned long		m_lTotal;		   	// 成交量(单位:合约单位)	->成交量

	long		m_lChiCangLiang;			// 持仓量(单位:合约单位)	->收盘价

	long		m_lBuyPrice1;				// 买一价	->买盘1
	long		m_lBuyCount1;				// 买一量	->最高收益率
	long		m_lSellPrice1;				// 卖一价	->卖盘1
	long		m_lSellCount1;				// 卖一量	->最低收益率

	long		m_lPreJieSuanPrice;			// 昨结算价 ->开盘收益率
	long		m_lJieSuanPrice;			// 今结算	->前收盘价
	long		m_lCurrentCLOSE;			// 今收盘	->最新收益率
	long		m_lHIS_HIGH;				// 史最高	->加权价
	long		m_lHIS_LOW;	 				// 史最低	->加权收益率
	long		m_lUPPER_LIM;				// 涨停板	->前收盘收益率
	long		m_lLOWER_LIM;				// 跌停板	->前加权收益率
	
	long		m_nHand;					// 每手股数

	long 		m_lPreCloseChiCang;			// 昨持仓量(单位:合约单位) ->收盘收益率
	long 		m_lLongPositionOpen;		// 多头开(单位:合约单位)   ->时间
	long 		m_lLongPositionFlat;		// 多头平(单位:合约单位)   ->报价方编号
	long 		m_lNominalOpen;				// 空头开(单位:合约单位)	
	long 		m_lNominalFlat;				// 平均价				   ->前加权价
	long		m_lPreClose;				// 前天收盘????
};

// 期货、外盘主推实时数据(精简)
struct HSQHRealTime_Min
{
	long		m_lOpen;         	// 今开盘
	long		m_lMaxPrice;     	// 最高价
	long		m_lMinPrice;     	// 最低价
	long		m_lNewPrice;     	// 最新价

	unsigned long		m_lTotal;		   	// 成交量(单位:合约单位)
	long				m_lChiCangLiang;    // 持仓量(单位:合约单位)

	long		m_lBuyPrice1;		// 买一价
	long		m_lBuyCount1;		// 买一量
	long		m_lSellPrice1;		// 卖一价
	long		m_lSellCount1;		// 卖一量

	long		m_lPreJieSuanPrice; // 昨结算价
};


// data struct in file lonhis.*
struct StockDetailedMinuteData
{
	unsigned long	m_nTotalHold;	   //期货总持	
	unsigned long	m_lOutside;        //外盘成交量
	short			m_nReserved[4];    //保留
};
//大盘分时数据
struct IndexDetailedMinuteData
{
	short	m_nRiseCount;       //上涨家数
	short	m_nFallCount;       //下跌家数
	short	m_nLead;            //领先指标
	short	m_nRiseTrend;       //上涨趋势
	short	m_nFallTrend;       //下跌趋势
	short   m_nReserved;        //4子节对齐，不用
	long	m_lADL;             //ADL指标
};

//个股分时
struct HSHistoryData
{
	time_t	m_time;				// UCT
	float	m_fPrice;
	float	m_fVolume;
	float	m_fAmount;
};

// 外汇实时数据
struct HSWHRealTime 
{
	long		m_lOpen;         	// 今开盘(1/10000元)
	long		m_lMaxPrice;     	// 最高价(1/10000元)
	long		m_lMinPrice;     	// 最低价(1/10000元)
	long		m_lNewPrice;     	// 最新价(1/10000元)

	long		m_lBuyPrice;		// 买价(1/10000元)
	long		m_lSellPrice;		// 卖价(1/10000元)

	// added by Ben 20100329
	/*long        m_lPrevClose;
	long        m_lVolume;
	long        m_lAmount;*/
	// end add

	//long		m_lPrevClose;	    // 昨天收盘
	//long		m_lPriceTimes;	    // 跳动量，一分钟价格变化次数

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{			
			m_lOpen = YlsIntIntelToComputer(m_lOpen);         		// 今开盘
			m_lMaxPrice	= YlsIntIntelToComputer(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntIntelToComputer(m_lMinPrice);     		// 最低价
			m_lNewPrice	= YlsIntIntelToComputer(m_lNewPrice);     		// 最新价
			m_lBuyPrice	= YlsIntIntelToComputer(m_lBuyPrice);				// 成交量(单位:股)
			m_lSellPrice = YlsIntIntelToComputer(m_lSellPrice);			// 成交金额
		}
		else
		{
			m_lOpen = YlsIntComputerToIntel(m_lOpen);         		// 今开盘
			m_lMaxPrice	= YlsIntComputerToIntel(m_lMaxPrice);     		// 最高价
			m_lMinPrice = YlsIntComputerToIntel(m_lMinPrice);     		// 最低价
			m_lNewPrice	= YlsIntComputerToIntel(m_lNewPrice);     		// 最新价
			m_lBuyPrice	= YlsIntComputerToIntel(m_lBuyPrice);				// 成交量(单位:股)
			m_lSellPrice = YlsIntComputerToIntel(m_lSellPrice);			// 成交金额
		}		
#endif
	}

};

// 指标类NOW数据
struct IndexRealTime : public HSIndexRealTime
{
	unsigned long  m_lOutside;    // 外盘成交量
};


// 港股对盘分类定义
#define HSHK_OrderType_A			'A' // 自动对盘非两边客交易
#define HSHK_OrderType_M			'M' // 非"自动对盘或特别买卖单位"的非两边客交易
#define HSHK_OrderType_X			'X' // 非"自动对盘或特别买卖单位"的两边客交易
#define HSHK_OrderType_Y			'Y' // 自动对盘两边客交易
#define HSHK_OrderType_D			'D' // 碎股成交
#define HSHK_OrderType_P			'P' // 开市前成交
#define HSHK_OrderType_U			'U' // 开市前竟价盘成交
#define HSHK_OrderType_N			'*' // 被取消的成交

struct StockTickDetailTime
{
	char m_nBuyOrSell;
	char m_nSecond;
};

// 分笔记录
struct StockTick
{
	short		   m_nTime;			   // 当前时间（距开盘分钟数）

	union
	{

		short			    m_nBuyOrSellOld;	 // 旧的，保留
		char			    m_nBuyOrSell;	     // 是按价成交还是按卖价成交(1 按买价 0 按卖价)
		StockTickDetailTime m_sDetailTime;       // 包含秒数据
	};

	long		   m_lNewPrice;        // 成交价
	unsigned long  m_lCurrent;		   // 成交量

	long		   m_lBuyPrice;        // 委买价
	long		   m_lSellPrice;       // 委卖价

	//
	unsigned long  m_nChiCangLiang;	   // 持仓量,深交所股票单笔成交数,港股成交盘分类(Y,M,X等，根据数据源再确定）

	//long  m_lOutside;         // 外盘
	//long  m_lInside;          // 内盘

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_nTime = YlsShortIntelToComputer(m_nTime);
			//m_nBuyOrSell = YlsShortIntelToComputer(m_nBuyOrSell);

			m_lNewPrice = YlsIntIntelToComputer(m_lNewPrice);
			m_lCurrent = YlsIntIntelToComputer(m_lCurrent);

			m_lBuyPrice = YlsIntIntelToComputer(m_lBuyPrice);
			m_lSellPrice = YlsIntIntelToComputer(m_lSellPrice);

			m_nChiCangLiang = YlsIntIntelToComputer(m_nChiCangLiang);
		}
		else
		{
			m_nTime = YlsShortComputerToIntel(m_nTime);
			//m_nBuyOrSell = YlsShortComputerToIntel(m_nBuyOrSell);

			m_lNewPrice = YlsIntComputerToIntel(m_lNewPrice);
			m_lCurrent = YlsIntComputerToIntel(m_lCurrent);

			m_lBuyPrice = YlsIntComputerToIntel(m_lBuyPrice);
			m_lSellPrice = YlsIntComputerToIntel(m_lSellPrice);

			m_nChiCangLiang = YlsIntComputerToIntel(m_nChiCangLiang);
		}
#endif
	}
};

//历史分时1分钟数据
struct StockCompHistoryData
{
	long	m_lNewPrice;		// 最新价
	unsigned long	m_lTotal;			/* 成交量 //对于股票(单位:股)
										对于指数(单位:百股) */
	float	m_fAvgPrice;		/*成交金额 */
	long	m_lBuyCount;        // 委买量
	long	m_lSellCount;       // 委卖量

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_lNewPrice = YlsIntIntelToComputer(m_lNewPrice);
			m_lTotal = YlsIntIntelToComputer(m_lTotal);
			m_fAvgPrice = YlsFloatComputerToIntel(m_fAvgPrice);
			m_lBuyCount = YlsIntIntelToComputer(m_lBuyCount);
			m_lSellCount = YlsIntIntelToComputer(m_lSellCount);
		}
		else
		{
			m_lNewPrice = YlsIntComputerToIntel(m_lNewPrice);
			m_lTotal = YlsIntComputerToIntel(m_lTotal);
			m_fAvgPrice = YlsFloatComputerToIntel(m_fAvgPrice);
			m_lBuyCount = YlsIntComputerToIntel(m_lBuyCount);
			m_lSellCount = YlsIntComputerToIntel(m_lSellCount);
		}
#endif
	}

};

// 历史分时走势数据
struct StockHistoryTrendHead 
{
	long				m_lDate;		// 日期
	long				m_lPrevClose;	// 昨收

	union
	{
		HSStockRealTime			m_stStockData;		// 个股实时基本数据
		HSIndexRealTime			m_stIndexData;		// 指数实时基本数据
		HSHKStockRealTime		m_hkData;		// 港股实时基本数据
		HSQHRealTime			m_qhData;		// 期货实时基本数据
		HSWHRealTime			m_whData;		// 外汇实时基本数据
	};

	short			    m_nSize;		//  每天数据总个数
	short				m_nAlignment;   //  对齐用

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_lDate = YlsIntIntelToComputer(m_lDate);
			m_lPrevClose = YlsIntIntelToComputer(m_lPrevClose);

			m_nSize = YlsShortIntelToComputer(m_nSize);
			m_nAlignment = YlsShortIntelToComputer(m_nAlignment);
		}
		else
		{
			m_lDate = YlsIntComputerToIntel(m_lDate);
			m_lPrevClose = YlsIntComputerToIntel(m_lPrevClose);

			m_nSize = YlsShortComputerToIntel(m_nSize);
			m_nAlignment = YlsShortComputerToIntel(m_nAlignment);
		}
#endif
	}
};

//分时走势包含的数据
struct StockHistoryTrend 
{
	StockHistoryTrendHead	m_shHead;		// 历史分时走势数据
	StockCompHistoryData	m_shData[1];	// 历史分时1分钟数据
};

struct StockOtherDataDetailTime
{
	unsigned short m_nTime;
	unsigned short m_nSecond;
};

//struct JYSStatusInfo
//{
//	unsigned char m_nStatus1;
//	unsigned char m_nStatus2;
//	unsigned char m_nStatus3;
//	unsigned char m_nStatus4;
//};

// 各股票其他数据
struct StockOtherData
{
	union
	{
		unsigned long					 m_nTimeOld;	  // 现在时间	
		unsigned short					 m_nTime;		  // 现在时间	
		StockOtherDataDetailTime		 m_sDetailTime;
	};

	unsigned long  m_lCurrent;    // 现在总手

	unsigned long  m_lOutside;    // 外盘
	unsigned long  m_lInside;     // 内盘

	union
	{
		unsigned long  m_lKaiCang;    // 今开仓,深交所股票单笔成交数,港股交易宗数

		unsigned long  m_lPreClose;   // 对于外汇时，昨收盘数据		
	};

	union
	{
		unsigned long  m_rate_status; // 对于外汇时，报价状态
									  // 对于股票，信息状态标志,
									  // MAKELONG(MAKEWORD(nStatus1,nStatus2),MAKEWORD(nStatus3,nStatus4))

		unsigned long  m_lPingCang;   // 今平仓

		long		   m_lSortValue;  // 排名时，为排序后的值
	};

	void GetStatus(BYTE& bStatus1,BYTE& bStatus2,BYTE& bStatus3,BYTE& bStatus4)
	{

#ifndef HS_SUPPORT_UNIX
		WORD lo = LOWORD(m_rate_status);
		WORD hi = HIWORD(m_rate_status);
		
		bStatus1 = LOBYTE(lo);
		bStatus2 = HIBYTE(lo);
		bStatus3 = LOBYTE(hi);
		bStatus4 = HIBYTE(hi);
#endif
	}

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{			
		}
		else
		{
			m_sDetailTime.m_nTime = YlsShortIntelToComputer(m_sDetailTime.m_nTime);
			m_sDetailTime.m_nSecond = YlsShortIntelToComputer(m_sDetailTime.m_nSecond);
			m_lCurrent = YlsIntComputerToIntel(m_lCurrent);

			m_lOutside = YlsIntComputerToIntel(m_lOutside);
			m_lInside = YlsIntComputerToIntel(m_lInside);

			m_lKaiCang = YlsIntComputerToIntel(m_lKaiCang);
			m_rate_status =YlsIntComputerToIntel(m_rate_status);
		}
#endif
	}
};

//K线数据
struct StockDay
{
	long	m_lDate;  			/*year-month-day ,example: 19960616
								分钟数据的表示方法如下：yymmddhhnn(年月日时分)
								yy指的是year - 1990，故年份表达范围：1990 - 2011
								如0905131045，指的是：1999年5月13号10点45分。*/
	long	m_lOpenPrice;		//开
	long	m_lMaxPrice;		//高
	long	m_lMinPrice;		//低
	long	m_lClosePrice;		//收
	long	m_lMoney;			//成交金额
	unsigned long	m_lTotal;			//成交量   单位：百股（手）

	//#ifdef SUPPORT_NETVALUE
	long	m_lNationalDebtRatio; // 国债利率(单位为0.1分),基金净值
	//#endif

	union
	{
		long	m_lPrevClose;
		struct
		{
			short	m_nMessageMask;	//资料信息掩码
			short	m_nOpenVolumn;  //开盘成交量
		};
	};
	//#ifndef SUPPORT_NETVALUE
	//	unsigned  m_lOutside : 28;		//外盘成交量
	//	unsigned  m_nVersion : 4;		//版本号，0：旧版本，1：新版本
	//#endif

	union
	{
		struct
		{
			short  m_nVolAmount;   //成交次数
			short  m_nZeroVol;	   //对倒成交量。
		};
		long* m_pDataEx;		   // 如果是除权，是 ChuQuanData 结构数据指针
	};

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_lDate = YlsIntIntelToComputer(m_lDate);
			m_lOpenPrice = YlsIntIntelToComputer(m_lOpenPrice);
			m_lMaxPrice = YlsIntIntelToComputer(m_lMaxPrice);
			m_lMinPrice = YlsIntIntelToComputer(m_lMinPrice);
			m_lClosePrice = YlsIntIntelToComputer(m_lClosePrice);
			m_lMoney = YlsIntIntelToComputer(m_lMoney);
			m_lTotal = YlsIntIntelToComputer(m_lTotal);
			m_lNationalDebtRatio = YlsIntIntelToComputer(m_lNationalDebtRatio);
		}
		else
		{
			m_lDate = YlsIntComputerToIntel(m_lDate);
			m_lOpenPrice = YlsIntComputerToIntel(m_lOpenPrice);
			m_lMaxPrice = YlsIntComputerToIntel(m_lMaxPrice);
			m_lMinPrice = YlsIntComputerToIntel(m_lMinPrice);
			m_lClosePrice = YlsIntComputerToIntel(m_lClosePrice);
			m_lMoney = YlsIntComputerToIntel(m_lMoney);
			m_lTotal = YlsIntComputerToIntel(m_lTotal);
			m_lNationalDebtRatio = YlsIntComputerToIntel(m_lNationalDebtRatio);
		}
#endif
	}
};

// 除权数据文件代码信息
struct HSExRight 
{
	CodeInfo	m_CodeInfo;
	long 		m_lLastDate;	//最新日期
	long	    m_lCount;		//参考使用
};
// 除权数据结构
struct HSExRightItem
{
	int   	m_nTime;			// 时间
	float 	m_fGivingStock;		// 送股
	float 	m_fPlacingStock;	// 配股
	float 	m_fGivingPrice;		// 送股价
	float 	m_fBonus;			// 分红
};

//通用文件头结构
struct HSCommonFileHead
{
	long 	m_lFlag;   // 文件类型标识
	time_t 	m_lDate;   // 文件更新日期(长度:32bit)
	long 	m_lVersion; // 文件结构版本标识
	long 	m_lCount;  // 数据总个数	 
};

// 新加入结构 ============================================================
// 经纪信息定义
#define BROKER_LENGTH		4		// 经纪号长度
#define NAME_LENGTH			36		// 名称长度
// 席位号
struct HSHKBrokerCode
{
	char	m_sCode[BROKER_LENGTH];// 不出现在代码表中，所以，可以是0001-9999之间的任何代码。	

};
// 经纪信息定义
struct HSHKBroker
{
	char		 m_szEnglishName[NAME_LENGTH];	// 经纪(证券商)英文名
	char		 m_szChineseDesc[NAME_LENGTH];	// 经纪(证券商)中文名
	char		 m_szTel[16];					// 电话号码
	long		 m_lCount;						// 席位数
	HSHKBrokerCode m_ayCode[1];					// 所有席位号
};


// 单个经纪信息
struct HSHKBuyAndSellItem
{
	HSHKBrokerCode m_sCode;		// 经纪席位代码
	short		   m_nIsBuy;	// 是买还是卖 1: 买 0: 卖
	short		   m_nNum;		// 第几档

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		//m_sCode.To(cIntelToComputer);

		if( cIntelToComputer )
		{
			m_nIsBuy = YlsShortIntelToComputer(m_nIsBuy);
			m_nNum = YlsShortIntelToComputer(m_nNum);			
		}
		else
		{
			m_nIsBuy = YlsShortComputerToIntel(m_nIsBuy);
			m_nNum = YlsShortComputerToIntel(m_nNum);
		}
#endif
	}
};

// 买卖盘经纪详细信息(需要服务器主推)
struct HSHKBuyAndSell
{
	CodeInfo m_sCode;
	long	m_lBuyPrice;				// 买价
	long	m_lSellPrice;				// 卖价
	long	m_lCount;					// 总数
	HSHKBuyAndSellItem m_ayItem[1];		// 数据项
	// 注意，存放时按买一,买二，。。。
	//       到卖一，卖二。。。的顺序存放

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		m_sCode.To(cIntelToComputer);

		if( cIntelToComputer )
		{
			m_lBuyPrice = YlsIntIntelToComputer(m_lBuyPrice);
			m_lSellPrice = YlsIntIntelToComputer(m_lSellPrice);
			m_lCount = YlsIntIntelToComputer(m_lCount);
		}
		else
		{
			for(int i = 0; i < m_lCount; i++ )
			{
				m_ayItem[i].To(cIntelToComputer);
			}

			m_lBuyPrice = YlsIntComputerToIntel(m_lBuyPrice);
			m_lSellPrice = YlsIntComputerToIntel(m_lSellPrice);
			m_lCount = YlsIntComputerToIntel(m_lCount);
		}
#endif
	}

};


// 此功能可能要根据数据源决定是否使用（因为如此只是根据买卖盘计算，那是非常繁重的步骤）。
// 某支股票的委托情况
struct HSHKBrokerDealItem
{
	CodeInfo	m_sCode;
	char		m_cIsBuy;		// 是买还是卖 1: 买 0: 卖
	char		m_cNum;			// 第几档
	char		m_cCount;		// 所在档的个数
	char		m_cAlignment;	// 为了父结构对齐
};
// 经纪席位下委托情况
struct HSHKBrokerDeal
{
	HSHKBrokerCode	m_sCode;	// 席位号

	long	m_lCount;				// 个数
	HSHKBrokerDealItem m_ayCode[1]; // 个股委托情况
};


// 币种定义
#define		RMB		0			// 人民币
#define		HKD		1			// 港币	
#define		USA		2			// 美元
// 股票及认股证标志定义
#define FinFlag_H	0x0001		// 可进行卖空的指定证券名单’H’ 标志
#define FinFlag_3	0x0002		// # 已纳入中央结算及交收系统’#’ 标志
#define FinFlag_O	0x0004		// 已纳入股票期权’O’ 标志
#define FinFlag_F	0x0008		// 已纳入股票期货’F’ 标志
#define FinFlag_S	0x0010		// 表示买卖该股票需要印花税’S’ 标志
#define FinFlag_P	0x0020		// 认沽证’P’ 标志
#define FinFlag_C	0x0040		// 认购证’C’ 标志
#define FinFlag_X	0x0080		// 特种认股证’X’ 标志
#define FinFlag_E	0x0100		// 欧式设股证’E’ 标志
#define FinFlag_2	0x0200		// 现金结算’@’标志
#define FinFlag_8	0x0400		// 实物股票交收’*’标志
// 相关股票结构
struct HSHKLineStockAndWarrants
{
	short m_nLineStock;		// 相关股票数
	short m_nBuyCount;		// 相关认购证数
	short m_nSellCount;		// 相关认沽证数
	short m_nAlignment;		// 为了父结构对齐
	CodeInfo m_ayCode[1];   // 存储方式：相关股票+认购证+认沽证
};
// 个股信息（如相关认股证，相关股票，交易单位，交易币种）
struct HSHKStaticData
{
	CodeInfo	m_sCode;						// 代码
	char		m_szEnglishName[NAME_LENGTH];	// 英名
	char		m_szChineseName[NAME_LENGTH];	// 中名
	char		m_szFinanceText[40];			// 财务标志文本

	short		m_cMoney;						// 货币代码
	short 		m_cFinFlag;						// 财务标志
	long		m_lLotUnit;						// 交易单位(是否可放入代码表????）

	// 以下字段认股证专用
	float		m_fPrice;						// 认股价
	float		m_fExplicateAmplitude;			// 引申波幅
	long		m_lExerDate;					// 到期日
	HSHKBrokerCode		m_sLineBroker;			// 相关经纪

	HSHKLineStockAndWarrants m_sLineStock;		// 相关股票及认股证
};

struct HSHKSource
{
	char szFlags[4]; // EXHK
	char szCode[10]; // CODE
	char szMsg[40];  // MSG
	char szEnd[2];
};

// etf 时时数据
struct ETFAllStockNow
{
	// 一组股票
	long			m_lETFBuyPrice1;
	unsigned long	m_lETFBuyCount1;
	long			m_lETFBuyPrice2;
	unsigned long	m_lETFBuyCount2;
	long			m_lETFBuyPrice3;
	unsigned long	m_lETFBuyCount3;	
	long			m_lETFBuyPrice4;
	unsigned long	m_lETFBuyCount4;
	long			m_lETFBuyPrice5;
	unsigned long	m_lETFBuyCount5;

	long			m_lETFSellPrice1;
	unsigned long	m_lETFSellCount1;
	long			m_lETFSellPrice2;
	unsigned long	m_lETFSellCount2;
	long			m_lETFSellPrice3;
	unsigned long	m_lETFSellCount3;
	long			m_lETFSellPrice4;
	unsigned long	m_lETFSellCount4;
	long			m_lETFSellPrice5;
	unsigned long	m_lETFSellCount5;

	// 自动
	long			m_lBuyAutoPrice;		// 自动
	unsigned long	m_lBuyAutoCount;		// 自动
	long			m_lSellAutoPrice;		// 自动
	unsigned long	m_lSellAutoCount;		// 自动

	long			m_lETFBuyAutoPrice;		// 自动
	unsigned long	m_lETFBuyAutoCount;		// 自动	
	long			m_lETFSellAutoPrice;	// 自动
	unsigned long	m_lETFSellAutoCount;	// 自动	

	long    m_lIPVE;

	//
	unsigned long    m_lReserv[9]; // 保留

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
		}
		else
		{
			m_lETFBuyPrice1	=   YlsIntComputerToIntel(m_lETFBuyPrice1);
			m_lETFBuyCount1	=	YlsIntComputerToIntel(m_lETFBuyCount1);
			m_lETFBuyPrice2	=	YlsIntComputerToIntel(m_lETFBuyPrice2);
			m_lETFBuyCount2	=	YlsIntComputerToIntel(m_lETFBuyCount2);
			m_lETFBuyPrice3	=	YlsIntComputerToIntel(m_lETFBuyPrice3);
			m_lETFBuyCount3	=	YlsIntComputerToIntel(m_lETFBuyCount3);	
			m_lETFBuyPrice4	=	YlsIntComputerToIntel(m_lETFBuyPrice4);
			m_lETFBuyCount4	=	YlsIntComputerToIntel(m_lETFBuyCount4);
			m_lETFBuyPrice5	=	YlsIntComputerToIntel(m_lETFBuyPrice5);
			m_lETFBuyCount5	=	YlsIntComputerToIntel(m_lETFBuyCount5);

			m_lETFSellPrice1	=   YlsIntComputerToIntel(m_lETFSellPrice1);
			m_lETFSellCount1	=	YlsIntComputerToIntel(m_lETFSellCount1);
			m_lETFSellPrice2	=	YlsIntComputerToIntel(m_lETFSellPrice2);
			m_lETFSellCount2	=	YlsIntComputerToIntel(m_lETFSellCount2);
			m_lETFSellPrice3	=	YlsIntComputerToIntel(m_lETFSellPrice3);
			m_lETFSellCount3	=	YlsIntComputerToIntel(m_lETFSellCount3);
			m_lETFSellPrice4	=	YlsIntComputerToIntel(m_lETFSellPrice4);
			m_lETFSellCount4	=	YlsIntComputerToIntel(m_lETFSellCount4);
			m_lETFSellPrice5	=	YlsIntComputerToIntel(m_lETFSellPrice5);
			m_lETFSellCount5	=	YlsIntComputerToIntel(m_lETFSellCount5);

			// 自动
			m_lBuyAutoPrice = YlsIntComputerToIntel(m_lBuyAutoPrice);		// 自动
			m_lBuyAutoCount = YlsIntComputerToIntel(m_lBuyAutoCount);		// 自动
			m_lSellAutoPrice = YlsIntComputerToIntel(m_lSellAutoPrice);		// 自动
			m_lSellAutoCount = YlsIntComputerToIntel(m_lSellAutoCount);		// 自动

			m_lETFBuyAutoPrice = YlsIntComputerToIntel(m_lETFBuyAutoPrice);		// 自动
			m_lETFBuyAutoCount = YlsIntComputerToIntel(m_lETFBuyAutoCount);		// 自动	
			m_lETFSellAutoPrice = YlsIntComputerToIntel(m_lETFSellAutoPrice);	// 自动
			m_lETFSellAutoCount = YlsIntComputerToIntel(m_lETFSellAutoCount);	// 自动	

			m_lIPVE = YlsIntComputerToIntel(m_lIPVE);

			//
			for(int i = 0; i < 9; i++ )
				m_lReserv[i] = YlsIntComputerToIntel(m_lReserv[i]); // 保留			
		}
#endif
	}
};

struct ETFStockNowData
{
	HSStockRealTime  m_stStockData;
	ETFAllStockNow   m_etf;

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		m_stStockData.To(cIntelToComputer);
		m_etf.To(cIntelToComputer);
#endif
	}
};

// 时时数据分类
union ShareRealTimeData
{
	HSStockRealTime			m_nowData;		// 个股实时基本数据
	HSStockRealTime			m_stStockData;

	HSIndexRealTime			m_indData;		// 指数实时基本数据
	HSHKStockRealTime		m_hkData;		// 港股实时基本数据
	HSQHRealTime			m_qhData;		// 期货实时基本数据
	HSWHRealTime			m_whData;		// 外汇实时基本数据	

	HSQHRealTime_Min		m_qhMin;
};

union ShareRealTimeData_ETF
{
	HSStockRealTime			m_nowData;		// 个股实时基本数据
	HSIndexRealTime			m_indData;		// 指数实时基本数据
	HSHKStockRealTime		m_hkData;		// 港股实时基本数据
	HSQHRealTime			m_qhData;		// 期货实时基本数据
	HSWHRealTime			m_whData;		// 外汇实时基本数据

	ETFStockNowData			m_etfData;

	HSQHRealTime_Min		m_qhMin;
};

struct CalcData_Share // 股票计算
{
	long 			m_lMa10;			// 10天，20天，50天收盘均价
	long 			m_lMa20;
	long 			m_lMa50;

	long 			m_lMonthMax;		// 月最高最低
	long 			m_lMonthMin;		

	long 			m_lYearMax;			// 年最高最低
	long 			m_lYearMin;

	long			m_lHisAmplitude;	// 历史波幅(使用时除1000为百分比数）

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_lMa10 = YlsIntIntelToComputer(m_lMa10);
			m_lMa20 = YlsIntIntelToComputer(m_lMa20);
			m_lMa50 = YlsIntIntelToComputer(m_lMa50);

			m_lMonthMax = YlsIntIntelToComputer(m_lMonthMax);
			m_lMonthMin = YlsIntIntelToComputer(m_lMonthMin);

			m_lYearMax = YlsIntIntelToComputer(m_lYearMax);
			m_lYearMin = YlsIntIntelToComputer(m_lYearMin);

			m_lHisAmplitude = YlsIntIntelToComputer(m_lHisAmplitude);
		}
		else
		{
			m_lMa10 = YlsIntComputerToIntel(m_lMa10);
			m_lMa20 = YlsIntComputerToIntel(m_lMa20);
			m_lMa50 = YlsIntComputerToIntel(m_lMa50);

			m_lMonthMax = YlsIntComputerToIntel(m_lMonthMax);
			m_lMonthMin = YlsIntComputerToIntel(m_lMonthMin);

			m_lYearMax = YlsIntComputerToIntel(m_lYearMax);
			m_lYearMin = YlsIntComputerToIntel(m_lYearMin);

			m_lHisAmplitude = YlsIntComputerToIntel(m_lHisAmplitude);
		}
#endif
	}
};

struct CalcData_GP
{
};

struct CalcData_GG // 港股计算
{
	union
	{
		long 		m_lPEratio;			// 预期市盈率 股票相关
		long		m_fOverFlowPrice;	// 溢价 认股证相关
	};

	union
	{
		long 		m_lWeekratio;		// 预期周息率 股票相关
		long		m_FCSRatio;			// 控股比例 认股证相关
	};

	union
	{
		long 		m_lCutPriceRatio;	// 拆让率 股票相关
		long		m_fValidCSRatio;	// 有效控股比例 认股证相关
	};

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_lPEratio = YlsIntIntelToComputer(m_lPEratio);
			m_lWeekratio = YlsIntIntelToComputer(m_lWeekratio);
			m_lCutPriceRatio = YlsIntIntelToComputer(m_lCutPriceRatio);
		}
		else
		{
			m_lPEratio = YlsIntComputerToIntel(m_lPEratio);
			m_lWeekratio = YlsIntComputerToIntel(m_lWeekratio);
			m_lCutPriceRatio = YlsIntComputerToIntel(m_lCutPriceRatio);
		}
#endif
	}
};

struct CalcData_QH // 期货计算
{
	long 			m_lLotUnit;			// 每手单位(相当于实时数据里的nHand,建议是否可将其放入代码表里？）
	long 			m_lDiffUnit;		// 变动单位(1/1000元)
	long 			m_lMaxDeal;			// 最大申报量(单位：合约单位)
	long			m_lMarginRatio;		// 保证金比例(1/100 = ?%)

	void To(char cIntelToComputer )
	{
#ifdef WORDS_BIGENDIAN
		if( cIntelToComputer )
		{
			m_lLotUnit = YlsIntIntelToComputer(m_lLotUnit);
			m_lDiffUnit = YlsIntIntelToComputer(m_lDiffUnit);
			m_lMaxDeal = YlsIntIntelToComputer(m_lMaxDeal);
			m_lMarginRatio = YlsIntIntelToComputer(m_lMarginRatio);
		}
		else
		{
			m_lLotUnit = YlsIntComputerToIntel(m_lLotUnit);
			m_lDiffUnit = YlsIntComputerToIntel(m_lDiffUnit);
			m_lMaxDeal = YlsIntComputerToIntel(m_lMaxDeal);
			m_lMarginRatio = YlsIntComputerToIntel(m_lMarginRatio);
		}
#endif
	}
};

// data struct in file loncdp.*
struct StockCdp
{
	CodeInfo		m_CodeInfo;
	long			m_lNoUse[5];		//保留
	unsigned long	m_l5DayVol;			//五日平均总手

	union
	{
		CalcData_GP m_GP;
		CalcData_GG m_GG;
		CalcData_QH m_QH;
	};

	CalcData_Share  m_Share;
};

// added by Ben 20100505
#define GETBUYCOUNT(X)  (X->m_lBuyCount1 + X->m_lBuyCount2 + X->m_lBuyCount3 + X->m_lBuyCount4 + X->m_lBuyCount5)
#define GETSELLCOUNT(X) (X->m_lSellCount1 + X->m_lSellCount2 + X->m_lSellCount3 + X->m_lSellCount4 + X->m_lSellCount5)
// end add

#ifndef HS_SUPPORT_UNIX
#pragma pack()
#else
#endif
#endif