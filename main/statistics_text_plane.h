#ifndef STATISTICS_TEXT_PLANE_H
#define STATISTICS_TEXT_PLANE_H

#include "text.h"
#include "session.h"

class Statistics_Text_Plane : public Text_Plane
{
public:
    Statistics_Text_Plane();
    virtual ~Statistics_Text_Plane();

    void generate(std::list< SRef<Session *> > sessions);
};

#endif // STATISTICS_TEXT_PLANE_H
