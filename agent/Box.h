/*************************************************************************
 *
 *    DESCRIPTION : interface for the CBox class.
 *
 *    FILE 	 : Box.h
 *
 *    AUTHOR     : 
 *
 *    $Revision: 2.3 $
 *
 *    $Id: Box.h,v 2.3 2004/03/05 07:03:44 anton Exp $
 *
 ************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your opfion) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//////////////////////////////////////////////////////////////////////

#ifndef _BOX_H_	
#define _BOX_H_

const int MIN_LONG = -2147483647;
const int MAX_LONG = 2147483646;
#include <vector>
#include <fstream>
using namespace std;
class CTerm  
{
public:
  void CorrectBox(const CTerm &box);
  int m_StudyNum;
  bool Compare(CTerm& box);
  int m_Size;
  CTerm(const CTerm& src);
  CTerm(int size);
  CTerm(int size, double val, double* cntr);
  CTerm(ifstream& file);
  double* m_Center;
  double m_Val;
  long* m_Hi;
  long* m_Lo;
  CTerm();
  virtual ~CTerm();
  void operator =(const CTerm &src);

  void Load(ifstream& file);
  void Save(ofstream& file);
};

typedef vector<CTerm> CTermArray;


#endif
