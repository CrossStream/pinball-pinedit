/***************************************************************************
                          texcoorddialog.cpp  -  description
                             -------------------
    begin                : Thu Jul 11 19:39:03 EET 2002
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
// qt includes
#include <qpushbutton.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <q3buttongroup.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3BoxLayout>
#include <Q3HBoxLayout>
// application includes
#include "pineditdoc.h"
#include "texcoorddialog.h"
#include "commandtexcoord.h"
// emilia includes
#include "Private.h"

TexCoordDialog::TexCoordDialog(PinEditDoc * doc, QWidget * parent, const char * name, Qt::WFlags f) 
: QDialog(parent, name, f) {
	assert(doc != NULL);
	p_Doc = doc;
	p_CommandTexCoord = new CommandTexCoord(p_Doc);

	p_RadioButtonXY = new QRadioButton("XY", this);
	p_RadioButtonXZ = new QRadioButton("XZ", this);
	p_RadioButtonZY = new QRadioButton("ZY", this);

	QPushButton * texcoordbutton = new QPushButton("texcoord", this);
	connect(texcoordbutton, SIGNAL(clicked()), this, SLOT(slotTexCoord()));
	QPushButton * cancelbutton = new QPushButton("cancel", this);
	connect(cancelbutton, SIGNAL(clicked()), this, SLOT(slotCancel()));

	Q3BoxLayout * layout = new Q3VBoxLayout(this);
	Q3BoxLayout * hlayout = new Q3HBoxLayout(layout);
	hlayout->addWidget(p_RadioButtonXY);
	hlayout->addWidget(p_RadioButtonXZ);
	hlayout->addWidget(p_RadioButtonZY);
	layout->addWidget(texcoordbutton);
	layout->addWidget(cancelbutton);

	Q3ButtonGroup * buttongroup = new Q3ButtonGroup();
	buttongroup->insert(p_RadioButtonXY);
	buttongroup->insert(p_RadioButtonXZ);
	buttongroup->insert(p_RadioButtonZY);
}

TexCoordDialog::~TexCoordDialog() {
}

void TexCoordDialog::slotTexCoord() {
  EM_CERR("TexCoordDialog::slotTexCoord");
  CommandContext context;
  context.clear();
  context.shape = p_Doc->getCurrentShape();
  if (context.shape == NULL) {
    QMessageBox::information( this, "TexCoord", "No Shape selected.");
    this->done(0);
    return;
  }
  
  context.type = XY;
  if (p_RadioButtonXZ->isChecked()) { 
    context.type = XZ;
  }	else if (p_RadioButtonZY->isChecked()) { 
    context.type = ZY;
  }
  
  p_CommandTexCoord->build()->execute(context);
  this->done(0);
};

void TexCoordDialog::slotCancel() {
  EM_CERR("TexCoordDialog::slotCancel");
  this->done(0);
};
