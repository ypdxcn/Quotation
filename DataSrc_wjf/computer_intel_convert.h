
#ifndef ComputerToIntel_Convert
#define ComputerToIntel_Convert

//#define	WORDS_BIGENDIAN

#define	bswap_16(x)		((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define	bswap_32(x)		\
	((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |		\
	 (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#ifdef	WORDS_BIGENDIAN

/* 机器字节顺序转换成Intel字节，short类型 */
#define	htois(x)	bswap_16(x)
/* Intel字节顺序转换成机器字节，short类型 */
#define	itohs(x)	bswap_16(x)
/* 机器字节顺序转换成Intel字节，int类型 */
#define	htoil(x)	bswap_32(x)
/* Intel字节顺序转换成机器字节，int类型 */
#define	itohl(x)	bswap_32(x)

#else

/* 机器字节顺序转换成Intel字节，short类型 */
#define	htois(x)	(x)
/* Intel字节顺序转换成机器字节，short类型 */
#define	itohs(x)	(x)
/* 机器字节顺序转换成Intel字节，int类型 */
#define	htoil(x)	(x)
/* Intel字节顺序转换成机器字节，int类型 */
#define	itohl(x)	(x)

#endif

#define YlsShortIntelToComputer(x)	itohs(x)
#define YlsShortComputerToIntel(x)  htois(x)

#define YlsIntIntelToComputer(x)	itohl(x)
#define YlsIntComputerToIntel(x)	htoil(x)

static float YlsFloatComputerToIntel(float fValue)
{
#ifdef WORDS_BIGENDIAN
	unsigned char *p, *q;

	float s = 0.0;

	p = (unsigned char *)(char*)&fValue;
	q = (unsigned char *)(char*)&s;

	for (int i = 0; i < 4; i++)
		*(q + 3 - i) = *(p + i);

	return s;
#endif
	return fValue;
}

static float YlsByteTofloat(char *byte)
{
#ifdef WORDS_BIGENDIAN
	unsigned char *p1, *p2;
	int	i;
	float s;

	p1 = (unsigned char *)byte;
	p2 = (unsigned char *)&s;
	for (i = 0; i < 4; i++)
		*(p2 + 3 - i) = *(p1 + i);

	return s;
#endif

	return (*(float*)byte);
}

static int YlsByteToInt(char *byte)
{
#ifdef WORDS_BIGENDIAN
	unsigned char	*p1, *p2;

	int s = 0;

	p1 = (unsigned char *)byte;
	p2 = (unsigned char *)&s;

	p2 += 3;
	*p2-- = *p1++;
	*p2-- = *p1++;
	*p2-- = *p1++;
	*p2 = *p1;

	return s;	
#endif
	 return (*(int*)byte);
}

static short YlsByteToShort(char *byte)
{	
#ifdef WORDS_BIGENDIAN	

	short s = 0;
	unsigned char	*p1, *p2;

	p1 = (unsigned char *)byte;
	p2 = (unsigned char *)&s;

	p2++;
	*p2-- = *p1++;
	*p2 = *p1;

	return s;
#endif
	
	return (*(short*)byte);
}

static void  byte2short(char *byte, short *s)
{
	unsigned char	*p1, *p2;

	p1 = (unsigned char *)byte;
	p2 = (unsigned char *)s;
#ifndef WORDS_BIGENDIAN
	*p2++ = *p1++;
#else
	p2++;
	*p2-- = *p1++;
#endif
	*p2 = *p1;
}

static void short2byte(short s, char *byte)
{
	unsigned char	*p1, *p2;

	p1 = (unsigned char *)&s;
	p2 = (unsigned char *)byte;
#ifndef WORDS_BIGENDIAN
	*p2++ = *p1++;
#else
	p2++;
	*p2-- = *p1++;
#endif
	*p2 = *p1;
}

static void byte2int(char *byte, int *s)
{
	unsigned char	*p1, *p2;

	p1 = (unsigned char *)byte;
	p2 = (unsigned char *)s;
#ifndef WORDS_BIGENDIAN
	*p2++ = *p1++;
	*p2++ = *p1++;
	*p2++ = *p1++;
#else
	p2 += 3;
	*p2-- = *p1++;
	*p2-- = *p1++;
	*p2-- = *p1++;
#endif
	*p2 = *p1;
}

static void int2byte(int s, char *byte)
{
	unsigned char	*p1, *p2;

	p1 = (unsigned char *)&s;
	p2 = (unsigned char *)byte;
#ifndef WORDS_BIGENDIAN
	*p2++ = *p1++;
	*p2++ = *p1++;
	*p2++ = *p1++;
#else
	p2 += 3;
	*p2-- = *p1++;
	*p2-- = *p1++;
	*p2-- = *p1++;
#endif
	*p2 = *p1;
}

static void float2byte(float s, char *byte)
{
	unsigned char	*p1, *p2;
	int	floatlen, i;

	floatlen = sizeof(float);
	p1 = (unsigned char *)&s;
	p2 = (unsigned char *)byte;
#ifndef WORDS_BIGENDIAN
	for (i = 0; i < floatlen; i++)
		*(p2 + i) = *(p1 + i);
#else
	for (i = 0; i < floatlen; i++)
		*(p2 + floatlen - 1 - i) = *(p1 + i);
#endif
}

static void byte2float(char *byte, float *s)
{
	unsigned char	*p1, *p2;
	int	floatlen, i;

	floatlen = sizeof(float);
	p1 = (unsigned char *)byte;
	p2 = (unsigned char *)s;
#ifndef WORDS_BIGENDIAN
	for (i = 0; i < floatlen; i++)
		*(p2 + i) = *(p1 + i);
#else
	for (i = 0; i < floatlen; i++)
		*(p2 + floatlen - 1 - i) = *(p1 + i);
#endif
}

#ifdef WORDS_BIGENDIAN
	#define YlsTo(Obj,flag)		  ((Obj)->To((flag)))

	#define YlsShortC(x)      YlsShortComputerToIntel(x)
	#define YlsIntC(x)        YlsIntComputerToIntel(x) 

	#define YlsShortI(x)      YlsShortIntelToComputer(x)
	#define YlsIntI(x)        YlsIntIntelToComputer(x) 
#else
	#define YlsTo(Obj,flag)

	#define YlsShortC(x) (x)
	#define YlsIntC(x)	 (x)

	#define YlsShortI(x) (x)
	#define YlsIntI(x)	 (x)
#endif

#endif
