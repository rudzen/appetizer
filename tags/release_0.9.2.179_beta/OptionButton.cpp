/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "OptionButton.h"
#include "Controller.h"
#include "FilePaths.h"


extern Controller gController;


OptionButton::OptionButton(wxWindow *owner, int id, wxPoint point, wxSize size):
ImageButton(owner, id, point, size) {
  LoadImage(FilePaths::SkinDirectory + _T("/OptionButton"));
  FitToImage();
}