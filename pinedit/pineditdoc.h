/***************************************************************************
                          pineditdoc.h  -  description
                             -------------------
    begin                : Tue Nov 27 19:39:03 EET 2001
    copyright            : (C) 2001 by Henrik Enqvist IB
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
#ifndef PINEDITDOC_H
#define PINEDITDOC_H

// general includes
#include <vector>
#include <map>
// include files for QT
#include <qobject.h>
#include <qstring.h>
// application specific includes

// emilia includes
#include "EMath.h"
//#include "TextureUtil.h"

class FileUtil;
class Engine;
class Group;
class Shape3D;
class Command;
class Polygon;

/** Classes may inherit this class and register itself to the PinEditDoc
 * to get notifications every time the user performs changes on the
 * context. The doUpdate function should be a fast function. If the
 * class requiers more complex recalculation use the Rebuilable class.
 * @see Rebuildable */
class Updateable {
 public:
	Updateable() {};
	virtual ~Updateable() {};
	virtual void doUpdate() = 0;
};

/** Classes may inherit this class and register itself to the PinEditDoc
 * to get notifications every time the user performs changes on the
 * context. If doRebuild function is a fast function. You may use
 * the Updateable class instead.
 * @see Updateable */
class Rebuildable {
 public:
	Rebuildable() {};
	virtual ~Rebuildable() {};
	virtual void doRebuild() = 0;
};

/** The Document Class */
class PinEditDoc : public QObject {
  Q_OBJECT

 public:
	PinEditDoc();
	~PinEditDoc();
	void newDoc();
	bool save();
	bool saveAs(const QString & filename);
	bool saveGroup(const QString & filename, Group * group);
	bool saveShape(const QString & filename, Shape3D * shape);
	bool load(const QString & filename);
	bool loadShape(const QString & filename);
	bool loadGroup(const QString & filename);
	QImage * loadQImage(const QString & filename);

	bool isModified() const;
	void setModified(bool m) { modified = m; };  	

	void setCommand(Command * command);
	Command * buildCommand();
	const char * getCommandName();
	void pushUndo(Command * command);

	/** Adds an updateable object to the list of updatables. 
	 ** @see Updateable */
	void registerUpdateable(Updateable * u);
	/** Triggers the 'doUpdate()' function of all updateable objects. */
	void updateAll();
	/** Triggers the 'doUpdate()' function of all updateable objects but u. */
	void updateAllExclude(Updateable * u);
	/** Adds an rebuildable object to the list of rebuildables. 
	 ** @see Rebuildable */
	void registerRebuildable(Rebuildable * r);
	/** Triggers the 'doRebuild()' function of all rebuildable objects. */
	void rebuildAll();
	/** Triggers the 'doRebuild()' function of all rebuildable objects but r. */
	void rebuildAllExclude(Rebuildable * r);

	const QString & getFileName();

	Engine * getEngine();
	Group * getCameraRot();
	Group * getCameraTrans();
	Shape3D * getCurrentShape();
	Group * getCurrentGroup();
	/** Also set the current shape to the first shape in the group */
	void setCurrentGroup(Group * g);
	/** Also set the current group to the group owning the shape */
	void setCurrentShape(Shape3D * s);
	
	void clearSelectedVertex();
	void clearSelectedPolygon();
	bool isVertexSelected(int index);
	void selectVertex(int index);
	void selectVertexExtra(int index);
	void selectPolygon(Polygon * poly);
	void unSelectVertex(int index);
	int getSelectedVertexExtra();
	int getSelectedVertex(int index);
	Polygon * getSelectedPolygon(int index);
	void doSelectPolygons();
	void getSelectedCenter(Vertex3D & vtxM);
	
 signals:
	void documentChanged();
	
 protected:
	bool modified;
	
 private:
	QString m_sFileName;
	FileUtil * p_FileUtil;
	Engine * p_Engine;
	Group * p_GroupCameraRot;
	Group * p_GroupCameraTrans;
	Command * p_Command;
	Group * p_CurrentGroup;
	Shape3D * p_CurrentShape;
	int m_iVertexExtra;
	
	map<QString, QImage*> m_hQImage;
	vector<int> m_vSelectedVertex;
	vector<Polygon*> m_vSelectedPolygon;
	vector<Updateable*> m_vUpdateable;
	vector<Rebuildable*> m_vRebuildable;
};

#endif
