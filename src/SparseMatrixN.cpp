/****************************************************************************
 * Copyright 2013 Evan Drumwright
 * This library is distributed under the terms of the GNU  General Public 
 * License (found in COPYING).
 ****************************************************************************/

SPARSEMATRIXN::SPARSEMATRIXN()
{
  _rows = _columns = 0;
}

SPARSEMATRIXN::SPARSEMATRIXN(unsigned m, unsigned n, const map<pair<unsigned, unsigned>, REAL>& values)
{
  set(m, n, values);
}

SPARSEMATRIXN::SPARSEMATRIXN(unsigned m, unsigned n, shared_array<unsigned> ptr, shared_array<unsigned> indices, shared_array<REAL> data) 
{
  _rows = m;
  _columns = n;
  _data = data; 
  _ptr = ptr; 
  _indices = indices; 
}

/// Creates a sparse matrix from a dense matrix
SPARSEMATRIXN::SPARSEMATRIXN(const MATRIXN& m)
{
  map<pair<unsigned, unsigned>, REAL> values;
  for (unsigned i=0; i< m.rows(); i++)
    for (unsigned j=0; j< m.columns(); j++)
      if (std::fabs(m(i,j)) > EPS)
        values[make_pair(i,j)] = m(i,j);

  // setup the matrix
  set(m.rows(), m.columns(), values);
}

/// Sets up an identity matrix from a sparse matrix
SPARSEMATRIXN SPARSEMATRIXN::identity(unsigned n)
{
  SPARSEMATRIXN m;

  // init matrix dimensions
  m._rows = m._columns = n;

  // initialize arrays
  m._data = shared_array<REAL>(new REAL[n]);
  m._ptr = shared_array<unsigned>(new unsigned[n+1]);
  m._indices = shared_array<unsigned>(new unsigned[n]);

  // populate the matrix data, indices, and row pointers
  for (unsigned i=0; i< n; i++)
  {
    m._data[i] = (REAL) 1.0;
    m._indices[i] = i;
    m._ptr[i] = i;
  }
  m._ptr[n] = n;  

  return m;
}

/// Sets up a sparse matrix from a map 
void SPARSEMATRIXN::set(unsigned m, unsigned n, const map<pair<unsigned, unsigned>, REAL>& values)
{
  const unsigned nv = values.size();
  _rows = m;
  _columns = n;
  _data = shared_array<REAL>(new REAL[nv]);
  _ptr = shared_array<unsigned>(new unsigned[m+1]);
  _indices = shared_array<unsigned>(new unsigned[nv]);

  // populate the matrix data
  map<pair<unsigned, unsigned>, REAL>::const_iterator i = values.begin();
  unsigned j;
  for (j=0; j< nv; j++, i++)
    _data[j] = i->second;

  // setup ptr
  j = 0;
  unsigned k=0;
  _ptr[0] = j;
  for (unsigned r=0; r< m; r++)
  {
    for (unsigned s=0; s< n; s++)
      if (values.find(make_pair(r, s)) != values.end())
      {
        j++;
        _indices[k++] = s;
      }
    _ptr[r+1] = j;
  }
}

/// Gets a column of the sparse matrix as a sparse vector
SPARSEVECTORN& SPARSEMATRIXN::get_column(unsigned i, SPARSEVECTORN& column) const
{
  if (i >= _columns)
    throw InvalidIndexException();

  // determine the number of elements
  unsigned nelm = 0;
  for (unsigned j=0; j< _ptr[_rows]; j++)
    if (_indices[j] == i)
      nelm++;

  // create new arrays
  shared_array<unsigned> indices(new unsigned[nelm]);
  shared_array<REAL> data(new REAL[nelm]);
  unsigned elm = 0;
  for (unsigned row=0; row< _rows; row++)
    for (unsigned k=_ptr[row]; k< _ptr[row+1]; k++)
    {
      unsigned col = _indices[k];
   
      // look for early exit
      if (col > i)
        break; 

      // look for match
      if (col == i)
      {
        indices[elm] = row;
        data[elm] = _data[k];
        elm++;
      }
    }

  // create the sparse vector
  column = SPARSEVECTORN(_rows, nelm, indices, data);
  return column;
}

