// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _GccAna_Lin2dTanObl_HeaderFile
#define _GccAna_Lin2dTanObl_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>

#include <Standard_Boolean.hxx>
#include <Standard_Integer.hxx>
#include <TColgp_Array1OfLin2d.hxx>
#include <GccEnt_Array1OfPosition.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <Standard_Real.hxx>
#include <GccEnt_Position.hxx>
class Standard_OutOfRange;
class GccEnt_BadQualifier;
class StdFail_NotDone;
class gp_Pnt2d;
class gp_Lin2d;
class GccEnt_QualifiedCirc;


//! This class implements the algorithms used to
//! create 2d line tangent to a circle or a point and
//! making an angle with a line.
//! The angle is in radians.
//! The origin of the solution is the tangency point
//! with the first argument.
//! Its direction is making an angle Angle with the
//! second argument.
class GccAna_Lin2dTanObl 
{
public:

  DEFINE_STANDARD_ALLOC

  
  //! This class implements the algorithms used to
  //! create 2d line passing through a point and
  //! making an angle with a line.
  Standard_EXPORT GccAna_Lin2dTanObl(const gp_Pnt2d& ThePoint, const gp_Lin2d& TheLine, const Standard_Real TheAngle);
  
  //! This class implements the algorithms used to
  //! create 2d line tangent to a circle and
  //! making an angle with a line.
  //! Exceptions
  //! GccEnt_BadQualifier if a qualifier is inconsistent with
  //! the argument it qualifies (for example, enclosed for a circle).
  Standard_EXPORT GccAna_Lin2dTanObl(const GccEnt_QualifiedCirc& Qualified1, const gp_Lin2d& TheLine, const Standard_Real TheAngle);
  
  //! Returns True if the algorithm succeeded.
  //! Note: IsDone protects against a failure arising from a
  //! more internal intersection algorithm, which has reached
  //! its numeric limits.
  Standard_EXPORT   Standard_Boolean IsDone()  const;
  
  //! Returns the number of  of lines, representing solutions computed by this algorithm.
  //! Raises NotDone if the construction algorithm didn't succeed.
  Standard_EXPORT   Standard_Integer NbSolutions()  const;
  
  //! Returns the solution number Index.
  //! Be careful: the Index is only a way to get all the
  //! solutions, but is not associated to theses outside the
  //! context of the algorithm-object.
  //! raises NotDone if the construction algorithm didn't succeed.
  //! It raises OutOfRange if Index is greater than the   number of solutions.
  Standard_EXPORT   gp_Lin2d ThisSolution (const Standard_Integer Index)  const;
  
  //! Returns the qualifier Qualif1 of the tangency argument
  //! for the solution of index Index computed by this algorithm.
  //! The returned qualifier is:
  //! -   that specified at the start of construction when the
  //! solutions are defined as enclosing or outside with
  //! respect to the argument, or
  //! -   that computed during construction (i.e. enclosing or
  //! outside) when the solutions are defined as unqualified
  //! with respect to the argument, or
  //! -   GccEnt_noqualifier if the tangency argument is a point.
  //! Exceptions
  //! Standard_OutOfRange if Index is less than zero or
  //! greater than the number of solutions computed by this algorithm.
  //! StdFail_NotDone if the construction fails.
  Standard_EXPORT   void WhichQualifier (const Standard_Integer Index, GccEnt_Position& Qualif1)  const;
  
  //! Returns informations about the tangency point between the
  //! result number Index and the first argument.
  //! ParSol is the intrinsic parameter of the point ParSol on
  //! the solution curv.
  //! ParArg is the intrinsic parameter of the point ParArg on
  //! the argument curv. Raises NotDone if the construction algorithm
  //! didn't succeed.
  //! It raises OutOfRange if Index is greater than the  number of solutions.
  Standard_EXPORT   void Tangency1 (const Standard_Integer Index, Standard_Real& ParSol, Standard_Real& ParArg, gp_Pnt2d& PntSol)  const;
  
  //! Returns informations about the intersection between the
  //! result number Index and the third argument.
  //! Raises NotDone if the construction algorithm  didn't succeed.
  //! It raises OutOfRange if Index is greater than the number of solutions.
  Standard_EXPORT   void Intersection2 (const Standard_Integer Index, Standard_Real& ParSol, Standard_Real& ParArg, gp_Pnt2d& PntSol)  const;




protected:





private:



  Standard_Boolean WellDone;
  Standard_Integer NbrSol;
  TColgp_Array1OfLin2d linsol;
  GccEnt_Array1OfPosition qualifier1;
  TColgp_Array1OfPnt2d pnttg1sol;
  TColgp_Array1OfPnt2d pntint2sol;
  TColStd_Array1OfReal par1sol;
  TColStd_Array1OfReal par2sol;
  TColStd_Array1OfReal pararg1;
  TColStd_Array1OfReal pararg2;


};







#endif // _GccAna_Lin2dTanObl_HeaderFile
