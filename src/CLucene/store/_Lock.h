/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_intlLock_
#define _lucene_store_intlLock_

#include "Lock.h"
#include <limits.h>

CL_NS_DEF(store)


class LocksType: public CL_NS(util)::CLHashSet<const char*, CL_NS(util)::Compare::Char, CL_NS(util)::Deletor::acArray>
{
public:
	LocksType(bool del)
	{
		setDoDelete(del);
	}
	virtual ~LocksType(){
	}
};

  class SingleInstanceLock: public LuceneLock {
  private:
	  const char* lockName;
	  LocksType* locks;
	  
  public:
	  SingleInstanceLock( LocksType* locks, const char* lockName );

	  bool obtain();
	  void release();
	  bool isLocked();
	  TCHAR* toString();
  };



  class NoLock: public LuceneLock {
	  bool obtain();
	  void release();
	  bool isLocked();
	  TCHAR* toString();
  };
  
  class FSLock: public LuceneLock {
  private:
	  char* lockFile;
  	  char* lockDir;
	  
  public:
	  FSLock( const char* _lockDir, const char* name );
	  ~FSLock();
	  
	  bool obtain();
	  void release();
	  bool isLocked();
	  TCHAR* toString();	  	  	  
  };
  
  // Utility class for executing code with exclusive access.
  template<typename T>
  class LuceneLockWith {
  private:
    LuceneLock* lock;
    int64_t lockWaitTimeout;

  protected:
    // Code to execute with exclusive access.
    virtual T doBody() = 0;

  // Constructs an executor that will grab the named lock.
  public:   
    /** Constructs an executor that will grab the named lock.
     *  Defaults lockWaitTimeout to LUCENE_COMMIT_LOCK_TIMEOUT.
     *  @deprecated Kept only to avoid breaking existing code.
     */
    LuceneLockWith(LuceneLock* lock, int64_t lockWaitTimeout) {
      this->lock = lock;
      this->lockWaitTimeout = lockWaitTimeout;
    }
    virtual ~LuceneLockWith(){
	} 

    /** Calls {@link #doBody} while <i>lock</i> is obtained.  Blocks if lock
     * cannot be obtained immediately.  Retries to obtain lock once per second
     * until it is obtained, or until it has tried ten times. Lock is released when
     * {@link #doBody} exits. */
    T runAndReturn() {
        bool locked = false;
        T ret = NULL;
        try {
            locked = lock->obtain(lockWaitTimeout);
            ret = doBody();
        }_CLFINALLY(
            if (locked) 
                lock->release();
        );
		return ret;
    }

	/** @see runAndReturn
     * Same as runAndReturn, except doesn't return any value.
	 * The only difference is that no void values are used
	 */
	void run() {
        bool locked = false;
        try {
            locked = lock->obtain(lockWaitTimeout);
            doBody();
        }_CLFINALLY(
            if (locked) 
                lock->release();
        );
    }
  };

CL_NS_END
#endif
