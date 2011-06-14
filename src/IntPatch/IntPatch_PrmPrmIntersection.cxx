// File:      IntPatch_PrmPrmIntersection.cxx
// Created:   Tue Feb  2 18:05:43 1993
// Author:    Laurent BUCHARD
// Copyright: OPEN CASCADE 1993

// modified by Edward AGAPOV (eap) Tue Jan 22 12:29:55 2002
// modified by Oleg FEDYAED  (ofv) Fri Nov 29 16:08:02 2002

#include <IntPatch_PrmPrmIntersection.ixx>

#include <IntPatch_TheInterfPolyhedron.hxx>
#include <IntPatch_ThePWalkingInter.hxx>
#include <IntPatch_WLine.hxx>
#include <IntPatch_RstInt.hxx>

#include <IntPatch_Line.hxx>

#include <Handle_IntSurf_LineOn2S.hxx>
#include <IntSurf_PntOn2S.hxx>
#include <IntSurf_LineOn2S.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <Intf_PIType.hxx>
#include <Intf_SectionLine.hxx>
#include <Intf_TangentZone.hxx>
#include <Intf_SectionPoint.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <IntPolyh_Intersection.hxx>

#include <TColStd_SequenceOfInteger.hxx>
#include <IntSurf_ListIteratorOfListOfPntOn2S.hxx>
#include <TColStd_HArray1OfReal.hxx>

static void SectionPointToParameters(const Intf_SectionPoint& Sp,
				     const IntPatch_Polyhedron& Surf1,
				     const IntPatch_Polyhedron& Surf2,
				     Standard_Real& u1,
				     Standard_Real& v1,
				     Standard_Real& u2,
				     Standard_Real& v2);


static 
  void AdjustOnPeriodic(const Handle(Adaptor3d_HSurface)& Surf1,
			const Handle(Adaptor3d_HSurface)& Surf2,
			IntPatch_SequenceOfLine& aSLin);

//==================================================================================
// function : 
// purpose  : 
//==================================================================================
IntPatch_PrmPrmIntersection::IntPatch_PrmPrmIntersection(): done(Standard_False)
{
}

//==================================================================================
// function : Perform
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::Perform (const Handle(Adaptor3d_HSurface)&   Surf1,
					   const Handle(Adaptor3d_TopolTool)& D1,
					   const Standard_Real  TolTangency,
					   const Standard_Real  Epsilon,
					   const Standard_Real  Deflection,
					   const Standard_Real  Increment)
{ 
  IntPatch_Polyhedron Poly1( Surf1, D1->NbSamplesU(), D1->NbSamplesV() );
  Perform( Surf1, Poly1, D1, TolTangency, Epsilon, Deflection, Increment );
}

//==================================================================================
// function : Perform
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::Perform (const Handle(Adaptor3d_HSurface)&   Surf1,
					   const IntPatch_Polyhedron& Poly1,
					   const Handle(Adaptor3d_TopolTool)& D1,
					   const Handle(Adaptor3d_HSurface)&   Surf2,
					   const Handle(Adaptor3d_TopolTool)& D2,
					   const Standard_Real  TolTangency,
					   const Standard_Real  Epsilon,
					   const Standard_Real  Deflection,
					   const Standard_Real  Increment)
{ 
  IntPatch_Polyhedron Poly2( Surf2 );
  Perform( Surf1, Poly1, D1, Surf2, Poly2, D2, TolTangency, Epsilon, Deflection, Increment);
}

//==================================================================================
// function : Perform
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::Perform (const Handle(Adaptor3d_HSurface)&   Surf1,
					   const Handle(Adaptor3d_TopolTool)& D1,
					   const Handle(Adaptor3d_HSurface)&   Surf2,
					   const IntPatch_Polyhedron& Poly2,
					   const Handle(Adaptor3d_TopolTool)& D2,
					   const Standard_Real  TolTangency,
					   const Standard_Real  Epsilon,
					   const Standard_Real  Deflection,
					   const Standard_Real  Increment)
{ 
  IntPatch_Polyhedron Poly1( Surf1 );    
  Perform( Surf1, Poly1, D1, Surf2, Poly2, D2, TolTangency, Epsilon, Deflection, Increment );
}

//==================================================================================
// function : Perform
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::Perform (const Handle(Adaptor3d_HSurface)&    Surf1,
					   const IntPatch_Polyhedron&  Poly1,
					   const Handle(Adaptor3d_TopolTool)& D1,
					   const Handle(Adaptor3d_HSurface)&    Surf2,
					   const IntPatch_Polyhedron&  Poly2,
					   const Handle(Adaptor3d_TopolTool)& D2,
					   const Standard_Real   TolTangency,
					   const Standard_Real   Epsilon,
					   const Standard_Real   Deflection,
					   const Standard_Real   Increment)
{ 
  IntPatch_TheInterfPolyhedron Interference(Poly1,Poly2);
  empt = Standard_True;
  done = Standard_True;
  SLin.Clear();  

  Standard_Real Deflection2 = Deflection*Deflection;
  Standard_Integer nbLigSec = Interference.NbSectionLines();
  Standard_Integer nbTanZon = Interference.NbTangentZones();

  Standard_Integer NbPntOn2SOnLine;
  Standard_Integer NbLigCalculee = 0;

  Standard_Real U1,U2,V1,V2;
  Standard_Real pu1,pu2,pv1,pv2;

  TColStd_Array1OfReal StartParams(1,4);

  IntPatch_ThePWalkingInter PW( Surf1, Surf2, TolTangency, Epsilon, Deflection, Increment );
    
  Standard_Real    SeuildPointLigne = 15.0 * Increment * Increment; //-- 10 est insuffisant
  Standard_Real    incidence;
  Standard_Real    dminiPointLigne;

  Standard_Boolean HasStartPoint,RejetLigne;

  IntSurf_PntOn2S StartPOn2S;

  Standard_Integer ver;

  gp_Pnt Point3dDebut,Point3dFin;

  if( nbLigSec >= 1 ) {
    Standard_Integer *TabL = new Standard_Integer [nbLigSec+1];
    Standard_Integer ls;
    for( ls = 1; ls <= nbLigSec; ls++ )
      TabL[ls]=ls;

    Standard_Boolean triok;
    do { 
      triok=Standard_True;
      for(Standard_Integer b=2; b<=nbLigSec; b++) {
	Standard_Integer nb_B = Interference.LineValue(TabL[b]).NumberOfPoints();
	Standard_Integer nb_A = Interference.LineValue(TabL[b-1]).NumberOfPoints();
	if( nb_B > nb_A ) { 
	  Standard_Integer tyu=TabL[b]; 
	  TabL[b]=TabL[b-1];
	  TabL[b-1]=tyu;
	  triok=Standard_False;
	}
      }
    } while( triok==Standard_False );

    for( ls = 1; ls <= nbLigSec; ls++) {
      const Intf_SectionLine& LineSec=Interference.LineValue(TabL[ls]);
      Standard_Integer nbp = LineSec.NumberOfPoints();

      Standard_Integer *TabPtDep = new Standard_Integer [nbp+1];
      Standard_Integer ilig;
      for( ilig = 1; ilig <= nbp; ilig++) 
	TabPtDep[ilig]=0;

      Standard_Real UminLig1,VminLig1,UmaxLig1,VmaxLig1;
      Standard_Real UminLig2,VminLig2,UmaxLig2,VmaxLig2;

      SectionPointToParameters(LineSec.GetPoint(1),Poly1,Poly2,UminLig1,VminLig1,UminLig2,VminLig2);

      UmaxLig1=UminLig1;
      VmaxLig1=VminLig1;
      UmaxLig2=UminLig2;
      VmaxLig2=VminLig2;

      for( ilig = 2; ilig <= nbp; ilig++ ) { 
	SectionPointToParameters(LineSec.GetPoint(ilig),Poly1,Poly2,U1,V1,U2,V2);

	if(U1>UmaxLig1) UmaxLig1=U1;
	if(V1>VmaxLig1) VmaxLig1=V1;
	if(U2>UmaxLig2) UmaxLig2=U2;
	if(V2>VmaxLig2) VmaxLig2=V2;

	if(U1<UminLig1) UminLig1=U1;
	if(V1<VminLig1) VminLig1=V1;
	if(U2<UminLig2) UminLig2=U2;
	if(V2<VminLig2) VminLig2=V2;
      }
	
      Standard_Integer nbps2 = (nbp>3)? (nbp/2) :  1;
      Standard_Integer NombreDePointsDeDepartDuCheminement = 0;
      Standard_Integer IndicePointdeDepart1 = 0,IndicePointdeDepart2 = 0;
      Standard_Boolean lignetrouvee=Standard_False;

      do { 
	NombreDePointsDeDepartDuCheminement++;
	if(NombreDePointsDeDepartDuCheminement == 1) { 
	  incidence=3.0;
	  Standard_Integer nbp1_4=nbp/4;
	  Standard_Integer nbp3_4=nbp-nbp1_4;

	  Standard_Integer nsp;
	  for(nsp=nbp/2; nsp<nbp3_4; nsp++) { 
	    Standard_Real CurrentIncidence =  LineSec.GetPoint(nsp).Incidence();
	    if(CurrentIncidence < incidence) { 
	      nbps2 = nsp;
	      incidence = 0.9*CurrentIncidence;
	    }
	  }

	  for(nsp=nbp/2; nsp>nbp1_4; nsp--) { 
	    Standard_Real CurrentIncidence =  LineSec.GetPoint(nsp).Incidence();
	    if(CurrentIncidence < incidence) { 
	      nbps2 = nsp;
	      incidence = 0.9*CurrentIncidence;
	    }
	  }

	  if(nbp<3) 
	    NombreDePointsDeDepartDuCheminement=3;

	  IndicePointdeDepart1 = nbps2;
	}
	else if(NombreDePointsDeDepartDuCheminement == 2) { 
	  if(IndicePointdeDepart1 == 1) { 
	    nbps2 = nbp/2;
	    IndicePointdeDepart2 = nbps2;
	  }
	  else { 
	    nbps2 = 1;
	    IndicePointdeDepart2 = 1;
	  }
	}
	else if(NombreDePointsDeDepartDuCheminement == 3) {
	  if(IndicePointdeDepart1 == nbp)
	    nbps2 = (IndicePointdeDepart1+IndicePointdeDepart2)/2;
	  else
	    nbps2 = nbp;
	}
	else { 
	  nbps2 = NombreDePointsDeDepartDuCheminement-3;
	  NombreDePointsDeDepartDuCheminement++;
	} 

	if(TabPtDep[nbps2]==0) { 
	  TabPtDep[nbps2]=1;
	  SectionPointToParameters(LineSec.GetPoint(nbps2),Poly1,Poly2,U1,V1,U2,V2);

	  StartParams(1) = U1;
	  StartParams(2) = V1;
	  StartParams(3) = U2;
	  StartParams(4) = V2;
	    
	  HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);
	  dminiPointLigne = SeuildPointLigne + SeuildPointLigne;
		  
	  if(HasStartPoint) { 
	    StartPOn2S.Parameters(pu1,pv1,pu2,pv2);
	    NbLigCalculee = SLin.Length();
	    Standard_Integer l;
	    for( l=1; (l <= NbLigCalculee) && (dminiPointLigne >= SeuildPointLigne); l++) { 
	      Handle(IntPatch_WLine)& testwline = *((Handle(IntPatch_WLine)*)&SLin.ChangeValue(l));
	      if( (testwline->IsOutSurf1Box(gp_Pnt2d(pu1,pv1))==Standard_False) &&
		  (testwline->IsOutSurf2Box(gp_Pnt2d(pu2,pv2))==Standard_False) &&
		  (testwline->IsOutBox(StartPOn2S.Value())==Standard_False) ) { 
		NbPntOn2SOnLine = testwline->NbPnts();
		Standard_Integer ll;
		for( ll=1; (ll < NbPntOn2SOnLine) && (dminiPointLigne >= SeuildPointLigne); ll++) { 
		  const gp_Pnt &Pa = testwline->Point(ll).Value();
		  const gp_Pnt &Pb = testwline->Point(ll+1).Value();
		  const gp_Pnt &PStart = StartPOn2S.Value();
		  gp_Vec AM(Pa,PStart);
		  gp_Vec MB(PStart,Pb);
		  Standard_Real AMMB = AM.Dot(MB);		    
		  if(AMMB > 0.0) {
		    gp_Dir ABN(Pb.X()-Pa.X(),Pb.Y()-Pa.Y(),Pb.Z()-Pa.Z());
		    Standard_Real lan =  ABN.X()*AM.X()+ABN.Y()*AM.Y()+ABN.Z()*AM.Z();
		    gp_Vec AH(lan*ABN.X(),lan*ABN.Y(),lan*ABN.Z());
		    gp_Vec HM(AM.X()-AH.X(),AM.Y()-AH.Y(),AM.Z()-AH.Z());
		    Standard_Real d = 0.0;
		    if(HM.X() < Deflection) { 
		      d+=HM.X()*HM.X();
		      if(HM.Y() < Deflection) { 
			d+=HM.Y()*HM.Y(); 
			if(HM.Z() < Deflection)
			  d+=HM.Z()*HM.Z(); 
			else
			  d=Deflection2;
		      }
		      else
			d=Deflection2;
		    } 
		    else 
		      d=Deflection2; 

		    if(d<Deflection2)
		      dminiPointLigne = 0.0;
		  }
		  else {
		    Standard_Real dab = Pa.SquareDistance(Pb);
		    Standard_Real dap = Pa.SquareDistance(PStart);
		    if(dap < dab)
		      dminiPointLigne=0;
		    else { 
		      Standard_Real dbp = Pb.SquareDistance(PStart);
		      if(dbp < dab)
			dminiPointLigne=0;
		    }
		  }
		}
	      }
	    } // for ( l ...

	    if(dminiPointLigne > SeuildPointLigne) { 
	      PW.Perform(StartParams,UminLig1,VminLig1,UminLig2,VminLig2,UmaxLig1,VmaxLig1,UmaxLig2,VmaxLig2);
	      if(PW.IsDone()) {
		if(PW.NbPoints()>2) { 
		  RejetLigne = Standard_False;
		  Point3dDebut = PW.Value(1).Value();
		  Point3dFin   = PW.Value(PW.NbPoints()).Value();
		  for( ver = 1 ; (!RejetLigne) && (ver<= NbLigCalculee) ; ver++) { 
		    const Handle(IntPatch_WLine)& verwline = *((Handle(IntPatch_WLine)*)&SLin.Value(ver));
		    const IntSurf_PntOn2S& verPointDebut = verwline->Point(1);
		    const IntSurf_PntOn2S& verPointFin = verwline->Point(verwline->NbPnts());
		    if( Point3dDebut.Distance(verPointDebut.Value()) <= TolTangency ) { 
		      if(Point3dFin.Distance(verPointFin.Value()) <= TolTangency)
			RejetLigne = Standard_True; 
		    }
		  }

		  if(!RejetLigne) { 
		    // Calculation transition
		    IntSurf_TypeTrans trans1,trans2;
		    Standard_Real locu,locv;
		    gp_Vec norm1,norm2,d1u,d1v;
		    gp_Pnt ptbid;
		    Standard_Integer indextg;
		    gp_Vec tgline(PW.TangentAtLine(indextg));
		    PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
		    Surf1->D1(locu,locv,ptbid,d1u,d1v);
		    norm1 = d1u.Crossed(d1v);
		    PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
		    Surf2->D1(locu,locv,ptbid,d1u,d1v);
		    norm2 = d1u.Crossed(d1v);
		    if(tgline.DotCross(norm2,norm1)>0.) {
		      trans1 = IntSurf_Out;
		      trans2 = IntSurf_In;
		    }
		    else {
		      trans1 = IntSurf_In;
		      trans2 = IntSurf_Out;
		    }

		    Standard_Real TolTang = TolTangency;
		    Handle(IntPatch_WLine) wline = new IntPatch_WLine(PW.Line(),Standard_False,trans1,trans2);
		    IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf2,Standard_True,TolTang);
		    IntPatch_RstInt::PutVertexOnLine(wline,Surf2,D2,Surf1,Standard_False,TolTang);

		    if(wline->NbVertex() == 0) {
		      IntPatch_Point vtx;
		      IntSurf_PntOn2S POn2S = PW.Line()->Value(1);
		      POn2S.Parameters(pu1,pv1,pu2,pv2);
		      vtx.SetValue(Point3dDebut,TolTang,Standard_False);
		      vtx.SetParameters(pu1,pv1,pu2,pv2);
		      vtx.SetParameter(1);
		      wline->AddVertex(vtx);

		      POn2S = PW.Line()->Value(wline->NbPnts());
		      POn2S.Parameters(pu1,pv1,pu2,pv2);
		      vtx.SetValue(Point3dFin,TolTang,Standard_False);
		      vtx.SetParameters(pu1,pv1,pu2,pv2);
		      vtx.SetParameter(wline->NbPnts());
		      wline->AddVertex(vtx);
		    }

		    lignetrouvee = Standard_True;
		    SLin.Append(wline);
		    empt = Standard_False;
		  }// !RejetLigne
		}// PW.NbPoints()>2
	      }// done is True
	    }// dminiPointLigne > SeuildPointLigne
	  }// HasStartPoint
	}// TabPtDep[nbps2]==0
      } while( nbp>5 && ( !( ( (NombreDePointsDeDepartDuCheminement >= 3) && lignetrouvee ) || 
			( (NombreDePointsDeDepartDuCheminement-3>=nbp) && !lignetrouvee ) ) ) );

      delete [] TabPtDep;
    }// for( ls ...

    delete [] TabL;
  }// nbLigSec >= 1

  Standard_Real UminLig1,VminLig1,UmaxLig1,VmaxLig1;
  Standard_Real UminLig2,VminLig2,UmaxLig2,VmaxLig2;
    
  UminLig1=VminLig1=UminLig2=VminLig2=RealLast();
  UmaxLig1=VmaxLig1=UmaxLig2=VmaxLig2=-UminLig1;

  Standard_Integer z;
  for(z=1; z <= nbTanZon; z++) { 
    const Intf_TangentZone& TangentZone=Interference.ZoneValue(z);
    for(Standard_Integer pz=1; pz<=TangentZone.NumberOfPoints(); pz++) { 
      SectionPointToParameters(TangentZone.GetPoint(pz),Poly1,Poly2,U1,V1,U2,V2);

      if(U1>UmaxLig1) UmaxLig1=U1;
      if(V1>VmaxLig1) VmaxLig1=V1;
      if(U2>UmaxLig2) UmaxLig2=U2;
      if(V2>VmaxLig2) VmaxLig2=V2;
	
      if(U1<UminLig1) UminLig1=U1;
      if(V1<VminLig1) VminLig1=V1;
      if(U2<UminLig2) UminLig2=U2;
      if(V2<VminLig2) VminLig2=V2;
    }
  }

  for(z=1; z <= nbTanZon; z++) {
    const Intf_TangentZone& TangentZone=Interference.ZoneValue(z);
    Standard_Integer pz;
    for( pz=1; pz<=TangentZone.NumberOfPoints(); pz++) { 
      SectionPointToParameters(TangentZone.GetPoint(pz),Poly1,Poly2,U1,V1,U2,V2);

      StartParams(1) = U1;
      StartParams(2) = V1;
      StartParams(3) = U2;
      StartParams(4) = V2;
	
      HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);
      if(HasStartPoint) { 
	StartPOn2S.Parameters(pu1,pv1,pu2,pv2);
	NbLigCalculee = SLin.Length();
	dminiPointLigne = SeuildPointLigne + SeuildPointLigne; 
	Standard_Integer l;
	for( l = 1; (l <= NbLigCalculee) && (dminiPointLigne >= SeuildPointLigne); l++)	{ 
	  Handle(IntPatch_WLine)& testwline = *((Handle(IntPatch_WLine)*)&SLin.ChangeValue(l));
	  if( (testwline->IsOutSurf1Box(gp_Pnt2d(pu1,pv1))==Standard_False) &&
	      (testwline->IsOutSurf2Box(gp_Pnt2d(pu2,pv2))==Standard_False) &&
	      (testwline->IsOutBox(StartPOn2S.Value())==Standard_False) ) { 
	    NbPntOn2SOnLine = testwline->NbPnts();
	    Standard_Integer ll;
	    for( ll = 1; (ll < NbPntOn2SOnLine) && (dminiPointLigne >= SeuildPointLigne); ll++)	{ 
	      const gp_Pnt& Pa = testwline->Point(ll).Value();
	      const gp_Pnt& Pb = testwline->Point(ll+1).Value();
	      const gp_Pnt& PStart=StartPOn2S.Value();
	      gp_Vec AM(Pa,PStart);
	      gp_Vec MB(PStart,Pb);
	      Standard_Real AMMB = AM.Dot(MB);
	      if(AMMB > 0.0) {
		gp_Dir ABN(Pb.X()-Pa.X(),Pb.Y()-Pa.Y(),Pb.Z()-Pa.Z());
		Standard_Real lan =  ABN.X()*AM.X()+ABN.Y()*AM.Y()+ABN.Z()*AM.Z();
		gp_Vec AH(lan*ABN.X(),lan*ABN.Y(),lan*ABN.Z());
		gp_Vec HM(AM.X()-AH.X(),AM.Y()-AH.Y(),AM.Z()-AH.Z()); 
		Standard_Real d = 0.0;
		if(HM.X() < Deflection)	{ 
		  d+=HM.X()*HM.X();
		  if(HM.Y() < Deflection) { 
		    d+=HM.Y()*HM.Y(); 
		    if(HM.Z() < Deflection)
		      d+=HM.Z()*HM.Z(); 
		    else
		      d=Deflection2;
		  }
		  else
		    d=Deflection2;
		} 
		else
		  d=Deflection2;

		if(d<Deflection2)
		  dminiPointLigne = 0.0;
	      }
	      else { 
		Standard_Real dab = Pa.Distance(Pb);
		Standard_Real dap = Pa.Distance(PStart);
		if(dap < dab)
		  dminiPointLigne=0;
		else { 
		  Standard_Real dbp = Pb.Distance(PStart);
		  if(dbp < dab)
		    dminiPointLigne=0;
		}
	      }		
	    }// for( ll ...
	  }// if ...
	}// for( l ...

	if(dminiPointLigne > SeuildPointLigne) { 
	  PW.Perform(StartParams,UminLig1,VminLig1,UminLig2,VminLig2,UmaxLig1,VmaxLig1,UmaxLig2,VmaxLig2);
	  if(PW.IsDone()) {
	    if(PW.NbPoints()>2)	{ 
	      RejetLigne = Standard_False;
	      Point3dDebut = PW.Value(1).Value();
	      Point3dFin   = PW.Value(PW.NbPoints()).Value();
	      for(ver=1 ; (!RejetLigne) && (ver<= NbLigCalculee) ; ver++) { 
		const Handle(IntPatch_WLine)& verwline = *((Handle(IntPatch_WLine)*)&SLin.Value(ver));
		const IntSurf_PntOn2S& verPointDebut = verwline->Point(1);
		const IntSurf_PntOn2S& verPointFin   = verwline->Point(verwline->NbPnts());
		if(Point3dDebut.Distance(verPointDebut.Value()) < TolTangency)
		  RejetLigne = Standard_True; 
		else { 
		  if(Point3dFin.Distance(verPointFin.Value()) < TolTangency)
		    RejetLigne = Standard_True; 
		}
	      }
	      
	      if(!RejetLigne) { 
		IntSurf_TypeTrans trans1,trans2;
		Standard_Real locu,locv;
		gp_Vec norm1,norm2,d1u,d1v;
		gp_Pnt ptbid;
		Standard_Integer indextg;
		gp_Vec tgline(PW.TangentAtLine(indextg));
		PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
		Surf1->D1(locu,locv,ptbid,d1u,d1v);
		norm1 = d1u.Crossed(d1v);
		PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
		Surf2->D1(locu,locv,ptbid,d1u,d1v);
		norm2 = d1u.Crossed(d1v);
		if(tgline.DotCross(norm2,norm1)>0.) {
		  trans1 = IntSurf_Out;
		  trans2 = IntSurf_In;
		}
		else {
		  trans1 = IntSurf_In;
		  trans2 = IntSurf_Out;
		}

		Standard_Real TolTang = TolTangency;
		Handle(IntPatch_WLine) wline = new IntPatch_WLine(PW.Line(),Standard_False,trans1,trans2);
		IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf2,Standard_True,TolTang);
		IntPatch_RstInt::PutVertexOnLine(wline,Surf2,D2,Surf1,Standard_False,TolTang);

		if(wline->NbVertex() == 0) {
		  IntPatch_Point vtx;
		  IntSurf_PntOn2S POn2S = PW.Line()->Value(1);
		  POn2S.Parameters(pu1,pv1,pu2,pv2);
		  vtx.SetValue(Point3dDebut,TolTang,Standard_False);
		  vtx.SetParameters(pu1,pv1,pu2,pv2);
		  vtx.SetParameter(1);
		  wline->AddVertex(vtx);
		      
		  POn2S = PW.Line()->Value(wline->NbPnts());
		  POn2S.Parameters(pu1,pv1,pu2,pv2);
		  vtx.SetValue(Point3dFin,TolTang,Standard_False);
		  vtx.SetParameters(pu1,pv1,pu2,pv2);
		  vtx.SetParameter(wline->NbPnts());
		  wline->AddVertex(vtx);
		}

		SLin.Append(wline);
		empt = Standard_False;
	      }// if !RejetLigne
	    }// PW.NbPoints()>2
	  }// done is True
	}// dminiPointLigne > SeuildPointLigne
      }// HasStartPoint
    }// for( pz ...
  }// for( z ...
}

