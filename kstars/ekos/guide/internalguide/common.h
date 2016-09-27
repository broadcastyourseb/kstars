/*  Ekos guide tool
    Copyright (C) 2012 Andrew Stepanenko

    Modified by Jasem Mutlaq <mutlaqja@ikarustech.com> for KStars.

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 */


#ifndef COMMON_H_
#define COMMON_H_

#include <QPainter>
#include <QMouseEvent>
#include <QWidget>

#include "../indi/indicommon.h"



struct guide_dir_desc
{
    GuideDirection dir;
    const char desc[10];
};





#endif /* COMMON_H_ */
