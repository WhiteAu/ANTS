#ifndef LOCATION_H_
#define LOCATION_H_

#include <iostream>

struct LocationOffset {
	int row_offset, col_offset;

	LocationOffset() {
		row_offset = col_offset = 0;
	}

	LocationOffset(int r, int c)
	{
		row_offset = r;
		col_offset = c;
	}

	friend bool operator==(const LocationOffset& l, const LocationOffset& r) {
		return (l.row_offset == r.row_offset && l.col_offset == r.col_offset);
	}

	friend bool operator!=(const LocationOffset& l, const LocationOffset& r) {
		return !(l == r);
	}

	friend bool operator<(const LocationOffset& l, const LocationOffset& r) {
		return (l.row_offset < r.row_offset || 
			(l.row_offset == r.row_offset && l.col_offset < r.col_offset));
	}

	friend std::ostream& operator<<(std::ostream &os, const LocationOffset & offset) {
		return os << "(" << offset.row_offset << ", " << offset.col_offset << ")";
	}
};


/*
    struct for representing locations in the grid.
*/
struct Location
{
    int row, col;

    Location()
    {
        row = col = 0;
    };

    Location(int r, int c)
    {
        row = r;
        col = c;
    };

	friend bool operator==(const Location& l, const Location& r) {
		return (l.row == r.row && l.col == r.col);
	}

	friend bool operator!=(const Location& l, const Location& r) {
		return !(l == r);
	}

	friend bool operator<(const Location& l, const Location& r) {
		return (l.row < r.row || (l.row == r.row && l.col < r.col));
	}

	friend std::ostream& operator<<(std::ostream &os, const Location & loc) {
		return os << "(" << loc.row << ", " << loc.col << ")";
	}
};

static const Location NULL_LOCATION = Location(9999, 9999);

#endif //LOCATION_H_
