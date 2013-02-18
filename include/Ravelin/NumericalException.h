/****************************************************************************
 * Copyright 2008 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

#ifndef _NUMERICAL_EXCEPTION_H_
#define _NUMERICAL_EXCEPTION_H_

#include <stdexcept>

namespace Ravelin {

/// Exception thrown when general numerical error occurs 
class NumericalException : public std::runtime_error
{
  public:
    NumericalException() : std::runtime_error("NumericalException error") {}
    NumericalException(const char* error) : std::runtime_error(error) {}
}; // end class


} // end namespace

#endif