//==================================================================================
// function : Perform
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::Perform (const Handle(Adaptor3d_HSurface)&    Surf1,
					   const IntPatch_Polyhedron&  Poly1,
					   const Handle(Adaptor3d_TopolTool)& D1,
					   const Standard_Real   TolTangency,
					   const Standard_Real   Epsilon,
					   const Standard_Real   Deflection,
					   const Standard_Real   Increment)
{ 
  IntPatch_TheInterfPolyhedron Interference(Poly1);
  empt = Standard_True;
  done = Standard_True;
  SLin.Clear();  

  Standard_Integer nbLigSec = Interference.NbSectionLines();
  Standard_Integer nbTanZon = Interference.NbTangentZones();

  Standard_Integer NbPntOn2SOnLine;
  Standard_Integer NbLigCalculee = 0;

  Standard_Real U1,U2,V1,V2;
  Standard_Real pu1,pu2,pv1,pv2;

  TColStd_Array1OfReal StartParams(1,4);
  IntPatch_ThePWalkingInter PW(Surf1,Surf1,TolTangency,Epsilon,Deflection,Increment);
   
  Standard_Real    SeuildPointLigne = 15.0 * Increment * Increment; //-- 10 est insuffisant
  Standard_Real    incidence;
  Standard_Real    dminiPointLigne;

  Standard_Boolean HasStartPoint,RejetLigne;

  IntSurf_PntOn2S StartPOn2S;

  Standard_Integer ver;

  gp_Pnt Point3dDebut,Point3dFin;

  if(nbLigSec>=1) {
    Standard_Integer ls;
    for( ls = 1; ls <= nbLigSec; ls++) {
      const Intf_SectionLine& LineSec=Interference.LineValue(ls);
      Standard_Integer nbp = LineSec.NumberOfPoints();
      Standard_Integer nbps2 = (nbp>3)? (nbp/2) :  1;
      Standard_Integer NombreDePointsDeDepartDuCheminement = 0;
      Standard_Integer IndicePointdeDepart1 = 0, IndicePointdeDepart2 = 0;
      do { 
	NombreDePointsDeDepartDuCheminement++;
	if(NombreDePointsDeDepartDuCheminement == 1) { 
	  incidence = 0.0;
	  Standard_Integer nsp1;
	  for( nsp1= nbp/2; nsp1 >= 1; nsp1--) { 
	    SectionPointToParameters(LineSec.GetPoint(nsp1),Poly1,Poly1,U1,V1,U2,V2);
	    Standard_Real CurrentIncidence =  Abs(U1-U2)+Abs(V1-V2);
	    if(CurrentIncidence > incidence) { 
	      nbps2 = nsp1;
	      incidence = CurrentIncidence;
	    }
	  }
	  for( nsp1 = nbp/2; nsp1 <= nbp; nsp1++) { 
	    SectionPointToParameters(LineSec.GetPoint(nsp1),Poly1,Poly1,U1,V1,U2,V2);
	    Standard_Real CurrentIncidence =  Abs(U1-U2)+Abs(V1-V2);
	    if(CurrentIncidence > incidence) { 
	      nbps2 = nsp1;
	      incidence = CurrentIncidence;
	    }
	  }

	  if(nbp<3) 
	    NombreDePointsDeDepartDuCheminement=3;
		  
	  IndicePointdeDepart1 = nbps2;
	}
	else if(NombreDePointsDeDepartDuCheminement == 2) { 
	  if(IndicePointdeDepart1 == 1) { 
	    nbps2 = nbp/2;
	    IndicePointdeDepart2 = nbps2;
	  }
	  else { 
	    nbps2 = 1;
	    IndicePointdeDepart2 = 1;
	  }
	}
	else {
	  if(IndicePointdeDepart1 == nbp)
	    nbps2 = (IndicePointdeDepart1+IndicePointdeDepart2)/2;
	  else
	    nbps2 = nbp;
	}

	SectionPointToParameters(LineSec.GetPoint(nbps2),Poly1,Poly1,U1,V1,U2,V2);

	StartParams(1) = U1;
	StartParams(2) = V1;
	StartParams(3) = U2;
	StartParams(4) = V2;

	HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);
	dminiPointLigne = SeuildPointLigne + SeuildPointLigne;
	if(HasStartPoint) { 
	  StartPOn2S.Parameters(pu1,pv1,pu2,pv2);
	  if(Abs(pu1-pu2)>1e-7 || Abs(pv1-pv2)>1e-7) { 
	    NbLigCalculee = SLin.Length();
	    Standard_Integer l;
	    for( l = 1; (l <= NbLigCalculee) && (dminiPointLigne >= SeuildPointLigne); l++) { 
	      Handle(IntPatch_WLine)& testwline = *((Handle(IntPatch_WLine)*)&SLin.ChangeValue(l));
	      if( (testwline->IsOutSurf1Box(gp_Pnt2d(pu1,pv1))==Standard_False) &&
		  (testwline->IsOutSurf2Box(gp_Pnt2d(pu2,pv2))==Standard_False) &&
		  (testwline->IsOutBox(StartPOn2S.Value())==Standard_False) ) { 
		NbPntOn2SOnLine = testwline->NbPnts();
		Standard_Integer ll;
		for( ll=1; (ll < NbPntOn2SOnLine) && (dminiPointLigne >= SeuildPointLigne); ll++) { 
		  Standard_Real t,Au1,Av1,Au2,Av2,Bu1,Bv1,Bu2,Bv2;
		  testwline->Point(ll).Parameters(Au1,Av1,Au2,Av2);
		  testwline->Point(ll+1).Parameters(Bu1,Bv1,Bu2,Bv2);
		  if(Au1>Bu1) {
		    t=Au1;
		    Au1=Bu1;
		    Bu1=t;
		  } 
		  if(Av1>Bv1) {
		    t=Av1;
		    Av1=Bv1;
		    Bv1=t;
		  } 
		  Au1-=1.0e-7;
		  Av1-=1.0e-7;
		  Bu1+=1.0e-7;
		  Bv1+=1.0e-7;
		  
		  if((pu1>=Au1) && (pu1<=Bu1) && (pv1>=Av1) && (pv1<=Bv1))
		    dminiPointLigne = 0.0; 
		  else { 
		    if((pu2>=Au1) && (pu2<=Bu1) && (pv2>=Av1) && (pv2<=Bv1))
		      dminiPointLigne = 0.0;
		  }
		}// for( ll ...
	      }// if ...
	    }// for( l ...

	    if(dminiPointLigne > SeuildPointLigne) { 
	      PW.Perform(StartParams);
	      if(PW.IsDone()) {
		if(PW.NbPoints()>2) { 
		  RejetLigne = Standard_False;
		  Point3dDebut = PW.Value(1).Value();
		  Point3dFin   = PW.Value(PW.NbPoints()).Value();
		  for(ver=1 ; (!RejetLigne) && (ver<= NbLigCalculee) ; ver++) { 
		    const Handle(IntPatch_WLine)& verwline = *((Handle(IntPatch_WLine)*)&SLin.Value(ver));
		    const IntSurf_PntOn2S& verPointDebut = verwline->Point(1);
		    const IntSurf_PntOn2S& verPointFin = verwline->Point(verwline->NbPnts());
		    if(Point3dDebut.Distance(verPointDebut.Value()) < TolTangency)
		      RejetLigne = Standard_True; 
		    else { 
		      if(Point3dFin.Distance(verPointFin.Value()) < TolTangency)
			RejetLigne = Standard_True; 
		    }
		  }

		  if(!RejetLigne) { 
		    IntSurf_TypeTrans trans1,trans2;
		    Standard_Real locu,locv;
		    gp_Vec norm1,norm2,d1u,d1v;
		    gp_Pnt ptbid;
		    Standard_Integer indextg;
		    gp_Vec tgline(PW.TangentAtLine(indextg));
		    PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
		    Surf1->D1(locu,locv,ptbid,d1u,d1v);
		    norm1 = d1u.Crossed(d1v);
		    PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
		    Surf1->D1(locu,locv,ptbid,d1u,d1v);
		    norm2 = d1u.Crossed(d1v);
		    if (tgline.DotCross(norm2,norm1)>0.) {
		      trans1 = IntSurf_Out;
		      trans2 = IntSurf_In;
		    }
		    else {
		      trans1 = IntSurf_In;
		      trans2 = IntSurf_Out;
		    }

		    IntSurf_LineOn2S LineOn2S;
		    Standard_Integer nbpw,imin,imax,i;
		    nbpw = PW.Line()->NbPoints();
		    Standard_Real u1,v1,u2,v2;
		    i=0;
		    do { 
		      i++;
		      imin=i;
		      const IntSurf_PntOn2S& Pi   = PW.Line()->Value(i);
		      Pi.Parameters(u1,v1,u2,v2);
		    } while((i<nbpw)&&(Abs(u1-u2)<=1e-6 && Abs(v1-v2)<=1e-6));
			      
		    if(imin>2)
		      imin--;

		    i=nbpw+1;
		    do { 
		      i--;
		      imax=i;
		      const IntSurf_PntOn2S& Pi   = PW.Line()->Value(i);
		      Pi.Parameters(u1,v1,u2,v2);
		    } while((i>2)&&(Abs(u1-u2)<=1e-6 && Abs(v1-v2)<=1e-6));
				      
		    if(imax<nbpw)
		      imax++;
		    
		    if(imin<imax) { 
		      Handle_IntSurf_LineOn2S PWLine = new IntSurf_LineOn2S();
		      for(i=imin;i<=imax;i++) 
			PWLine->Add(PW.Line()->Value(i));
		      
		      Standard_Real TolTang = TolTangency;
		      Handle(IntPatch_WLine) wline = new IntPatch_WLine(PWLine,Standard_False,trans1,trans2);
		      const IntSurf_PntOn2S& POn2SDeb = wline->Point(1);
		      const IntSurf_PntOn2S& POn2SFin = wline->Point(wline->NbPnts());
		      if((POn2SDeb.Value()).Distance(POn2SFin.Value()) <= TolTangency) { 
			Standard_Real u1t,v1t,u2t,v2t; 
			POn2SDeb.Parameters(u1t,v1t,u2t,v2t);
			IntPatch_Point vtx;
			vtx.SetValue(POn2SDeb.Value(),TolTang,Standard_False);
			vtx.SetParameters(u2t,v2t,u1t,v1t);
			vtx.SetParameter(wline->NbPnts());
			wline->SetPoint(wline->NbPnts(),vtx);
		      }
		      IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf1,Standard_True,TolTang);
		      if(wline->NbVertex() == 0) {
			IntPatch_Point vtx;
			IntSurf_PntOn2S POn2S = PW.Line()->Value(1);
			POn2S.Parameters(pu1,pv1,pu2,pv2);
			vtx.SetValue(Point3dDebut,TolTang,Standard_False);
			vtx.SetParameters(pu1,pv1,pu2,pv2);
			vtx.SetParameter(1);
			wline->AddVertex(vtx);
					      
			POn2S = PW.Line()->Value(wline->NbPnts());
			POn2S.Parameters(pu1,pv1,pu2,pv2);
			vtx.SetValue(Point3dFin,TolTang,Standard_False);
			vtx.SetParameters(pu1,pv1,pu2,pv2);
			vtx.SetParameter(wline->NbPnts());
			wline->AddVertex(vtx);
		      }
		      SLin.Append(wline);
		      empt = Standard_False;
		    }// imin<imax
		  }// !RejetLigne
		}// PW.NbPoints()>2
	      }// done is True
	    }// dminiPointLigne > SeuildPointLigne
	  }// Abs || Abs
	}// HasStartPoint
      } while(nbp>5 && NombreDePointsDeDepartDuCheminement<3);
    }// for( ls ...
  }// nbLigSec>=1

  Standard_Integer z;
  for( z = 1; z <= nbTanZon; z++) { 
    const Intf_TangentZone& TangentZone=Interference.ZoneValue(z);
    for(Standard_Integer pz=1; pz<=TangentZone.NumberOfPoints(); pz++) { 
      SectionPointToParameters(TangentZone.GetPoint(pz),Poly1,Poly1,U1,V1,U2,V2);

      StartParams(1) = U1;
      StartParams(2) = V1;
      StartParams(3) = U2;
      StartParams(4) = V2;

      HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);	
      if(HasStartPoint) { 
	StartPOn2S.Parameters(pu1,pv1,pu2,pv2);
	if(Abs(pu1-pu2)>1e-7 || Abs(pv1-pv2)>1e-7) { 
	  NbLigCalculee = SLin.Length();
	  dminiPointLigne = SeuildPointLigne + SeuildPointLigne; 
	  Standard_Integer l;
	  for( l = 1; (l <= NbLigCalculee) && (dminiPointLigne >= SeuildPointLigne); l++) { 
	    Handle(IntPatch_WLine)& testwline = *((Handle(IntPatch_WLine)*)&SLin.ChangeValue(l));
	    if( (testwline->IsOutSurf1Box(gp_Pnt2d(pu1,pv1))==Standard_False) &&
	        (testwline->IsOutSurf2Box(gp_Pnt2d(pu2,pv2))==Standard_False) &&
	        (testwline->IsOutBox(StartPOn2S.Value())==Standard_False) ) { 
	      NbPntOn2SOnLine = testwline->NbPnts();
	      Standard_Integer ll;
	      for( ll = 1; (ll < NbPntOn2SOnLine) && (dminiPointLigne >= SeuildPointLigne); ll++) { 
		Standard_Real t,Au1,Av1,Au2,Av2,Bu1,Bv1,Bu2,Bv2;
		testwline->Point(ll).Parameters(Au1,Av1,Au2,Av2);
		testwline->Point(ll+1).Parameters(Bu1,Bv1,Bu2,Bv2);
		if(Au1>Bu1) {
		  t=Au1;
		  Au1=Bu1;
		  Bu1=t;
		} 
		if(Av1>Bv1) {
		  t=Av1;
		  Av1=Bv1;
		  Bv1=t;
		} 
		Au1-=1.0e-7;
		Av1-=1.0e-7;
		Bu1+=1.0e-7;
		Bv1+=1.0e-7;
		if((pu1>=Au1) && (pu1<=Bu1) && (pv1>=Av1) && (pv1<=Bv1))
		  dminiPointLigne = 0.0; 
		else { 
		  if((pu2>=Au1) && (pu2<=Bu1) && (pv2>=Av1) && (pv2<=Bv1))
		    dminiPointLigne = 0.0;
		}
	      }// for( ll ...
	    }// if ...
	  }// for( l ...
	    
	  if(dminiPointLigne > SeuildPointLigne) { 
	    PW.Perform(StartParams);
	    if(PW.IsDone()) {
	      if(PW.NbPoints()>2) { 
		RejetLigne = Standard_False;
		Point3dDebut = PW.Value(1).Value();
		Point3dFin   = PW.Value(PW.NbPoints()).Value();
		for( ver = 1 ; (!RejetLigne) && (ver<= NbLigCalculee) ; ver++) { 
		  const Handle(IntPatch_WLine)& verwline = *((Handle(IntPatch_WLine)*)&SLin.Value(ver));
		  const IntSurf_PntOn2S& verPointDebut = verwline->Point(1);
		  const IntSurf_PntOn2S& verPointFin = verwline->Point(verwline->NbPnts());
		  if(Point3dDebut.Distance(verPointDebut.Value()) < TolTangency)
		    RejetLigne = Standard_True; 
		  else { 
		    if(Point3dFin.Distance(verPointFin.Value()) < TolTangency)
		      RejetLigne = Standard_True; 
		  }
		}
		  
		if(!RejetLigne)	{ 
		  IntSurf_TypeTrans trans1,trans2;
		  Standard_Real locu,locv;
		  gp_Vec norm1,norm2,d1u,d1v;
		  gp_Pnt ptbid;
		  Standard_Integer indextg;
		  gp_Vec tgline(PW.TangentAtLine(indextg));
		  PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
		  Surf1->D1(locu,locv,ptbid,d1u,d1v);
		  norm1 = d1u.Crossed(d1v);
		  PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
		  Surf1->D1(locu,locv,ptbid,d1u,d1v);
		  norm2 = d1u.Crossed(d1v);
		  if(tgline.DotCross(norm2,norm1)>0.) {
		    trans1 = IntSurf_Out;
		    trans2 = IntSurf_In;
		  }
		  else {
		    trans1 = IntSurf_In;
		    trans2 = IntSurf_Out;
		  }

		  IntSurf_LineOn2S LineOn2S;
		  Standard_Integer nbp,imin,imax,i;
		  nbp = PW.Line()->NbPoints();
		  Standard_Real u1,v1,u2,v2;
		  i=0;
		  do { 
		    i++;
		    imin=i;
		    const IntSurf_PntOn2S& Pi   = PW.Line()->Value(i);
		    Pi.Parameters(u1,v1,u2,v2);
		  } while((i<nbp)&&(Abs(u1-u2)<=1e-6 && Abs(v1-v2)<=1e-6));

		  if(imin>2)
		    imin--;
				  
		  i=nbp+1;
		  do { 
		    i--;
		    imax=i;
		    const IntSurf_PntOn2S& Pi   = PW.Line()->Value(i);
		    Pi.Parameters(u1,v1,u2,v2);
		  } while((i>2)&&(Abs(u1-u2)<=1e-6 && Abs(v1-v2)<=1e-6));

		  if(imax<nbp)
		    imax++;

		  if(imin<imax) { 
		    Handle_IntSurf_LineOn2S PWLine = new IntSurf_LineOn2S();
		    for(i=imin;i<=imax;i++)
		      PWLine->Add(PW.Line()->Value(i));
				      
		    Standard_Real TolTang = TolTangency;
		    Handle(IntPatch_WLine) wline = new IntPatch_WLine(PWLine,Standard_False,trans1,trans2);
		    const IntSurf_PntOn2S& POn2SDeb = wline->Point(1);
		    const IntSurf_PntOn2S& POn2SFin = wline->Point(wline->NbPnts());
		    if((POn2SDeb.Value()).Distance(POn2SFin.Value()) <= TolTangency) { 
		      Standard_Real u1t,v1t,u2t,v2t; 
		      POn2SDeb.Parameters(u1t,v1t,u2t,v2t);
		      IntPatch_Point vtx;
		      vtx.SetValue(POn2SDeb.Value(),TolTang,Standard_False);
		      vtx.SetParameters(u2t,v2t,u1t,v1t);
		      vtx.SetParameter(wline->NbPnts());
		      wline->SetPoint(wline->NbPnts(),vtx);
		    }

		    IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf1,Standard_True,TolTang);

		    if(wline->NbVertex() == 0) {
		      IntPatch_Point vtx;
		      IntSurf_PntOn2S POn2S = PW.Line()->Value(1);
		      POn2S.Parameters(pu1,pv1,pu2,pv2);
		      vtx.SetValue(Point3dDebut,TolTang,Standard_False);
		      vtx.SetParameters(pu1,pv1,pu2,pv2);
		      vtx.SetParameter(1);
		      wline->AddVertex(vtx);
			
		      POn2S = PW.Line()->Value(wline->NbPnts());
		      POn2S.Parameters(pu1,pv1,pu2,pv2);
		      vtx.SetValue(Point3dFin,TolTang,Standard_False);
		      vtx.SetParameters(pu1,pv1,pu2,pv2);
		      vtx.SetParameter(wline->NbPnts());
		      wline->AddVertex(vtx);
		    }

		    SLin.Append(wline);
		    empt = Standard_False;
		  }// imin<imax
		}// !RejetLigne
	      }// PW.NbPoints()>2
	    }// done a True
	  }// dminiPointLigne > SeuildPointLigne
	}// Abs || Abs
      }// HasStartPoint
    }// for ( pz ...
  }// for( z ...
}