/// Gets a row of the sparse matrix as a sparse vector
SPARSEVECTORN& SPARSEMATRIXN::get_row(unsigned i, SPARSEVECTORN& row) const
{
  if (i >= _rows)
    throw InvalidIndexException();

  // get the number of elements
  unsigned nelm = _ptr[i+1] - _ptr[i];
  
  // create new arrays
  shared_array<unsigned> indices(new unsigned[nelm]);
  shared_array<REAL> data(new REAL[nelm]);
  for (unsigned j=_ptr[i], k=0; j< _ptr[i+1]; j++, k++)
  {
    indices[k] = _indices[j];
    data[k] = _data[j];
  }

  row = SPARSEVECTORN(_columns, nelm, indices, data);
  return row;
}

/// Gets a submatrix of the sparse matrix
SPARSEMATRIXN SPARSEMATRIXN::get_sub_mat(unsigned rstart, unsigned rend, unsigned cstart, unsigned cend) const
{
  SPARSEMATRIXN sub(rend - rstart, cend - cstart);

  if (rend < rstart || cend < cstart)
    throw InvalidIndexException();
  if (rend > _rows || cend > _columns)
    throw InvalidIndexException();

  // determine how many values are in the sparse matrix
  unsigned nv = 0;
  for (unsigned row=rstart; row < rend; row++)
    for (unsigned col=_ptr[row]; col < _ptr[row+1]; col++)
      if (_indices[col] >= cstart && _indices[col] < cend)
        nv++;

  // setup arrays
  shared_array<REAL> data(new REAL[nv]);
  shared_array<unsigned> ptr(new unsigned[rend - rstart + 1]);
  shared_array<unsigned> indices(new unsigned[nv]);

  // copy the data
  for (unsigned row=rstart, didx=0; row < rend; row++)
    for (unsigned col=_ptr[row]; col < _ptr[row+1]; col++)
      if (_indices[col] >= cstart && _indices[col] < cend)
        data[didx++] = _data[col];

  // setup ptr and indices
  ptr[0] = 0;
  unsigned j = 0;
  for (unsigned row=rstart; row < rend; row++)
  {
    ptr[row - rstart + 1] = ptr[row - rstart];
    for (unsigned col=_ptr[row]; col < _ptr[row+1]; col++)
      if (_indices[col] >= cstart && _indices[col] < cend)
      {
        ptr[row - rstart + 1]++;
        indices[j++] = _indices[col] - cstart;
      }
  }

  // setup pointers
  sub._ptr = ptr;
  sub._indices = indices;
  sub._data = data;

  return sub;
}

/// Multiplies this sparse matrix by a dense matrix
MATRIXN& SPARSEMATRIXN::mult(const MATRIXN& m, MATRIXN& result) const
{
  if (_columns != m.rows())
    throw MissizeException();

  // setup the result matrix
  result.set_zero(_rows, m.columns());

  // do the calculation
  for (unsigned col=0; col < m.columns(); col++)
    for (unsigned row=0; row < _rows; row++)
    {
      REAL dot = (REAL) 0.0;
      unsigned row_start = _ptr[row];
      unsigned row_end = _ptr[row+1];
      for (unsigned jj= row_start; jj< row_end; jj++)
        dot += _data[jj] * m(_indices[jj], col);
      result(row,col) += dot;
    }

  return result;
}

/// Multiplies this sparse matrix by a dense vector
VECTORN& SPARSEMATRIXN::mult(const VECTORN& x, VECTORN& result) const
{
  if (_columns != x.size())
    throw MissizeException();

  // setup the result matrix
  result.set_zero(_rows);

  for (unsigned row=0; row < _rows; row++)
  {
    REAL dot = (REAL) 0.0;
    unsigned row_start = _ptr[row];
    unsigned row_end = _ptr[row+1];
    for (unsigned jj= row_start; jj< row_end; jj++)
      dot += _data[jj] * x[_indices[jj]];

    result[row] += dot;
  }

  return result;
}

