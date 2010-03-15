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

#ifndef _NANOS_THREAD_TEAM
#define _NANOS_THREAD_TEAM

#include <vector>
#include "basethread.hpp"
#include "schedule.hpp"
#include "barrier.hpp"


namespace nanos
{

   class ThreadTeam
   {

      private:
         std::vector<BaseThread *>    _threads;
         int                          _idleThreads;
         int                          _numTasks;
         Barrier &                    _barrier;
         int                          _singleGuardCount;
         SchedulePolicy &             _schedulePolicy;
         ScheduleTeamData *           _scheduleData;

         // disable copy constructor & assignment operation
         ThreadTeam( const ThreadTeam &sys );
         const ThreadTeam & operator= ( const ThreadTeam &sys );

      public:

         ThreadTeam ( int maxThreads, SchedulePolicy &policy, ScheduleTeamData *data, Barrier &barrier )
                    : _idleThreads( 0 ), _numTasks( 0 ), _barrier(barrier), _singleGuardCount( 0 ),
                      _schedulePolicy(policy), _scheduleData(data)
         {
               _threads.reserve( maxThreads );
         }

         ~ThreadTeam ()
         {
            delete &_barrier;
            ensure(size() == 0, "Destroying non-empty team!");
         }


         unsigned size() const { return _threads.size(); }

         /*! \brief Initializes team structures dependent on the number of threads.
          *
          *  This method initializes the team structures that depend on the number of threads.
          *  It *must* be called after all threads have entered the team
          *  It *must* be called by a single thread
          */
         void init ()
         {
            _barrier.init( size() );
         }

         /*! This method should be called when there's a change in the team size to readjust all structures
          *  \warn Not implemented yet!
          */
         void resized ()
         {
            // TODO
            _barrier.resize(size());
         }

         BaseThread & getThread ( int i ) const { return *_threads[i]; }
         BaseThread & getThread ( int i ) { return *_threads[i]; }

         const BaseThread & operator[]  ( int i ) const { return getThread(i); }
         BaseThread & operator[]  ( int i ) { return getThread(i); }

         /*! \brief adds a thread to the team pool, returns the thread id in the team
          *
          */
         unsigned addThread ( BaseThread *thread ) {
            unsigned id = size();
            _threads.push_back( thread );
            return id;
         }

         void barrier() { _barrier.barrier( myThread->getTeamId() ); }

         bool singleGuard( int local );

         ScheduleTeamData * getScheduleData() const { return _scheduleData; }
         SchedulePolicy & getSchedulePolicy() const { return _schedulePolicy; }
   };

}

#endif
