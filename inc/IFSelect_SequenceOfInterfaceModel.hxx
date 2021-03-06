// This file is generated by WOK (CPPExt).
// Please do not edit this file; modify original file instead.
// The copyright and license terms as defined for the original file apply to 
// this header file considered to be the "object code" form of the original source.

#ifndef _IFSelect_SequenceOfInterfaceModel_HeaderFile
#define _IFSelect_SequenceOfInterfaceModel_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>

#include <TCollection_BaseSequence.hxx>
#include <Handle_Interface_InterfaceModel.hxx>
#include <Handle_IFSelect_SequenceNodeOfSequenceOfInterfaceModel.hxx>
#include <Standard_Integer.hxx>
class Standard_NoSuchObject;
class Standard_OutOfRange;
class Interface_InterfaceModel;
class IFSelect_SequenceNodeOfSequenceOfInterfaceModel;



class IFSelect_SequenceOfInterfaceModel  : public TCollection_BaseSequence
{
public:

  DEFINE_STANDARD_ALLOC

  
    IFSelect_SequenceOfInterfaceModel();
  
  Standard_EXPORT IFSelect_SequenceOfInterfaceModel(const IFSelect_SequenceOfInterfaceModel& Other);
  
  Standard_EXPORT   void Clear() ;
~IFSelect_SequenceOfInterfaceModel()
{
  Clear();
}
  
  Standard_EXPORT  const  IFSelect_SequenceOfInterfaceModel& Assign (const IFSelect_SequenceOfInterfaceModel& Other) ;
 const  IFSelect_SequenceOfInterfaceModel& operator = (const IFSelect_SequenceOfInterfaceModel& Other) 
{
  return Assign(Other);
}
  
  Standard_EXPORT   void Append (const Handle(Interface_InterfaceModel)& T) ;
  
      void Append (IFSelect_SequenceOfInterfaceModel& S) ;
  
  Standard_EXPORT   void Prepend (const Handle(Interface_InterfaceModel)& T) ;
  
      void Prepend (IFSelect_SequenceOfInterfaceModel& S) ;
  
      void InsertBefore (const Standard_Integer Index, const Handle(Interface_InterfaceModel)& T) ;
  
      void InsertBefore (const Standard_Integer Index, IFSelect_SequenceOfInterfaceModel& S) ;
  
  Standard_EXPORT   void InsertAfter (const Standard_Integer Index, const Handle(Interface_InterfaceModel)& T) ;
  
      void InsertAfter (const Standard_Integer Index, IFSelect_SequenceOfInterfaceModel& S) ;
  
  Standard_EXPORT  const  Handle(Interface_InterfaceModel)& First()  const;
  
  Standard_EXPORT  const  Handle(Interface_InterfaceModel)& Last()  const;
  
      void Split (const Standard_Integer Index, IFSelect_SequenceOfInterfaceModel& Sub) ;
  
  Standard_EXPORT  const  Handle(Interface_InterfaceModel)& Value (const Standard_Integer Index)  const;
 const  Handle(Interface_InterfaceModel)& operator() (const Standard_Integer Index)  const
{
  return Value(Index);
}
  
  Standard_EXPORT   void SetValue (const Standard_Integer Index, const Handle(Interface_InterfaceModel)& I) ;
  
  Standard_EXPORT   Handle(Interface_InterfaceModel)& ChangeValue (const Standard_Integer Index) ;
  Handle(Interface_InterfaceModel)& operator() (const Standard_Integer Index) 
{
  return ChangeValue(Index);
}
  
  Standard_EXPORT   void Remove (const Standard_Integer Index) ;
  
  Standard_EXPORT   void Remove (const Standard_Integer FromIndex, const Standard_Integer ToIndex) ;




protected:





private:





};

#define SeqItem Handle(Interface_InterfaceModel)
#define SeqItem_hxx <Interface_InterfaceModel.hxx>
#define TCollection_SequenceNode IFSelect_SequenceNodeOfSequenceOfInterfaceModel
#define TCollection_SequenceNode_hxx <IFSelect_SequenceNodeOfSequenceOfInterfaceModel.hxx>
#define Handle_TCollection_SequenceNode Handle_IFSelect_SequenceNodeOfSequenceOfInterfaceModel
#define TCollection_SequenceNode_Type_() IFSelect_SequenceNodeOfSequenceOfInterfaceModel_Type_()
#define TCollection_Sequence IFSelect_SequenceOfInterfaceModel
#define TCollection_Sequence_hxx <IFSelect_SequenceOfInterfaceModel.hxx>

#include <TCollection_Sequence.lxx>

#undef SeqItem
#undef SeqItem_hxx
#undef TCollection_SequenceNode
#undef TCollection_SequenceNode_hxx
#undef Handle_TCollection_SequenceNode
#undef TCollection_SequenceNode_Type_
#undef TCollection_Sequence
#undef TCollection_Sequence_hxx




#endif // _IFSelect_SequenceOfInterfaceModel_HeaderFile
