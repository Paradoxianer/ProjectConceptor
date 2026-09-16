#include "SmartGuides.h"
#include <math.h>


static void ConsiderHorizontalMatch(float candidateY, const BRect *target,
	float &bestDist, float &bestOffset, bool &found, float &matchedY,
	float &spanLeft, float &spanRight, float candidateLeft, float candidateRight)
{
	float targetYs[3] = { target->top, target->bottom, (target->top+target->bottom)/2.0f };
	for (int32 i = 0; i < 3; i++) {
		float diff = targetYs[i] - candidateY;
		float dist = fabsf(diff);
		if (dist <= bestDist) {
			bestDist	= dist;
			bestOffset	= diff;
			found		= true;
			matchedY	= targetYs[i];
			spanLeft	= (candidateLeft < target->left) ? candidateLeft : target->left;
			spanRight	= (candidateRight > target->right) ? candidateRight : target->right;
		}
	}
}

static void ConsiderVerticalMatch(float candidateX, const BRect *target,
	float &bestDist, float &bestOffset, bool &found, float &matchedX,
	float &spanTop, float &spanBottom, float candidateTop, float candidateBottom)
{
	float targetXs[3] = { target->left, target->right, (target->left+target->right)/2.0f };
	for (int32 i = 0; i < 3; i++) {
		float diff = targetXs[i] - candidateX;
		float dist = fabsf(diff);
		if (dist <= bestDist) {
			bestDist	= dist;
			bestOffset	= diff;
			found		= true;
			matchedX	= targetXs[i];
			spanTop		= (candidateTop < target->top) ? candidateTop : target->top;
			spanBottom	= (candidateBottom > target->bottom) ? candidateBottom : target->bottom;
		}
	}
}


GuideSnapResult ComputeGuideSnap(BRect startFrame, float rawDx, float rawDy,
	BList *targetFrames, float threshold)
{
	GuideSnapResult	result;
	result.horizontal.active	= false;
	result.vertical.active		= false;
	result.dx					= rawDx;
	result.dy					= rawDy;

	BRect	candidate	= startFrame;
	candidate.OffsetBy(rawDx, rawDy);
	int32	count		= targetFrames->CountItems();

	// ---- Horizontal alignment (a row): top/bottom/center on Y ----
	{
		float	candidateYs[3]	= { candidate.top, candidate.bottom,
			(candidate.top+candidate.bottom)/2.0f };
		bool	found			= false;
		float	bestDist		= threshold;
		float	bestOffset		= 0;
		float	matchedY		= 0;
		float	spanLeft		= candidate.left;
		float	spanRight		= candidate.right;
		for (int32 c = 0; c < 3; c++) {
			for (int32 i = 0; i < count; i++) {
				BRect	*target	= (BRect *)targetFrames->ItemAt(i);
				ConsiderHorizontalMatch(candidateYs[c], target, bestDist, bestOffset,
					found, matchedY, spanLeft, spanRight, candidate.left, candidate.right);
			}
		}

		// Equal-spacing: nearest neighbour directly above and below (actual
		// column neighbours, i.e. their horizontal extent overlaps ours) -
		// wins over a plain edge/center match when both exist, since it's
		// the more specific signal.
		// The neighbour search itself is not threshold-limited - a
		// neighbour can be arbitrarily far away and still be "the
		// nearest one"; threshold only gates the resulting offset below.
		BRect	*above		= NULL;
		BRect	*below		= NULL;
		float	aboveGap	= 0;
		float	belowGap	= 0;
		for (int32 i = 0; i < count; i++) {
			BRect	*target		= (BRect *)targetFrames->ItemAt(i);
			bool	overlapsX	= (target->left < candidate.right) && (target->right > candidate.left);
			if (!overlapsX)
				continue;
			if (target->bottom <= candidate.top) {
				float gap = candidate.top - target->bottom;
				if ((above == NULL) || (gap < aboveGap)) {
					above		= target;
					aboveGap	= gap;
				}
			} else if (target->top >= candidate.bottom) {
				float gap = target->top - candidate.bottom;
				if ((below == NULL) || (gap < belowGap)) {
					below		= target;
					belowGap	= gap;
				}
			}
		}
		if ((above != NULL) && (below != NULL)) {
			float	height		= candidate.Height();
			float	desiredTop	= (below->top - height + above->bottom) / 2.0f;
			float	offset		= desiredTop - candidate.top;
			if (fabsf(offset) <= threshold) {
				found		= true;
				bestOffset	= offset;
				matchedY	= desiredTop;
				spanLeft	= candidate.left;
				if (above->left < spanLeft) spanLeft = above->left;
				if (below->left < spanLeft) spanLeft = below->left;
				spanRight	= candidate.right;
				if (above->right > spanRight) spanRight = above->right;
				if (below->right > spanRight) spanRight = below->right;
			}
		}

		if (found) {
			result.dy					= rawDy + bestOffset;
			result.horizontal.active	= true;
			result.horizontal.linePos	= matchedY;
			result.horizontal.lineStart	= spanLeft;
			result.horizontal.lineEnd	= spanRight;
		}
	}

	// ---- Vertical alignment (a column): left/right/center on X ----
	{
		float	candidateXs[3]	= { candidate.left, candidate.right,
			(candidate.left+candidate.right)/2.0f };
		bool	found			= false;
		float	bestDist		= threshold;
		float	bestOffset		= 0;
		float	matchedX		= 0;
		float	spanTop			= candidate.top;
		float	spanBottom		= candidate.bottom;
		for (int32 c = 0; c < 3; c++) {
			for (int32 i = 0; i < count; i++) {
				BRect	*target	= (BRect *)targetFrames->ItemAt(i);
				ConsiderVerticalMatch(candidateXs[c], target, bestDist, bestOffset,
					found, matchedX, spanTop, spanBottom, candidate.top, candidate.bottom);
			}
		}

		BRect	*left		= NULL;
		BRect	*right		= NULL;
		for (int32 i = 0; i < count; i++) {
			BRect	*target		= (BRect *)targetFrames->ItemAt(i);
			bool	overlapsY	= (target->top < candidate.bottom) && (target->bottom > candidate.top);
			if (!overlapsY)
				continue;
			if (target->right <= candidate.left) {
				if ((left == NULL) || ((candidate.left-target->right) < (candidate.left-left->right)))
					left	= target;
			} else if (target->left >= candidate.right) {
				if ((right == NULL) || ((target->left-candidate.right) < (right->left-candidate.right)))
					right	= target;
			}
		}
		if ((left != NULL) && (right != NULL)) {
			float	width		= candidate.Width();
			float	desiredLeft	= (right->left - width + left->right) / 2.0f;
			float	offset		= desiredLeft - candidate.left;
			if (fabsf(offset) <= threshold) {
				found		= true;
				bestOffset	= offset;
				matchedX	= desiredLeft;
				spanTop		= candidate.top;
				if (left->top < spanTop) spanTop = left->top;
				if (right->top < spanTop) spanTop = right->top;
				spanBottom	= candidate.bottom;
				if (left->bottom > spanBottom) spanBottom = left->bottom;
				if (right->bottom > spanBottom) spanBottom = right->bottom;
			}
		}

		if (found) {
			result.dx				= rawDx + bestOffset;
			result.vertical.active	= true;
			result.vertical.linePos	= matchedX;
			result.vertical.lineStart	= spanTop;
			result.vertical.lineEnd	= spanBottom;
		}
	}

	return result;
}
