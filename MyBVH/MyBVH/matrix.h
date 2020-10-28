#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;




class Matrix
{
public:
  float * values;
  int rows;
  int columns;


  // constructor
  Matrix(int r, int c, std::vector<float> vals)
  {
    // Error Handling
    if(r < 1){ throw invalid_argument("Invalid Rows"); }
    if(c < 1){ throw invalid_argument("Invalid Columns"); }
    if((int)vals.size() != r*c){ throw invalid_argument("Invalid Values"); }

    this->rows = r;
    this->columns = c;
    this->values = (float *) malloc(sizeof(float) * rows * columns);

    // Copy into array
    for(int i = 0; i < rows * columns; i ++)
    {
      values[i] = vals[i];
    }
  }

  // alternate constructor
  Matrix(int r, int c)
  {
    // Error Handling
    if(r < 1){ throw invalid_argument("Invalid Rows"); }
    if(c < 1){ throw invalid_argument("Invalid Columns"); }

    this->rows = r;
    this->columns = c;
    this->values = (float *) malloc(sizeof(float) * rows * columns);

    // Copy into array
    for(int i = 0; i < rows * columns; i ++)
    {
      values[i] = 0.;
    }
  }


  // Friend for +
	friend Matrix operator+(const Matrix &m, const Matrix &n);

  // Friend for -
	friend Matrix operator-(const Matrix &m, const Matrix &n);

  // Friend for /
  friend Matrix operator/(const Matrix &m, const Matrix &n);
  friend Matrix operator/(const Matrix &m, const float &n);

  // Friend for *
  friend Matrix operator/(const Matrix &m, const Matrix &n);
  friend Matrix operator/(const Matrix &m, const float &n);

  // Friend for <<
  friend ostream& operator<<(ostream &out, Matrix * m);

};

// Printing
ostream& operator<< (std::ostream &out, const Matrix &m)
{
    for(int i = 0; i < m.rows; i ++)
    {
      for(int j = 0; j < m.columns; j++)
      {
        out << m.values[j + i*m.columns] << " ";
      }
      out << '\n';
    }
    return out;
}

// addition
Matrix operator+(const Matrix &m, const Matrix &n)
{

  // Error Handling
  if(m.rows != n.rows)
  {
    throw invalid_argument("Matricies must have same number of rows");
  }

  if(m.columns != n.columns)
  {
    throw invalid_argument("Matricies must have same number of columns");
  }

  Matrix out{m.rows, m.columns};
  int idx; // index
  for(int i = 0; i < m.rows; i ++)
  {
    for(int j = 0; j < m.columns; j++)
    {
      idx = j + i*m.columns;
      out.values[idx] = m.values[idx] + n.values[idx];
    }
  }
  return out;
}

// subtraction
Matrix operator-(const Matrix &m, const Matrix &n)
{
  // Error Handling
  if(m.rows != n.rows)
  {
    throw invalid_argument("Matricies must have same number of rows");
  }

  if(m.columns != n.columns)
  {
    throw invalid_argument("Matricies must have same number of columns");
  }

  Matrix out{m.rows, m.columns};
  int idx; // index
  for(int i = 0; i < m.rows; i ++)
  {
    for(int j = 0; j < m.columns; j++)
    {
      idx = j + i*m.columns;
      out.values[idx] = m.values[idx] - n.values[idx];
    }
  }
  return out;
}

// multiplication
Matrix operator*(const Matrix &m, const Matrix &n)
{
  // Error Handling
  if(m.columns != n.rows)
  {
    throw invalid_argument("Columns Matrix A must equal Rows Matrix B");
  }

  Matrix out{m.rows, n.columns};
  int idx; // index
  for(int i = 0; i < m.rows; i ++)
  {
    for(int j = 0; j < n.columns; j++)
    {
      idx = j + i*n.columns;

      for(int k = 0; k < m.columns; k++)
      {
        //std::cout << idx << ": " << "   "<< m.values[i*m.columns + k] << " * " << n.values[k*n.columns + j] << " = " << m.values[i*m.columns + k] * n.values[k*n.columns + j] << '\n';
        out.values[idx] += m.values[i*m.columns + k] * n.values[k*n.columns + j];
      }
    }
  }
  return out;
}

// multiplication
Matrix operator*(const Matrix &m, const float n)
{
  Matrix out{m.rows, m.columns};
  int idx; // index
  for(int i = 0; i < m.rows; i ++)
  {
    for(int j = 0; j < m.columns; j++)
    {
      idx = j + i*m.columns;
      out.values[idx] = m.values[idx] * n;
    }
  }
  return out;
}


// division
Matrix operator/(const Matrix &m, const Matrix &n)
{
    throw  invalid_argument("Cannot divide matricies");
}

// divison
Matrix operator/(const Matrix &m, const float n)
{
  Matrix out{m.rows, m.columns};
  int idx; // index
  for(int i = 0; i < m.rows; i ++)
  {
    for(int j = 0; j < m.columns; j++)
    {
      idx = j + i*m.columns;
      out.values[idx] = m.values[idx] / n;
    }
  }
  return out;
}


#endif // MATRIX_H
