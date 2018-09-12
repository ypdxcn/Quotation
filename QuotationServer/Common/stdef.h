#ifndef _STDEF_H
#define _STDEF_H

// 业务层公共数据结构
#define HSMarketDataType unsigned short
#define ULONG unsigned long
#define MARKETNAME_SIZE    32
#define CODE_SIZE          8
#define NAME_SIZE          32



/*
市场类别定义：
各位含义表示如下：
15		   12		8					0
|			|	  	  |					|
| 金融分类	|市场分类 |	交易品种分类	|
*/


/*国内股票市场*/
#define STOCK_MARKET			 0X1000   // 股票
#	define SH_BOURSE			 0x0100   // 上海
#	define SZ_BOURSE			 0x0200   // 深圳
#	define SYSBK_BOURSE			 0x0400   // 系统板块
#	define USERDEF_BOURSE		 0x0800   // 自定义（自选股或者自定义板块）
#		define KIND_INDEX		 0x0000   // 指数 
#		define KIND_STOCKA		 0x0001   // A股 
#		define KIND_STOCKB		 0x0002   // B股 
#		define KIND_BOND		 0x0003   // 债券
#		define KIND_FUND		 0x0004   // 基金
#		define KIND_THREEBOAD	 0x0005   // 三板
#		define KIND_SMALLSTOCK	 0x0006   // 中小盘股
#		define KIND_PLACE		 0x0007	  // 配售
#		define KIND_LOF			 0x0008	  // LOF
#		define KIND_ETF			 0x0009   // ETF
#		define KIND_QuanZhen	 0x000A   // 权证

#		define KIND_OtherIndex	 0x000E   // 第三方行情分类，如:中信指数
#		define SC_Others		 0x000F   // 其他 0x09
#		define KIND_USERDEFINE	 0x0010   // 自定义指数


// 港股市场
#define HK_MARKET				 0x2000 // 港股分类
#	define HK_BOURSE			 0x0100 // 主板市场
#	define	GE_BOURSE			 0x0200 // 创业板市场(Growth Enterprise Market)
#	define	INDEX_BOURSE		 0x0300	// 指数市场	
#		define HK_KIND_INDEX			 0x0000   // 港指
#		define HK_KIND_FUTURES_INDEX	 0x0001   // 期指

#	define SYSBK_BOURSE			 0x0400 // 港股板块(H股指数成份股，讯捷指数成份股）。
#	define USERDEF_BOURSE		 0x0800 // 自定义（自选股或者自定义板块）

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
#				define KIND_QHGOLD		 0x0005	// 上海黄金

#		define ZHENGZHOU_BOURSE	  0x0300	// 郑州
#				define KIND_XIAOM		 0x0001	// 郑州小麦
#				define KIND_MIANH		 0x0002	// 郑州棉花
#				define KIND_BAITANG		 0x0003	// 郑州白糖
#				define KIND_PTA			 0x0004	// 郑州PTA
#				define KIND_CZY			 0x0005	// 菜籽油

#		define GUZHI_BOURSE		  0x0500		// 股指期货
#				define KIND_GUZHI		 0x0001	// 股指期货

