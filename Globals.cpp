#include "Globals.h"

Camera Globals::camera = Camera();

Cube Globals::cube = Cube(10.0);

Light Globals::light = Light();

DrawData Globals::drawData = DrawData();
UpdateData Globals::updateData = UpdateData();

OFFObject *Globals::offObject = new OFFObject("brain.off");