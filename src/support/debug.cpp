/*************************************************************************************/
/*      Copyright 2013 Barcelona Supercomputing Center                               */
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

#include "debug.hpp"
#include "os.hpp"
#include <stdlib.h>
#include <execinfo.h>
#include <iostream>

void nanos::printBt( std::ostream &o ) {
   void* tracePtrs[100];
   int count = backtrace( tracePtrs, 100 );
   char** funcNames = backtrace_symbols( tracePtrs, count );
   o << "+--------------------------------------" << std::endl;

   // Print the stack trace
   for( int ii = 0; ii < count; ii++ )
      o << "| " << funcNames[ii] << std::endl;

   // Free the string pointers
   free( funcNames );
   o << "+--------------------------------------" << std::endl;
}

void nanos::printCpuSet( std::ostream &o, const std::string str, const cpu_set_t *cpu_set )
{
   std::ostringstream bitset;
   for ( int i = 0; i < OS::getMaxProcessors(); i++ ) {
      ( CPU_ISSET( i, cpu_set ) ) ? bitset << "1 " : bitset << "0 ";
   }
   o << str << bitset.str() << std::endl;
}
