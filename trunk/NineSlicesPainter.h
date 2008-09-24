#ifndef __NineSlicesPainter_H
#define __NineSlicesPainter_H

#include "wx/wx.h" 
#include "Imaging.h"


class NineSlicesPainter {

  private:

    wxRect grid_;
    wxString filePath_;
    bool gridIsExplicitelySet_;
    wxBitmap sourceBitmap_;
    wxMemoryDC sourceDC_;
    
  public:

    NineSlicesPainter();
    void LoadImage(const wxString& filePath);
    void SetGrid(wxRect *grid);
    void Draw(wxDC *destDC, wxCoord x, wxCoord y, wxCoord width, wxCoord height);

};

#endif