//==================================================================================
// function : NewLine
// purpose  : 
//==================================================================================
Handle_IntPatch_Line IntPatch_PrmPrmIntersection::NewLine (const Handle(Adaptor3d_HSurface)&    Surf1,
							   const Handle(Adaptor3d_HSurface)&    Surf2,
							   const Standard_Integer NumLine,
							   const Standard_Integer Low,
							   const Standard_Integer High,
							   const Standard_Integer NbPntsToInsert) const 
{ 
  Standard_Integer NbPnts = NbPntsToInsert + High - Low;
  if(NumLine>NbLines() || NumLine<1  || Low>=High ) 
    Standard_OutOfRange::Raise(" IntPatch_PrmPrmIntersection NewLine "); 
  //------------------------------------------------------------------
  //--  Indice     :   Low       Low+1     I    I+1         High    --
  //--                                                              --
  //--  Abs.Curv.  :  S(Low)              S(I)  S(I+1)      S(High) --
  //--                                                              --
  //--                On echantillonne a abcisse curviligne         --
  //--                constante.                                    --
  //--                L abcisse est calculee sur les params U1,V1   --
  //------------------------------------------------------------------
  TColStd_Array1OfReal U1(Low,High);
  TColStd_Array1OfReal V1(Low,High);
  TColStd_Array1OfReal U2(Low,High);
  TColStd_Array1OfReal V2(Low,High);
  TColStd_Array1OfReal AC(Low,High);

  Standard_Real s,ds;
  Handle(IntPatch_WLine) TheLine = Handle(IntPatch_WLine)::DownCast(Line(NumLine));
  const IntSurf_PntOn2S& Point=TheLine->Point(Low);
  Standard_Real u1,v1,u2,v2;
  Point.Parameters(u1,v1,u2,v2);
  U1(Low) = u1;
  V1(Low) = v1;
  U2(Low) = u2;
  V2(Low) = v2;
  AC(Low) =0.0;
  
  IntPatch_ThePWalkingInter PW(Surf1,Surf2,0.000001,0.000001,0.001,0.001);
  
  Standard_Integer i;
  for(i=Low+1; i<=High; i++)
    {
      const IntSurf_PntOn2S& Pointi=TheLine->Point(i);
      Pointi.Parameters(u1,v1,u2,v2);
      U1(i) = u1;
      V1(i) = v1;
      U2(i) = u2;
      V2(i) = v2;
    
      Standard_Real du1=u1-U1(i-1);
      Standard_Real dv1=v1-V1(i-1);
    
      AC(i) = AC(i-1) + Sqrt((du1*du1)+(dv1*dv1));
    }

  Handle(IntSurf_LineOn2S) ResultPntOn2SLine = new IntSurf_LineOn2S();
  
  IntSurf_PntOn2S StartPOn2S;  
  TColStd_Array1OfReal StartParams(1,4);
  
  ResultPntOn2SLine->Add(TheLine->Point(Low));
  
  ds = AC(High) / (NbPnts-1);
  Standard_Integer Indice = Low;
  
  Standard_Real dsmin = ds*0.3;
  Standard_Real smax  = AC(High);

  for(i=2,s=ds; (i<NbPnts)&&(s<smax); i++,s+=ds)
    { 
      while(AC(Indice+1) <= s)
	{ 
	  ResultPntOn2SLine->Add(TheLine->Point(Indice));
	  Indice++;
	}
      Standard_Real a = s - AC(Indice);
      Standard_Real b = AC(Indice+1) - s;
      Standard_Real nab = 1.0/(a+b);
      //----------------------------------------------------------
      //-- Verification :  Si Dist au prochain  point < dsmin   --
      //--                 Si Dist au precedent point < dsmin   --
      //--                                                      --
      //----------------------------------------------------------
      if((nab > ds)&&(a>dsmin)&&(b>dsmin))
	{
	  StartParams(1) = (U1(Indice) * b   +  U1(Indice+1) * a) * nab;
	  StartParams(2) = (V1(Indice) * b   +  V1(Indice+1) * a) * nab;
	  StartParams(3) = (U2(Indice) * b   +  U2(Indice+1) * a) * nab;
	  StartParams(4) = (V2(Indice) * b   +  V2(Indice+1) * a) * nab;
      
	  Standard_Boolean HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);
	  if(HasStartPoint)
	    ResultPntOn2SLine->Add(StartPOn2S);
	}
      else
	s+=dsmin; 
    }
  
  ResultPntOn2SLine->Add(TheLine->Point(High));
  
  return(new IntPatch_WLine(ResultPntOn2SLine,Standard_False));
}

//==================================================================================
// function : SectionPointToParameters
// purpose  : 
//==================================================================================
void SectionPointToParameters(const Intf_SectionPoint& Sp,
			      const IntPatch_Polyhedron& Poly1,
			      const IntPatch_Polyhedron& Poly2,
			      Standard_Real& u1,
			      Standard_Real& v1,
			      Standard_Real& u2,
			      Standard_Real& v2)
{
  Intf_PIType       typ;
  Standard_Integer  Adr1,Adr2;
  Standard_Real     Param,u,v;
  gp_Pnt P(Sp.Pnt());
  
  Standard_Integer Pt1,Pt2,Pt3;
  
  Sp.InfoFirst(typ,Adr1,Adr2,Param);
  switch(typ) { 
  case Intf_VERTEX:   //-- Adr1 est le numero du vertex
    {
      Poly1.Parameters(Adr1,u1,v1);
      break;
    }
  case Intf_EDGE:
    {
      Poly1.Parameters(Adr1,u1,v1);    
      Poly1.Parameters(Adr2,u,v);
      u1+= Param * (u-u1);
      v1+= Param * (v-v1);
      break;
    }
  case Intf_FACE:
    {
      Standard_Real ua,va,ub,vb,uc,vc,ca,cb,cc,cabc;
      Poly1.Triangle(Adr1,Pt1,Pt2,Pt3);
      gp_Pnt PA(Poly1.Point(Pt1));
      gp_Pnt PB(Poly1.Point(Pt2));
      gp_Pnt PC(Poly1.Point(Pt3));
      Poly1.Parameters(Pt1,ua,va);
      Poly1.Parameters(Pt2,ub,vb);
      Poly1.Parameters(Pt3,uc,vc);
      gp_Vec Normale(gp_Vec(PA,PB).Crossed(gp_Vec(PA,PC)));
      cc = (gp_Vec(PA,PB).Crossed(gp_Vec(PA,P))).Dot(Normale);
      ca = (gp_Vec(PB,PC).Crossed(gp_Vec(PB,P))).Dot(Normale);
      cb = (gp_Vec(PC,PA).Crossed(gp_Vec(PC,P))).Dot(Normale);
      cabc = ca + cb + cc;
      
      ca/=cabc;     cb/=cabc;       cc/=cabc;
      
      u1 = ca * ua + cb * ub + cc * uc;
      v1 = ca * va + cb * vb + cc * vc;
      break;
    }
  default: 
    {
      //-- cout<<" Default dans SectionPointToParameters "<<endl;
      break;
    }
  }
  
  
  Sp.InfoSecond(typ,Adr1,Adr2,Param);
  switch(typ) { 
  case Intf_VERTEX:   //-- Adr1 est le numero du vertex
    {
      Poly2.Parameters(Adr1,u2,v2);
      break;
    }
  case Intf_EDGE:
    {
      Poly2.Parameters(Adr1,u2,v2);    
      Poly2.Parameters(Adr2,u,v);
      u2+= Param * (u-u2);
      v2+= Param * (v-v2);
      break;
    }
  case Intf_FACE:
    {
      Standard_Real ua,va,ub,vb,uc,vc,ca,cb,cc,cabc;
      Poly2.Triangle(Adr1,Pt1,Pt2,Pt3);
      gp_Pnt PA(Poly2.Point(Pt1));
      gp_Pnt PB(Poly2.Point(Pt2));
      gp_Pnt PC(Poly2.Point(Pt3));
      Poly2.Parameters(Pt1,ua,va);
      Poly2.Parameters(Pt2,ub,vb);
      Poly2.Parameters(Pt3,uc,vc);
      gp_Vec Normale(gp_Vec(PA,PB).Crossed(gp_Vec(PA,PC)));
      cc = (gp_Vec(PA,PB).Crossed(gp_Vec(PA,P))).Dot(Normale);
      ca = (gp_Vec(PB,PC).Crossed(gp_Vec(PB,P))).Dot(Normale);
      cb = (gp_Vec(PC,PA).Crossed(gp_Vec(PC,P))).Dot(Normale);
      cabc = ca + cb + cc;
      
      ca/=cabc;     cb/=cabc;       cc/=cabc;
      
      u2 = ca * ua + cb * ub + cc * uc;
      v2 = ca * va + cb * vb + cc * vc;
      break;
    }
  default: 
    {
      //-- cout<<" Default dans SectionPointToParameters "<<endl;
      break;
    }
  }
} 

