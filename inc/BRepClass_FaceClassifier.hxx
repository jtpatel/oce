// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _BRepClass_FaceClassifier_HeaderFile
#define _BRepClass_FaceClassifier_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>

#include <BRepClass_FClassifier.hxx>
#include <Standard_Real.hxx>
class BRepClass_FaceExplorer;
class gp_Pnt2d;
class TopoDS_Face;
class gp_Pnt;


//! Provides Constructors with a Face.
class BRepClass_FaceClassifier  : public BRepClass_FClassifier
{
public:

  DEFINE_STANDARD_ALLOC

  
  //! Empty constructor, undefined algorithm.
  Standard_EXPORT BRepClass_FaceClassifier();
  
  //! Creates an algorithm to classify the Point  P with
  //! Tolerance <T> on the face described by <F>.
  Standard_EXPORT BRepClass_FaceClassifier(BRepClass_FaceExplorer& F, const gp_Pnt2d& P, const Standard_Real Tol);
  
  //! Creates an algorithm to classify the Point  P with
  //! Tolerance <T> on the face <F>.
  Standard_EXPORT BRepClass_FaceClassifier(const TopoDS_Face& F, const gp_Pnt2d& P, const Standard_Real Tol);
  
  //! Classify  the Point  P  with  Tolerance <T> on the
  //! face described by <F>.
  Standard_EXPORT   void Perform (const TopoDS_Face& F, const gp_Pnt2d& P, const Standard_Real Tol) ;
  
  //! Creates an algorithm to classify the Point  P with
  //! Tolerance <T> on the face <F>.
  Standard_EXPORT BRepClass_FaceClassifier(const TopoDS_Face& F, const gp_Pnt& P, const Standard_Real Tol);
  
  //! Classify  the Point  P  with  Tolerance <T> on the
  //! face described by <F>.
  Standard_EXPORT   void Perform (const TopoDS_Face& F, const gp_Pnt& P, const Standard_Real Tol) ;




protected:





private:





};







#endif // _BRepClass_FaceClassifier_HeaderFile