/// Multiplies the transpose of this sparse matrix by a dense vector
VECTORN& SPARSEMATRIXN::transpose_mult(const VECTORN& x, VECTORN& result) const
{
  if (_rows != x.size())
    throw MissizeException();

  // setup the result vector
  result.set_zero(_columns);

  for (unsigned row=0; row< _rows; row++)
    for (unsigned k=_ptr[row]; k< _ptr[row+1]; k++)
      result[_indices[k]] += _data[k] * x[row];

  return result;
}

/// Multiplies the transpose of this sparse matrix by a dense matrix
MATRIXN& SPARSEMATRIXN::transpose_mult(const MATRIXN& m, MATRIXN& result) const
{
  if (_rows != m.rows())
    throw MissizeException();

  result.set_zero(_columns, m.columns());

  for (unsigned col=0; col< m.columns(); col++)
    for (unsigned row=0; row< _rows; row++)
      for (unsigned k=_ptr[row]; k< _ptr[row+1]; k++)
        result(_indices[k],col) += _data[k] * m(row, col);

  return result;
}

/// Multiplies this matrix by the transpose of a dense matrix
MATRIXN& SPARSEMATRIXN::mult_transpose(const MATRIXN& m, MATRIXN& result) const
{
  if (_columns != m.columns())
    throw MissizeException();

  // setup the result matrix
  result.set_zero(_rows, m.rows());

  for (unsigned col=0; col < m.rows(); col++)
    for (unsigned row=0; row < _rows; row++)
    {
      REAL dot = (REAL) 0.0;
      unsigned row_start = _ptr[row];
      unsigned row_end = _ptr[row+1];
      for (unsigned jj= row_start; jj< row_end; jj++)
        dot += _data[jj] * m(col, _indices[jj]);
      result(row,col) += dot;
    }

  return result;
}

/// Multiplies the transpose of this sparse matrix by the transpose of a dense matrix
MATRIXN& SPARSEMATRIXN::transpose_mult_transpose(const MATRIXN& m, MATRIXN& result) const
{
  if (_rows != m.columns())
    throw MissizeException();

  // setup the result matrix
  result.set_zero(_columns, m.rows());

  for (unsigned col=0; col< m.rows(); col++)
    for (unsigned row=0; row< _rows; row++)
      for (unsigned k=_ptr[row]; k< _ptr[row+1]; k++)
        result(_indices[k],col) += _data[k] * m(col, row);

  return result;
}

/// Gets a dense matrix from this sparse matrix
MATRIXN& SPARSEMATRIXN::to_dense(MATRIXN& m) const
{
  // resize m and make it zero
  m.set_zero(_rows, _columns);

  for (unsigned row=0; row< _rows; row++)
    for (unsigned k=_ptr[row]; k< _ptr[row+1]; k++)
      m(row, _indices[k]) = _data[k];

  return m;
}