#define NBI 200

static gp_Pnt P1[NBI][NBI];
static gp_Pnt P2[NBI][NBI];
static Standard_Integer IP1[NBI][NBI];
static Standard_Integer IP2[NBI][NBI];

static char             P1DS2[NBI][NBI];
static char             P2DS1[NBI][NBI];

#define _DECAL    7
#define _DECAL2   14
#define _BASE     128
#define _BASEM1   127

long unsigned nbappelstri=0;
long unsigned nbappelslin=0;


//==================================================================================
// function : RemplitLin
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::RemplitLin(const Standard_Integer x1,
					     const Standard_Integer y1,
					     const Standard_Integer z1,
					     const Standard_Integer x2,
					     const Standard_Integer y2,
					     const Standard_Integer z2,
					     IntPatch_PrmPrmIntersection_T3Bits& Map) const 
{
  int xg,yg,zg;
  xg=x1-x2; if(xg<0) xg=-xg; 
  yg=y1-y2; if(yg<0) yg=-yg; 
  zg=z1-z2; if(zg<0) zg=-zg; 
  if(DansGrille(x1) && DansGrille(y1) && DansGrille(z1)) { 
    Standard_Integer t = GrilleInteger(x1,y1,z1); 
    Map.Add(t);
  }
  if(xg<=1 && yg<=1 && zg<=1) return;
  xg = (x1+x2)>>1;
  yg = (y1+y2)>>1;
  zg = (z1+z2)>>1;
  RemplitLin(x1,y1,z1,xg,yg,zg,Map);
  RemplitLin(x2,y2,z2,xg,yg,zg,Map);
}

//==================================================================================
// function : RemplitTri
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::RemplitTri(const Standard_Integer x1,
					     const Standard_Integer y1,
					     const Standard_Integer z1,
					     const Standard_Integer x2,
					     const Standard_Integer y2,
					     const Standard_Integer z2,
					     const Standard_Integer x3,
					     const Standard_Integer y3,
					     const Standard_Integer z3,
					     IntPatch_PrmPrmIntersection_T3Bits& Map) const 
{ 
  if(x1==x2 && x1==x3 && y1==y2 && y1==y3 && z1==z2 && z1==z3) {
    if(DansGrille(x1) && DansGrille(y1) && DansGrille(z1)) { 
      Standard_Integer t = GrilleInteger(x1,y1,z1); 
      Map.Add(t);
    }
    return;
  }
  else { 
    Standard_Integer xg=(x1+x2+x3)/3;
    Standard_Integer yg=(y1+y2+y3)/3;
    Standard_Integer zg=(z1+z2+z3)/3;
    if(xg==x1 && yg==y1 && zg==z1) { 
      RemplitLin(x1,y1,z1,x2,y2,z2,Map);
      RemplitLin(x1,y1,z1,x3,y3,z3,Map);
      return;
    }
    if(xg==x2 && yg==y2 && zg==z2) { 
      RemplitLin(x2,y2,z2,x1,y1,z1,Map);
      RemplitLin(x2,y2,z2,x3,y3,z3,Map);
      return;
    }
    if(xg==x3 && yg==y3 && zg==z3) { 
      RemplitLin(x3,y3,z3,x2,y2,z2,Map);
      RemplitLin(x3,y3,z3,x1,y1,z1,Map);
      return;
    }
    if(DansGrille(xg) && DansGrille(yg) && DansGrille(zg)) {
      Standard_Integer t = GrilleInteger(xg,yg,zg); 
      Map.Add(t);
    }
    if(xg!=x3 || yg!=y3 || zg!=z3) RemplitTri(x1,y1,z1, x2,y2,z2, xg,yg,zg, Map);
    if(xg!=x1 || yg!=y1 || zg!=z1) RemplitTri(xg,yg,zg, x2,y2,z2, x3,y3,z3, Map);
    if(xg!=x2 || yg!=y2 || zg!=z2) RemplitTri(x1,y1,z1, xg,yg,zg, x3,y3,z3, Map);
  }
}

//==================================================================================
// function : Remplit
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::Remplit(const Standard_Integer a,
					  const Standard_Integer b,
					  const Standard_Integer c,
					  IntPatch_PrmPrmIntersection_T3Bits& Map) const 
{ 
  int iax,iay,iaz,ibx,iby,ibz,icx,icy,icz;
  if(a!=-1) Map.Add(a);
  if(b!=-1) Map.Add(b);
  if(c!=-1) Map.Add(c);
  
  if(a!=-1 && b!=-1 && c!=-1 ) { 
    IntegerGrille(a,iax,iay,iaz);
    IntegerGrille(b,ibx,iby,ibz);
    IntegerGrille(c,icx,icy,icz);
    RemplitTri(iax,iay,iaz,ibx,iby,ibz,icx,icy,icz,Map);
  }
}

