/***************************************************************************
                          commandmirror.cpp  -  description
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

// application includes
#include "commandmirror.h"
#include "pineditdoc.h"
// emilia includes
#include "Private.h"
#include "Polygon.h"
#include "Shape3D.h"

CommandMirror::CommandMirror(PinEditDoc * doc) : Command(doc) {
	m_iXYZ = 0;
}

CommandMirror::~CommandMirror() {
}

void CommandMirror::clearObjects() {
}

void CommandMirror::execute(const CommandContext & context) {
	EM_CERR("CommandMirror::execute");
	assert(context.shape != NULL);
	p_Context->copy(context);
	m_vIndex.clear();
	m_vPolygon.clear();

	// mirror
	int index = 0;
	int vtxindex = p_Doc->getSelectedVertex(index);
	while (vtxindex != -1) {
		m_vIndex.push_back(vtxindex);
		Vertex3D * vtx = context.shape->getVertex3D(vtxindex);
		assert(vtx != NULL);
 		switch (m_iXYZ) {
			case 0: vtx->x = -vtx->x; break;
			case 1: vtx->y = -vtx->y; break;
			default: vtx->z = -vtx->z; break;
		}
		++index;
		vtxindex = p_Doc->getSelectedVertex(index);
	}

	// flip polygons
	index = 0;
	Polygon * poly = p_Doc->getSelectedPolygon(index);
	while (poly != NULL) {
		m_vPolygon.push_back(poly);
		// oooh, this is a ugly and slow implementation
		vector<int> vIndex;
		vector<int>::reverse_iterator riter = poly->m_vIndex.rbegin();
		vector<int>::reverse_iterator rend = poly->m_vIndex.rend();
		// TODO mirror color and texcoord or not?
		for (; riter != rend; riter++) {
			vIndex.push_back(*riter);
		}			
		vector<int>::iterator iter = vIndex.begin();
		vector<int>::iterator end = vIndex.end();
		poly->m_vIndex.clear();
		for (; iter != end; iter++) {
			poly->m_vIndex.push_back(*iter);
		}
		poly->countNormal();
		index++;
		poly = p_Doc->getSelectedPolygon(index);
	}
	context.shape->countNormals();

	//p_Context = new CommandContext(context);
	p_Doc->setModified(true);
	p_Doc->rebuildAll("polygon");
	p_Doc->updateAll("polygon");
	p_Doc->pushUndo(this);
}

void CommandMirror::undo() {
	EM_CERR("CommandMirror::undo");
	assert(p_Context->shape != NULL);

	// mirror
	vector<int>::iterator viter = m_vIndex.begin();
	vector<int>::iterator vend = m_vIndex.end();
	for (; viter != vend; ++viter) {
		Vertex3D * vtx = p_Context->shape->getVertex3D(*viter);
		assert(vtx != NULL);
 		switch (m_iXYZ) {
			case 0: vtx->x = -vtx->x; break;
			case 1: vtx->y = -vtx->y; break;
			default: vtx->z = -vtx->z; break;
		}
	}

	// flip
	vector<Polygon*>::iterator polyiter = m_vPolygon.begin();
	vector<Polygon*>::iterator polyend = m_vPolygon.end();
	for (; polyiter != polyend; ++polyiter) {
		// oooh, this is a ugly and slow implementation
		vector<int> vIndex;
		vector<int>::reverse_iterator riter = (*polyiter)->m_vIndex.rbegin();
		vector<int>::reverse_iterator rend = (*polyiter)->m_vIndex.rend();
		// TODO mirror color and texcoord or not?
		for (; riter != rend; riter++) {
			vIndex.push_back(*riter);
		}			
		vector<int>::iterator iter = vIndex.begin();
		vector<int>::iterator end = vIndex.end();
		(*polyiter)->m_vIndex.clear();
		for (; iter != end; iter++) {
			(*polyiter)->m_vIndex.push_back(*iter);
		}
		(*polyiter)->countNormal();
	}
	p_Context->shape->countNormals();
	p_Doc->rebuildAll("polygon");
	p_Doc->updateAll("polygon");
}

Command * CommandMirror::build() {
	EM_CERR("CommandMirror::bulld");
	return new CommandMirror(p_Doc);
}
