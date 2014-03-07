/*
 * tiny_assert.h
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */

#ifndef TINY_ASSERT_H_
#define TINY_ASSERT_H_


#include "tiny_constdef.h"
#include <stdio.h>
#include <stdlib.h>


#ifndef RELEASE
#define T_ASSERT(cond, opt) if(!(cond)) \
		{\
			fprintf(stderr, "FILE[%s],LINE[%d] ASSERT ERROR[%s]\n", __FILE__, __LINE__, #cond);\
			opt; \
		}


#define T_ERROR_VAL(cond) T_ASSERT(cond, return TINY_ERROR)
#define T_ERROR_PTR(cond) T_ASSERT(cond, return NULL)
#define T_ERROR_VOID(cond) T_ASSERT(cond, return)

#else
#define T_ERROR_VAL(cond)
#define T_ERROR_PTR(cond)
#define T_ERROR_VOID(cond)
#endif


#endif /* TINY_ASSERT_H_ */