//==================================================================================
// function : PointDepart
// purpose  : 
//==================================================================================
void IntPatch_PrmPrmIntersection::PointDepart(Handle(IntSurf_LineOn2S)& LineOn2S,
					      const Handle(Adaptor3d_HSurface)& S1,
					      const Standard_Integer SU_1,
					      const Standard_Integer SV_1,
					      const Handle(Adaptor3d_HSurface)& S2,
					      const Standard_Integer SU_2,
					      const Standard_Integer SV_2) const 
{ 
  
  Standard_Integer SU1 =SU_1 ;
  Standard_Integer SV1 =SV_1 ;
  Standard_Integer SU2 =SU_2 ;
  Standard_Integer SV2 =SV_2 ;

  SU1*=15;
  SU2*=15;
  SV1*=15;
  SV2*=15;

  if(NBI<SU1) SU1 = NBI;
  if(NBI<SV1) SV1 = NBI;
  if(NBI<SU2) SU2 = NBI;
  if(NBI<SV2) SV2 = NBI;

  //-- cout<<"\n SU1 "<<SU1<<" SV1 "<<SV1<<"  SU2 "<<SU2<<" SV2 "<<SV2<<endl;
  Bnd_Box Box1;
  Bnd_Box Box2;

  Standard_Real U0 = S1->FirstUParameter();
  Standard_Real U1 = S1->LastUParameter();
  Standard_Real V0 = S1->FirstVParameter();
  Standard_Real V1 = S1->LastVParameter();

  Standard_Real resu0,resu1,resv0,resv1;
  resu0=U0;resu1=U1;resv1=V1;resv0=V0;

  Standard_Real  du1,du2,dv1,dv2;
  du1 = (U1-U0)/(SU1-1);
  dv1 = (V1-V0)/(SV1-1);
  Standard_Real U,V;

  Standard_Integer i,j;

  Standard_Real dmaxOn1 = 0.0;
  Standard_Real dmaxOn2 = 0.0;


  for(U=U0,i=0; i<SU1; i++,U+=du1) { 
    for(V=V0,j=0; j<SV1; V+=dv1,j++) { 
      P1[i][j] = S1->Value(U,V);
      Box1.Add(P1[i][j]);
      if(i>0 && j>0) { 
	Standard_Real x0,y0,z0;
	Standard_Real x1,y1,z1;
	P1[i][j].Coord(x0,y0,z0);
	P1[i-1][j-1].Coord(x1,y1,z1);
       
	Standard_Real d =  Abs(x1-x0)+Abs(y1-y0)+Abs(z1-z0);
	if(d>dmaxOn1) dmaxOn1 = d;
      }
    }
  }
  //-- cout<<" P(.,.)  du 1 fini "<<endl;
  U0 = S2->FirstUParameter();
  U1 = S2->LastUParameter();
  V0 = S2->FirstVParameter();
  V1 = S2->LastVParameter();
  
  du2 = (U1-U0)/(SU2-1);   dv2 = (V1-V0)/(SV2-1);

  for(U=U0,i=0; i<SU2; i++,U+=du2) { 
    for(V=V0,j=0; j<SV2; V+=dv2,j++) { 
      P2[i][j] = S2->Value(U,V);
      Box2.Add(P2[i][j]);
      if(i>0 && j>0) { 
	Standard_Real x0,y0,z0;
	Standard_Real x1,y1,z1;
	P2[i][j].Coord(x0,y0,z0);
	P2[i-1][j-1].Coord(x1,y1,z1);
	
	Standard_Real d =  Abs(x1-x0)+Abs(y1-y0)+Abs(z1-z0);
	if(d>dmaxOn2) dmaxOn2 = d;
      }
    }
  }
  //-- cout<<" P(.,.)  du 1 fini "<<endl;
  Box1.Enlarge(1e-8);
  Box2.Enlarge(1e-8);

  if(Box1.IsOut(Box2)) { 
    //-- cout<<" Rejection entre boites "<<endl;
    return;
  }
  Standard_Real x10,y10,z10,x11,y11,z11;
  Standard_Real x20,y20,z20,x21,y21,z21;
  Standard_Real x0,y0,z0,x1,y1,z1;

  Box1.Get(x10,y10,z10,x11,y11,z11);
  Box2.Get(x20,y20,z20,x21,y21,z21);
  x0 = (x10>x20)? x10 : x20;
  y0 = (y10>y20)? y10 : y20;
  z0 = (z10>z20)? z10 : z20;
  
  x1 = (x11<x21)? x11 : x21;
  y1 = (y11<y21)? y11 : y21;
  z1 = (z11<z21)? z11 : z21;
  if(dmaxOn2 > dmaxOn1) dmaxOn1 = dmaxOn2;
  dmaxOn1+=dmaxOn1;
  x0-=dmaxOn1;
  y0-=dmaxOn1;
  z0-=dmaxOn1;
  x1+=dmaxOn1;
  y1+=dmaxOn1;
  z1+=dmaxOn1;
  

  x10-=dmaxOn1;   y10-=dmaxOn1;   z10-=dmaxOn1; 
  x11+=dmaxOn1;   y11+=dmaxOn1;   z11+=dmaxOn1; 

  x20-=dmaxOn1;   y20-=dmaxOn1;   z20-=dmaxOn1; 
  x21+=dmaxOn1;   y21+=dmaxOn1;   z21+=dmaxOn1; 


  Standard_Real dx = (x1-x0)/NbPointsGrille();
  Standard_Real dy = (y1-y0)/NbPointsGrille();
  Standard_Real dz = (z1-z0)/NbPointsGrille();

  //Standard_Integer INCRX,INCRY,INCRZ;
  
  Standard_Real dmax = dx;
  if(dy>dmax) dmax = dy;
  if(dz>dmax) dmax = dz;
  
  
  if(dx<dmax*0.01) dx = dmax*0.01;
  if(dy<dmax*0.01) dy = dmax*0.01;
  if(dz<dmax*0.01) dz = dmax*0.01;
  Standard_Real dx2 = dx*0.5;
  Standard_Real dy2 = dy*0.5;
  Standard_Real dz2 = dz*0.5 ;
  


  IntPatch_PrmPrmIntersection_T3Bits M1(_BASE);
  IntPatch_PrmPrmIntersection_T3Bits M2(_BASE);

  for(i=0;i<SU1;i++) { 
    for(j=0;j<SV1;j++) { 
      IP1[i][j]=-1;
      const gp_Pnt& P=P1[i][j];
      P1DS2[i][j] = (char) CodeReject(x20,y20,z20,x21,y21,z21,P.X(),P.Y(),P.Z());
      int ix = (int)((P.X()-x0  + dx2 )/dx);
      if(DansGrille(ix)) { 
	int iy = (int)((P.Y()-y0 + dy2)/dy);
	if(DansGrille(iy)) {
	  int iz = (int)((P.Z()-z0 + dz2)/dz);
	  if(DansGrille(iz)) {
	    IP1[i][j] = GrilleInteger(ix,iy,iz);
	  }
	}
      }
    }
  }
  //-- cout<<" Grille  du 1 fini "<<endl;
  for(i=0;i<SU2;i++) { 
    for(j=0;j<SV2;j++) { 
      IP2[i][j]=-1;
      const gp_Pnt& P=P2[i][j];
      P2DS1[i][j] = (char) CodeReject(x10,y10,z10,x11,y11,z11,P.X(),P.Y(),P.Z());
      int ix = (int)((P.X()-x0 + dx2)/dx);
      if(DansGrille(ix)) { 
	int iy = (int)((P.Y()-y0 + dy2)/dy);
	if(DansGrille(iy)) {
	  int iz = (int)((P.Z()-z0 + dz2)/dz);
	  if(DansGrille(iz)) {
	    IP2[i][j] =  GrilleInteger(ix,iy,iz);
	  }
	}
      }
    }
  }
  //-- cout<<" Grille  du 2 fini "<<endl;
  for(i=0;i<SU1-1;i+=1) {
    for(j=0;j<SV1-1;j+=1) { 
      if(!((P1DS2[i][j] & P1DS2[i+1][j]) || (P1DS2[i][j] & P1DS2[i+1][j+1])) )
	Remplit(IP1[i][j],IP1[i+1][j],IP1[i+1][j+1],M1);
      if(!((P1DS2[i][j] & P1DS2[i][j+1]) || (P1DS2[i][j] & P1DS2[i+1][j+1])))
	Remplit(IP1[i][j],IP1[i][j+1],IP1[i+1][j+1],M1);	
    }
  }	
  //-- cout<<" Remplissage du 1 fini "<<endl;
  for(i=0;i<SU2-1;i+=1) {
    for(j=0;j<SV2-1;j+=1) { 
      if(!((P2DS1[i][j] & P2DS1[i+1][j]) || (P2DS1[i][j] & P2DS1[i+1][j+1])))
	Remplit(IP2[i][j],IP2[i+1][j],IP2[i+1][j+1],M2);
      if(!((P2DS1[i][j] & P2DS1[i][j+1]) || (P2DS1[i][j] & P2DS1[i+1][j+1])))
	Remplit(IP2[i][j],IP2[i][j+1],IP2[i+1][j+1],M2);	
    }
  }	

  //-- cout<<" Remplissage du 2 fini "<<endl;


  M1.ResetAnd();
  M2.ResetAnd();
#ifdef DEB
  long unsigned ind=0;
#endif
//POP
  int newind=0;
//  long unsigned newind=0;
  long unsigned Compt=0;
  //long unsigned ComptTrouve=0;
  int ok=0;
  int indicepointtraite = 0;

  Standard_Integer k;
  Standard_Integer nu,nv;
  
  do { 
    indicepointtraite--;
    ok = M1.And(M2,newind);
    if(ok) { 
      IntegerGrille(newind,i,j,k);


      int nb=0;
      int LIM=3;
      if(   DansGrille(i-1) && DansGrille(j-1) && DansGrille(k-1) 
	 && DansGrille(i+1) && DansGrille(j+1) && DansGrille(k+1)) { 
	int si,sj,sk;
	for(si=-1; si<= 1 && nb<LIM; si++) { 
	  for(sj=-1; sj<= 1 && nb<LIM; sj++) { 
	    for(sk=-1; sk<= 1 && nb<LIM; sk++) { 
	      long unsigned lu=GrilleInteger(i+si,j+sj,k+sk);
	      if(M1.Val(lu) && M2.Val(lu)) { 
		nb++;
	      }
	    }
	  }
	}
	if(nb>=LIM) { 
	  for(si=-1; si<= 1; si++) { 
	    for(sj=-1; sj<= 1; sj++) { 
	      for(sk=-1; sk<= 1; sk++) { 
		if(si || sj || sk) { 
		  long unsigned lu=GrilleInteger(i+si,j+sj,k+sk);
		  M1.Raz(lu);
		}
	      }
	    }
	  }
	}
      }


      gp_Pnt P(dx*i + x0, dy*j + y0, dz*k+z0);
      
      //Standard_Integer ok12 = 1;
      Standard_Integer nu1=-1,nu2=-1;
#ifndef DEB
      Standard_Integer nv1=0, nv2=0;
#else
      Standard_Integer nv1, nv2;
#endif
      int nbsur1 = 0;
      for(nu=0;nu1<0 && nu<SU1;nu++) { 
	for(nv=0;nu1<0 && nv<SV1;nv++) { 
#ifndef DEB
	  if( IP1[nu][nv]==(Standard_Integer) newind )  { 
#else
	  if( IP1[nu][nv]==newind )  { 
#endif
	    nbsur1++;
	    IP1[nu][nv]=indicepointtraite;
	    nu1=nu; nv1=nv;
	  }
	}
      }
      if(nu1>=0) { 
	int nbsur2 = 0;
	for(nu=0;nu2<0 && nu<SU2;nu++) { 
	  for(nv=0;nu2<0 && nv<SV2;nv++) { 
#ifndef DEB
	    if( IP2[nu][nv]==(Standard_Integer) newind )  { 
#else
	    if( IP2[nu][nv]==newind )  {     
#endif
	      nbsur2++;
	      IP2[nu][nv]=indicepointtraite;
	      nu2=nu; nv2=nv;
	    }
	  }
	}
      }
      if(nu1>=0 && nu2>=0) { 
	IntSurf_PntOn2S POn2S;
	POn2S.SetValue(P, 
		       S1->FirstUParameter()+nu1*du1,
		       S1->FirstVParameter()+nv1*dv1,
		       S2->FirstUParameter()+nu2*du2,
		       S2->FirstVParameter()+nv2*dv2);
	LineOn2S->Add(POn2S);
	Compt++;
      }
      else { 
	//-- aucun point du triangle n a ete trouve assez proche
	//-- on recherche les 3 points les plus proches de P 
	//-- dans chacun des tableaux 
	Standard_Real Dist3[3],u3[3],v3[3];
	Dist3[0]=Dist3[1]=Dist3[2]=RealLast();
	u3[0]=u3[1]=u3[2]=0;
	v3[0]=v3[1]=v3[2]=0;
	for(U=resu0,i=0; i<SU1; i++,U+=du1) { 
	  for(V=resv0,j=0; j<SV1; V+=dv1,j++) {       
	    //-- On place les 3 meilleures valeurs dans Dist1,Dist2,Dist3
	    Standard_Real t = P1[i][j].SquareDistance(P);
	    //-- On remplace la plus grande valeur ds Dist[.] par la val courante
	    if(Dist3[0]<Dist3[1]) { 
	      Standard_Real z;
	      z=Dist3[0]; Dist3[0]=Dist3[1]; Dist3[1]=z;
	      z=u3[0]; u3[0]=u3[1]; u3[1]=z;
	      z=v3[0]; v3[0]=v3[1]; v3[1]=z;
	    }
	    if(Dist3[1]<Dist3[2]) { 
	      Standard_Real z;
	      z=Dist3[1]; Dist3[1]=Dist3[2]; Dist3[2]=z;
	      z=u3[1]; u3[1]=u3[2]; u3[2]=z;
	      z=v3[1]; v3[1]=v3[2]; v3[2]=z;
	    }
	    if(Dist3[0]<Dist3[1]) { 
	      Standard_Real z;
	      z=Dist3[0]; Dist3[0]=Dist3[1]; Dist3[1]=z;
	      z=u3[0]; u3[0]=u3[1]; u3[1]=z;
	      z=v3[0]; v3[0]=v3[1]; v3[1]=z;
	    }
	    //-- la plus grande valeur est dans Dist3[0]
	    if(t<Dist3[0]) { 
	      Dist3[0]=t; u3[0]=U; v3[0]=V;
	    }
	  }
	}
	//
	Standard_Real U1_3 = (u3[0]+u3[1]+u3[2])/3.0;
	Standard_Real V1_3 = (v3[0]+v3[1]+v3[2])/3.0;
	
	Dist3[0]=Dist3[1]=Dist3[2]=RealLast();
	for(U=U0,i=0; i<SU2; i++,U+=du2) { 
	  for(V=V0,j=0; j<SV2; V+=dv2,j++) {       
	    //-- On place les 3 meilleures valeurs dans Dist1,Dist2,Dist3
	    Standard_Real t = P2[i][j].SquareDistance(P);
	    //-- On remplace la plus grande valeur ds Dist3[.] par la val courante
	    if(Dist3[0]<Dist3[1]) { 
	      Standard_Real z;
	      z=Dist3[0]; Dist3[0]=Dist3[1]; Dist3[1]=z;
	      z=u3[0]; u3[0]=u3[1]; u3[1]=z;
	      z=v3[0]; v3[0]=v3[1]; v3[1]=z;
	    }
	    if(Dist3[1]<Dist3[2]) { 
	      Standard_Real z;
	      z=Dist3[1]; Dist3[1]=Dist3[2]; Dist3[2]=z;
	      z=u3[1]; u3[1]=u3[2]; u3[2]=z;
	      z=v3[1]; v3[1]=v3[2]; v3[2]=z;
	    }
	    if(Dist3[0]<Dist3[1]) { 
	      Standard_Real z;
	      z=Dist3[0]; Dist3[0]=Dist3[1]; Dist3[1]=z;
	      z=u3[0]; u3[0]=u3[1]; u3[1]=z;
	      z=v3[0]; v3[0]=v3[1]; v3[1]=z;
	    }
	    //-- la plus grande valeur est dans Dist3[0]
	    if(t<Dist3[0]) { 
	      Dist3[0]=t; u3[0]=U; v3[0]=V;
	    }
	  }
	}
	//
	Standard_Real U2_3 = (u3[0]+u3[1]+u3[2])/3.0;
	Standard_Real V2_3 = (v3[0]+v3[1]+v3[2])/3.0;
	
	IntSurf_PntOn2S POn2S;
	POn2S.SetValue(P,U1_3,V1_3,U2_3,V2_3);
	LineOn2S->Add(POn2S);
	Compt++;	
      }
    }
  }
  while(ok);
}

//==================================================================================
// function : Perform
// purpose  : base SS Int. function
//==================================================================================
void IntPatch_PrmPrmIntersection::Perform (const Handle(Adaptor3d_HSurface)&    Surf1,
					   const Handle(Adaptor3d_TopolTool)& D1,
					   const Handle(Adaptor3d_HSurface)&    Surf2,
					   const Handle(Adaptor3d_TopolTool)& D2,
					   const Standard_Real   TolTangency,
					   const Standard_Real   Epsilon,
					   const Standard_Real   Deflection,
					   const Standard_Real   Increment,
                                           const Standard_Boolean ClearFlag) 
{

  D1->SamplePnts(Deflection, 10, 10);
  D2->SamplePnts(Deflection, 10, 10);

//=================================================================================
  Standard_Integer NbU1 = D1->NbSamplesU();
  Standard_Integer NbV1 = D1->NbSamplesV();
  Standard_Integer NbU2 = D2->NbSamplesU();
  Standard_Integer NbV2 = D2->NbSamplesV();
  TColStd_Array1OfReal anUpars1(1, NbU1), aVpars1(1, NbV1);
  TColStd_Array1OfReal anUpars2(1, NbU2), aVpars2(1, NbV2);
  D1->UParameters(anUpars1); D1->VParameters(aVpars1);
  D2->UParameters(anUpars2); D2->VParameters(aVpars2);
//=======================================================================
  Standard_Integer Limit = 2500;

  if((NbU1*NbV1<=Limit && NbV2*NbU2<=Limit)) {  
    empt = Standard_True;
    // modified by NIZHNY-AMV  Tue Oct 18 12:37:02 2005.BEGIN
    if (ClearFlag)
      SLin.Clear();
    // modified by NIZHNY-AMV  Tue Oct 18 12:37:02 2005.END
    //
    //IntPolyh_Intersection Interference(Surf1,NbU1,NbV1,Surf2,NbU2,NbV2);
    //     IntPolyh_Intersection Interference(Surf1, anUpars1, aVpars1, 
    // 				       Surf2, anUpars2, aVpars2);

    IntPolyh_Intersection* pInterference = NULL;

    if ( D1->IsUniformSampling() || D2->IsUniformSampling() ) {
      pInterference = new IntPolyh_Intersection(Surf1,NbU1,NbV1,Surf2,NbU2,NbV2);
    }
    else {
      pInterference = new IntPolyh_Intersection(Surf1, anUpars1, aVpars1, 
						Surf2, anUpars2, aVpars2 );
    }
    if ( !pInterference ) {
      done = Standard_False;
      return;
    }
    //
    IntPolyh_Intersection& Interference = *pInterference;
    //
    done = Interference.IsDone();
    if( !done ) {
      if ( pInterference ) {
	delete pInterference;
	pInterference = NULL;
      }
      return;
    }
    
    Standard_Real Deflection2 = Deflection*Deflection;
    Standard_Integer nbLigSec = Interference.NbSectionLines();
    Standard_Integer nbTanZon = Interference.NbTangentZones();
    Standard_Real SeuildPointLigne = 15.0 * Increment * Increment;

    Standard_Integer NbPntOn2SOnLine = 0, NbLigCalculee = 0, ver;
    Standard_Real U1,U2,V1,V2, pu1,pu2,pv1,pv2, incidence, dminiPointLigne;
    Standard_Boolean HasStartPoint,RejetLigne;
    IntSurf_PntOn2S StartPOn2S;
    gp_Pnt Point3dDebut,Point3dFin;

    TColStd_Array1OfReal StartParams(1,4);
    IntPatch_ThePWalkingInter PW(Surf1,Surf2,TolTangency,Epsilon,Deflection,Increment);

    if(nbLigSec>=1) {  
      Standard_Integer *TabL = new Standard_Integer [nbLigSec+1];
      Standard_Integer ls;
      for(ls=1;ls<=nbLigSec;ls++)
	TabL[ls]=ls;

      Standard_Boolean triok;
      do { 
	triok=Standard_True;
	Standard_Integer b;
	for( b = 2; b <= nbLigSec; b++ ) { 
	  Standard_Integer nb_B = Interference.NbPointsInLine(TabL[b]);
	  Standard_Integer nb_A = Interference.NbPointsInLine(TabL[b-1]);
	  if( nb_B > nb_A ) { 
	    Standard_Integer tyu=TabL[b]; 
	    TabL[b]=TabL[b-1];
	    TabL[b-1]=tyu;
	    triok=Standard_False;
	  }
	}
      } while(triok==Standard_False);
      //
      for( ls = 1; ls <= nbLigSec; ls++) {
	Standard_Integer nbp, ilig, *TabPtDep;
	//
	nbp = Interference.NbPointsInLine(TabL[ls]);
	//modified by NIZNHY-PKV Fri Sep  5 15:19:28 2008f
	if (!nbp) {
	  continue;
	}
	//modified by NIZNHY-PKV Fri Sep  5 15:19:31 2008t
	TabPtDep = new Standard_Integer [nbp+1];
	//
	for( ilig = 1; ilig <= nbp; ilig++ )
	  TabPtDep[ilig]=0;

	Standard_Real UminLig1,VminLig1,UmaxLig1,VmaxLig1;
	Standard_Real UminLig2,VminLig2,UmaxLig2,VmaxLig2;
	Standard_Real _x,_y,_z;

	Interference.GetLinePoint(TabL[ls],1,_x,_y,_z,UminLig1,VminLig1,UminLig2,VminLig2,incidence);
	
	UmaxLig1=UminLig1;
	VmaxLig1=VminLig1;
	UmaxLig2=UminLig2;
	VmaxLig2=VminLig2;

	for( ilig = 2; ilig <= nbp; ilig++ ) { 
	  Interference.GetLinePoint(TabL[ls],ilig,_x,_y,_z,U1,V1,U2,V2,incidence);
	  
	  if(U1>UmaxLig1) UmaxLig1=U1;
	  if(V1>VmaxLig1) VmaxLig1=V1;
	  if(U2>UmaxLig2) UmaxLig2=U2;
	  if(V2>VmaxLig2) VmaxLig2=V2;
	  
	  if(U1<UminLig1) UminLig1=U1;
	  if(V1<VminLig1) VminLig1=V1;
	  if(U2<UminLig2) UminLig2=U2;
	  if(V2<VminLig2) VminLig2=V2;
	}
	
	Standard_Integer nbps2 = (nbp>3)? (nbp/2) :  1;
	Standard_Integer NombreDePointsDeDepartDuCheminement = 0;
	Standard_Boolean lignetrouvee=Standard_False;
	const Standard_Integer NbDePointsDeDepartDuChmLimit = 5;
	//
	do { 
	  NombreDePointsDeDepartDuCheminement++;
	  switch (NombreDePointsDeDepartDuCheminement) {
	  case 1:
	    nbps2 = (nbp > 1) ? nbp/2 : 1;
	    if(nbp<3) 
	      NombreDePointsDeDepartDuCheminement = NbDePointsDeDepartDuChmLimit;
	    break;
	  case 2:
	    nbps2 = 1;
	    break;
	  case 3:
	    nbps2 = nbp-1;
	    break;
	  case 4:
	    nbps2 = 3 * nbp / 4;
	    break;
	  case 5:
	    nbps2 = nbp / 4;
	    break;
	  default:
	    nbps2 = NombreDePointsDeDepartDuCheminement-3;
	    NombreDePointsDeDepartDuCheminement++;
	  }
	  //
	  if(TabPtDep[nbps2] == 0) {
	    TabPtDep[nbps2] = 1;
	    Interference.GetLinePoint(TabL[ls],nbps2,_x,_y,_z,U1,V1,U2,V2,incidence);
		      
	    StartParams(1) = U1;
	    StartParams(2) = V1;
	    StartParams(3) = U2;
	    StartParams(4) = V2;

	    HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);
	    dminiPointLigne = SeuildPointLigne + SeuildPointLigne;
	    if(HasStartPoint) {
	      StartPOn2S.Parameters(pu1,pv1,pu2,pv2);
	      NbLigCalculee = SLin.Length();
	      Standard_Integer l;
	      for( l = 1; (l <= NbLigCalculee) && (dminiPointLigne >= SeuildPointLigne); l++) { 
		Handle(IntPatch_WLine)& testwline = *((Handle(IntPatch_WLine)*)&SLin.ChangeValue(l));
		if( (testwline->IsOutSurf1Box(gp_Pnt2d(pu1,pv1))==Standard_False) &&
		    (testwline->IsOutSurf2Box(gp_Pnt2d(pu2,pv2))==Standard_False) &&
		    (testwline->IsOutBox(StartPOn2S.Value())==Standard_False) )	{
		  NbPntOn2SOnLine = testwline->NbPnts();
		  Standard_Integer ll;
		  for( ll = 1; (ll < NbPntOn2SOnLine) && (dminiPointLigne >= SeuildPointLigne); ll++) { 
		    const gp_Pnt &Pa = testwline->Point(ll).Value();
		    const gp_Pnt &Pb = testwline->Point(ll+1).Value();
		    const gp_Pnt &PStart = StartPOn2S.Value();
		    gp_Vec AM(Pa,PStart);
		    gp_Vec MB(PStart,Pb);
		    Standard_Real AMMB = AM.Dot(MB);		    
		    if(AMMB > 0.0) {
		      gp_Dir ABN(Pb.X()-Pa.X(),Pb.Y()-Pa.Y(),Pb.Z()-Pa.Z());
		      Standard_Real lan =  ABN.X()*AM.X()+ABN.Y()*AM.Y()+ABN.Z()*AM.Z();
		      gp_Vec AH(lan*ABN.X(),lan*ABN.Y(),lan*ABN.Z());
		      gp_Vec HM(AM.X()-AH.X(),AM.Y()-AH.Y(),AM.Z()-AH.Z());
		      Standard_Real d = 0.0;
		      if(HM.X() < Deflection) { 
			d+=HM.X()*HM.X();
			if(HM.Y() < Deflection) { 
			  d+=HM.Y()*HM.Y(); 
			  if(HM.Z() < Deflection)
			    d+=HM.Z()*HM.Z(); 
			  else
			    d=Deflection2;
			}
			else
			  d=Deflection2;
		      } 
		      else
			d=Deflection2; 

		      if(d<Deflection2)
			dminiPointLigne = 0.0;
		    }
		    else {
		      Standard_Real dab = Pa.SquareDistance(Pb);
		      Standard_Real dap = Pa.SquareDistance(PStart);
		      if(dap < dab)
			dminiPointLigne=0;
		      else { 
			Standard_Real dbp = Pb.SquareDistance(PStart);
			if(dbp < dab)
			  dminiPointLigne=0;
		      }
		    }
		  }// for( ll ...
		}// if ...
	      }// for( l ...

	      if(dminiPointLigne > SeuildPointLigne) {
		PW.Perform(StartParams,UminLig1,VminLig1,UminLig2,VminLig2,UmaxLig1,VmaxLig1,UmaxLig2,VmaxLig2);
		//xf
		Standard_Boolean bPWIsDone;
		Standard_Integer iPWNbPoints, aNbPointsVer;
		Standard_Real aD11, aD12, aD21, aD22, aDx;
		//
		bPWIsDone=PW.IsDone();
		//xt
		if(bPWIsDone) {
		  iPWNbPoints=PW.NbPoints();
		  //
		  if( iPWNbPoints > 2 ) {
		    RejetLigne = Standard_False;
		    Point3dDebut = PW.Value(1).Value();
		    Point3dFin   = PW.Value(PW.NbPoints()).Value();
		    for( ver = 1 ; (!RejetLigne) && (ver<= NbLigCalculee) ; ver++) { 
		      const Handle(IntPatch_WLine)& verwline = *((Handle(IntPatch_WLine)*)&SLin.Value(ver));
		      //
		      aNbPointsVer=verwline->NbPnts();
		      if (aNbPointsVer<3) {
			continue;
		      }
		      //
		      const IntSurf_PntOn2S& verPointDebut = verwline->Point(1);
		      const IntSurf_PntOn2S& verPointFin = verwline->Point(verwline->NbPnts());
		      //xf
		      const gp_Pnt& aP21=verPointDebut.Value();
		      const gp_Pnt& aP22=verPointFin.Value();
		      //
		      aD11=Point3dDebut.Distance(aP21);
		      aD12=Point3dDebut.Distance(aP22);
		      aD21=Point3dFin.Distance(aP21);
		      aD22=Point3dFin.Distance(aP22);
		      //
		      if((aD11<=TolTangency && aD22<=TolTangency) ||
			 (aD12<=TolTangency && aD21<=TolTangency)) {
			Standard_Integer m, mx;
			//
			mx=aNbPointsVer/2;
			if (aNbPointsVer%2) {
			  ++mx; 
			}
			//
			const gp_Pnt& aPx=verwline->Point(mx).Value();
			for(m=1; m<iPWNbPoints; ++m){
			  const gp_Pnt& aP1=PW.Value(m).Value();
			  const gp_Pnt& aP2=PW.Value(m+1).Value();
			  gp_Vec aVec12(aP1, aP2);
			  if (aVec12.SquareMagnitude()<1.e-20){
			    continue;
			  }
			  //
			  gp_Dir aDir12(aVec12);
			  gp_Lin aLin12(aP1, aDir12);
			  aDx=aLin12.Distance(aPx);
			  if (aDx<=TolTangency) {
			    RejetLigne = Standard_True;
			    break;
			  }
			}
		      }
		    }
		    //
		    if(!RejetLigne) {
		      IntSurf_TypeTrans trans1,trans2;
		      Standard_Real locu,locv;
		      gp_Vec norm1,norm2,d1u,d1v;
		      gp_Pnt ptbid;
		      Standard_Integer indextg;
		      gp_Vec tgline(PW.TangentAtLine(indextg));
		      PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
		      Surf1->D1(locu,locv,ptbid,d1u,d1v);
		      norm1 = d1u.Crossed(d1v);
		      PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
		      Surf2->D1(locu,locv,ptbid,d1u,d1v);
		      norm2 = d1u.Crossed(d1v);
		      if( tgline.DotCross(norm2,norm1) >= 0. ) {
			trans1 = IntSurf_Out;
			trans2 = IntSurf_In;
		      }
		      else {
			trans1 = IntSurf_In;
			trans2 = IntSurf_Out;
		      }

		      Standard_Real TolTang = TolTangency;
		      Handle(IntPatch_WLine) wline = new IntPatch_WLine(PW.Line(),Standard_False,trans1,trans2);
		      IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf2,Standard_True,TolTang);
		      IntPatch_RstInt::PutVertexOnLine(wline,Surf2,D2,Surf1,Standard_False,TolTang);

		      if(wline->NbVertex() == 0) {
			IntPatch_Point vtx;
			IntSurf_PntOn2S POn2S = PW.Line()->Value(1);
			POn2S.Parameters(pu1,pv1,pu2,pv2);
			vtx.SetValue(Point3dDebut,TolTang,Standard_False);
			vtx.SetParameters(pu1,pv1,pu2,pv2);
			vtx.SetParameter(1);
			wline->AddVertex(vtx);
			
			POn2S = PW.Line()->Value(wline->NbPnts());
			POn2S.Parameters(pu1,pv1,pu2,pv2);
			vtx.SetValue(Point3dFin,TolTang,Standard_False);
			vtx.SetParameters(pu1,pv1,pu2,pv2);
			vtx.SetParameter(wline->NbPnts());
			wline->AddVertex(vtx);
		      }
		      
		      lignetrouvee = Standard_True;

		      Standard_Integer slinlen = SLin.Length();
		      if( slinlen > 0 ) {
			Standard_Integer cnbV = wline->NbVertex();
			Standard_Integer ciV;
			for( ciV = 1; ciV <= cnbV; ciV++ ) {
			  Standard_Real pntDMin = 1.e+100;
			  Standard_Integer VDMin = 0;
			  Standard_Integer WLDMin = 0;
			  gp_Pnt cPV = wline->Vertex(ciV).Value();
			  Standard_Integer iL;
			  for( iL = 1; iL <= slinlen; iL++) {
			    const Handle(IntPatch_Line)& aSLine = SLin.Value(iL);
			    IntPatch_IType aType = aSLine->ArcType();
			    if( aType != IntPatch_Walking)
			      continue;
			    const Handle(IntPatch_WLine)&  aWLine = (*((Handle(IntPatch_WLine)*)&aSLine));
			    Standard_Integer tnbV = aWLine->NbVertex();
			    Standard_Integer tiV;
			    for( tiV = 1; tiV <= tnbV; tiV++ ) {
			      gp_Pnt tPV = aWLine->Vertex(tiV).Value();
			      Standard_Real tDistance = cPV.Distance(tPV);
			      Standard_Real uRs1 = Surf1->Surface().UResolution(tDistance);
			      Standard_Real vRs1 = Surf1->Surface().VResolution(tDistance);
			      Standard_Real uRs2 = Surf2->Surface().UResolution(tDistance);
			      Standard_Real vRs2 = Surf2->Surface().VResolution(tDistance);
			      Standard_Real RmaxS1 = Max(uRs1,vRs1);
			      Standard_Real RmaxS2 = Max(uRs2,vRs2);
			      if(RmaxS1 < 1.e-4 && RmaxS2 < 1.e-4) {
				if( pntDMin > tDistance && tDistance > 1.e-9) {
				  pntDMin = tDistance;
				  VDMin = tiV;
				  WLDMin = iL;
				}
			      }
			    }
			  }
			  
			  if( VDMin != 0 ) {
			    const Handle(IntPatch_Line)& aSLine = SLin.Value(WLDMin);
			    const Handle(IntPatch_WLine)&  aWLine = (*((Handle(IntPatch_WLine)*)&aSLine));
			    Standard_Integer tiVpar = (Standard_Integer)aWLine->Vertex(VDMin).ParameterOnLine();
			    Standard_Integer ciVpar = (Standard_Integer)wline->Vertex(ciV).ParameterOnLine();
			    Standard_Real u11 = 0., u12 = 0., v11 = 0., v12 = 0.;
			    Standard_Real u21 = 0., u22 = 0., v21 = 0., v22 = 0.;
			    wline->Point(ciVpar).Parameters(u11,v11,u12,v12);
			    aWLine->Point(tiVpar).Parameters(u21,v21,u22,v22);

			    Handle(IntSurf_LineOn2S) newL2s = new IntSurf_LineOn2S();
			    IntSurf_PntOn2S replacePnt = aWLine->Point(tiVpar);
			    Standard_Integer cNbP = wline->NbPnts();

			    TColStd_SequenceOfInteger VPold;
			    Standard_Integer iPo;
			    for( iPo = 1; iPo <= cnbV; iPo++ ) {
			      Standard_Real Po = wline->Vertex(iPo).ParameterOnLine();
			      Standard_Integer IPo = (Standard_Integer) Po;
			      VPold.Append(IPo);
			    }
			    
			    Standard_Boolean removeNext = Standard_False;
			    Standard_Boolean removePrev = Standard_False;
			    if( ciV == 1) {
			      Standard_Integer dPar = Abs( VPold.Value(ciV) - VPold.Value(ciV+1));
			      if(dPar > 10) {
				removeNext = Standard_True;
				for( iPo = (ciV+1); iPo <= cnbV; iPo++ )
				  VPold.SetValue(iPo, VPold.Value(iPo) - 1 );
			      }
			    }
			    else if( ciV == cnbV) {
			      Standard_Integer dPar = Abs( VPold.Value(ciV) - VPold.Value(ciV-1));
			      if(dPar > 10) {
				removePrev = Standard_True;
				VPold.SetValue(ciV, VPold.Value(ciV) - 1 );
			      }
			    }
			    else {
			      Standard_Integer dParMi = Abs( VPold.Value(ciV) - VPold.Value(ciV-1));
			      Standard_Integer dParMa = Abs( VPold.Value(ciV) - VPold.Value(ciV+1));
			      if(dParMi > 10) {
				removePrev = Standard_True;
				VPold.SetValue(ciV, VPold.Value(ciV) - 1 );
			      }
			      if(dParMa > 10) {
				removeNext = Standard_True;
				for( iPo = (ciV+1); iPo <= cnbV; iPo++ ) {
				  if(dParMi > 10)
				    VPold.SetValue(iPo, VPold.Value(iPo) - 2 );
				  else
				    VPold.SetValue(iPo, VPold.Value(iPo) - 1 );
				}
			      }
			      else {
				if(dParMi > 10)
				  for( iPo = (ciV+1); iPo <= cnbV; iPo++ )
				    VPold.SetValue(iPo, VPold.Value(iPo) - 1 );
			      } 
			    }
			    Standard_Integer pI = (Standard_Integer) ciVpar;

			    Standard_Integer iP;
			    for( iP = 1; iP <= cNbP; iP++) {
			      if( pI == iP )
				newL2s->Add(replacePnt);
			      else if(removeNext && iP == (pI + 1))
				continue;
			      else if(removePrev && iP == (pI - 1))
				continue;
			      else
				newL2s->Add(wline->Point(iP));
			    }

			    IntPatch_Point newVtx;
			    gp_Pnt Pnt3dV = aWLine->Vertex(VDMin).Value();
			    newVtx.SetValue(Pnt3dV,TolTang,Standard_False);
			    newVtx.SetParameters(u21,v21,u22,v22);
			    newVtx.SetParameter(VPold.Value(ciV));

			    Handle(IntPatch_WLine) NWLine = new IntPatch_WLine(newL2s,Standard_False,trans1,trans2);

			    Standard_Integer iV;
			    for( iV = 1; iV <= cnbV; iV++ ) {
			      if( iV == ciV )
				NWLine->AddVertex(newVtx);
			      else {
				IntPatch_Point theVtx = wline->Vertex(iV);
				theVtx.SetParameter(VPold.Value(iV));
				NWLine->AddVertex(theVtx);
			      }
			    }

			    wline = NWLine;
			  }
			}
		      }// SLin.Length > 0

		      SLin.Append(wline);
		      empt = Standard_False;
		    }// !RejetLigne
		  }// PW points > 2
		}// done is True
	      }// dminiPointLigne > SeuildPointLigne
	    }// HasStartPoint
	  }// if TabPtDep[nbps2] == 0
	} while(nbp>5 && !( (NombreDePointsDeDepartDuCheminement >= NbDePointsDeDepartDuChmLimit && lignetrouvee) || 
			    (NombreDePointsDeDepartDuCheminement-3 >= nbp && (!lignetrouvee))));
	delete [] TabPtDep;
      }// for( ls ...

      delete [] TabL;

    }// if nbLigSec >= 1
    //
    AdjustOnPeriodic(Surf1, Surf2, SLin);
    //

    //--------------------------------------------------------------------
    //-- Calcul des parametres approches a partir des Zones De Tangence --
    //--------------------------------------------------------------------
    Standard_Real UminLig1,VminLig1,UmaxLig1,VmaxLig1;
    Standard_Real UminLig2,VminLig2,UmaxLig2,VmaxLig2;
    
    UminLig1=VminLig1=UminLig2=VminLig2=RealLast();
    UmaxLig1=VmaxLig1=UmaxLig2=VmaxLig2=-UminLig1;

    // NbPointsInTangentZone always == 1 (eap)
    
    Standard_Integer z;
    for( z=1; z <= nbTanZon; z++) { 
      //Standard_Integer NbPointsInTangentZone=Interference.NbPointsInTangentZone(z);
      //for(Standard_Integer pz=1; pz<=NbPointsInTangentZone; pz++) {
	Standard_Integer pz=1;
	Standard_Real _x,_y,_z;
	Interference.GetTangentZonePoint(z,pz,_x,_y,_z,U1,V1,U2,V2);

	if(U1>UmaxLig1) UmaxLig1=U1;
	if(V1>VmaxLig1) VmaxLig1=V1;
	if(U2>UmaxLig2) UmaxLig2=U2;
	if(V2>VmaxLig2) VmaxLig2=V2;
	
	if(U1<UminLig1) UminLig1=U1;
	if(V1<VminLig1) VminLig1=V1;
	if(U2<UminLig2) UminLig2=U2;
	if(V2<VminLig2) VminLig2=V2;
      //}
    }
    for(z=1; z <= nbTanZon; z++) {    

      //Standard_Integer NbPointsInTangentZone=Interference.NbPointsInTangentZone(z);
      //for(Standard_Integer pz=1; pz<=NbPointsInTangentZone; pz++) { 
	Standard_Integer pz=1;
	Standard_Real _x,_y,_z;
	Interference.GetTangentZonePoint(z,pz,_x,_y,_z,U1,V1,U2,V2);

	StartParams(1) = U1;
	StartParams(2) = V1;
	StartParams(3) = U2;
	StartParams(4) = V2;
	
	//-----------------------------------------------------------------------
	//-- Calcul du premier point de cheminement a partir du point approche --
	//-----------------------------------------------------------------------
	HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);	
	if(HasStartPoint) { 
	  //-------------------------------------------------
	  //-- Un point a ete trouve                       --
	  //-- On verifie qu il n appartient pas           --
	  //--  a une ligne de cheminement deja calculee.  --
	  //-------------------------------------------------
	  StartPOn2S.Parameters(pu1,pv1,pu2,pv2);
	  
	  NbLigCalculee = SLin.Length();
	  dminiPointLigne = SeuildPointLigne + SeuildPointLigne; 

	  for(Standard_Integer l=1; 
	      (l <= NbLigCalculee) && (dminiPointLigne >= SeuildPointLigne); 
	      l++) { 
	    Handle(IntPatch_WLine)& testwline = *((Handle(IntPatch_WLine)*)&SLin.ChangeValue(l));
	
	    if(  (testwline->IsOutSurf1Box(gp_Pnt2d(pu1,pv1))==Standard_False)
	       &&(testwline->IsOutSurf2Box(gp_Pnt2d(pu2,pv2))==Standard_False)
	       &&(testwline->IsOutBox(StartPOn2S.Value())==Standard_False)) { 
	      
	      
	      //-- Handle(IntPatch_WLine) testwline=Handle(IntPatch_WLine)::DownCast(SLin.Value(l));
	      NbPntOn2SOnLine = testwline->NbPnts();
	      for(Standard_Integer ll=1; 
		  (ll < NbPntOn2SOnLine) && (dminiPointLigne >= SeuildPointLigne); 
		  ll++) { 
		const gp_Pnt& Pa = testwline->Point(ll).Value();
		const gp_Pnt& Pb = testwline->Point(ll+1).Value();
		const gp_Pnt& PStart=StartPOn2S.Value();
		gp_Vec AM(Pa,PStart);
		gp_Vec MB(PStart,Pb);
		Standard_Real AMMB = AM.Dot(MB);
		if(AMMB > 0.0) {
		  gp_Dir ABN(Pb.X()-Pa.X(),Pb.Y()-Pa.Y(),Pb.Z()-Pa.Z());
		  Standard_Real lan =  ABN.X()*AM.X()+ABN.Y()*AM.Y()+ABN.Z()*AM.Z();
		  gp_Vec AH(lan*ABN.X(),lan*ABN.Y(),lan*ABN.Z());
		  gp_Vec HM(AM.X()-AH.X(),AM.Y()-AH.Y(),AM.Z()-AH.Z()); 
		  
		  Standard_Real d = 0.0;
		  if(HM.X() < Deflection) { 
		    d+=HM.X()*HM.X();
		    if(HM.Y() < Deflection) { 
		      d+=HM.Y()*HM.Y(); 
		      if(HM.Z() < Deflection) { 
			d+=HM.Z()*HM.Z(); 
		      }
		      else { 
			d=Deflection2;
		      }
		    }
		    else { 
		      d=Deflection2;
		    }
		  } 
		  else { 
		    d=Deflection2; 
		  }
		  if(d<Deflection2) { 
		    dminiPointLigne = 0.0;
		  }
		}
		else { 
		  Standard_Real dab = Pa.Distance(Pb);
		  Standard_Real dap = Pa.Distance(PStart);
		  if(dap < dab) { 
		    dminiPointLigne=0;
		  }
		  else { 
		    Standard_Real dbp = Pb.Distance(PStart);
		    if(dbp < dab) { 
		      dminiPointLigne=0;
		    }
		  }
		}		
	      }
	    } //-- rejection sur les boites 
	  }


	  //-- Fin d exploration des lignes
	  if(dminiPointLigne > SeuildPointLigne) { 
	    //---------------------------------------------------
	    //-- Le point de depart du nouveau cheminement     --
	    //-- n est present dans aucune ligne deja calculee.--
	    //---------------------------------------------------
	    PW.Perform(StartParams,
			 UminLig1,VminLig1,UminLig2,VminLig2,
			 UmaxLig1,VmaxLig1,UmaxLig2,VmaxLig2);
	    if(PW.IsDone()) {
	      if(PW.NbPoints()>2) { 
		//-----------------------------------------------
		//-- Verification a posteriori : 
		//-- On teste si le point de depart et de fin de 
		//-- la ligne de cheminement est present dans une 
		//-- autre ligne . 
		//-----------------------------------------------
		RejetLigne = Standard_False;
		Point3dDebut = PW.Value(1).Value();
		Point3dFin   = PW.Value(PW.NbPoints()).Value();
		
		for(ver=1 ; (!RejetLigne) && (ver<= NbLigCalculee) ; ver++) { 
		  const Handle(IntPatch_WLine)& verwline = *((Handle(IntPatch_WLine)*)&SLin.Value(ver));
		  //-- Handle(IntPatch_WLine) verwline=Handle(IntPatch_WLine)::DownCast(SLin.Value(ver));
		  const IntSurf_PntOn2S& verPointDebut = verwline->Point(1);
		  const IntSurf_PntOn2S& verPointFin   = verwline->Point(verwline->NbPnts());
		  if(Point3dDebut.Distance(verPointDebut.Value()) < TolTangency) { 
		    RejetLigne = Standard_True; 
		  }
		  else { 
		    if(Point3dFin.Distance(verPointFin.Value()) < TolTangency) { 
		      RejetLigne = Standard_True; 
		    }
		  }
		}
		
		if(!RejetLigne) { 

		  IntSurf_TypeTrans trans1,trans2;
		  Standard_Real locu,locv;
		  gp_Vec norm1,norm2,d1u,d1v;
		  gp_Pnt ptbid;
		  Standard_Integer indextg;
		  gp_Vec tgline(PW.TangentAtLine(indextg));
		  PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
		  Surf1->D1(locu,locv,ptbid,d1u,d1v);
		  norm1 = d1u.Crossed(d1v);
		  PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
		  Surf2->D1(locu,locv,ptbid,d1u,d1v);
		  norm2 = d1u.Crossed(d1v);
		  if (tgline.DotCross(norm2,norm1)>0.) {
		    trans1 = IntSurf_Out;
		    trans2 = IntSurf_In;
		  }
		  else {
		    trans1 = IntSurf_In;
		    trans2 = IntSurf_Out;
		  }
		  
		  
		  
		  Standard_Real TolTang = TolTangency;
		  Handle(IntPatch_WLine) wline = new IntPatch_WLine(PW.Line(),Standard_False,trans1,trans2);
		  IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf2,Standard_True,TolTang);
		  IntPatch_RstInt::PutVertexOnLine(wline,Surf2,D2,Surf1,Standard_False,TolTang);
		  
//---------------
                    if(wline->NbVertex() == 0) {
		      IntPatch_Point vtx;
		      IntSurf_PntOn2S POn2S = PW.Line()->Value(1);
		      POn2S.Parameters(pu1,pv1,pu2,pv2);
		      vtx.SetValue(Point3dDebut,TolTang,Standard_False);
		      vtx.SetParameters(pu1,pv1,pu2,pv2);
		      vtx.SetParameter(1);
		      wline->AddVertex(vtx);
		      
		      POn2S = PW.Line()->Value(wline->NbPnts());
		      POn2S.Parameters(pu1,pv1,pu2,pv2);
		      vtx.SetValue(Point3dFin,TolTang,Standard_False);
		      vtx.SetParameters(pu1,pv1,pu2,pv2);
		      vtx.SetParameter(wline->NbPnts());
		      wline->AddVertex(vtx);
		    }
		    
//---------------
		  SLin.Append(wline);
		  empt = Standard_False;
		  
		}
		else { 
		  //-- cout<<" ----- REJET DE LIGNE (POINT DE DEPART) ----- "<<endl;
		}
		//------------------------------------------------------------		
	      }
	    }  //--  le cheminement a reussi (done a True)
	  }  //--  le point approche ne renvoie pas sur une ligne existante
	} //-- Si HasStartPoint
      //} //-- Boucle Sur les Points de la Tangent Zone
    } //-- Boucle sur Les Tangent Zones

    if ( pInterference ) {
      delete pInterference;
      pInterference = NULL;
    }
    return;
  }

  Handle(IntSurf_LineOn2S) LOn2S = new IntSurf_LineOn2S();
  PointDepart( LOn2S, Surf1, NbU1, NbV1, Surf2, NbU2, NbV2 );
  empt = Standard_True;
  done = Standard_True;
  SLin.Clear();  
  Standard_Real Deflection2 = Deflection*Deflection;
  
  Standard_Integer NbPntOn2SOnLine;
  Standard_Integer NbLigCalculee = 0;
  Standard_Real U1,U2,V1,V2;
  Standard_Real pu1,pu2,pv1,pv2;
  
  TColStd_Array1OfReal StartParams(1,4);
  Standard_Integer MaxOscill = NbU1;
  if(MaxOscill < NbU2) MaxOscill=NbU2;
  if(MaxOscill < NbV1) MaxOscill=NbV1;
  if(MaxOscill < NbV2) MaxOscill=NbV2;
  
  Standard_Real nIncrement=Increment;
  //if(MaxOscill>10)
    //nIncrement/=0.5*MaxOscill;

  IntPatch_ThePWalkingInter PW(Surf1,Surf2,TolTangency,Epsilon,Deflection,nIncrement);
  Standard_Real    SeuildPointLigne = 15.0 * Increment * Increment; //-- 10 est insuffisant
  Standard_Real    dminiPointLigne;
  Standard_Boolean HasStartPoint,RejetLigne;
  IntSurf_PntOn2S StartPOn2S;
  Standard_Integer ver;
  gp_Pnt Point3dDebut,Point3dFin;

  //------------------------------------------------------------
  //-- Calcul des parametres approches a partir des Zones De Tangence --
  //--------------------------------------------------------------------
  Standard_Integer nbTanZon = LOn2S->NbPoints();
  for(Standard_Integer z=1; z <= nbTanZon; z++) { 
    const IntSurf_PntOn2S& POn2S = LOn2S->Value(z);
    POn2S.Parameters(U1,V1,U2,V2);
    StartParams(1) = U1;
    StartParams(2) = V1;
    StartParams(3) = U2;
    StartParams(4) = V2;
    
    //-----------------------------------------------------------------------
    //-- Calcul du premier point de cheminement a partir du point approche --
    //-----------------------------------------------------------------------
    HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);	
    if(HasStartPoint) { 
      //-------------------------------------------------
      //-- Un point a ete trouve                       --
      //-- On verifie qu il n appartient pas           --
      //--  a une ligne de cheminement deja calculee.  --
      //-------------------------------------------------
      StartPOn2S.Parameters(pu1,pv1,pu2,pv2);
      
      NbLigCalculee = SLin.Length();
      dminiPointLigne = SeuildPointLigne + SeuildPointLigne; 
      
      for(Standard_Integer l=1; 
	  (l <= NbLigCalculee) && (dminiPointLigne >= SeuildPointLigne); 
	  l++) { 
	const Handle(IntPatch_WLine)& testwline = *((Handle(IntPatch_WLine)*)&SLin.Value(l));
	
	if(  (testwline->IsOutSurf1Box(gp_Pnt2d(pu1,pv1))==Standard_False)
	   &&(testwline->IsOutSurf2Box(gp_Pnt2d(pu2,pv2))==Standard_False)
	   &&(testwline->IsOutBox(StartPOn2S.Value())==Standard_False)) { 
	  
	  NbPntOn2SOnLine = testwline->NbPnts();
	  for(Standard_Integer ll=1; 
	      (ll < NbPntOn2SOnLine) && (dminiPointLigne >= SeuildPointLigne); 
	      ll++) { 
	    
	    const gp_Pnt& Pa = testwline->Point(ll).Value();
	    const gp_Pnt& Pb = testwline->Point(ll+1).Value();
	    const gp_Pnt& PStart=StartPOn2S.Value();
	    gp_Vec AM(Pa,PStart);
	    gp_Vec MB(PStart,Pb);
	    Standard_Real AMMB = AM.Dot(MB);
	    if(AMMB > 0.0) {
	      gp_Dir ABN(Pb.X()-Pa.X(),Pb.Y()-Pa.Y(),Pb.Z()-Pa.Z());
	      Standard_Real lan =  ABN.X()*AM.X()+ABN.Y()*AM.Y()+ABN.Z()*AM.Z();
	      gp_Vec AH(lan*ABN.X(),lan*ABN.Y(),lan*ABN.Z());
	      gp_Vec HM(AM.X()-AH.X(),AM.Y()-AH.Y(),AM.Z()-AH.Z()); 
	      
	      Standard_Real d = 0.0;
	      if(HM.X() < Deflection) { 
		d+=HM.X()*HM.X();
		if(HM.Y() < Deflection) { 
		  d+=HM.Y()*HM.Y(); 
		  if(HM.Z() < Deflection) { 
		    d+=HM.Z()*HM.Z(); 
		  }
		  else { 
		    d=Deflection2;
		  }
		}
		else { 
		  d=Deflection2;
		}
	      } 
	      else { 
		d=Deflection2; 
	      }
	      if(d<Deflection2) { 
		dminiPointLigne = 0.0;
	      }
	    }
	    else { 
	      Standard_Real dab = Pa.Distance(Pb);
	      Standard_Real dap = Pa.Distance(PStart);
	      if(dap < dab) { 
		dminiPointLigne=0;
	      }
	      else { 
		Standard_Real dbp = Pb.Distance(PStart);
		if(dbp < dab) { 
		  dminiPointLigne=0;
		}
	      }
	    }
	  }
	} //-- Rejections Boites 
	//--	  else { 
	//--	    cout<<" Rejet Boites "<<endl;
	//--	  }
      }
      
      
      //-- Fin d exploration des lignes
      if(dminiPointLigne > SeuildPointLigne) { 
	//---------------------------------------------------
	//-- Le point de depart du nouveau cheminement     --
	//-- n est present dans aucune ligne deja calculee.--
	//---------------------------------------------------
	PW.Perform(StartParams);
	if(PW.IsDone()) {
	  if(PW.NbPoints()>2) { 
	    //-----------------------------------------------
	    //-- Verification a posteriori : 
	    //-- On teste si le point de depart et de fin de 
	    //-- la ligne de cheminement est present dans une 
	    //-- autre ligne . 
	    //-----------------------------------------------
	    RejetLigne = Standard_False;
	    Point3dDebut = PW.Value(1).Value();
	    Point3dFin   = PW.Value(PW.NbPoints()).Value();
	    
	    for(ver=1 ; (!RejetLigne) && (ver<= NbLigCalculee) ; ver++) { 
	      const Handle(IntPatch_WLine)& verwline = *((Handle(IntPatch_WLine)*)&SLin.Value(ver));
	      //-- Handle(IntPatch_WLine) verwline=Handle(IntPatch_WLine)::DownCast(SLin.Value(ver));
	      const IntSurf_PntOn2S& verPointDebut = verwline->Point(1);
	      const IntSurf_PntOn2S& verPointFin   = verwline->Point(verwline->NbPnts());
	      if(Point3dDebut.Distance(verPointDebut.Value()) < TolTangency) { 
		RejetLigne = Standard_True; 
	      }
	      else { 
		if(Point3dFin.Distance(verPointFin.Value()) < TolTangency) { 
		  RejetLigne = Standard_True; 
		}
	      }
	    }
	    
	    if(!RejetLigne) { 
	      
	      IntSurf_TypeTrans trans1,trans2;
	      Standard_Real locu,locv;
	      gp_Vec norm1,norm2,d1u,d1v;
	      gp_Pnt ptbid;
	      Standard_Integer indextg;
	      gp_Vec tgline(PW.TangentAtLine(indextg));
	      PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
	      Surf1->D1(locu,locv,ptbid,d1u,d1v);
	      norm1 = d1u.Crossed(d1v);
	      PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
	      Surf2->D1(locu,locv,ptbid,d1u,d1v);
	      norm2 = d1u.Crossed(d1v);
	      if (tgline.DotCross(norm2,norm1)>0.) {
		trans1 = IntSurf_Out;
		trans2 = IntSurf_In;
	      }
	      else {
		trans1 = IntSurf_In;
		trans2 = IntSurf_Out;
	      }
	      
	      
	      
	      Standard_Real TolTang = TolTangency;
	      Handle(IntPatch_WLine) wline = new IntPatch_WLine(PW.Line(),Standard_False,trans1,trans2);
	      IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf2,Standard_True,TolTang);
	      IntPatch_RstInt::PutVertexOnLine(wline,Surf2,D2,Surf1,Standard_False,TolTang);
	      
	      //---------------
	      if(wline->NbVertex() == 0) {
		IntPatch_Point vtx;
		const IntSurf_PntOn2S& POn2Sf = PW.Line()->Value(1);
		POn2Sf.Parameters(pu1,pv1,pu2,pv2);
		vtx.SetValue(Point3dDebut,TolTang,Standard_False);
		vtx.SetParameters(pu1,pv1,pu2,pv2);
		vtx.SetParameter(1);
		wline->AddVertex(vtx);
		
		const IntSurf_PntOn2S& POn2Sl = PW.Line()->Value(wline->NbPnts());
		POn2Sl.Parameters(pu1,pv1,pu2,pv2);
		vtx.SetValue(Point3dFin,TolTang,Standard_False);
		vtx.SetParameters(pu1,pv1,pu2,pv2);
		vtx.SetParameter(wline->NbPnts());
		wline->AddVertex(vtx);
	      }
	      
	      //---------------
	      SLin.Append(wline);
	      empt = Standard_False;
	      
	    }
	    else { 
	      //-- cout<<" ----- REJET DE LIGNE (POINT DE DEPART) ----- "<<endl;
	    }
	    //------------------------------------------------------------		
	  }
	}  //--  le cheminement a reussi (done a True)
      }  //--  le point approche ne renvoie pas sur une ligne existante
    } //-- Si HasStartPoint
  } //-- Boucle sur Les Tangent Zones
    
}
// modified by NIZHNY-AMV  Tue Oct 18 12:37:02 2005
void IntPatch_PrmPrmIntersection::Perform (const Handle(Adaptor3d_HSurface)&    Surf1,
					   const Handle(Adaptor3d_TopolTool)& D1,
					   const Handle(Adaptor3d_HSurface)&    Surf2,
					   const Handle(Adaptor3d_TopolTool)& D2,
					   const Standard_Real   TolTangency,
					   const Standard_Real   Epsilon,
					   const Standard_Real   Deflection,
					   const Standard_Real   Increment,
                                           IntSurf_ListOfPntOn2S& LOfPnts,
                                           const Standard_Boolean RestrictLine)
{
  if (LOfPnts.IsEmpty()){
    done = Standard_True;
    return;
  }
    
  empt = Standard_True;
  SLin.Clear();  
  
  Standard_Real UminLig1,VminLig1,UmaxLig1,VmaxLig1;
  Standard_Real UminLig2,VminLig2,UmaxLig2,VmaxLig2;
  Standard_Real U1,U2,V1,V2;
  
  UminLig1 = Surf1->FirstUParameter();
  VminLig1 = Surf1->FirstVParameter();
  UmaxLig1 = Surf1->LastUParameter();
  VmaxLig1 = Surf1->LastVParameter();
  UminLig2 = Surf2->FirstUParameter();
  VminLig2 = Surf2->FirstVParameter();
  UmaxLig2 = Surf2->LastUParameter();
  VmaxLig2 = Surf2->LastVParameter();
  
  IntSurf_ListIteratorOfListOfPntOn2S IterLOP1(LOfPnts);

  for(; IterLOP1.More(); IterLOP1.Next()){
    IntSurf_PntOn2S Pnt = IterLOP1.Value();
    Pnt.Parameters(U1, V1, U2, V2);
    if(U1>UmaxLig1) UmaxLig1=U1;
    if(V1>VmaxLig1) VmaxLig1=V1;
    if(U2>UmaxLig2) UmaxLig2=U2;
    if(V2>VmaxLig2) VmaxLig2=V2;
    
    if(U1<UminLig1) UminLig1=U1;
    if(V1<VminLig1) VminLig1=V1;
    if(U2<UminLig2) UminLig2=U2;
    if(V2<VminLig2) VminLig2=V2; 
  }

  Standard_Real Deflection2 = Deflection*Deflection;
  
  Standard_Real SeuildPointLigne = 15.0 * Increment * Increment;
  
  Standard_Integer NbPntOn2SOnLine = 0, NbLigCalculee = 0, ver;
  Standard_Real pu1,pu2,pv1,pv2, dminiPointLigne;
  Standard_Boolean HasStartPoint,RejetLigne;
  IntSurf_PntOn2S StartPOn2S;
  gp_Pnt Point3dDebut,Point3dFin;

  TColStd_Array1OfReal StartParams(1,4);
  IntPatch_ThePWalkingInter PW(Surf1,Surf2,TolTangency,Epsilon,Deflection,Increment);  
    
  IntSurf_ListIteratorOfListOfPntOn2S IterLOP2(LOfPnts);
  for(; IterLOP2.More(); IterLOP2.Next() ){
    
    IntSurf_PntOn2S cPnt = IterLOP2.Value();
    cPnt.Parameters(U1, V1, U2, V2);
    
    StartParams(1) = U1;
    StartParams(2) = V1;
    StartParams(3) = U2;
    StartParams(4) = V2;
    
    HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);
    dminiPointLigne = SeuildPointLigne + SeuildPointLigne;
    if(HasStartPoint) {
      StartPOn2S.Parameters(pu1,pv1,pu2,pv2);
      NbLigCalculee = SLin.Length();
      Standard_Integer l;
      for( l = 1; (l <= NbLigCalculee) && (dminiPointLigne >= SeuildPointLigne); l++) { 
        Handle(IntPatch_WLine)& testwline = *((Handle(IntPatch_WLine)*)&SLin.ChangeValue(l));
        if( (testwline->IsOutSurf1Box(gp_Pnt2d(pu1,pv1))==Standard_False) &&
           (testwline->IsOutSurf2Box(gp_Pnt2d(pu2,pv2))==Standard_False) &&
           (testwline->IsOutBox(StartPOn2S.Value())==Standard_False) )	{
          NbPntOn2SOnLine = testwline->NbPnts();
          Standard_Integer ll;
          for( ll = 1; (ll < NbPntOn2SOnLine) && (dminiPointLigne >= SeuildPointLigne); ll++) { 
            const gp_Pnt &Pa = testwline->Point(ll).Value();
            const gp_Pnt &Pb = testwline->Point(ll+1).Value();
            const gp_Pnt &PStart = StartPOn2S.Value();
            gp_Vec AM(Pa,PStart);
            gp_Vec MB(PStart,Pb);
            Standard_Real AMMB = AM.Dot(MB);		    
            if(AMMB > 0.0) {
              gp_Dir ABN(Pb.X()-Pa.X(),Pb.Y()-Pa.Y(),Pb.Z()-Pa.Z());
              Standard_Real lan =  ABN.X()*AM.X()+ABN.Y()*AM.Y()+ABN.Z()*AM.Z();
              gp_Vec AH(lan*ABN.X(),lan*ABN.Y(),lan*ABN.Z());
              gp_Vec HM(AM.X()-AH.X(),AM.Y()-AH.Y(),AM.Z()-AH.Z());
              Standard_Real d = 0.0;
              if(HM.X() < Deflection) { 
                d+=HM.X()*HM.X();
                if(HM.Y() < Deflection) { 
                  d+=HM.Y()*HM.Y(); 
                  if(HM.Z() < Deflection)
                    d+=HM.Z()*HM.Z(); 
                  else
                    d=Deflection2;
                }
                else
                  d=Deflection2;
              } 
              else
                d=Deflection2; 
              
              if(d<Deflection2)
                dminiPointLigne = 0.0;
            }
            else {
              Standard_Real dab = Pa.SquareDistance(Pb);
              Standard_Real dap = Pa.SquareDistance(PStart);
              if(dap < dab)
                dminiPointLigne=0;
              else { 
                Standard_Real dbp = Pb.SquareDistance(PStart);
                if(dbp < dab)
                  dminiPointLigne=0;
              }
            }
          }// for( ll ...
        }// if ...
      }// for( l ...
      
      if(dminiPointLigne > SeuildPointLigne) {
        PW.Perform(StartParams,UminLig1,VminLig1,UminLig2,VminLig2,UmaxLig1,VmaxLig1,UmaxLig2,VmaxLig2);
        if(PW.IsDone())	{
          if(PW.NbPoints()>2) {
            RejetLigne = Standard_False;
            Point3dDebut = PW.Value(1).Value();
            Point3dFin   = PW.Value(PW.NbPoints()).Value();
            for( ver = 1 ; (!RejetLigne) && (ver<= NbLigCalculee) ; ver++) { 
              const Handle(IntPatch_WLine)& verwline = *((Handle(IntPatch_WLine)*)&SLin.Value(ver));
              const IntSurf_PntOn2S& verPointDebut = verwline->Point(1);
              const IntSurf_PntOn2S& verPointFin = verwline->Point(verwline->NbPnts());
              if(Point3dDebut.Distance(verPointDebut.Value()) <= TolTangency) { 
                if(Point3dFin.Distance(verPointFin.Value()) <= TolTangency)
                  RejetLigne = Standard_True; 
              }
            }
            
            if(!RejetLigne) {
              IntSurf_TypeTrans trans1,trans2;
              Standard_Real locu,locv;
              gp_Vec norm1,norm2,d1u,d1v;
              gp_Pnt ptbid;
              Standard_Integer indextg;
              gp_Vec tgline(PW.TangentAtLine(indextg));
              PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
              Surf1->D1(locu,locv,ptbid,d1u,d1v);
              norm1 = d1u.Crossed(d1v);
              PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
              Surf2->D1(locu,locv,ptbid,d1u,d1v);
              norm2 = d1u.Crossed(d1v);
              if( tgline.DotCross(norm2,norm1) >= 0. ) {
                trans1 = IntSurf_Out;
                trans2 = IntSurf_In;
              }
              else {
                trans1 = IntSurf_In;
                trans2 = IntSurf_Out;
              }
              
              Standard_Real TolTang = TolTangency;
              Handle(IntPatch_WLine) wline = new IntPatch_WLine(PW.Line(),Standard_False,trans1,trans2);
              if (RestrictLine){
                IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf2,Standard_True,TolTang);
                IntPatch_RstInt::PutVertexOnLine(wline,Surf2,D2,Surf1,Standard_False,TolTang);
              }
              
              if(wline->NbVertex() == 0) {
                IntPatch_Point vtx;
                IntSurf_PntOn2S POn2S = PW.Line()->Value(1);
                POn2S.Parameters(pu1,pv1,pu2,pv2);
                vtx.SetValue(Point3dDebut,TolTang,Standard_False);
                vtx.SetParameters(pu1,pv1,pu2,pv2);
                vtx.SetParameter(1);
                wline->AddVertex(vtx);
                
                POn2S = PW.Line()->Value(wline->NbPnts());
                POn2S.Parameters(pu1,pv1,pu2,pv2);
                vtx.SetValue(Point3dFin,TolTang,Standard_False);
                vtx.SetParameters(pu1,pv1,pu2,pv2);
                vtx.SetParameter(wline->NbPnts());
                wline->AddVertex(vtx);
              }              

              Standard_Integer slinlen = SLin.Length();
              if( slinlen > 0 ) {
                Standard_Integer cnbV = wline->NbVertex();
                Standard_Integer ciV;
                for( ciV = 1; ciV <= cnbV; ciV++ ) {
                  Standard_Real pntDMin = 1.e+100;
                  Standard_Integer VDMin = 0;
                  Standard_Integer WLDMin = 0;
                  gp_Pnt cPV = wline->Vertex(ciV).Value();
                  Standard_Integer iL;
                  for( iL = 1; iL <= slinlen; iL++) {
                    const Handle(IntPatch_Line)& aSLine = SLin.Value(iL);
                    IntPatch_IType aType = aSLine->ArcType();
                    if( aType != IntPatch_Walking)
                      continue;
                    const Handle(IntPatch_WLine)&  aWLine = (*((Handle(IntPatch_WLine)*)&aSLine));
                    Standard_Integer tnbV = aWLine->NbVertex();
                    Standard_Integer tiV;
                    for( tiV = 1; tiV <= tnbV; tiV++ ) {
                      gp_Pnt tPV = aWLine->Vertex(tiV).Value();
                      Standard_Real tDistance = cPV.Distance(tPV);
                      Standard_Real uRs1 = Surf1->Surface().UResolution(tDistance);
                      Standard_Real vRs1 = Surf1->Surface().VResolution(tDistance);
                      Standard_Real uRs2 = Surf2->Surface().UResolution(tDistance);
                      Standard_Real vRs2 = Surf2->Surface().VResolution(tDistance);
                      Standard_Real RmaxS1 = Max(uRs1,vRs1);
                      Standard_Real RmaxS2 = Max(uRs2,vRs2);
                      if(RmaxS1 < 1.e-4 && RmaxS2 < 1.e-4) {
                        if( pntDMin > tDistance && tDistance > 1.e-9) {
                          pntDMin = tDistance;
                          VDMin = tiV;
                          WLDMin = iL;
                        }
                      }
                    }
                  }
                  
                  if( VDMin != 0 ) {
                    const Handle(IntPatch_Line)& aSLine = SLin.Value(WLDMin);
                    const Handle(IntPatch_WLine)&  aWLine = (*((Handle(IntPatch_WLine)*)&aSLine));
                    Standard_Integer tiVpar = (Standard_Integer)aWLine->Vertex(VDMin).ParameterOnLine();
                    Standard_Integer ciVpar = (Standard_Integer)wline->Vertex(ciV).ParameterOnLine();
                    Standard_Real u11 = 0., u12 = 0., v11 = 0., v12 = 0.;
                    Standard_Real u21 = 0., u22 = 0., v21 = 0., v22 = 0.;
                    wline->Point(ciVpar).Parameters(u11,v11,u12,v12);
                    aWLine->Point(tiVpar).Parameters(u21,v21,u22,v22);
                    
                    Handle(IntSurf_LineOn2S) newL2s = new IntSurf_LineOn2S();
                    IntSurf_PntOn2S replacePnt = aWLine->Point(tiVpar);
                    Standard_Integer cNbP = wline->NbPnts();
                    
                    TColStd_SequenceOfInteger VPold;
                    Standard_Integer iPo;
                    for( iPo = 1; iPo <= cnbV; iPo++ ) {
                      Standard_Real Po = wline->Vertex(iPo).ParameterOnLine();
                      Standard_Integer IPo = (Standard_Integer) Po;
                      VPold.Append(IPo);
                    }
                    
                    Standard_Boolean removeNext = Standard_False;
                    Standard_Boolean removePrev = Standard_False;
                    if( ciV == 1) {
                      Standard_Integer dPar = Abs( VPold.Value(ciV) - VPold.Value(ciV+1));
                      if(dPar > 10) {
				removeNext = Standard_True;
				for( iPo = (ciV+1); iPo <= cnbV; iPo++ )
				  VPold.SetValue(iPo, VPold.Value(iPo) - 1 );
			      }
                    }
                    else if( ciV == cnbV) {
                      Standard_Integer dPar = Abs( VPold.Value(ciV) - VPold.Value(ciV-1));
                      if(dPar > 10) {
                        removePrev = Standard_True;
                        VPold.SetValue(ciV, VPold.Value(ciV) - 1 );
                      }
                    }
                    else {
                      Standard_Integer dParMi = Abs( VPold.Value(ciV) - VPold.Value(ciV-1));
                      Standard_Integer dParMa = Abs( VPold.Value(ciV) - VPold.Value(ciV+1));
                      if(dParMi > 10) {
                        removePrev = Standard_True;
                        VPold.SetValue(ciV, VPold.Value(ciV) - 1 );
                      }
                      if(dParMa > 10) {
                        removeNext = Standard_True;
                        for( iPo = (ciV+1); iPo <= cnbV; iPo++ ) {
                          if(dParMi > 10)
                            VPold.SetValue(iPo, VPold.Value(iPo) - 2 );
                          else
                            VPold.SetValue(iPo, VPold.Value(iPo) - 1 );
                        }
                      }
                      else {
                        if(dParMi > 10)
                          for( iPo = (ciV+1); iPo <= cnbV; iPo++ )
                            VPold.SetValue(iPo, VPold.Value(iPo) - 1 );
                      } 
                    }
                    Standard_Integer pI = (Standard_Integer) ciVpar;
                    
                    Standard_Integer iP;
                    for( iP = 1; iP <= cNbP; iP++) {
                      if( pI == iP )
                        newL2s->Add(replacePnt);
                      else if(removeNext && iP == (pI + 1))
                        continue;
                      else if(removePrev && iP == (pI - 1))
                        continue;
                      else
                        newL2s->Add(wline->Point(iP));
                    }
                    
                    IntPatch_Point newVtx;
                    gp_Pnt Pnt3dV = aWLine->Vertex(VDMin).Value();
                    newVtx.SetValue(Pnt3dV,TolTang,Standard_False);
                    newVtx.SetParameters(u21,v21,u22,v22);
                    newVtx.SetParameter(VPold.Value(ciV));
                    
                    Handle(IntPatch_WLine) NWLine = new IntPatch_WLine(newL2s,Standard_False,trans1,trans2);
                    
                    Standard_Integer iV;
                    for( iV = 1; iV <= cnbV; iV++ ) {
                      if( iV == ciV )
                        NWLine->AddVertex(newVtx);
                      else {
                        IntPatch_Point theVtx = wline->Vertex(iV);
                        theVtx.SetParameter(VPold.Value(iV));
                        NWLine->AddVertex(theVtx);
                      }
                    }
                    
                    wline = NWLine;
                  }
                }
              }// SLin.Length > 0
              
              SLin.Append(wline);
              empt = Standard_False;
            }// !RejetLigne
          }// PW points > 2
        }// done is True
      }// dminiPointLigne > SeuildPointLigne
    }// HasStartPoint  
  }// for( IterLOP ...
  done = Standard_True;
  return;      
}

