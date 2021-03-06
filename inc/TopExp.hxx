// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _TopExp_HeaderFile
#define _TopExp_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>

#include <TopAbs_ShapeEnum.hxx>
#include <Standard_Boolean.hxx>
class TopoDS_Shape;
class TopTools_IndexedMapOfShape;
class TopTools_IndexedDataMapOfShapeListOfShape;
class TopoDS_Vertex;
class TopoDS_Edge;
class TopoDS_Wire;
class TopExp_Explorer;


//! This package   provides  basic tools  to   explore the
//! topological data structures.
//!
//! * Explorer : A tool to find all sub-shapes of  a given
//! type. e.g. all faces of a solid.
//!
//! * Package methods to map sub-shapes of a shape.
//!
//! Level : Public
//! All methods of all  classes will be public.
class TopExp 
{
public:

  DEFINE_STANDARD_ALLOC

  
  //! Tool to explore a topological data structure.
  //! Stores in the map <M> all  the sub-shapes of <S>
  //! of type <T>.
  //!
  //! Warning: The map is not cleared at first.
  Standard_EXPORT static   void MapShapes (const TopoDS_Shape& S, const TopAbs_ShapeEnum T, TopTools_IndexedMapOfShape& M) ;
  
  //! Stores in the map <M> all  the sub-shapes of <S>.
  Standard_EXPORT static   void MapShapes (const TopoDS_Shape& S, TopTools_IndexedMapOfShape& M) ;
  
  //! Stores in the map <M> all the subshape of <S> of
  //! type <TS>  for each one append  to  the list all
  //! the ancestors of type <TA>.  For example map all
  //! the edges and bind the list of faces.
  //! Warning: The map is not cleared at first.
  Standard_EXPORT static   void MapShapesAndAncestors (const TopoDS_Shape& S, const TopAbs_ShapeEnum TS, const TopAbs_ShapeEnum TA, TopTools_IndexedDataMapOfShapeListOfShape& M) ;
  
  //! Returns the Vertex of orientation FORWARD in E. If
  //! there is none returns a Null Shape.
  //! CumOri = True : taking account the edge orientation
  Standard_EXPORT static   TopoDS_Vertex FirstVertex (const TopoDS_Edge& E, const Standard_Boolean CumOri = Standard_False) ;
  
  //! Returns the Vertex of orientation REVERSED in E. If
  //! there is none returns a Null Shape.
  //! CumOri = True : taking account the edge orientation
  Standard_EXPORT static   TopoDS_Vertex LastVertex (const TopoDS_Edge& E, const Standard_Boolean CumOri = Standard_False) ;
  
  //! Returns in Vfirst, Vlast the  FORWARD and REVERSED
  //! vertices of the edge <E>. May be null shapes.
  //! CumOri = True : taking account the edge orientation
  Standard_EXPORT static   void Vertices (const TopoDS_Edge& E, TopoDS_Vertex& Vfirst, TopoDS_Vertex& Vlast, const Standard_Boolean CumOri = Standard_False) ;
  
  //! Returns  in  Vfirst,  Vlast   the first   and last
  //! vertices of the open wire <W>. May be null shapes.
  //! if   <W>  is closed Vfirst and Vlast  are a same
  //! vertex on <W>.
  //! if <W> is no manifold. VFirst and VLast are null
  //! shapes.
  Standard_EXPORT static   void Vertices (const TopoDS_Wire& W, TopoDS_Vertex& Vfirst, TopoDS_Vertex& Vlast) ;
  
  //! Finds   the  vertex <V> common   to  the two edges
  //! <E1,E2>, returns True if this vertex exists.
  //!
  //! Warning: <V> has sense only if the value <True> is returned
  Standard_EXPORT static   Standard_Boolean CommonVertex (const TopoDS_Edge& E1, const TopoDS_Edge& E2, TopoDS_Vertex& V) ;




protected:





private:




friend class TopExp_Explorer;

};







#endif // _TopExp_HeaderFile
