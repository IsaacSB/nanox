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

#include "mpiprocessor.hpp"
#include "schedule.hpp"
#include "debug.hpp"
#include "system.hpp"
#include <iostream>
#include <sched.h>
#include <unistd.h>
#include "mpi_ult.hpp"
#include "instrumentation.hpp"

using namespace nanos;
using namespace nanos::ext;

void * mpi_bootthread ( void *arg )
{
   MPIThread *self = static_cast<MPIThread *>( arg );

   self->run();
   
   pthread_exit ( 0 );
}

// TODO: detect at configure
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

void MPIThread::start ()
{
   pthread_attr_t attr;
   pthread_attr_init(&attr);

   // user-defined stack size
   if ( _stackSize > 0 ) {
     if ( _stackSize < PTHREAD_STACK_MIN ) {
       warning("specified thread stack too small, adjusting it to minimum size");
       _stackSize = PTHREAD_STACK_MIN;
     }

     if (pthread_attr_setstacksize( &attr, _stackSize ) )
       warning("couldn't set pthread stack size stack");
   }

   if ( pthread_create( &_pth, &attr, mpi_bootthread, this ) )
      fatal( "couldn't create thread" );
}

void MPIThread::runDependent ()
{
   WD &work = getThreadWD();
   setCurrentWD( work );

   MPIDD &dd = ( MPIDD & ) work.activateDevice( MPI );

   dd.getWorkFct()( work.getData() );
}

void MPIThread::join ()
{
   pthread_join( _pth,NULL );
   joined();
}

void MPIThread::bind( void )
{
   cpu_set_t cpu_set;
   long ncpus = sysconf( _SC_NPROCESSORS_ONLN );
   int cpu_idx = ( getCpuId() * sys.getBindingStride() ) + sys.getBindingStart();
   int cpu_id = ( (cpu_idx + ( cpu_idx/ncpus) ) % ncpus);
   
   // If using the socket scheduler...
   if ( sys.getDefaultSchedule() == "socket" )
   {
      // Set the number of socket
      int socket = cpu_id / sys.getCoresPerSocket();
      
      if ( socket >= sys.getNumSockets() ) {
         warning( "cpu id " << cpu_id << " is in socket #" << socket <<
                 ", while there are only " << sys.getNumSockets() << " sockets." );
      }
      
      setSocket( socket );
   }

   ensure( ( ( cpu_id >= 0 ) && ( cpu_id < ncpus ) ), "invalid value for cpu id" );
   CPU_ZERO( &cpu_set );
   CPU_SET( cpu_id, &cpu_set );
   verbose( "Binding thread " << getId() << " to cpu " << cpu_id );
   sched_setaffinity( ( pid_t ) 0, sizeof( cpu_set ), &cpu_set );
}

void MPIThread::yield()
{
   if (sched_yield() != 0)
      warning("sched_yield call returned an error");
}

// This is executed in between switching stacks
void MPIThread::switchHelperDependent ( WD *oldWD, WD *newWD, void *oldState  )
{
   MPIDD & dd = ( MPIDD & )oldWD->getActiveDevice();
   dd.setState( (intptr_t *) oldState );
}

bool MPIThread::inlineWorkDependent ( WD &wd )
{
   // Now the WD will be inminently run
   wd.start(WD::IsNotAUserLevelThread);

   MPIDD &dd = ( MPIDD & )wd.getActiveDevice();

   NANOS_INSTRUMENT ( static nanos_event_key_t key = sys.getInstrumentation()->getInstrumentationDictionary()->getEventKey("user-code") );
   NANOS_INSTRUMENT ( nanos_event_value_t val = wd.getId() );
   NANOS_INSTRUMENT ( sys.getInstrumentation()->raiseOpenStateAndBurst ( NANOS_RUNNING, key, val ) );
   ( dd.getWorkFct() )( wd.getData() );
   NANOS_INSTRUMENT ( sys.getInstrumentation()->raiseCloseStateAndBurst ( key ) );
   return true;
}

void MPIThread::switchTo ( WD *wd, SchedulerHelper *helper )
{
   // wd MUST have an active MPI Device when it gets here
   ensure( wd->hasActiveDevice(),"WD has no active MPI device" );
   MPIDD &dd = ( MPIDD & )wd->getActiveDevice();
   ensure( dd.hasStack(), "DD has no stack for ULT");

   ::switchStacksMpi(
       ( void * ) getCurrentWD(),
       ( void * ) wd,
       ( void * ) dd.getState(),
       ( void * ) helper );
}

void MPIThread::exitTo ( WD *wd, SchedulerHelper *helper)
{
   // wd MUST have an active MPI Device when it gets here
   ensure( wd->hasActiveDevice(),"WD has no active MPI device" );
   MPIDD &dd = ( MPIDD & )wd->getActiveDevice();
   ensure( dd.hasStack(), "DD has no stack for ULT");

   //TODO: optimize... we don't really need to save a context in this case
   ::switchStacksMpi(
      ( void * ) getCurrentWD(),
      ( void * ) wd,
      ( void * ) dd.getState(),
      ( void * ) helper );
}
