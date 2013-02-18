/****************************************************************************
 * Copyright 2008 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

#ifndef _INVALID_INDEX_EXCEPTION_H_
#define _INVALID_INDEX_EXCEPTION_H_

#include <stdexcept>

namespace Ravelin {

/// Exception thrown when trying to access the index beyond the range of the data 
class InvalidIndexException : public std::runtime_error
{
  public:
    InvalidIndexException() : std::runtime_error("Invalid index") {}
}; // end class


} // end namespace

#endif

