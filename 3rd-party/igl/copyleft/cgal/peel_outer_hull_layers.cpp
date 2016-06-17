// This file is part of libigl, a simple c++ geometry processing library.
// 
// Copyright (C) 2015 Alec Jacobson <alecjacobson@gmail.com>
// 
// This Source Code Form is subject to the terms of the Mozilla Public License 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.
#include "peel_outer_hull_layers.h"
#include "outer_hull.h"
#include <vector>
#include <iostream>
//#define IGL_PEEL_OUTER_HULL_LAYERS_DEBUG
#ifdef IGL_PEEL_OUTER_HULL_LAYERS_DEBUG
#include "../../writePLY.h"
#include "../../writeDMAT.h"
#include "../../STR.h"
#endif

template <
  typename DerivedV,
  typename DerivedF,
  typename DerivedI,
  typename Derivedflip>
IGL_INLINE size_t igl::copyleft::cgal::peel_outer_hull_layers(
  const Eigen::PlainObjectBase<DerivedV > & V,
  const Eigen::PlainObjectBase<DerivedF > & F,
  Eigen::PlainObjectBase<DerivedI> & I,
  Eigen::PlainObjectBase<Derivedflip > & flip)
{
  using namespace Eigen;
  using namespace std;
  typedef typename DerivedF::Index Index;
  typedef Matrix<typename DerivedF::Scalar,Dynamic,DerivedF::ColsAtCompileTime> MatrixXF;
  typedef Matrix<Index,Dynamic,1> MatrixXI;
  typedef Matrix<typename Derivedflip::Scalar,Dynamic,Derivedflip::ColsAtCompileTime> MatrixXflip;
  const Index m = F.rows();
#ifdef IGL_PEEL_OUTER_HULL_LAYERS_DEBUG
  cout<<"peel outer hull layers..."<<endl;
#endif
#ifdef IGL_PEEL_OUTER_HULL_LAYERS_DEBUG
  cout<<"calling outer hull..."<<endl;
  writePLY(STR("peel-outer-hull-input.ply"),V,F);
#endif

#ifdef IGL_PEEL_OUTER_HULL_LAYERS_DEBUG
  cout<<"resize output ..."<<endl;
#endif
  // keep track of iteration parity and whether flipped in hull
  MatrixXF Fr = F;
  I.resize(m,1);
  flip.resize(m,1);
  // Keep track of index map
  MatrixXI IM = MatrixXI::LinSpaced(m,0,m-1);
  // This is O(n * layers)
  MatrixXI P(m,1);
  Index iter = 0;
  while(Fr.size() > 0)
  {
    assert(Fr.rows() == IM.rows());
    // Compute outer hull of current Fr
    MatrixXF Fo;
    MatrixXI Jo;
    MatrixXflip flipr;
#ifdef IGL_PEEL_OUTER_HULL_LAYERS_DEBUG
  {
      cout<<"calling outer hull..." << iter <<endl;
      std::stringstream ss;
      ss << "outer_hull_" << iter << ".ply";
      Eigen::MatrixXd vertices(V.rows(), V.cols());
      std::transform(V.data(), V.data() + V.rows()*V.cols(),
              vertices.data(),
              [](typename DerivedV::Scalar val)
              {return CGAL::to_double(val); });
      writePLY(ss.str(), vertices, Fr);
  }
#endif
    outer_hull_legacy(V,Fr,Fo,Jo,flipr);
#ifdef IGL_PEEL_OUTER_HULL_LAYERS_DEBUG
  writePLY(STR("outer-hull-output-"<<iter<<".ply"),V,Fo);
  cout<<"reindex, flip..."<<endl;
#endif
    assert(Fo.rows() != 0);
    assert(Fo.rows() == Jo.rows());
    // all faces in Fo of Fr
    vector<bool> in_outer(Fr.rows(),false);
    for(Index g = 0;g<Jo.rows();g++)
    {
      I(IM(Jo(g))) = iter;
      P(IM(Jo(g))) = iter;
      in_outer[Jo(g)] = true;
      flip(IM(Jo(g))) = flipr(Jo(g));
    }
    // Fr = Fr - Fo
    // update IM
    MatrixXF prev_Fr = Fr;
    MatrixXI prev_IM = IM;
    Fr.resize(prev_Fr.rows() - Fo.rows(),F.cols());
    IM.resize(Fr.rows());
    {
      Index g = 0;
      for(Index f = 0;f<prev_Fr.rows();f++)
      {
        if(!in_outer[f])
        {
          Fr.row(g) = prev_Fr.row(f);
          IM(g) = prev_IM(f);
          g++;
        }
      }
    }
    iter++;
  }
  return iter;
}

#ifdef IGL_STATIC_LIBRARY
// Explicit template specialization
template size_t igl::copyleft::cgal::peel_outer_hull_layers<Eigen::Matrix<double, -1, 3, 0, -1, 3>, Eigen::Matrix<int, -1, 3, 0, -1, 3>, Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<int, -1, 1, 0, -1, 1> >(Eigen::PlainObjectBase<Eigen::Matrix<double, -1, 3, 0, -1, 3> > const&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 3, 0, -1, 3> > const&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&);
#endif