//======================================================================     
void IntPatch_PrmPrmIntersection::Perform(const Handle(Adaptor3d_HSurface)&    Surf1,
					  const Handle(Adaptor3d_TopolTool)& D1,
					  const Handle(Adaptor3d_HSurface)&    Surf2,
					  const Handle(Adaptor3d_TopolTool)& D2,
					  const Standard_Real   U1Depart,
					  const Standard_Real   V1Depart,
					  const Standard_Real   U2Depart,
					  const Standard_Real   V2Depart,
					  const Standard_Real   TolTangency,
					  const Standard_Real   Epsilon,
					  const Standard_Real   Deflection,
					  const Standard_Real   Increment) { 
  
  
  
//    Standard_Integer NbU1 = D1->NbSamplesU();
//    Standard_Integer NbV1 = D1->NbSamplesV();
//    Standard_Integer NbU2 = D2->NbSamplesU();
//    Standard_Integer NbV2 = D2->NbSamplesV();

  //-- Traitement des Lignes de sections
  empt = Standard_True;
  done = Standard_True;
  SLin.Clear();  
  
  //------------------------------------------------------------

  Standard_Real pu1,pu2,pv1,pv2;
  
  TColStd_Array1OfReal StartParams(1,4);
  
//    Standard_Integer MaxOscill = NbU1;
//    if(MaxOscill < NbU2) MaxOscill=NbU2;
//    if(MaxOscill < NbV1) MaxOscill=NbV1;
//    if(MaxOscill < NbV2) MaxOscill=NbV2;
  
//    Standard_Real nIncrement=Increment;
//    if(MaxOscill>10) { 
//  #ifdef DEB 
//      cout<<"\n IntPatch_PrmPrmIntersection.gxx : Increment:"<<Increment<<" -> "<<Increment/(0.5*MaxOscill)<<endl;
//  #endif
//      nIncrement/=0.5*MaxOscill;
//    }
  
  IntPatch_ThePWalkingInter PW(Surf1,Surf2,
			       TolTangency,
			       Epsilon,
			       Deflection,
			       Increment); //nIncrement);
  
  
  //Standard_Real    SeuildPointLigne = 15.0 * Increment * Increment; //-- 10 est insuffisant
  //Standard_Real    incidence;
  //Standard_Real    dminiPointLigne;
  
  Standard_Boolean HasStartPoint;//,RejetLigne;
  
  IntSurf_PntOn2S StartPOn2S;
  
  //Standard_Integer ver;
  
  gp_Pnt Point3dDebut,Point3dFin;
  
  //------------------------------------------------------------
  
  StartParams(1) = U1Depart;
  StartParams(2) = V1Depart;
  StartParams(3) = U2Depart;
  StartParams(4) = V2Depart;
  
  //-----------------------------------------------------------------------
  //-- Calcul du premier point de cheminement a partir du point approche --
  //-----------------------------------------------------------------------
  HasStartPoint = PW.PerformFirstPoint(StartParams,StartPOn2S);	
  if(HasStartPoint) { 
    //-------------------------------------------------
    //-- Un point a ete trouve                       --
    //-- On verifie qu il n appartient pas           --
    //--  a une ligne de cheminement deja calculee.  --
    //-------------------------------------------------
    
    PW.Perform(StartParams);
    if(PW.IsDone()) {
      
      Point3dDebut = PW.Value(1).Value();
      Point3dFin   = PW.Value(PW.NbPoints()).Value();

      IntSurf_TypeTrans trans1,trans2;
      Standard_Real locu,locv;
      gp_Vec norm1,norm2,d1u,d1v;
      gp_Pnt ptbid;
      Standard_Integer indextg;
      gp_Vec tgline(PW.TangentAtLine(indextg));
      PW.Line()->Value(indextg).ParametersOnS1(locu,locv);
      Surf1->D1(locu,locv,ptbid,d1u,d1v);
      norm1 = d1u.Crossed(d1v);
      PW.Line()->Value(indextg).ParametersOnS2(locu,locv);
      Surf2->D1(locu,locv,ptbid,d1u,d1v);
      norm2 = d1u.Crossed(d1v);
      if (tgline.DotCross(norm2,norm1)>0.) {
	trans1 = IntSurf_Out;
	trans2 = IntSurf_In;
      }
      else {
	trans1 = IntSurf_In;
	trans2 = IntSurf_Out;
      }
      
      
      
      Standard_Real TolTang = TolTangency;
      Handle(IntPatch_WLine) wline = new IntPatch_WLine(PW.Line(),Standard_False,trans1,trans2);
      IntPatch_RstInt::PutVertexOnLine(wline,Surf1,D1,Surf2,Standard_True,TolTang);
      IntPatch_RstInt::PutVertexOnLine(wline,Surf2,D2,Surf1,Standard_False,TolTang);
      
      //---------------
      if(wline->NbVertex() == 0) {
	IntPatch_Point vtx;
	IntSurf_PntOn2S POn2S = PW.Line()->Value(1);
	POn2S.Parameters(pu1,pv1,pu2,pv2);
	vtx.SetValue(Point3dDebut,TolTang,Standard_False);
	vtx.SetParameters(pu1,pv1,pu2,pv2);
	vtx.SetParameter(1);
	wline->AddVertex(vtx);
	
	POn2S = PW.Line()->Value(wline->NbPnts());
	POn2S.Parameters(pu1,pv1,pu2,pv2);
	vtx.SetValue(Point3dFin,TolTang,Standard_False);
	vtx.SetParameters(pu1,pv1,pu2,pv2);
	vtx.SetParameter(wline->NbPnts());
	wline->AddVertex(vtx);
      }
      
      //---------------
      SLin.Append(wline);
      empt = Standard_False;
      
    }
  }
}


