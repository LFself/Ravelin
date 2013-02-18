/****************************************************************************
 * Copyright 2005 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

/// Gets the specified sub matrix
/**
 * \param row_start the row to start (inclusive)
 * \param row_end the row to end (exclusive)
 * \param col_start the column to start (inclusive)
 * \param col_end the column to end (exclusive)
 * \param transpose determines whether to store the transpose of the submatrix into m
 * \return a (row_end - row_start) x (col_end - col_start) sized matrix
 */
template <class M>
M& MatrixNf::get_sub_mat(unsigned row_start, unsigned row_end, unsigned col_start, unsigned col_end, M& m, MatrixNf::Transposition trans) const
{
  if (row_start > row_end || row_end > _rows || col_start > col_end || col_end > _columns)
    throw InvalidIndexException();

  // resize the matrix
  if (trans == MatrixNf::eNoTranspose)
    m.resize(row_end - row_start, col_end - col_start);
  else
    m.resize(col_end - col_start, row_end - row_start);

  // see whether to exit now
  if (_rows == 0 || _columns == 0)
    return m;

  // copy each column using BLAS
  if (trans == MatrixNf::eNoTranspose)
    for (unsigned i=0; i< m.columns(); i++)
      CBLAS::copy(m.rows(), _data.get()+row_start+(col_start+i) * _rows, 1, m.data()+i*m.rows(), 1);
  else
    for (unsigned i=0; i< m.rows(); i++)
      CBLAS::copy(m.columns(), _data.get()+row_start+(col_start+i) * _rows, 1, m.data()+i, m.rows());
  
  return m;
}

/// Sets the specified sub matrix
/**
 * \param row_start the row to start (inclusive)
 * \param col_start the column to start (inclusive)
 * \param m the source matrix
 * \param transpose determines whether the m is to be transposed 
 * \note fails assertion if m is too large to insert into this
 */
template <class M>
MatrixNf& MatrixNf::set_sub_mat(unsigned row_start, unsigned col_start, const M& m, MatrixNf::Transposition trans)
{
  if (trans == MatrixNf::eNoTranspose)
  {
    if (row_start + m.rows() > _rows || col_start + m.columns() > _columns)
      throw MissizeException();
  }
  else if (row_start + m.columns() > _rows || col_start + m.rows() > _columns)
    throw MissizeException();

  // make sure there is data to copy
  if (m.rows() == 0 || m.columns() == 0)
    return *this;

  // copy each column of m using BLAS
  if (trans == MatrixNf::eNoTranspose)
    for (unsigned i=0; i< m.columns(); i++)
      CBLAS::copy(m.rows(), m.data()+i*m.rows(), 1, _data.get()+row_start+(col_start+i) * _rows, 1);
  else
    for (unsigned i=0; i< m.columns(); i++)
      CBLAS::copy(m.rows(), m.data()+i*m.rows(), 1, _data.get()+row_start+col_start*_rows + i, _rows);

  return *this;
}

/// Gets a submatrix of columns (not necessarily a block)
template <class ForwardIterator>
MatrixNf& MatrixNf::select_columns(ForwardIterator col_start, ForwardIterator col_end, MatrixNf& m) const
{
  // setup vectors of selections
  const unsigned ncols = std::distance(col_start, col_end);

  // resize matrix 
  m.resize(_rows, ncols);

  // make sure there is data to copy
  if (_rows == 0 || ncols == 0)
    return m;

  // populate m
  unsigned mi;
  ForwardIterator i;
  for (i=col_start, mi=0; i != col_end; i++, mi++)
    CBLAS::copy(_rows, begin()+_rows*(*i), 1, m.begin()+_rows*mi, 1);

  return m;
}

/// Gets a submatrix of rows (not necessarily a block)
template <class ForwardIterator>
MatrixNf& MatrixNf::select_rows(ForwardIterator row_start, ForwardIterator row_end, MatrixNf& m) const
{
  // setup vectors of selections
  const unsigned nrows = std::distance(row_start, row_end);

  // resize matrix 
  m.resize(nrows, _columns);

  // make sure there is data to copy
  if (nrows == 0 || _columns == 0)
    return m;

  // populate m
  unsigned mi;
  ForwardIterator i;
  for (i=row_start, mi=0; i != row_end; i++, mi++)
    CBLAS::copy(_columns, begin()+*i, _rows, m.begin()+mi, nrows);

  return m;
}

/// Gets a submatrix of rows (not necessarily a block)
template <class ForwardIterator>
MatrixNf MatrixNf::select_columns(ForwardIterator col_start, ForwardIterator col_end) const
{
  MatrixNf m;
  select_columns(col_start, col_end, m);
  return m;
}

/// Gets a submatrix of rows (not necessarily a block)
template <class ForwardIterator>
MatrixNf MatrixNf::select_rows(ForwardIterator row_start, ForwardIterator row_end) const
{
  MatrixNf m;
  select_rows(row_start, row_end, m);
  return m;
}