/*外盘大类*/
#define WP_MARKET				 ((HSMarketDataType)0x5000) // 外盘
#		define WP_INDEX				0x0100	// 国际指数
#		define WP_INDEX_RJ	 		0x0001 //"日经"; 7120
#		define WP_INDEX_HZ	 		0x0002 //"恒指"; 7121
#		define WP_INDEX_NH	 		0x0003 //"南韩综合";7122
#		define WP_INDEX_TG	 		0x0004 //"台股加权";7123
#		define WP_INDEX_XG	 		0x0005 //"星股海峡";7124
#		define WP_INDEX_MG	 		0x0006 //"马股综合";7125
#		define WP_INDEX_TGZH 		0x0007 //"泰股综合";7126
#		define WP_INDEX_YN 		    0x0008 //"印尼综合";7127
#		define WP_INDEX_AZ 		    0x0009 //"澳洲综合";7128
#		define WP_INDEX_NXL  		0x000a //"纽西兰";  7129
#		define WP_INDEX_SGX         0x000b //"SGX摩台"; 7130
#		define WP_INDEX_SENSEX      0x000c //"印SENSEX";7164
#		define WP_INDEX_KOSPI       0x000d //"KOSPI200";7185
#		define WP_INDEX_DQGY        0x000e //"道琼工业";7301
#		define WP_INDEX_DQYS        0x000f //"道琼运输";7302
#		define WP_INDEX_DQGG        0x0010 //"道琼公共";7303
#		define WP_INDEX_NSDK        0x0011 //"纳斯达克" 7304
#		define WP_INDEX_BZPE        0x0012 //"标准普尔" 7305
#		define WP_INDEX_CRBYX       0x0013 //"CRB延续"  7306
#		define WP_INDEX_CRBZS       0x0014 //"CRB指数"  7307
#		define WP_INDEX_JND         0x0015 //"加拿大"   7308
#		define WP_INDEX_FS100       0x0016 //"富时100"  7309
#		define WP_INDEX_FACAC       0x0017 //"法CAC40" 7310
#		define WP_INDEX_DEDAX       0x0018 //"德DAX"   7312
#		define WP_INDEX_HEAEX       0x0019 //"荷兰AEX" 7313
#		define WP_INDEX_DMKFX       0x001a //"丹麦KFX" 7314
#		define WP_INDEX_BLS         0x001b //"比利时"  7315
#		define WP_INDEX_RSSSMI      0x001c //"瑞士SSMI" 7316
#		define WP_INDEX_BXBVSP      0x001d //"巴西BVSP" 7317
#		define WP_INDEX_BDI         0x001e //"BDI指数"  7321
#		define WP_INDEX_BP100       0x001f //"标普100"  7322
#		define WP_INDEX_ERTS        0x0020 //"俄RTS"    7323
#		define WP_INDEX_YFTMIB      0x0021 //"意FTMIB"  7324


#		define WP_LME				0x0200	// LME		// 不用了
#		define WP_CBOT				0x0300	// CBOT			
#		define WP_NYMEX	 			0x0400	// NYMEX	 	
#		    define WP_NYMEX_YY			0x0000	//"原油";
#		    define WP_NYMEX_RY			0x0001	//"燃油";
#		    define WP_NYMEX_QY			0x0002	//"汽油";

#		define WP_COMEX	 			0x0500	// COMEX	 	
#		define WP_TOCOM	 			0x0600	// TOCOM	 	
#		define WP_IPE				0x0700	// IPE			
#		define WP_NYBOT				0x0800	// NYBOT		
#		define WP_NOBLE_METAL		0x0900	// 贵金属	
#		   define WP_NOBLE_METAL_XH	    0x0000  //"现货";
#		   define WP_NOBLE_METAL_HJ   	0x0001  //"黄金";
#		   define WP_NOBLE_METAL_BY	    0x0002  //"白银";

#		define WP_FUTURES_INDEX		0x0a00	// 期指
#		define WP_SICOM				0x0b00	// SICOM		
#		define WP_LIBOR				0x0c00	// LIBOR		
#		define WP_NYSE				0x0d00	// NYSE
#		define WP_CEC				0x0e00	// CEC


/*外汇大类*/
#define FOREIGN_MARKET			 ((HSMarketDataType)0x8000) // 外汇
#	define WH_BASE_RATE			0x0100	// 基本汇率
#	define WH_ACROSS_RATE		0x0200	// 交叉汇率
#	define WH_FUTURES_RATE			0x0300  // 期汇

/*黄金大类*/
#define HJ_MARKET			 ((HSMarketDataType)0x6000) // 黄金
#		define SGE_BOURSE	     0x0100		// 黄金交易所
#				define KIND_TD		     0x0001	// 延期
#				define KIND_TN		     0x0002	// 远期
#				define KIND_SPOT		 0x0003	// 现货
#	define HJ_SH_QH		        0x0200	// 上海期货
#	define HJ_WORLD	        0x0300	// 国际市场
#	define HJ_OTHER	        0x0400	// 其它市场

