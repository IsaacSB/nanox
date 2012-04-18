/*************************************************************************************/
/*      Copyright 2009 Barcelona Supercomputing Center                               */
/*                                                                                   */
/*      This file is part of the NANOS++ library.                                    */
/*                                                                                   */
/*      NANOS++ is free software: you can redistribute it and/or modify              */
/*      it under the terms of the GNU Lesser General Public License as published by  */
/*      the Free Software Foundation, either version 3 of the License, or            */
/*      (at your option) any later version.                                          */
/*                                                                                   */
/*      NANOS++ is distributed in the hope that it will be useful,                   */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/*      GNU Lesser General Public License for more details.                          */
/*                                                                                   */
/*      You should have received a copy of the GNU Lesser General Public License     */
/*      along with NANOS++.  If not, see <http://www.gnu.org/licenses/>.             */
/*************************************************************************************/

#define NANOS_REDUCTION_OP_ADD(a,b) (a + b)
#define NANOS_REDUCTION_OP_SUB(a,b) (a - b)
#define NANOS_REDUCTION_OP_PROD(a,b) (a * b)
#define NANOS_REDUCTION_OP_AND(a,b) (a & b)
#define NANOS_REDUCTION_OP_OR(a,b) (a | b)
#define NANOS_REDUCTION_OP_XOR(a,b) (a ^ b)
#define NANOS_REDUCTION_OP_LAND(a,b) (a && b)
#define NANOS_REDUCTION_OP_LOR(a,b) (a || b)
#define NANOS_REDUCTION_OP_MAX(a,b) ((a > b)? a : b)
#define NANOS_REDUCTION_OP_MIN(a,b) ((a < b)? a : b)

#define NANOS_REDUCTION_DECL(Op,Type)\
   void nanos_reduction_bop_##Op## _##Type ( void *arg1, void *arg2); \
   void nanos_reduction_vop_##Op## _##Type ( int i, void *arg1, void *arg2); \

#define NANOS_REDUCTION_DEF(Op,Op2,Type,Type2) \
   void nanos_reduction_bop_##Op## _##Type ( void *arg1, void *arg2) \
   { \
      Type2 *s = (Type2 *) arg1; \
      Type2 *v = (Type2 *) arg2; \
      *s = Op2(*s,*v); \
   } \
   void nanos_reduction_vop_##Op## _##Type ( int n, void *arg1, void *arg2) \
   { \
      int i; \
      Type2 *s = (Type2 *) arg1; \
      Type2 *v = (Type2 *) arg2; \
      for (i = 0; i < n; i++) *s = Op2(*s,v[i]); \
   }

#define NANOS_REDUCTION_INT_TYPES_DECL(Op) \
   NANOS_REDUCTION_DECL(Op,char) \
   NANOS_REDUCTION_DECL(Op,uchar) \
   NANOS_REDUCTION_DECL(Op,schar) \
   NANOS_REDUCTION_DECL(Op,short) \
   NANOS_REDUCTION_DECL(Op,ushort) \
   NANOS_REDUCTION_DECL(Op,int) \
   NANOS_REDUCTION_DECL(Op,uint) \
   NANOS_REDUCTION_DECL(Op,long) \
   NANOS_REDUCTION_DECL(Op,ulong) \
   NANOS_REDUCTION_DECL(Op,longlong) \
   NANOS_REDUCTION_DECL(Op,ulonglong)

#define NANOS_REDUCTION_REAL_TYPES_DECL(Op) \
   NANOS_REDUCTION_DECL(Op,float) \
   NANOS_REDUCTION_DECL(Op,double) \
   NANOS_REDUCTION_DECL(Op,longdouble) \
   NANOS_REDUCTION_DECL(Op,_Bool) 

#define NANOS_REDUCTION_INT_TYPES_DEF(Op,Op2) \
   NANOS_REDUCTION_DEF(Op,Op2,char,char) \
   NANOS_REDUCTION_DEF(Op,Op2,uchar,unsigned char) \
   NANOS_REDUCTION_DEF(Op,Op2,schar,signed char) \
   NANOS_REDUCTION_DEF(Op,Op2,short,short) \
   NANOS_REDUCTION_DEF(Op,Op2,ushort,unsigned short) \
   NANOS_REDUCTION_DEF(Op,Op2,int,int) \
   NANOS_REDUCTION_DEF(Op,Op2,uint,unsigned int) \
   NANOS_REDUCTION_DEF(Op,Op2,long,long) \
   NANOS_REDUCTION_DEF(Op,Op2,ulong,unsigned long) \
   NANOS_REDUCTION_DEF(Op,Op2,longlong,long long) \
   NANOS_REDUCTION_DEF(Op,Op2,ulonglong,unsigned long long)

#define NANOS_REDUCTION_REAL_TYPES_DEF(Op,Op2) \
   NANOS_REDUCTION_DEF(Op,Op2,float,float) \
   NANOS_REDUCTION_DEF(Op,Op2,double,double) \
   NANOS_REDUCTION_DEF(Op,Op2,longdouble,long double)

// REDUCTION BUILTIN DECLARATION
#ifndef _NANOS_REDUCTION_H_
#define _NANOS_REDUCTION_H_

NANOS_REDUCTION_INT_TYPES_DECL(add)
NANOS_REDUCTION_REAL_TYPES_DECL(add)

NANOS_REDUCTION_INT_TYPES_DECL(sub)
NANOS_REDUCTION_REAL_TYPES_DECL(sub)

NANOS_REDUCTION_INT_TYPES_DECL(prod)
NANOS_REDUCTION_REAL_TYPES_DECL(prod)

NANOS_REDUCTION_INT_TYPES_DECL(and)

NANOS_REDUCTION_INT_TYPES_DECL(or)

NANOS_REDUCTION_INT_TYPES_DECL(xor)

NANOS_REDUCTION_INT_TYPES_DECL(land)
NANOS_REDUCTION_REAL_TYPES_DECL(land) /* XXX */

NANOS_REDUCTION_INT_TYPES_DECL(lor)
NANOS_REDUCTION_REAL_TYPES_DECL(lor) /* XXX */

NANOS_REDUCTION_INT_TYPES_DECL(max)
NANOS_REDUCTION_REAL_TYPES_DECL(max)

NANOS_REDUCTION_INT_TYPES_DECL(min)
NANOS_REDUCTION_REAL_TYPES_DECL(min)

#endif

