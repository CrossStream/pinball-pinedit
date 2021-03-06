/***************************************************************************
                          view2d.cpp  -  description
                             -------------------
    begin                : Wed Apr 10 2002
    copyright            : (C) 2002 by Henrik Enqvist
    email                : henqvist@excite.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// general includes
#include <iostream>
#include <assert.h>
// qt includes
#include <qpainter.h>
#include <qpen.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
// application includes
#include "view2d.h"
#include "pinedit.h"
#include "pineditdoc.h"
#include "command.h"
#include "pineditview.h"
// emilia includes
#include "Private.h"
#include "Engine.h"
#include "Group.h"
#include "Shape3D.h"
#include "Polygon.h"
#include "EMath.h"

View2D::View2D(int type, PinEditDoc * doc, QWidget * parent, const char * name, Qt::WFlags f) 
  : QWidget(parent, name, f ) {
  assert(doc != NULL);
  p_Doc = doc;
  p_Doc->registerUpdateable(this, "polygon");

  m_iMode = EM_SHAPE_MODE;
  m_bGrid = true;
  m_fZoom = 10;
  m_iXOffset = 150;
  m_iYOffset = 120;
  m_iMouse = 0;
  m_iType = type;
  m_iX1 = m_iY1 = m_iX2 = m_iY2 = 0;
  p_Context = new CommandContext();
  this->setBackgroundMode(Qt::PaletteBase);
}

View2D::~View2D(){
}

void View2D::resizeEvent(QResizeEvent *) {
  EM_CERR("View2D::resizeEvent");
}

void View2D::mousePressEvent(QMouseEvent * e) {
  EM_CERR("View2D::mousePressEvent");
  m_iX2 = m_iX1 = e->x();
  m_iY2 = m_iY1 = e->y();
  if (e->button() ==  Qt::LeftButton) {
    m_iMouse = 1;
  } else if (e->button() == Qt::RightButton) {
    m_iMouse = 2;
  } else if (e->button() == Qt::MidButton) {
    m_iMouse = 0;
    m_iXOffset = this->size().width()/2;
    m_iYOffset = this->size().height()/2;
  }
}

void View2D::mouseReleaseEvent(QMouseEvent * e) {
  EM_CERR("View2D::mouseReleaseEvent");
  m_iMouse = 0;
  m_iX2 = e->x();
  m_iY2 = e->y();
  if (e->button() ==  Qt::LeftButton) {
    CommandContext context;
    context.clear();
    Command * command = p_Doc->buildCommand();
    if (command != NULL) {
      switch (m_iType) {
      case XY: 
	context.x1 = this->localx(m_iX1);	context.x2 = this->localx(m_iX2);
	context.y1 = -this->localy(m_iY1); context.y2 = -this->localy(m_iY2);
	context.z1 = 0;             context.z2 = 0; 
	break;
      case XZ: 
	context.x1 = this->localx(m_iX1);	context.x2 = this->localx(m_iX2);
	context.y1 = 0;             context.y2 = 0;
	context.z1 = this->localy(m_iY1); context.z2 = this->localy(m_iY2); 
	break;
      case ZY: 
	context.x1 = 0;             context.x2 = 0;
	context.y1 = -this->localy(m_iY1); context.y2 = -this->localy(m_iY2);
	context.z1 = this->localx(m_iX1); context.z2 = this->localx(m_iX2); 
	break;
      }
      context.shape = p_Doc->getCurrentShape();
      context.group = p_Doc->getCurrentGroup();
      context.type = m_iType;
      if (m_iMode == EM_SHAPE_MODE && context.shape == NULL) {
	QMessageBox::information(this, "Command", "No Shape selected.");
      } else if (m_iMode == EM_GROUP_MODE && context.group == NULL) {
	QMessageBox::information(this, "Command", "No Group selected.");
      } else {
	command->execute(context);
      }
    }
  } else if (e->button() == Qt::RightButton) {
  } else if (e->button() == Qt::MidButton) {
  }
  m_iX1 = m_iX2;
  m_iY1 = m_iY2;
  this->repaint();
}

void View2D::mouseMoveEvent(QMouseEvent* e) {
  if (m_iMouse == 1) {
  }
  if (m_iMouse == 2) {
    m_iXOffset += (e->x() - m_iX2);
    m_iYOffset += (e->y() - m_iY2);
  }
  m_iX2 = e->x();
  m_iY2 = e->y();
  this->repaint();
  //	PinEditApp::p_CurrentApp->statusBar()->message(QString().setNum(this->localx(m_iX2)) + " " + 
  //																								 QString().setNum(this->localy(m_iY2)));
}

void View2D::drawPolygon(QPainter &painter, Shape3D * shape, Polygon3D * poly, const Matrix & mtx) {
  //EM_CERR("View2D::drawPolygon");
  assert(shape != NULL);
  assert(poly != NULL);
  assert(shape->getPolygonIndex(poly) >= 0);
  // find first vertex
  int polyindex = 0;
  int vtxindex = poly->getIndex(polyindex);
  Vertex3D * vtx0 = shape->getVertex3D(vtxindex);
  // loop throgh the rest
  if (vtx0 != NULL) {
    polyindex++;
    vtxindex = poly->getIndex(polyindex);
    Vertex3D vtx1 = *vtx0;
    Vertex3D vtx2 = *vtx0; 
    Vertex3D vtxTrans1;
    Vertex3D vtxTrans2;
    while (vtxindex != -1) {
      vtx1 = vtx2;
      vtx2 = *(shape->getVertex3D(vtxindex));
      EMath::applyMatrix(mtx, vtx1, vtxTrans1);
      EMath::applyMatrix(mtx, vtx2, vtxTrans2);
      switch (m_iType) {
      case XY: painter.drawLine(this->screenx(vtxTrans1.x), this->screeny(-vtxTrans1.y),
				    this->screenx(vtxTrans2.x), this->screeny(-vtxTrans2.y)); 
	break;
      case XZ: painter.drawLine(this->screenx(vtxTrans1.x), this->screeny(vtxTrans1.z),
				    this->screenx(vtxTrans2.x), this->screeny(vtxTrans2.z)); 
	break;
      case ZY: painter.drawLine(this->screenx(vtxTrans1.z), this->screeny(-vtxTrans1.y),
				    this->screenx(vtxTrans2.z), this->screeny(-vtxTrans2.y));
	break;
      }
      polyindex++;
      vtxindex = poly->getIndex(polyindex);
    }
    // the line from the last to the first
    vtx1 = *vtx0;
    EMath::applyMatrix(mtx, vtx1, vtxTrans1);
    EMath::applyMatrix(mtx, vtx2, vtxTrans2);
    switch (m_iType) {
    case XY: painter.drawLine(this->screenx(vtxTrans2.x), this->screeny(-vtxTrans2.y),
				  this->screenx(vtxTrans1.x), this->screeny(-vtxTrans1.y)); 
      break;
    case XZ: painter.drawLine(this->screenx(vtxTrans2.x), this->screeny(vtxTrans2.z),
				  this->screenx(vtxTrans1.x), this->screeny(vtxTrans1.z)); 
      break;
    case ZY: painter.drawLine(this->screenx(vtxTrans2.z), this->screeny(-vtxTrans2.y),
				  this->screenx(vtxTrans1.z), this->screeny(-vtxTrans1.y)); 
      break;
    }
  }
}

void View2D::drawVertex(QPainter &painter, Shape3D * shape, const Vertex3D & vtx, const Matrix & mtx) {
  //EM_CERR("View2D::drawPolygon");
  assert(shape != NULL);
  Vertex3D vtxA;
  EMath::applyMatrix(mtx, vtx, vtxA);
  switch (m_iType) {
  case XY: painter.drawRect(this->screenx(vtxA.x)-1, this->screeny(-vtxA.y)-1, 3, 3); break;
  case XZ: painter.drawRect(this->screenx(vtxA.x)-1, this->screeny(vtxA.z)-1,	3, 3); break;
  case ZY: painter.drawRect(this->screenx(vtxA.z)-1, this->screeny(-vtxA.y)-1, 3, 3); break;
  }
}

void View2D::drawShapeMode(QPainter &painter) {
  EM_CERR("View2D::drawShapeMode");
  // draw other shapes in group with lightgray
  if (p_Doc->getCurrentGroup() == NULL) return;

  int sh = 0;
  Shape3D * shape = p_Doc->getCurrentGroup()->getShape3D(sh);
  while (shape != NULL) {
    painter.setPen(Qt::lightGray);
    int index = 0;
    Vertex3D * vtx = shape->getVertex3D(index);
    while (vtx != NULL) {
      //EM_CERR(vtx->x <<" "<< vtx->y <<" "<< vtx->z);
      switch (m_iType) {
      case XY: painter.drawRect(this->screenx(vtx->x)-1, this->screeny(-vtx->y)-1, 3, 3); break;
      case XZ: painter.drawRect(this->screenx(vtx->x)-1, this->screeny(vtx->z)-1, 3, 3); break;
      case ZY: painter.drawRect(this->screenx(vtx->z)-1, this->screeny(-vtx->y)-1, 3, 3); break;
      }
      index++;
      vtx = shape->getVertex3D(index);
    }
    index = 0;
    Polygon3D * poly = shape->getPolygon(index);
    while (poly != NULL) {
      this->drawPolygon(painter, shape, poly, EMath::identityMatrix);
      index++;
      poly = shape->getPolygon(index);
    }
    ++sh;
    shape = p_Doc->getCurrentGroup()->getShape3D(sh);
  }
  // current shape
  shape = p_Doc->getCurrentShape();
  if (shape == NULL) {
    return;
  }
  {	// normal vertices
    painter.setPen(Qt::black);
    int index = 0;
    Vertex3D * vtx = shape->getVertex3D(index);
    while (vtx != NULL) {
      //EM_CERR(vtx->x <<" "<< vtx->y <<" "<< vtx->z);
      switch (m_iType) {
      case XY: painter.drawRect(this->screenx(vtx->x)-1, this->screeny(-vtx->y)-1, 3, 3); break;
      case XZ: painter.drawRect(this->screenx(vtx->x)-1, this->screeny(vtx->z)-1, 3, 3); break;
      case ZY: painter.drawRect(this->screenx(vtx->z)-1, this->screeny(-vtx->y)-1, 3, 3); break;
      }
      index++;
      vtx = shape->getVertex3D(index);
    }
  }	

  { // selected vertices
    painter.setPen(Qt::red);
    int index = 0;
    Vertex3D * vtx = shape->getVertex3D(p_Doc->getSelectedVertex(index));
    while (vtx != NULL) {
      //EM_CERR(vtx->x <<" "<< vtx->y <<" "<< vtx->z);
      switch (m_iType) {
      case XY: painter.drawRect(this->screenx(vtx->x)-1, this->screeny(-vtx->y)-1, 3, 3); break;
      case XZ: painter.drawRect(this->screenx(vtx->x)-1, this->screeny(vtx->z)-1, 3, 3); break;
      case ZY: painter.drawRect(this->screenx(vtx->z)-1, this->screeny(-vtx->y)-1, 3, 3); break;
      }
      index++;
      vtx = shape->getVertex3D(p_Doc->getSelectedVertex(index));
    }
    vtx = shape->getVertex3D(p_Doc->getSelectedVertexExtra());
    if (vtx != NULL) {
      switch (m_iType) {
      case XY: painter.drawRect(this->screenx(vtx->x)-2, this->screeny(-vtx->y)-1, 5, 5); break;
      case XZ: painter.drawRect(this->screenx(vtx->x)-2, this->screeny(vtx->z)-1, 5, 5); break;
      case ZY: painter.drawRect(this->screenx(vtx->z)-2, this->screeny(-vtx->y)-1, 5, 5); break;
      }
    }
  }

  { // normal polygons
    painter.setPen(Qt::black);
    int index = 0;
    Polygon3D * poly = shape->getPolygon(index);
    while (poly != NULL) {
      this->drawPolygon(painter, shape, poly, EMath::identityMatrix);
      index++;
      poly = shape->getPolygon(index);
    }
  }

  { // selected polygons
    painter.setPen(Qt::red);
    int index = 0;
    Polygon3D * poly = p_Doc->getSelectedPolygon(index);
    while (poly != NULL) {
      this->drawPolygon(painter, shape, poly, EMath::identityMatrix);
      index++;
      poly = p_Doc->getSelectedPolygon(index);
    }
  }

  CommandContext context;
  context.clear();
  // TODO
  //Command * command = p_Doc->buildCommand();
  Command * command = p_Doc->getCommand();
  if (command != NULL) {
    switch (m_iType) {
    case XY: 
      context.x1 = this->localx(m_iX1);	context.x2 = this->localx(m_iX2);
      context.y1 = -this->localy(m_iY1); context.y2 = -this->localy(m_iY2);
      context.z1 = 0;             context.z2 = 0; 
      break;
    case XZ: 
      context.x1 = this->localx(m_iX1);	context.x2 = this->localx(m_iX2);
      context.y1 = 0;             context.y2 = 0;
      context.z1 = this->localy(m_iY1); context.z2 = this->localy(m_iY2); 
      break;
    case ZY: 
      context.x1 = 0;             context.x2 = 0;
      context.y1 = -this->localy(m_iY1); context.y2 = -this->localy(m_iY2);
      context.z1 = this->localx(m_iX1); context.z2 = this->localx(m_iX2); 
      break;
    }
    context.sx1 = m_iX1;
    context.sy1 = m_iY1;
    context.sx2 = m_iX2;
    context.sy2 = m_iY2;
    context.shape = p_Doc->getCurrentShape();
    context.group = p_Doc->getCurrentGroup();
    context.type = m_iType;
    command->preview(context, this, painter);
  }
}

void View2D::drawGroupMode(QPainter &painter) {
  EM_CERR("View2D::drawGroupMode");
  Matrix mtx = EMath::identityMatrix;
  // draw all groups
  this->drawGroup(painter, p_Doc->getEngine(), mtx);
}

/* input: group to draw, matrix stack before applying this group */
void View2D::drawGroup(QPainter &painter, Group * g, const Matrix & mtxP) {
  //EM_CERR("View3D::drawGroup");
  Matrix mtxT;
  EMath::matrixMulti(g->m_mtxSrc, mtxP, mtxT);
  // draw a cross to notate the centre of the group
  if (g == p_Doc->getCurrentGroup()) {
    painter.setPen(Qt::red);
  } else {
    painter.setPen(Qt::black);
  }
  switch (m_iType) {
  case XY: 
    painter.drawLine(this->screenx(mtxT.t[0])-2, this->screeny(-mtxT.t[1]),
			 this->screenx(mtxT.t[0])+2, this->screeny(-mtxT.t[1]));
    painter.drawLine(this->screenx(mtxT.t[0]), this->screeny(-mtxT.t[1])-2,
			 this->screenx(mtxT.t[0]), this->screeny(-mtxT.t[1])+2); break;
  case XZ: 
    painter.drawLine(this->screenx(mtxT.t[0])-2, this->screeny(mtxT.t[2]),
			 this->screenx(mtxT.t[0])+2, this->screeny(mtxT.t[2]));
    painter.drawLine(this->screenx(mtxT.t[0]), this->screeny(mtxT.t[2])-2,
			 this->screenx(mtxT.t[0]), this->screeny(mtxT.t[2])+2); break;
  case ZY: 
    painter.drawLine(this->screenx(mtxT.t[2])-2, this->screeny(-mtxT.t[1]),
			 this->screenx(mtxT.t[2])+2, this->screeny(-mtxT.t[1]));
    painter.drawLine(this->screenx(mtxT.t[2]), this->screeny(-mtxT.t[1])-2,
			 this->screenx(mtxT.t[2]), this->screeny(-mtxT.t[1])+2); break;
  }

  // draw all polygons
  int shindex = 0;
  Shape3D * shape = g->getShape3D(shindex);
  while (shape != NULL) {
    int polyindex = 0;
    Polygon3D * poly = shape->getPolygon(polyindex);
    while (poly != NULL) {
      this->drawPolygon(painter, shape, poly, mtxT);
      polyindex++;
      poly = shape->getPolygon(polyindex);
    }
    shindex++;
    shape = g->getShape3D(shindex);
  }

  CommandContext context;
  context.clear();
  // TODO
  //Command * command = p_Doc->buildCommand();
  Command * command = p_Doc->getCommand();
  if (command != NULL) {
    switch (m_iType) {
    case XY: 
      context.x1 = this->localx(m_iX1);	context.x2 = this->localx(m_iX2);
      context.y1 = -this->localy(m_iY1); context.y2 = -this->localy(m_iY2);
      context.z1 = 0;             context.z2 = 0; 
      break;
    case XZ: 
      context.x1 = this->localx(m_iX1);	context.x2 = this->localx(m_iX2);
      context.y1 = 0;             context.y2 = 0;
      context.z1 = this->localy(m_iY1); context.z2 = this->localy(m_iY2); 
      break;
    case ZY: 
      context.x1 = 0;             context.x2 = 0;
      context.y1 = -this->localy(m_iY1); context.y2 = -this->localy(m_iY2);
      context.z1 = this->localx(m_iX1); context.z2 = this->localx(m_iX2); 
      break;
    }
    context.sx1 = m_iX1;
    context.sy1 = m_iY1;
    context.sx2 = m_iX2;
    context.sy2 = m_iY2;
    context.shape = p_Doc->getCurrentShape();
    context.group = p_Doc->getCurrentGroup();
    context.type = m_iType;
    command->preview(context, this, painter);
  }
  // TODO recurse the temporary movement to child groups
  int index = 0;
  Group * group = g->getGroup(index);
  while (group != NULL) {
    this->drawGroup(painter, group, mtxT);
    index++;
    group = g->getGroup(index);
  }
}

