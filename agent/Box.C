/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Box.C
 *
 *    AUTHOR     : 
 *
 *    $Revision: 2.3 $
 *
 *    $Id: Box.C,v 2.3 2004/03/05 07:03:44 anton Exp $
 *
 ************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//////////////////////////////////////////////////////////////////////

#include <cmath>
#include "Box.h"
                     

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTerm::CTerm()
{
  m_StudyNum = 0;
}

CTerm::CTerm(int size)
{
  m_Hi = new long[size];
  m_Lo = new long[size];
  m_Center = new double[size];
  m_Size = size;
  m_StudyNum = 0;
  m_Val = 0;
}

CTerm::CTerm(const CTerm& src)
{
  m_Size = src.m_Size;
  m_Hi = new long[m_Size];
  m_Lo = new long[m_Size];
  m_Center = new double[m_Size];
  m_Val = src.m_Val;
  m_StudyNum = src.m_StudyNum;
  memcpy(m_Lo, src.m_Lo, m_Size*sizeof(long));
  memcpy(m_Hi, src.m_Hi, m_Size*sizeof(long));
  memcpy(m_Center, src.m_Center, m_Size*sizeof(double));
}

CTerm::CTerm(int size, double val, double* cntr)
{
  m_Hi = new long[size];
  m_Lo = new long[size];
  m_Center = new double[size];
  m_Size = size;
  m_Val = val;
  m_StudyNum = 1;
  memcpy(m_Center, cntr, size*sizeof(double));
  for(int i=0; i<size; i++)
    m_Lo[i] = m_Hi[i] = -1;
}

CTerm::CTerm(ifstream& file)
{
  char buf[255];
  int i;
  //let's get the "m_Size" var
  file.getline(buf,255);
  m_Size=atoi(buf);
  //now let's get the "m_StudyNum" var
  file.getline(buf,255);
  m_StudyNum=atoi(buf);
  //now let's get "m_Center" array
  m_Center=new double[m_Size];
  for (i=0;i<m_Size;i++) {
    file.getline(buf,255);
    m_Center[i]=strtod(buf,NULL);
  }
  //for now: "m_Val" variable
  file.getline(buf,255);
  m_Val=strtod(buf,NULL);
  //now:"m_Hi" var
  m_Hi=new long[m_Size];
  for (i=0;i<m_Size;i++) {
    file.getline(buf,255);
    m_Hi[i]=atol(buf);
  }
  //and at least "m_Lo" var
  m_Lo=new long[m_Size];
  for (i=0;i<m_Size;i++) {
    file.getline(buf,255);
    m_Lo[i]=atol(buf);
  }

}

CTerm::~CTerm()
{
  delete m_Hi;
  delete m_Lo;
  delete m_Center;
}

void CTerm::operator =(const CTerm &src)
{
  m_Size = src.m_Size;
  m_Hi = new long[m_Size];
  m_Lo = new long[m_Size];
  m_Center = new double[m_Size];
  m_Val = src.m_Val;
  m_StudyNum = src.m_StudyNum;
  memcpy(m_Lo, src.m_Lo, m_Size*sizeof(long));
  memcpy(m_Hi, src.m_Hi, m_Size*sizeof(long));
  memcpy(m_Center, src.m_Center, m_Size*sizeof(double));
}

bool CTerm::Compare(CTerm &box)
{
  if(m_Size!=box.m_Size)
    return false;
  for(int i=0; i<m_Size; i++)
    {
      if(m_Center[i]!=box.m_Center[i])
	return false;
    }
  return true;
}

void CTerm::CorrectBox(const CTerm &box)
{
  m_StudyNum++;
  m_Val = m_Val*(m_StudyNum-1)/m_StudyNum+box.m_Val/m_StudyNum;
}
void CTerm::Save(ofstream& file)
{	
  //save one term to a file,specified by "file"
  int i;
  file<<m_StudyNum<<" ";
  for (i=0;i<m_Size;i++) file<<m_Center[i]<<" ";

  file<<m_Val<<" ";
  for (i=0;i<m_Size;i++) file<<m_Hi[i]<<" ";

  for (i=0;i<m_Size;i++) file<<m_Lo[i]<<" ";
  file<<endl;    
}

void CTerm::Load(ifstream& file) {
  int i;
  file>>this->m_StudyNum;
	
  for(i =0;i<m_Size;i++) file>>this->m_Center[i];
  file>>this->m_Val;

  for(i =0;i<m_Size;i++) file>>this->m_Hi[i];

  for(i =0;i<m_Size;i++) file>>this->m_Lo[i];
}
