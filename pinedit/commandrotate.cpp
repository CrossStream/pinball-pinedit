/***************************************************************************
                          commandrotate.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Henrik Enqvist IB
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
#include "commandrotate.h"
#include "pineditdoc.h"
#include "view2d.h"
// emilia includes
#include "Private.h"
#include "Shape3D.h"
#include "EMath.h"

CommandRotate::CommandRotate(PinEditDoc * doc) : Command(doc) {
}

CommandRotate::~CommandRotate() {
}

void CommandRotate::execute(const CommandContext & context) {
	assert(context.shape != NULL);

	Matrix mtxA = EMath::identityMatrix;
	Vertex3D vtxTA = {0,0,0}, vtxRA = {0,0,0};
	switch (context.type) {
	case XY: vtxRA.z = (context.x2-context.x1)/PE_ROTATION_FACTOR; break;
	case XZ: vtxRA.y = (context.x2-context.x1)/PE_ROTATION_FACTOR; break;
	case ZY: vtxRA.x = (context.z2-context.z1)/PE_ROTATION_FACTOR; break;
	}
	EMath::getTransformationMatrix(mtxA, vtxTA, vtxRA);

	int index = 0;
	Vertex3D * vtx = context.shape->getVertex3D(p_Doc->getSelectedVertex(index));
	Vertex3D vtxTmp;
	while (vtx != NULL) {
		EMath::applyMatrixRot(mtxA, *vtx, vtxTmp);
		*vtx = vtxTmp;
		// TODO rotation
		index++;
		vtx = context.shape->getVertex3D(p_Doc->getSelectedVertex(index));
	}

	//p_Context = new CommandContext(context);
	p_Doc->setModified(true);
	p_Doc->updateAll();
	p_Doc->pushUndo(this);
	cerr << "CommandRotate::execute" << endl;
}

void CommandRotate::preview(const CommandContext & context, View2D * view2d) {
	if (context.shape == NULL) return;
	// build matrix stack for temporary translation
	// mtxB is global rotion matrix, mtxC fixes the translation in local rotation
	// mtxC is the final matrix
	Matrix mtxA = EMath::identityMatrix;
	Vertex3D vtxTA = {0,0,0}, vtxRA = {0,0,0};
	
	switch (context.type) {
	case XY: vtxRA.z = (context.x2-context.x1)/PE_ROTATION_FACTOR; break;
	case XZ: vtxRA.y = (context.x2-context.x1)/PE_ROTATION_FACTOR; break;
	case ZY: vtxRA.x = (context.z2-context.z1)/PE_ROTATION_FACTOR; break;
	}
	EMath::getTransformationMatrix(mtxA, vtxTA, vtxRA);

	// draw selected polygons
	view2d->getPainter()->setPen(Qt::green);
	int index = 0;
	Polygon * poly = p_Doc->getSelectedPolygon(index);
	while (poly != NULL) {
		view2d->drawPolygon(context.shape, poly, mtxA);
		index++;
		poly = p_Doc->getSelectedPolygon(index);
	}
	// draw selected vertices
	index = 0;
	Vertex3D * vtx = context.shape->getVertex3D(p_Doc->getSelectedVertex(index));
	Vertex3D vtxA;
	while (vtx != NULL) {
		// the matrix will rotate and reapply the translation
		view2d->drawVertex(context.shape, *vtx, mtxA);
		index++;
		vtx = context.shape->getVertex3D(p_Doc->getSelectedVertex(index));
	}
	cerr << "CommandRotate::preview" << endl;
}

void CommandRotate::undo() {
}

Command * CommandRotate::build() {
	return new CommandRotate(p_Doc);
}
