﻿/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#ifndef __MathUtil_H
#define __MathUtil_H

#include "wx/wx.h"



class MathUtil {

  public:

    static double GetPointDistance(const wxPoint& a, const wxPoint& b);

};

#endif // __MathUtil_H