// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Alec Jacobson <alecjacobson@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#include "setdiff.h"
#include "LinSpaced.h"
#include "list_to_matrix.h"
#include "sort.h"
#include "unique.h"

template <
  typename DerivedA,
  typename DerivedB,
  typename DerivedC,
  typename DerivedIA>
IGL_INLINE void igl::setdiff(
  const Eigen::DenseBase<DerivedA> & A,
  const Eigen::DenseBase<DerivedB> & B,
  Eigen::PlainObjectBase<DerivedC> & C,
  Eigen::PlainObjectBase<DerivedIA> & IA)
{
  using namespace Eigen;
  using namespace std;
  // boring base cases
  if(A.size() == 0)
  {
    C.resize(0,1);
    IA.resize(0,1);
    return;
  }

  // Get rid of any duplicates
  typedef Matrix<typename DerivedA::Scalar,Dynamic,1> VectorA;
  typedef Matrix<typename DerivedB::Scalar,Dynamic,1> VectorB;
  VectorA uA;
  VectorB uB;
  typedef DerivedIA IAType;
  IAType uIA,uIuA,uIB,uIuB;
  unique(A,uA,uIA,uIuA);
  unique(B,uB,uIB,uIuB);

  // Sort both
  VectorA sA;
  VectorB sB;
  IAType sIA,sIB;
  sort(uA,1,true,sA,sIA);
  sort(uB,1,true,sB,sIB);

  vector<typename DerivedB::Scalar> vC;
  vector<typename DerivedIA::Scalar> vIA;
  int bi = 0;
  // loop over sA
  bool past = false;
  bool sBempty = sB.size()==0;
  for(int a = 0;a<sA.size();a++)
  {
    while(!sBempty && !past && sA(a)>sB(bi))
    {
      bi++;
      past = bi>=sB.size();
    }
    if(sBempty || past || sA(a)<sB(bi))
    {
      // then sA(a) did not appear in sB
      vC.push_back(sA(a));
      vIA.push_back(uIA(sIA(a)));
    }
  }
  list_to_matrix(vC,C);
  list_to_matrix(vIA,IA);
}

#ifdef IGL_STATIC_LIBRARY
// Explicit template instantiation
// generated by autoexplicit.sh
template void igl::setdiff<Eigen::Matrix<int, -1, 2, 0, -1, 2>, Eigen::Matrix<int, -1, 2, 0, -1, 2>, Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<int, -1, 1, 0, -1, 1> >(Eigen::DenseBase<Eigen::Matrix<int, -1, 2, 0, -1, 2> > const&, Eigen::DenseBase<Eigen::Matrix<int, -1, 2, 0, -1, 2> > const&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&);
template void igl::setdiff<Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<int, -1, 1, 0, -1, 1> >(Eigen::DenseBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> > const&, Eigen::DenseBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> > const&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&);
template void igl::setdiff<Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<int, -1, 1, 0, -1, 1> >(Eigen::DenseBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> > const&, Eigen::DenseBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&);
template void igl::setdiff<Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1> >(Eigen::DenseBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, Eigen::DenseBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> >&);
#endif