static int	MakeMarket(HSMarketDataType x)
{
	return ((HSMarketDataType)((x) & 0xF000));
}
static int  MakeMainMarket(HSMarketDataType x)
{
	return ((HSMarketDataType)((x) & 0xFF00));
}

static int MakeSubMarket(HSMarketDataType x)
{
	if (MakeMainMarket(x) == (WP_MARKET | WP_INDEX))
		return ((HSMarketDataType)((x) & 0x00FF));
	else
		return ((HSMarketDataType)((x) & 0x000F));
}

static int MakeHexSubMarket(HSMarketDataType x)
{
	return ( (HSMarketDataType)((x) & 0x000F) );
}

static int MakeSubMarketPos(HSMarketDataType x)
{
	if (MakeMainMarket(x) == (WP_MARKET | WP_INDEX))
		return MakeSubMarket(x);
	return ( ((MakeHexSubMarket(x) / 16) * 10) + (MakeHexSubMarket(x) % 16) );
}

// 品种类型
typedef enum tag_StockType
{
	Kind_Index,
	Kind_StockA,
	Kind_StockB,
	Kind_Bond,
	Kind_Fund,
	Kind_ETF,
	Kind_Warrents,
	Kind_Spot,
	Kind_Futures,
	Kind_Foreign
}STOCKTYPE;


// 市场时间
typedef struct tag_DateTime
{
	unsigned short m_Year;
	unsigned char  m_Month;
	unsigned char  m_Day;
	unsigned char  m_Hour;
	unsigned char  m_Minute;
	unsigned char  m_Second;
	unsigned char  m_Reserved;
}QDateTime;

// 交易时段
typedef struct tag_TradePhase
{
	unsigned int m_nBeginTime;
	unsigned int m_nEndTime;
}QTradePhase;


typedef struct tag_StockInfo
{
	ULONG           m_ulMarketType;     // 市场类型
	STOCKTYPE       m_StockType;        // 品种类型
	char            m_szCode[CODE_SIZE];// 代码
	char            m_szName[NAME_SIZE];// 品种名称
	short           m_sDecimal;         // 小数点
	short           m_sUnit;            // 每手单位
	ULONG           m_Values[10];       // 常用数值 
	/*
	ULONG           m_lPrePrice;    昨收/昨结
	ULONG           m_lPreHolding;  昨持
	ULONG           m_l5DayVol;     5日成交量
	ULONG           m_lYearHigh;    年最高
	ULONG           m_lYearLow;     年最低
	ULONG           m_lMonthHigh;   月最高
	ULONG           m_lMonthLow;    月最低*/
}QStockInfo;


typedef struct tag_Market_Base
{
    HSMarketDataType m_MarketType;         // 市场类型
	char  m_acMarketName[MARKETNAME_SIZE]; // 市场名称
	ULONG m_ulVersion;                     // 版本
	short m_sStatus;                       // 市场状态
}QMarket_Base;

typedef struct tag_StockSort
{
	HSMarketDataType m_MarketType;
	char  m_szMarketName[MARKETNAME_SIZE]; // 板块名称
	short            m_sPhasesCount;
	QTradePhase*     m_pTradePhases;       // 时段数组指针
	char             m_acCodeRange;        // 代码范围
}QStockSort;

typedef struct tag_MarketInfo : public tag_Market_Base
{
    QDateTime        m_DateTime;               // 日期时间
	short            m_sPhasesCount;       // 时段数量
	QTradePhase*     m_pTradePhases;       // 时段数组指针
	short            m_sSortCount;         // 分类数
	QStockSort*      m_pStockSort;         // 分类表
	unsigned short   m_sStockCount;        // 品种数量
	QStockInfo*      m_pStockInfo;         // 品种数组指针
}QMarketInfo;











#endif