/// Subtracts a sparse matrix from this one -- attempts to do it in place
SPARSEMATRIXN& SPARSEMATRIXN::operator-=(const SPARSEMATRIXN& m)
{
  // check rows/columns match up
  if (_rows != m._rows || _columns != m._columns)
    throw MissizeException();

  // iterate through both matrices and see how many non-zero elements we have
  unsigned nz = 0;
  for (unsigned i=0; i< _rows; i++) 
    for (unsigned j=0; j< _columns; j++)
    {
      // determine whether (i,j) is stored in this
      bool found = false;
      for (unsigned k=_ptr[i]; k< _ptr[i+1]; k++)
        if (_indices[k] == j)
        {
          nz++;
          found = true;
          break;
        }
      if (found)
        continue;

      // wasn't found, see whether (i,j) is stored in m
      for (unsigned k=m._ptr[i]; k< m._ptr[i+1]; k++)
        if (m._indices[k] == j)
        {
          nz++;
          break;
        }
    }

  // two cases: nz == ptr[_rows] and nz != ptr[_rows]
  if (nz == _ptr[_rows])
  {
    // don't need to create a new matrix.. nonzero parts of matrices overlap
    for (unsigned i=0, r=0; i< _rows; i++) 
      for (unsigned j=0; j< _columns; j++)
      {
        // determine whether (i,j) is stored in m
        bool found = false;
        for (unsigned k=_ptr[i]; k< _ptr[i+1]; k++)
          if (_indices[k] == j)
          {
            found = true; 
            break;
          }

        // see whether (i,j) is stored in m
        for (unsigned k=m._ptr[i]; k< m._ptr[i+1]; k++)
          if (m._indices[k] == j)
          {
            _data[r] -= m._data[k];
            break; 
          }

        if (found)
          r++;
      }

    return *this;
  }


  // matrices do not match up..  need to create a new matrix
  shared_array<unsigned> nptr(new unsigned[_rows+1]);
  shared_array<unsigned> nindices(new unsigned[nz]);
  shared_array<REAL> ndata(new REAL[nz]);
  
  // setup ptr
  nptr[0] = 0;
  for (unsigned i=0; i< _rows; i++) 
  {
    nptr[i+1] = nptr[i];
    for (unsigned j=0; j< _columns; j++)
    {
      // determine whether (i,j) is stored in this
      bool found = false;
      for (unsigned k=_ptr[i]; k< _ptr[i+1]; k++)
        if (_indices[k] == j)
        {
          nptr[i+1]++;
          found = true;
          break;
        }
      if (found)
        continue;

      // wasn't found, see whether (i,j) is stored in m
      for (unsigned k=m._ptr[i]; k< m._ptr[i+1]; k++)
        if (m._indices[k] == j)
        {
          nptr[i+1]++;
          break;
        }
    }
  }

  // setup data and indices
  nz = 0;
  for (unsigned i=0; i< _rows; i++) 
    for (unsigned j=0; j< _columns; j++)
    {
      // determine whether (i,j) is stored in this
      bool found = false;
      for (unsigned k=_ptr[i]; k< _ptr[i+1]; k++)
        if (_indices[k] == j)
        {
          ndata[nz] = _data[k];
          nindices[nz] = _indices[k];
          found = true;
          break;
        }

      // see whether (i,j) is stored in m
      for (unsigned k=m._ptr[i]; k< m._ptr[i+1]; k++)
        if (m._indices[k] == j)
        {
          if (found)
            ndata[nz] -= m._data[k];
          else
          {
            ndata[nz] = -m._data[k];
            nindices[nz] = m._indices[k];
            found = true;
          }
          break;
        }

      // update nz
      if (found)
        nz++;
    }

  // setup pointers to new arrays
  this->_ptr = nptr;
  this->_indices = nindices;
  this->_data = ndata;
  return *this;
}

