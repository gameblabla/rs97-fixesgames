#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#if !defined(PLATFORM_BIG_ENDIAN) && !defined(PLATFORM_LITTLE_ENDIAN)
#	if defined(__BYTE_ORDER__)
#		if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#			define PLATFORM_BIG_ENDIAN
#		elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#			define PLATFORM_LITTLE_ENDIAN
#		elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#			error Sorry, pdp-endian is not supported.
#		endif
#	endif
#endif

#if !defined(PLATFORM_BIG_ENDIAN) && !defined(PLATFORM_LITTLE_ENDIAN)
	#warning No endianess could be detected. Assuming little-endian.
	#define PLATFORM_LITTLE_ENDIAN
#endif

#endif /* _ENDIAN_H_ */
