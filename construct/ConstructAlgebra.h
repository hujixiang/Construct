#ifndef ConstructAlgebra_h
#define ConstructAlgebra_h
#include "construct/ConstructField.h"
namespace Construct {

// Addition
template<typename T>
struct AdditionField : public ConstructFieldNode<T> {
 typedef typename ConstructFieldNode<T>::ptr Ptr;
 Ptr A, B;
 AdditionField(Ptr A, Ptr B) : A(A), B(B) { }
 T eval(const Vec3& x) const 
 { return A->eval(x) + B->eval(x); }
 typename FieldInfo<T>::GradType grad(const Vec3& x)
 { return A->grad(x) + B->grad(x); }
};
template<typename T>
Field<T> operator+(Field<T> A, Field<T> B)
{ return Field<T>(new AdditionField<T>(A.node,B.node)); }

// Subtraction
template<typename T>
struct SubtractionField : public ConstructFieldNode<T> {
 typedef typename ConstructFieldNode<T>::ptr Ptr;
 Ptr A, B;
 SubtractionField(Ptr A, Ptr B) : A(A), B(B) { }
 T eval(const Vec3& x) const 
 { return A->eval(x) - B->eval(x); }
 typename FieldInfo<T>::GradType grad(const Vec3& x)
 { return A->grad(x) - B->grad(x); }
};
template<typename T>
Field<T> operator-(Field<T> A, Field<T> B)
{ return Field<T>(new SubtractionField<T>(A.node,B.node)); }

template<typename LeftType, typename RightType, typename ResultType>
struct MultiplicationField : public ConstructFieldNode<ResultType> {
  typename ConstructFieldNode<LeftType>::ptr  A;
  typename ConstructFieldNode<RightType>::ptr B;
  MultiplicationField(
    typename ConstructFieldNode<LeftType>::ptr  A,
    typename ConstructFieldNode<RightType>::ptr B) : A(A), B(B) { }
  ResultType eval(const Vec3& x) const
  { return A->eval(x) * B->eval(x); }
  typename FieldInfo<ResultType>::GradType grad(const Vec3& x) const
  { return A->grad(x) * B->eval(x) + A->eval(x) * B->grad(x); }
};

template<> Mat3 MultiplicationField<Vec3,real,Vec3>::grad(const Vec3& x) const
{ return A->grad(x) * B->eval(x) + A->eval(x) * B->grad(x).transpose(); }

ScalarField operator*(ScalarField a, ScalarField b)
{ return ScalarField(new MultiplicationField<real,real,real>(a.node, b.node)); }
VectorField operator*(VectorField v, ScalarField s)
{ return VectorField(new MultiplicationField<Vec3,real,Vec3>(v.node, s.node)); }
VectorField operator*(ScalarField s, VectorField v)
{ return VectorField(new MultiplicationField<Vec3,real,Vec3>(v.node, s.node)); }
// TODO: Need to specify special Matrix-(Matrix,Vector) grads
//VectorField operator*(MatrixField m, VectorField v) 
//{ return VectorField(new MultiplicationField<Mat3,Vec3,Vec3>(m.node, v.node)); }
//MatrixField operator*(MatrixField a, MatrixField b) 
//{ return MatrixField(new MultiplicationField<Mat3,Mat3,Mat3>(a.node, b.node)); }

/////////////////////////////////////

// Identity field
struct IdentityField : public VectorFieldNode {
  Vec3 eval(const Vec3& x) const { return x; }
  Mat3 grad(const Vec3& x) const { return Mat3::Identity(); }
};
inline VectorField identity()
{ return new IdentityField(); }

// Inner Product
struct InnerProductField : public ScalarFieldNode {
  VFNodePtr A, B;
  InnerProductField(VFNodePtr A, VFNodePtr B) : A(A), B(B) { }
  real eval(const Vec3& x) const { return 
};

};
#endif