/// Gets a submatrix (not necessarily a block)
template <class ForwardIterator1, class ForwardIterator2>
MatrixNf& MatrixNf::select(ForwardIterator1 row_start, ForwardIterator1 row_end, ForwardIterator2 col_start, ForwardIterator2 col_end, MatrixNf& m) const
{
  // setup vectors of selections
  const unsigned nrows = std::distance(row_start, row_end);
  const unsigned ncols = std::distance(col_start, col_end);

  // resize matrix 
  m.resize(nrows, ncols);

  // make sure there is data to copy
  if (nrows == 0 || ncols == 0)
    return m;

  // get pointers to data
  Real* mdata = m.data();
  const Real* data = &operator()(*row_start, *col_start);

  // determine difference between first and last row
  unsigned rowbegin = *row_start;
  unsigned rowend = *row_start;
  for (ForwardIterator1 i = row_start; i != row_end; i++)
    rowend = *i;
  unsigned row_sub = rowend - rowbegin;

  // outer loop is over columns 
  for (ForwardIterator2 j=col_start; j != col_end; )
  {
    for (ForwardIterator1 i = row_start; i != row_end; )
    {
      // copy the data
      *mdata = *data;
      mdata++;

      // determine how we need to advance the rows
      unsigned row_diff = *i;
      i++;
      row_diff -= *i;
      row_diff = -row_diff;

      // if we're able, advance data_start 
      if (i != row_end)
        data += row_diff;
    }

    // determine how we need to advance the columns
    unsigned col_diff = *j;
    j++;
    col_diff -= *j;
    col_diff = -col_diff;

    // advance data
    data += (col_diff * _rows);
    data -= row_sub; 
  }

  return m;
}

/// Gets a submatrix (not necessarily a block)
template <class ForwardIterator1, class ForwardIterator2>
VectorNf& MatrixNf::select(ForwardIterator1 row_start, ForwardIterator1 row_end, ForwardIterator2 col_start, ForwardIterator2 col_end, VectorNf& v) const
{
  // setup vectors of selections
  const unsigned nrows = std::distance(row_start, row_end);
  const unsigned ncols = std::distance(col_start, col_end);

  // verify that either number of rows or number of columns is == 1 
  if (nrows != 1 && ncols != 1)
    throw MissizeException();

  // resize vector
  unsigned sz = (nrows == 1) ? ncols : nrows;
  v.resize(sz);

  // make sure there is data to copy
  if (sz == 0)
    return v;

  // get pointers to data
  Real* vdata = v.data();
  const Real* data = &operator()(*row_start, *col_start);

  // determine difference between first and last row
  unsigned rowbegin = *row_start;
  unsigned rowend = *row_start;
  for (ForwardIterator1 i = row_start; i != row_end; i++)
    rowend = *i;
  unsigned row_sub = rowend - rowbegin;

  // outer loop is over columns 
  for (ForwardIterator2 j=col_start; j != col_end; )
  {
    for (ForwardIterator1 i = row_start; i != row_end; )
    {
      // copy the data
      *vdata = *data;
      vdata++;

      // determine how we need to advance the rows
      unsigned row_diff = *i;
      i++;
      row_diff -= *i;
      row_diff = -row_diff;

      // if we're able, advance data 
      if (i != row_end)
        data += row_diff;
    }

    // determine how we need to advance the columns
    unsigned col_diff = *j;
    j++;
    col_diff -= *j;
    col_diff = -col_diff;

    // advance data
    data += (col_diff * _rows);
    data -= row_sub; 
  }

  return v;
}

/// Gets a submatrix (not necessarily a block)
template <class ForwardIterator1, class ForwardIterator2>
VectorNf MatrixNf::select(ForwardIterator1 row_start, ForwardIterator1 row_end, ForwardIterator2 col_start, ForwardIterator2 col_end) const
{
  VectorNf v;
  return select(row_start, row_end, col_start, col_end, v);
}

/// Gets a submatrix (not necessarily a block)
template <class ForwardIterator1, class ForwardIterator2>
MatrixNf MatrixNf::select(ForwardIterator1 row_start, ForwardIterator1 row_end, ForwardIterator2 col_start, ForwardIterator2 col_end) const
{
  MatrixNf m;
  return select(row_start, row_end, col_start, col_end, m);
}

template <class ForwardIterator>
MatrixNf& MatrixNf::select_square(ForwardIterator start, ForwardIterator end, MatrixNf& m) const
{
  return select(start, end, start, end, m);
}

template <class ForwardIterator>
MatrixNf MatrixNf::select_square(ForwardIterator start, ForwardIterator end) const
{
  // create new matrix
  MatrixNf m;
  return select_square(start, end, m);
}

