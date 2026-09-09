#ifndef GROUP_BOUNDARY_H
#define GROUP_BOUNDARY_H
/** Pure geometry for a group's outline (issue #38) - no BView/rendering
 * dependency, so it can be exercised directly by the test suite instead
 * of only indirectly through a live GroupRenderer::Draw(). See
 * ComputeGroupBoundary()'s own comment in GroupBoundary.cpp for the
 * algorithm.
 */

#include <interface/Point.h>
#include <interface/Rect.h>

#include <vector>

std::vector<BPoint>	ComputeGroupBoundary(const std::vector<BRect> &rects, float labelSpace);

#endif
