/*************************************************************************
 *
 *    DESCRIPTION :    interface for the CProcessor class.
 *
 *    FILE 	 : Processor.h
 *
 *    AUTHOR     : 
 *
 *    $Revision: 2.3 $
 *
 *    $Id: Processor.h,v 2.3 2004/03/05 07:03:44 anton Exp $
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

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "Box.h"
#include <fstream>	
class CProcessor  
{
public:
  bool LoadExample(const char* file_name);
  bool SaveData(const char* filename);
  bool LoadData(const char* filename);
  void LoadData(ifstream& file);
  bool IsInZone(double* coord, int ind);
  void Init(int size);
  void ReNormalize();
  bool GetExamp(int index, double *coord, double& val);
  void Normalize();
  int Add(double* coord, double val);
  int SetExamp(int i,double *coord, double val);
  double Process(double *coord, int& processed);
  double Process(int ind, double *coord);
  void Correct(int ind);
  int m_Size;
  CProcessor(int size);
  CTermArray m_Boxes;
  CProcessor();
  virtual ~CProcessor();
  void operator =(CProcessor &proc);
};


#endif
