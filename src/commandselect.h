/***************************************************************************
                          commandselect.h  -  description
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

#ifndef COMMANDSELECT_H
#define COMMANDSELECT_H

#include <command.h>

/** @author Henrik Enqvist */
class CommandSelect : public Command  {
 public:
	CommandSelect(PinEditDoc * doc);
	~CommandSelect();
	Command * build();
	void undo();
	void clearObjects();
	void execute(const CommandContext & context);
    void preview(const CommandContext & context, View2D * view2d, QPainter& painter);
    const char * type() { return "CommandSelect"; }
};

#endif
