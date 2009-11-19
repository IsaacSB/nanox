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

#ifndef _NANOS_ATOMIC
#define _NANOS_ATOMIC

/* TODO: move to configure
#include <ext/atomicity.h>
#ifndef _GLIBCXX_ATOMIC_BUILTINS
#error "Atomic gcc builtins support is mandatory at this point"
#endif
*/

namespace nanos
{

   template<typename T>

   class Atomic
   {

      private:
         volatile T     _value;

      public:
         // constructor
         Atomic () {}

         Atomic ( T init ) : _value( init ) {}

         // copy constructor
         Atomic ( const Atomic &atm ) : _value( atm._value ) {}

         // assignment operator
         Atomic & operator= ( const Atomic &atm );
         Atomic & operator= ( const T val );
         // destructor
         ~Atomic() {}

         T fetchAndAdd ( T val=1 ) { return __sync_fetch_and_add( &_value,val ); }
	 T addAndFetch ( T val=1 ) { return __sync_add_and_fetch( &_value,val ); }
	 T fetchAndSub ( T val=1 ) { return __sync_fetch_and_sub( &_value,val ); }
	 T subAndFetch ( T val=1 ) { return __sync_sub_and_fetch( &_value,val ); }
	 T value() const { return _value; }

         //! pre-increment ++
         T operator++ ()			{ return addAndFetch(); }
         T operator-- ()			{ return subAndFetch(); }

         //! post-increment ++
         T operator++ ( int val ) 		{ return fetchAndAdd(); }
	 T operator-- ( int val ) 		{ return fetchAndSub(); }

	 //! += operator
	 T operator+= ( const T val ) { return addAndFetch(val); }
	 T operator+= ( const Atomic<T> &val ) { return addAndFetch(val.value()); }

	 T operator-= ( const T val ) { return subAndFetch(val); }
	 T operator-= ( const Atomic<T> &val ) { return subAndFetch(val.value()); }

	 //! equal operator
	 bool operator== ( const Atomic<T> &val ) { return value() == val.value(); }
	 bool operator!= ( const Atomic<T> &val ) { return value() != val.value(); }

	 bool operator< (const Atomic<T> &val ) { return value() < val.value(); }
	 bool operator> ( const Atomic<T> &val ) { return value() > val.value(); }
	 bool operator<= ( const Atomic<T> &val ) { return value() <= val.value(); }
         bool operator>= ( const Atomic<T> &val ) { return value() >= val.value(); }

         volatile T & override () { return _value; }
   };

   template<typename T>
   Atomic<T> & Atomic<T>::operator= ( const T val )
   {
      this->_value = val;
      return *this;
   }

   template<typename T>
   Atomic<T> & Atomic<T>::operator= ( const Atomic<T> &val )
   {
      return operator=( val._value );
   }

   class Lock
   {

      private:
         typedef enum { FREE=0, BUSY=1 } state_t;

         volatile state_t      _state;

         // disable copy constructor and assignment operator
         Lock( const Lock &lock );
         const Lock & operator= ( const Lock& );

      public:
         // constructor
         Lock( state_t init=FREE ) : _state( init ) {};

         // destructor
         ~Lock() {}

         void acquire ( void );
         bool tryAcquire ( void );
         void release ( void );

         const state_t operator* () const { return _state; }

         const state_t getState () const { return _state; }

         void operator++ ( int val ) { acquire(); }

         void operator-- ( int val ) { release(); }
   };

   inline void Lock::acquire ( void )
   {

   spin:

      while ( _state == BUSY );

      if ( __sync_lock_test_and_set( &_state,BUSY ) ) goto spin;
   }

   inline bool Lock::tryAcquire ( void )
   {
      if ( _state == FREE ) {
         if ( __sync_lock_test_and_set( &_state,BUSY ) ) return false;
         else return true;
      } else return false;
   }

   inline void Lock::release ( void )
   {
      __sync_lock_release( &_state );
   }

   inline void memoryFence () { __sync_synchronize(); }

   inline bool compareAndSwap( int *ptr, int oldval, int newval ) { return __sync_bool_compare_and_swap ( ptr, oldval, newval );}

};

#endif