/// Adds a sparse matrix to this one -- attempts to do it in place
SPARSEMATRIXN& SPARSEMATRIXN::operator+=(const SPARSEMATRIXN& m)
{
  // check rows/columns match up
  if (_rows != m._rows || _columns != m._columns)
    throw MissizeException();

  // iterate through both matrices and see how many non-zero elements we have
  unsigned nz = 0;
  for (unsigned i=0; i< _rows; i++) 
    for (unsigned j=0; j< _columns; j++)
    {
      // determine whether (i,j) is stored in this
      bool found = false;
      for (unsigned k=_ptr[i]; k< _ptr[i+1]; k++)
        if (_indices[k] == j)
        {
          nz++;
          found = true;
          break;
        }
      if (found)
        continue;

      // wasn't found, see whether (i,j) is stored in m
      for (unsigned k=m._ptr[i]; k< m._ptr[i+1]; k++)
        if (m._indices[k] == j)
        {
          nz++;
          break;
        }
    }

  // two cases: nz == ptr[_rows] and nz != ptr[_rows]
  if (nz == _ptr[_rows])
  {
    // don't need to create a new matrix.. nonzero parts of matrices overlap
    for (unsigned i=0, r=0; i< _rows; i++) 
      for (unsigned j=0; j< _columns; j++)
      {
        // determine whether (i,j) is stored in m
        bool found = false;
        for (unsigned k=_ptr[i]; k< _ptr[i+1]; k++)
          if (_indices[k] == j)
          {
            found = true; 
            break;
          }

        // see whether (i,j) is stored in m
        for (unsigned k=m._ptr[i]; k< m._ptr[i+1]; k++)
          if (m._indices[k] == j)
          {
            _data[r] += m._data[k];
            break; 
          }

        if (found)
          r++;
      }

    return *this;
  }

  // matrices do not match up..  need to create a new matrix
  shared_array<unsigned> nptr(new unsigned[_rows+1]);
  shared_array<unsigned> nindices(new unsigned[nz]);
  shared_array<REAL> ndata(new REAL[nz]);
  
  // setup ptr
  nptr[0] = 0;
  for (unsigned i=0; i< _rows; i++) 
  {
    nptr[i+1] = nptr[i];
    for (unsigned j=0; j< _columns; j++)
    {
      // determine whether (i,j) is stored in this
      bool found = false;
      for (unsigned k=_ptr[i]; k< _ptr[i+1]; k++)
        if (_indices[k] == j)
        {
          nptr[i+1]++;
          found = true;
          break;
        }
      if (found)
        continue;

      // wasn't found, see whether (i,j) is stored in m
      for (unsigned k=m._ptr[i]; k< m._ptr[i+1]; k++)
        if (m._indices[k] == j)
        {
          nptr[i+1]++;
          break;
        }
    }
  }

  // setup data and indices
  nz = 0;
  for (unsigned i=0; i< _rows; i++) 
    for (unsigned j=0; j< _columns; j++)
    {
      // determine whether (i,j) is stored in this
      bool found = false;
      for (unsigned k=_ptr[i]; k< _ptr[i+1]; k++)
        if (_indices[k] == j)
        {
          ndata[nz] = _data[k];
          nindices[nz] = _indices[k];
          found = true;
          break;
        }

      // see whether (i,j) is stored in m
      for (unsigned k=m._ptr[i]; k< m._ptr[i+1]; k++)
        if (m._indices[k] == j)
        {
          if (found)
            ndata[nz] += m._data[k];
          else
          {
            ndata[nz] = m._data[k];
            nindices[nz] = m._indices[k];
            found = true;
          }
          break;
        }

      // update nz
      if (found)
        nz++;
    }

  // setup pointers to new arrays
  this->_ptr = nptr;
  this->_indices = nindices;
  this->_data = ndata;
  return *this;
}

/// Multiplies a sparse matrix by a scalar
SPARSEMATRIXN& SPARSEMATRIXN::operator*=(REAL scalar)
{
  for (unsigned i=0; i< _ptr[_rows]; i++)
    _data[i] *= scalar;
  return *this;
}

/// Negates this sparse matrix
SPARSEMATRIXN& SPARSEMATRIXN::negate()
{
  for (unsigned i=0; i< _ptr[_rows]; i++)
    _data[i] = -_data[i];
  return *this;
}

