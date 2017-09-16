/*************************************************************************
 *
 *    DESCRIPTION :    implementation of the CProcessor class.
 *
 *    FILE 	 : Processor.C
 *
 *    AUTHOR     : 
 *
 *    $Revision: 2.4 $
 *
 *    $Id: Processor.C,v 2.4 2004/08/29 14:07:21 anton Exp $
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
#include "Processor.h"
#include <fstream>
#include <iostream>

#define SIGN(n) (n>=0 ? 1 : -1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessor::CProcessor()
{

}

CProcessor::~CProcessor()
{
}

CProcessor::CProcessor(int size)
{
  m_Size = size;
}

double CProcessor::Process(double *coord, int& processed)
{

  double cur;
  double val = 0;
  double div = 0;
  for(unsigned int i=0; i<m_Boxes.size(); i++)
    {
      cur = Process(i, coord);
      val += m_Boxes[i].m_Val*cur;
      div += cur;
    }
  return div ? val/div : 0;
}

double CProcessor::Process(int ind, double *coord)
{
  if(!IsInZone(coord, ind)) return 0;
  double val, min = 1;
  int num;
  for(int i=0; i<m_Size; i++)
    {
      if(coord[i]>m_Boxes[ind].m_Center[i])
	num = m_Boxes[ind].m_Hi[i];
      else
	num = m_Boxes[ind].m_Lo[i];
      if(num==-1)
	val = 1;
      else
	val = (m_Boxes[num].m_Center[i]-coord[i])/(m_Boxes[num].m_Center[i]-m_Boxes[ind].m_Center[i]);
      if(min>val)
	min = val;
    }
  return min;
}

int CProcessor::Add(double *coord, double val)
{
  CTerm box(m_Size, val, coord);
  for( unsigned int j=0; j<m_Boxes.size(); j++)
    {
      if(box.Compare(m_Boxes[j]))
	{
	  m_Boxes[j].CorrectBox(box);
	  return j;
	}
    }
	
  m_Boxes.push_back(box);
  Correct(m_Boxes.size()-1);
  return -1;
}

int CProcessor::SetExamp(int i,double *coord, double val)
{
  CTerm box(m_Size, val, coord);
  for( unsigned int j=0; j<m_Boxes.size(); j++)
    {
      if(j!=(unsigned int)i && box.Compare(m_Boxes[j]))
	return j;
    }
  m_Boxes[i] = box;
  return -1;
}

void CProcessor::Normalize()
{
  for( unsigned int i=0; i<m_Boxes.size(); i++)
    {
      Correct(i);
    }
}

void CProcessor::Correct(int ind)

{
  int num;
  double delta;
  for( unsigned int i=0; i<m_Boxes.size(); i++)
    {
      if(i==(unsigned int)ind)
	continue;
      if(IsInZone(m_Boxes[ind].m_Center, i))
	{
	  delta = 0;
	  for(int j=0; j<m_Size; j++)
	    {
	      double d = m_Boxes[ind].m_Center[j]-m_Boxes[i].m_Center[j];
	      if(fabs(d)>fabs(delta))
		{
		  num = j;
		  delta = d;
		}
	    }
	  if(delta>0)
	    m_Boxes[i].m_Hi[num] = ind;
	  else
	    m_Boxes[i].m_Lo[num] = ind;
	}
      if(IsInZone(m_Boxes[i].m_Center, ind))
	{
	  delta = 0;
	  for(int j=0; j<m_Size; j++)
	    {
	      double d = m_Boxes[i].m_Center[j]-m_Boxes[ind].m_Center[j];
	      if(fabs(d)>fabs(delta))
		{
		  num = j;
		  delta = d;
		}
	    }
	  if(delta>0)
	    m_Boxes[ind].m_Hi[num] = i;
	  else
	    m_Boxes[ind].m_Lo[num] = i;
	}
    }
}

void CProcessor::operator =(CProcessor &proc)
{
  m_Size = proc.m_Size;
  m_Boxes = proc.m_Boxes;
}

bool CProcessor::GetExamp(int index, double *coord, double& val)
{
  if(index<0 || (unsigned int)index>=m_Boxes.size())
    return false;
  for(int i=0;i<m_Size;i++)
    {
      coord[i] = m_Boxes[index].m_Center[i];
      val = m_Boxes[index].m_Val;
    }
  return true;
}

void CProcessor::ReNormalize()
{
  for(unsigned int i=0; i<m_Boxes.size(); i++)
    {
      for(int j=0; j<m_Size; j++)
	m_Boxes[i].m_Lo[j] = m_Boxes[i].m_Hi[j] = -1;
    }
  Normalize();
}

void CProcessor::Init(int size)
{
  m_Boxes.clear();
  m_Size = size;
}

bool CProcessor::IsInZone(double *coord, int ind)
{
  int num;
  for(int i=0; i<m_Size; i++)
    {
      if(coord[i]>m_Boxes[ind].m_Center[i])
	num = m_Boxes[ind].m_Hi[i];
      else
	num = m_Boxes[ind].m_Lo[i];
      if(num!=-1)
	{
	  if((coord[i]-m_Boxes[ind].m_Center[i])*(coord[i]-m_Boxes[num].m_Center[i])>0)
	    return false;
	}
    }
  return true;
}

bool CProcessor::LoadExample(const char *file_name)
{
  return true;
}
bool CProcessor::SaveData(const char* filename) {
  ofstream out(filename);
  int size = m_Boxes.size();

  out<<size<<" "<<m_Size<<endl;
	
  for (int i=0;i<size;i++) m_Boxes[i].Save(out);
  out.close();
  return true;
}
bool CProcessor::LoadData(const char* filename)
{
  ifstream in;
  in.open(filename);
  if(!in) return false;
    	    
  int num_of_terms;
  int size;
	
  in>>num_of_terms;
  in>>size;
	
  CTerm term(size);
  Init(size);
	
  for(int j = 0;j<num_of_terms;j++) {
    term.Load(in);	
    m_Boxes.push_back(term);
  }	
  in.close();
  return true;
}
void CProcessor::LoadData(ifstream& file){
  int num_of_terms;
  int size;
	
  file>>num_of_terms;
  file>>size;
	
  CTerm term(size);
  Init(size);
	
  for(int j = 0;j<num_of_terms;j++) {
    term.Load(file);	
    m_Boxes.push_back(term);
  }	
}
