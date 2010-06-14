/*
  Copyright (C) 2008-2010 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include <stable.h>

#ifndef MainScene_H
#define MainScene_H

#include <GraphicsScene.h>
#include <GraphicsWindow.h>
#include <MainPanel.h>

namespace appetizer {


class MainScene : public GraphicsScene {

  Q_OBJECT

public:

  MainScene(GraphicsWindow* parentWindow);
  MainPanel* mainPanel() const;

protected:

  void drawBackground(QPainter* painter, const QRectF& rect);

private:

  struct ResizeDragData {
    QPoint startMouse;
    QSize startSize;
  };

  MainPanel* mainPanel_;
  GraphicsItem* resizeSprite_;
  ResizeDragData resizeDragData_;

public slots:

  void resizeSprite_mousePressed();
  void resizeSprite_mouseMoved();

};

}
#endif // MainScene_H
