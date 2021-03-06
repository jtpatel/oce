// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _Convert_TorusToBSplineSurface_HeaderFile
#define _Convert_TorusToBSplineSurface_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>

#include <Convert_ElementarySurfaceToBSplineSurface.hxx>
#include <Standard_Real.hxx>
#include <Standard_Boolean.hxx>
class Standard_DomainError;
class gp_Torus;



//! This algorithm converts a bounded Torus into a rational
//! B-spline surface. The torus is a Torus from package gp.
//! The parametrization of the torus is :
//! P (U, V) =
//! Loc  +  MinorRadius * Sin(V) * Zdir +
//! (MajorRadius+MinorRadius*Cos(V)) *  (Cos(U)*Xdir + Sin(U)*Ydir)
//! where Loc is the center of the torus, Xdir, Ydir and Zdir are the
//! normalized directions of the local cartesian coordinate system of
//! the Torus. The parametrization range is U [0, 2PI], V [0, 2PI].
//! KeyWords :
//! Convert, Torus, BSplineSurface.
class Convert_TorusToBSplineSurface  : public Convert_ElementarySurfaceToBSplineSurface
{
public:

  DEFINE_STANDARD_ALLOC

  

  //! The equivalent B-spline surface as the same orientation as the
  //! torus in the U and V parametric directions.
  //!
  //! Raised if U1 = U2 or U1 = U2 + 2.0 * Pi
  //! Raised if V1 = V2 or V1 = V2 + 2.0 * Pi
  Standard_EXPORT Convert_TorusToBSplineSurface(const gp_Torus& T, const Standard_Real U1, const Standard_Real U2, const Standard_Real V1, const Standard_Real V2);
  

  //! The equivalent B-spline surface as the same orientation as the
  //! torus in the U and V parametric directions.
  //!
  //! Raised if Param1 = Param2 or Param1 = Param2 + 2.0 * Pi
  Standard_EXPORT Convert_TorusToBSplineSurface(const gp_Torus& T, const Standard_Real Param1, const Standard_Real Param2, const Standard_Boolean UTrim = Standard_True);
  

  //! The equivalent B-spline surface as the same orientation as the
  //! torus in the U and V parametric directions.
  Standard_EXPORT Convert_TorusToBSplineSurface(const gp_Torus& T);




protected:





private:





};







#endif // _Convert_TorusToBSplineSurface_HeaderFile