//xf
//==================================================================================
// function : AdjustOnPeriodic
// purpose  : 
//==================================================================================
void AdjustOnPeriodic(const Handle(Adaptor3d_HSurface)& Surf1,
		      const Handle(Adaptor3d_HSurface)& Surf2,
		      IntPatch_SequenceOfLine& aSLin)
{
  Standard_Boolean bIsPeriodic[4], bModified, bIsNull, bIsPeriod;
  Standard_Integer i, j, k, aNbLines, aNbPx, aIndx, aIndq;
  Standard_Real aPeriod[4], dPeriod[4], ux[4], uq[4], aEps, du;
  //
  aEps=Precision::Confusion();
  //
  for (k=0; k<4; ++k) {
    aPeriod[k]=0.;
  }
  //
  bIsPeriodic[0]=Surf1->IsUPeriodic();
  bIsPeriodic[1]=Surf1->IsVPeriodic();
  bIsPeriodic[2]=Surf2->IsUPeriodic();
  bIsPeriodic[3]=Surf2->IsVPeriodic();
  //
  if (bIsPeriodic[0]){
    aPeriod[0]=Surf1->UPeriod();
  }
  if (bIsPeriodic[1]){
    aPeriod[1]=Surf1->VPeriod();
  }
  if (bIsPeriodic[2]){
    aPeriod[2]=Surf2->UPeriod();
  }
  if (bIsPeriodic[3]){
    aPeriod[3]=Surf2->VPeriod();
  }
  //
  for (k=0; k<4; ++k) {
    dPeriod[k]=0.25*aPeriod[k];
  }
  //
  aNbLines=aSLin.Length();
  for (i=1; i<=aNbLines; ++i) {
    Handle(IntPatch_WLine) aIL=Handle(IntPatch_WLine)::DownCast(aSLin.Value(i));
    Handle(IntSurf_LineOn2S) aL=aIL->Curve();

    aNbPx=aL->NbPoints();
    if (aNbPx<10) {
      continue;
    }
    //
    for (j=0; j<2; ++j) {
      bModified=Standard_False;
      aIndx=1;
      aIndq=2;
      if (j) {
	aIndx=aNbPx;
	aIndq=aNbPx-1;
      }
      //
      const IntSurf_PntOn2S& aPSx=aL->Value(aIndx);
      const IntSurf_PntOn2S& aPSq=aL->Value(aIndq);
      //
      aPSx.Parameters(ux[0], ux[1], ux[2], ux[3]);
      aPSq.Parameters(uq[0], uq[1], uq[2], uq[3]);
      //
      for (k=0; k<4; ++k) {
	bIsNull=Standard_False;
	bIsPeriod=Standard_False;
	//
	if (!bIsPeriodic[k]) {
	  continue;
	}
	//
	if (fabs(ux[k])<aEps) {
	  bModified=Standard_True;
	  bIsNull=Standard_True;
	}
	//
	else if (fabs(ux[k]-aPeriod[k])<aEps) {
	  bModified=Standard_True;
	  bIsPeriod=Standard_True;
	}
	//
	if (bModified) {
	  du=fabs(ux[k]-uq[k]);
	  if (du > dPeriod[k]) {
	    if(bIsNull){
	      ux[k]=aPeriod[k];
	    }
	    if(bIsPeriod) {
	      ux[k]=0.;
	    }
	  }
	}
      }//for (k=0; k<4; ++k) 
      if (bModified) {
	IntSurf_PntOn2S aPntOn2S;
	//
	aPntOn2S=aPSx;  
	aPntOn2S.SetValue(ux[0], ux[1], ux[2], ux[3]);
	aL->Value(aIndx, aPntOn2S);
      }
    }//for (j=0; j<1; ++j) {
  }//for (i=1; i<=aNbLines; ++i)
}
//xt
