/*! 
@file epCriticalSectionEx.h
@author Woong Gyu La a.k.a Chris. <juhgiyo@gmail.com>
@date April 16, 2011
@brief A CriticalSectionEx Interface
@version 2.0

@section LICENSE

Copyright (C) 2012  Woong Gyu La <juhgiyo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

@section DESCRIPTION

An Interface for CriticalSectionEx Class.

*/
#ifndef __EP_CRITICAL_SECTION_EX_H__
#define __EP_CRITICAL_SECTION_EX_H__
#include "epLib.h"
#include "epSystem.h"
#include "epBaseLock.h"

#if _DEBUG
#include <vector>
#endif //_DEBUG

namespace epl
{
	/*! 
	@class CriticalSectionEx epCriticalSectionEx.h
	@brief A class that handles the CriticalSectionEx functionality.
	*/
	class EP_FOUNDATION CriticalSectionEx :public BaseLock
	{
	public:
		/*!
		Default Constructor

		Initializes the lock
		*/
		CriticalSectionEx();

		/*!
		Default Destructor

		Deletes the lock
		*/
		virtual ~CriticalSectionEx();

		/*!
		Locks the Critical Section
		*/
		virtual void Lock();

		/*!
		Try to Lock the Critical Section

		If other thread is already in the Critical Section, it just returns false and continue, otherwise obtain the Critical Section.
		@return true if the lock is succeeded, otherwise false.
		*/
		virtual long TryLock();

		/*!
		Locks the Critical Section

		if other thread is already in the Critical Section,
		and if it fails to lock in given time, it returns false, otherwise lock and return true.
		@param[in] dwMilliSecond the wait time.
		@return true if the lock is succeeded, otherwise false.
		*/
		virtual long TryLockFor(const unsigned int dwMilliSecond);

		/*!
		Leave the Critical Section

		The Lock and Unlock has to be matched for each Critical Section.
		*/
		virtual void Unlock();

	private:
		/*!
		Default Copy Constructor

		Initializes the CriticalSectionEx
		**Should not call this
		@param[in] b the second object
		*/
		CriticalSectionEx(const CriticalSectionEx& b){EP_ASSERT(0);}
		/*!
		Assignment operator overloading
		**Should not call this
		@param[in] b the second object
		@return the new copied object
		*/
		CriticalSectionEx & operator=(const CriticalSectionEx&b)
		{
			EP_ASSERT(0);
			return *this;
		}

		/// the actual lock member.
		CRITICAL_SECTION m_criticalSection;
#if _DEBUG
		std::vector<int> m_threadList;
#endif //_DEBUG
	};

}

#endif //__EP_CRITICAL_SECTION_EX_H__