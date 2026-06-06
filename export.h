#ifndef EXPORT_H
#define EXPORT_H

#include "schedule.h"

int save_schedules_csv(const char *path, const char *titles[],
                       const Schedule schedules[], int count);

#endif