/// Calculates the outer product of a vector with itself and stores the result in a sparse matrix
SPARSEMATRIXN& SPARSEMATRIXN::outer_square(const SPARSEVECTORN& v, SPARSEMATRIXN& result)
{
  #ifdef REENTRANT
  FastThreadable<VECTORN> tmp;
  #else
  static FastThreadable<VECTORN> tmp;
  #endif

  // determine the size of the matrix
  unsigned n = v.size();

  // get the number of non-zero elements of v
  unsigned nz = v.num_elements();

  // get the indices of non-zero elements of v
  const unsigned* nz_indices = v.get_indices();
  const REAL* v_data = v.get_data();
  shared_array<bool> nz_elements(shared_array<bool>(new bool[n]));
  for (unsigned i=0; i< n; i++) nz_elements[i] = false;
  for (unsigned i=0; i< nz; i++) nz_elements[nz_indices[i]] = true;

  // setup ptr, indices, data
  shared_array<unsigned> ptr(new unsigned[n+1]);
  shared_array<unsigned> indices(new unsigned[nz*nz]);
  shared_array<REAL> data(new REAL[nz*nz]);

  // setup ptr
  ptr[0] = 0;
  for (unsigned i=0; i< n; i++)
  {
    ptr[i+1] = ptr[i];
    if (nz_elements[i])
      ptr[i+1] += nz;
  }

  // setup indices and data
  v.to_dense(tmp());
  for (unsigned i=0, k=0; i< n; i++)
  {
    // see whether to skip row
    if (!nz_elements[i])
      continue;

    for (unsigned j=0; j< n; j++)
    {
      // see whether to skip column
      if (!nz_elements[j])
        continue;

      // update indices
      assert(k < nz*nz);
      indices[k] = j;
      data[k] = tmp()[i]*tmp()[j];
      k++;
    }
  }

  // setup arrays
  result._rows = n;
  result._columns = n;
  result._ptr = ptr;
  result._indices = indices;
  result._data = data;

  return result;
}

/// Calculates the outer product of a vector with itself and stores the result in a sparse matrix
SPARSEMATRIXN& SPARSEMATRIXN::outer_square(const VECTORN& x, SPARSEMATRIXN& result)
{
  // determine the size of the matrix
  unsigned n = x.size();

  // determine the non-zero elements in x
  shared_array<bool> nz_elements(shared_array<bool>(new bool[n]));
  for (unsigned i=0; i< n; i++) nz_elements[i] = false;
  unsigned nz = 0;
  for (unsigned i=0; i< n; i++)
    if (std::fabs(x[i]) > EPS)
    {
      nz_elements[i] = true;
      nz++;
    }

  // setup ptr, indices, data
  shared_array<unsigned> ptr(new unsigned[n+1]);
  shared_array<unsigned> indices(new unsigned[nz*nz]);
  shared_array<REAL> data(new REAL[nz*nz]);

  // setup ptr
  ptr[0] = 0;
  for (unsigned i=0; i< n; i++)
  {
    ptr[i+1] = ptr[i];
    if (nz_elements[i])
      ptr[i+1] += nz;
  }

  // setup indices and data
  for (unsigned i=0, k=0; i< n; i++)
  {
    // see whether to skip row
    if (!nz_elements[i])
      continue;

    for (unsigned j=0; j< n; j++)
    {
      // see whether to skip column
      if (!nz_elements[j])
        continue;

      // update indices
      assert(k < nz*nz);
      indices[k] = j;
      data[k] = x[i]*x[j];
      k++;
    }
  }

  // setup arrays
  result._rows = n;
  result._columns = n;
  result._ptr = ptr;
  result._indices = indices;
  result._data = data;

  return result;
}

std::ostream& Ravelin::operator<<(std::ostream& out, const SPARSEMATRIXN& s)
{
  const unsigned* indices = s.get_indices();
  const unsigned* ptr = s.get_ptr();
  const REAL* data = s.get_data();
  vector<REAL> present(s.columns());

  out << "ptr:";
  for (unsigned i=0; i<= s.rows(); i++)
    out << " " << ptr[i];
  out << std::endl;

  out << "indices:";
  for (unsigned i=0; i< ptr[s.rows()]; i++)
    out << " " << indices[i];
  out << std::endl;

  out << "data:";
  for (unsigned i=0; i< ptr[s.rows()]; i++)
    out << " " << data[i];
  out << std::endl;

  for (unsigned i=0; i< s.rows(); i++)
  {
    // mark all as not present
    for (unsigned j=0; j< present.size(); j++) present[j] = (REAL) 0.0;

    // mark ones that are present
    for (unsigned j=ptr[i]; j< ptr[i+1]; j++)
      present[indices[j]] = data[j];

    for (unsigned j=0; j< s.columns(); j++)
      out << present[j] << " ";
    out << std::endl;
  }

  return out;
}

