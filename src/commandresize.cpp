/***************************************************************************
                          commandresize.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
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

// qt includes
#include <qpainter.h>
// application includes
#include "commandresize.h"
#include "pineditdoc.h"
#include "view2d.h"
// emilia
#include "Private.h"
#include "Shape3D.h"
#include "EMath.h"

CommandResize::CommandResize(PinEditDoc * doc) : Command(doc) {
}

CommandResize::~CommandResize() {
}

void CommandResize::execute(const CommandContext & context) {
	assert(context.shape != NULL);
	p_Context->copy(context);

	float dx = context.x2 - context.x1;
	float dy = context.y2 - context.y1;
	float dz = context.z2 - context.z1;
	int index = 0;
	Vertex3D * vtx = context.shape->getVertex3D(p_Doc->getSelectedVertex(index));
	while (vtx != NULL) {
		m_vVertex.push_back(*vtx);
		m_vIndex.push_back(p_Doc->getSelectedVertex(index));
		vtx->x *= (float)(PE_RESIZE_FACTOR + dx)/PE_RESIZE_FACTOR;
		vtx->y *= (float)(PE_RESIZE_FACTOR + dy)/PE_RESIZE_FACTOR;
		vtx->z *= (float)(PE_RESIZE_FACTOR + dz)/PE_RESIZE_FACTOR;
		index++;
		vtx = context.shape->getVertex3D(p_Doc->getSelectedVertex(index));
	}

	//p_Context = new CommandContext(context);
	p_Doc->setModified(true);
	p_Doc->updateAll("polygon");
	p_Doc->pushUndo(this);
	EM_CERR("CommandResize::execute " << context.x1 <<" "<< context.y1 <<" "<< context.z1 << 
		" to " << context.x2 <<"  "<< context.y2 <<" "<< context.z2);
}

void CommandResize::preview(const CommandContext & context, View2D * view2d, QPainter & painter) {
	assert(view2d != NULL);
    if (context.shape == NULL) return;
	float dx = context.x2 - context.x1;
	float dy = context.y2 - context.y1;
	float dz = context.z2 - context.z1;
    painter.setPen(Qt::green);
	int index = 0;
	Vertex3D * vtx = context.shape->getVertex3D(p_Doc->getSelectedVertex(index));
	Vertex3D vtxA;
	while (vtx != NULL) {
		// the matrix will rotate and reapply the translation
		vtxA.x = vtx->x * (float)(PE_RESIZE_FACTOR + dx)/PE_RESIZE_FACTOR;
		vtxA.y = vtx->y * (float)(PE_RESIZE_FACTOR + dy)/PE_RESIZE_FACTOR;
		vtxA.z = vtx->z * (float)(PE_RESIZE_FACTOR + dz)/PE_RESIZE_FACTOR;
		//EM_CERR(vtx->x <<" "<< vtx->y <<" "<< vtx->z);
		switch (context.type) {
        case XY: painter.drawRect(view2d->screenx(vtxA.x)-1,
							view2d->screeny(-vtxA.y)-1, 
							3, 3); break;
        case XZ: painter.drawRect(view2d->screenx(vtxA.x)-1,
							view2d->screeny(vtxA.z)-1, 
							3, 3); break;
        case ZY: painter.drawRect(view2d->screenx(vtxA.z)-1,
							view2d->screeny(-vtxA.y)-1, 
							3, 3); break;
		}
		index++;
		vtx = context.shape->getVertex3D(p_Doc->getSelectedVertex(index));
    }
	EM_CERR("commandmove::preview");
}

void CommandResize::undo() {
	EM_CERR("CommandResize::undo");
	assert(p_Context->shape != NULL);
	assert(m_vVertex.size() == m_vIndex.size());
	vector<int>::iterator indexiter = m_vIndex.begin();
	vector<int>::iterator indexend = m_vIndex.end();
	vector<Vertex3D>::iterator vtxiter = m_vVertex.begin();
	for (; indexiter != indexend; ++indexiter, ++vtxiter) {
		Vertex3D * vtx = p_Context->shape->getVertex3D(*indexiter);
		assert(vtx != NULL);
		vtx->x = (*vtxiter).x;
		vtx->y = (*vtxiter).y;
		vtx->z = (*vtxiter).z;
	}
	p_Doc->updateAll("polygon");
}

Command * CommandResize::build() {
	return new CommandResize(p_Doc);
}
