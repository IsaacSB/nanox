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

#include "os.hpp"
#include "compatibility.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef IS_BGQ_MACHINE
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#endif

extern char **environ;

using namespace nanos;


static void do_nothing(void *) {}
#define INIT_NULL { do_nothing, 0 }

// Make sure the two special linker sections exist
LINKER_SECTION(nanos_modules, const char *, NULL)
LINKER_SECTION(nanos_init, nanos_init_desc_t , INIT_NULL)
LINKER_SECTION(nanos_post_init, nanos_init_desc_t , INIT_NULL)

long OS::_argc = 0; 
char ** OS::_argv = 0;

OS::ModuleList * OS::_moduleList = 0;
OS::InitList * OS::_initList = 0;
OS::InitList * OS::_postInitList = 0;
cpu_set_t OS::_systemMask;
cpu_set_t OS::_processMask;

static void findArgs (long *argc, char ***argv) 
{
   long *p; 
   int i; 

   // variables are before environment 
   p=( long * )environ; 

   // go backwards until we find argc 
   p--; 

   for ( i = 0 ; *( --p ) != i; i++ ) {}

   *argc = *p; 
   *argv = ( char ** ) p+1; 
}

void OS::init ()
{
   findArgs(&_argc,&_argv);
   _moduleList = NEW ModuleList(&__start_nanos_modules,&__stop_nanos_modules);
   _initList = NEW InitList(&__start_nanos_init, &__stop_nanos_init);
   _postInitList = NEW InitList(&__start_nanos_post_init, &__stop_nanos_post_init);

   CPU_ZERO( &_systemMask );
   for (int i=0; i<OS::getMaxProcessors(); i++) {
      CPU_SET( i, &_systemMask );
   }

   CPU_ZERO( &_processMask );
#ifdef IS_BGQ_MACHINE
   uint32_t myT = Kernel_MyTcoord();
   uint64_t mask = Kernel_ThreadMask( myT );
   uint64_t x, t;

   x = mask;
   /* 64-bit reversing to mantain compatibility with cpu_set_t */
   x = (x << 32) | (x >> 32);
   x = (x & 0x0001FFFF0001FFFFLL) << 15 |
      (x & 0xFFFE0000FFFE0000LL) >> 17;
   t = (x ^ (x >> 10)) & 0x003F801F003F801FLL;
   x = (t | (t << 10)) ^ x;
   t = (x ^ (x >> 4)) & 0x0E0384210E038421LL;
   x = (t | (t << 4)) ^ x;
   t = (x ^ (x >> 2)) & 0x2248884222488842LL;
   x = (t | (t << 2)) ^ x;
   /***/
   mask = x;

   memcpy( &_processMask, &mask, sizeof(uint64_t) );
#else
   sched_getaffinity( 0, sizeof(cpu_set_t), &_processMask );
#endif
}

void * OS::loadDL( const std::string &dir, const std::string &name )
{
   std::string filename;
   if ( dir != "") {
      filename = dir + "/" + name + ".so";
   } else {
      filename = name + ".so";
   }
   /* open the module */
   return dlopen ( filename.c_str(), RTLD_NOW );
}

void * OS::loadLocalDL(  )
{
   return dlopen ( NULL, RTLD_NOW|RTLD_GLOBAL );
}

void * OS::dlFindSymbol( void *dlHandler, const std::string &symbolName )
{
   return dlsym ( dlHandler, symbolName.c_str() );
}

void * OS::dlFindSymbol( void *dlHandler, const char *symbolName )
{
   return dlsym ( dlHandler, symbolName );
}

void OS::getSystemAffinity( cpu_set_t *cpu_set )
{
   memcpy( cpu_set, &_systemMask, sizeof(cpu_set_t) );
}

void OS::getProcessAffinity( cpu_set_t *cpu_set )
{
   memcpy( cpu_set, &_processMask, sizeof(cpu_set_t) );
}

int OS::getMaxProcessors ( void )
{
#ifdef IS_BGQ_MACHINE
   return (int) 64;
#else
#ifdef _SC_NPROCESSORS_ONLN
   return (int) sysconf(_SC_NPROCESSORS_CONF);
#else
   return (int) 0;
#endif
#endif
}

int OS::nanosleep ( unsigned long long nanoseconds )
{
#ifdef IS_BGQ_MACHINE
   /* BG/Q has an Extended Thread Model where you are allowed to bind pthreads to
    * physical threads that were not originally allocated to that process.
    * In this mode, these particular pthreads have some restrictions, like for
    * example setting and handling the itimer.
    * For this reason we must ensure that this function is being called from a thread
    * originally allocated within the initial affinity mask.
    * */
   cpu_set_t cpu_set;
   CPU_ZERO( &cpu_set );
   sched_getaffinity( 0, sizeof(cpu_set_t), &cpu_set );
   CPU_AND( &cpu_set, &cpu_set, &_processMask );
   if ( CPU_COUNT( &cpu_set ) == 0 )
      return 0;
#endif

   struct timespec req, rem;
   req.tv_sec = (time_t) ( nanoseconds / 1000000000ULL );
   req.tv_nsec = (long) ( nanoseconds % 1000000000ULL );
   return ::nanosleep( &req, &rem );
}