void View2D::drawGrid(QPainter& painter) {
  //EM_CERR("View2D::drawGrid");
  QString str = QString().setNum(this->localx(m_iX2)) + " " + 
    QString().setNum(this->localy(m_iY2));
  painter.setPen(Qt::lightGray);
  float step = m_fZoom;
  int w = this->size().width();
  int h = this->size().height();
  int w_2 = w/2;
  int h_2 = h/2;
  float xoffset = (m_iXOffset - w_2)%(int)m_fZoom;
  float yoffset = (m_iYOffset - h_2)%(int)m_fZoom;
  for (float x=w_2; x<w; x += step) {
    painter.drawLine((int)(x+xoffset), 0, (int)(x+xoffset), h);
   }
  for (float x=w_2-step; x>0; x-=step) {
     painter.drawLine((int)(x+xoffset), 0, (int)(x+xoffset), h);
  }
  for (float y=h_2; y<h; y+=step) {
     painter.drawLine(0, (int)(y+yoffset), w, (int)(y+yoffset));
  }
  for (float y=h_2-step; y>0; y-=step) {
    painter.drawLine(0, (int)(y+yoffset), w, (int)(y+yoffset));
  }
  
  painter.setPen(Qt::darkGray);
  painter.drawLine(this->screenx(-100), this->screeny(0),
		       this->screenx(100), this->screeny(0));
  painter.drawLine(this->screenx(0), this->screeny(-100),
 		       this->screenx(0), this->screeny(100));
  // drawing text triggers strange no-more resize qt bug!
  //p_QPainter->setPen(Qt::black);
  //p_QPainter->drawText(0, 12, str, -1, QPainter::LTR);
}

// this is the function who draws stuff to the screen
void View2D::paintEvent(QPaintEvent *) {
  //EM_CERR("view2d::paintevent");

    QPainter painter(this);
  if (m_bGrid) {
    this->drawGrid(painter);
  }
  if (m_iMode == EM_SHAPE_MODE) {
    this->drawShapeMode(painter);
  } else if (m_iMode == EM_GROUP_MODE) {
    this->drawGroupMode(painter);
  }
}

void View2D::doUpdate() {
  EM_CERR("View2D::doUpdate");
  this->repaint();
}
