#pragma once
//------------------------------------------------------------------------------
/**
    @class VisBounds
    @brief a 2D bounding area in the VisTree
*/
#include "Core/Types.h"

class VisBounds {
public:
    /// default constructor
    VisBounds() : x0(0), x1(0), y0(0), y1(0) {};
    /// constructor
    VisBounds(int x0_, int x1_, int y0_, int y1_) : x0(x0_), x1(x1_), y0(y0_), y1(y1_) { };

    /// get closest Manhattan distance, 0 if inside
    int MinDist(int x, int y) const {
        int dx;
        int dx0 = x - this->x0;
        int dx1 = this->x1 - x;
        if ((dx0 >= 0) && (dx1 >= 0)) {
            dx = 0;
        }
        else {
            if (dx0 < 0) dx0 = -dx0;
            if (dx1 < 0) dx1 = -dx1;
            dx = dx0<dx1 ? dx0:dx1;
        }

        int dy;
        int dy0 = y - this->y0;
        int dy1 = this->y1 - y;
        if ((dy0 >= 0) && (dy1 >= 0)) {
            dy = 0;
        }
        else {
            if (dy0 < 0) dy0 = -dy0;
            if (dy1 < 0) dy1 = -dy1;
            dy = dy0<dy1 ? dy0:dy1;
        }

        int d = dx<dy ? dx:dy;
        return d;
    }

    int x0, x1, y0, y1;
};