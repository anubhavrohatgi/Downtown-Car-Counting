#include "CarCounter.h"

CarCounter::CarCounter(double distThresh=50, Rect* bounds=NULL, bool use_slope_prediction=false, double expPathSlope=0.2) :
    distanceThreshold(distThresh),
    carCount(0),
    bikeCount(0),
    streetcarCount(0),
    rosCreated(0),
    boundaries(bounds),
    useSlopeOfPathFilter(use_slope_prediction),
    expectedPathSlope(expPathSlope)
{

}

CarCounter::~CarCounter()
{

}

CvBlobs CarCounter::getBlobs()
{
    CvBlobs blobs;
    for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {
        RoadObject * ro = &*obj;
        pair<const unsigned int, CvBlob*> blob(ro->getLastBlob().label, &ro->getLastBlob());
        blobs.insert(blob);
    }
}


void CarCounter::setBoundaries(Rect &bounds) {
    boundaries = &bounds;
}

int CarCounter::getMaxFrameTimeout() {
    return MAX_FRAME_TIMEOUT;
}

double CarCounter::getAvgSpeed(int numFrames)
{
    if (numFrames < 2) return 0;

    double speedSum = 0;
    for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {
        RoadObject * ro = &*obj;
        speedSum += ro->speedPixelsPerFrame(numFrames);
    }
    return (speedSum / numFrames);
}

int CarCounter::updateStats(CvBlobs& blobs) {
    int newROs = 0;
    CvBlobs::iterator it = blobs.begin();

    if (objects.size() == 0 && blobs.size() != 0) {
        //printf ("NO ROs: CREATE NEW RO\n");
        // TODO: better memory mgmt - this is copied and deleted

        // TODO: Remove code duplication
        CvBlob * blob = blobs.begin()->second;
        if (boundaries) {
            // Only create new Road Objects that are near a boundary ... we don't have cars magically appear
            // in the middle of the road ...
            double pixelLimit = boundaries->width * BOUNDS_THRESHOLD_PCT;
            double deltaX1 = abs(blob->centroid.x - boundaries->x);
            double deltaX2 = abs((blob->centroid.x) - (boundaries->x + boundaries->width));
            if (deltaX1 > pixelLimit && deltaX2 > pixelLimit) return 0;
        }

        RoadObject obj(rosCreated++, *blob);
        objects.push_back(obj);
        it++;
        //newROs++;
    }

    for (; it != blobs.end(); ++it) {

        CvBlob blob = *(it->second);
#if 1
        RoadObject * closestFit = NULL;
        RoadObject * bestOverlap = NULL;

        double minDistance = 999999; //MAX_INT
        double maxOverlap = 0;

        for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

            // Road Object we're testing
            RoadObject * ro = &*obj;

            // Track RO that is the closest match
            double distanceFromLastPt = ro->distanceFromLastPoint(NUM_POINTS_TO_AVG, blob);
            if (distanceFromLastPt < minDistance) {
                minDistance = distanceFromLastPt;
                closestFit = ro;
            }

            double overlap = ro->pctRecentOverlap(NUM_POINTS_FOR_OVERLAP_CHECK, blob);
            if (overlap > maxOverlap) {
                maxOverlap = overlap;
                bestOverlap = ro;
            }

            //printf("BLOB %d CHECK RO %d DIST %f MIN_DST %f OVERLAP_PCT %f\n", blob.label, ro->getId(), distanceFromLastPt, minDistance, overlap);
        }

        // Look at metrics calculated and find the best RO to add the blob to, or create a new one
        if (closestFit && minDistance < distanceThreshold) {
            //printf("DISTANCE - ADD TO ID %d dist %f\n", closestFit->getId(), minDistance);
            closestFit->addBlob(blob);
        } else if (bestOverlap && maxOverlap >= OVERLAP_PCT_THRESHOLD) {
            //printf("OVERLAP - ADD TO ID %d overlap %f\n", bestOverlap->getId(), maxOverlap);
            bestOverlap->addBlob(blob);
        } else {
            double fitToOverlapRO = 999999;
            double fitToClosestRO = 999999;

            // Check linear regression as long as we're within 2X the distance threshold
            if (bestOverlap && (bestOverlap->distanceFromLastPoint(NUM_POINTS_TO_AVG, blob) <= 2 * distanceThreshold)) {
                fitToOverlapRO = bestOverlap->errFromLine(NUM_POINTS_FOR_LINEAR_REGRESSION, blob);
            }

            if (closestFit && (closestFit->distanceFromLastPoint(NUM_POINTS_TO_AVG, blob) <= 2 * distanceThreshold)) {
                fitToClosestRO = closestFit->errFromLine(NUM_POINTS_FOR_LINEAR_REGRESSION, blob);
            }

            if (fitToOverlapRO < NUM_PIXELS_LINEAR_REGRESSION_THRESHOLD || fitToClosestRO < NUM_PIXELS_LINEAR_REGRESSION_THRESHOLD) {
                if (fitToOverlapRO < fitToClosestRO) {
                    //printf("LINEAR_REGRESSION (BEST_OVERLAP) ADD TO ID %d dist_to_line %f\n", bestOverlap->getId(), fitToOverlapRO);
                    bestOverlap->addBlob(blob);
                } else {
                    //printf("LINEAR_REGRESSION (CLOSEST_FIT) ADD TO ID %d dist_to_line %f\n", closestFit->getId(), fitToClosestRO);
                    closestFit->addBlob(blob);
                }
            } else {
                RoadObject ro(rosCreated++, blob);
                printf("CREATE RO %d blob size %d at (%f,%f)\n", ro.getId(), it->second->area, blob.centroid.x, blob.centroid.y);
                newROs++;
                objects.push_back(ro);
            }

        }
#endif
    }

    // Iterate through RoadObjects and see if we can classify (or discard) them
    for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

        RoadObject * ro = &*obj;
        ro->incrementFrameCount(); // Update 'age' counter
        int lastSeen = ro->getLastSeenNFramesAgo();

        if (ro->getNumPoints() >= MIN_NUM_POINTS && lastSeen > MIN_FRAME_TIMEOUT && lastSeen < MAX_FRAME_TIMEOUT) {

            // Road object has disappeared for a bit, check if we can classify it yet ...
            bool pathWithinBounds = true;
            if (boundaries) {
                Rect objBounds = ro->getBounds();
                // Note: I am only checking x bounds, but y could also be checked
                // This makes sure the object has roughly travelled where we expect it to have to be a vehicle of some type
                double pixelLimit = boundaries->width * BOUNDS_THRESHOLD_PCT;
                double deltaX1 = abs(objBounds.x - boundaries->x);
                double deltaX2 = abs((objBounds.x + objBounds.width) - (boundaries->x + boundaries->width));
                pathWithinBounds = (deltaX1 < pixelLimit && deltaX2 < pixelLimit);
                //printf("deltaX1 %.2f deltaX2 %.2f limit %.2f Within Bounds: %d\n", deltaX1, deltaX2, pixelLimit, pathWithinBounds);
            }

            bool slopeWithinRange = true;
            if (useSlopeOfPathFilter) {
                double slope = ro->slopeOfPath();
                if (slope < (expectedPathSlope - SLOPE_FILTER_THRESHOLD) || slope > (expectedPathSlope + SLOPE_FILTER_THRESHOLD)) {
                    slopeWithinRange = false;
                }
                //printf("Slope %f within rage? %d\n", slope, slopeWithinRange);

            }

            if (pathWithinBounds && slopeWithinRange) {
                if (ro->size() > VEHICLE_SIZE_THRESHOLD) {
                    carCount++;
                } else {
                    bikeCount++;
                }
                printf("Adding ... np %d dist %f size %f slope %f speed (pixels/frame) %.2f\n", ro->getNumPoints(), ro->distanceTravelled(), ro->size(), ro->slopeOfPath(), ro->speedPixelsPerFrame());
                printf("Car Count: %d Bike Count %d\n", carCount, bikeCount);
                newROs++;
                obj = objects.erase(obj);
            }
        } else if (ro->getLastSeenNFramesAgo() >= MAX_FRAME_TIMEOUT) {
            // TODO: check if we can call this a questionable car
            obj = objects.erase(obj);
            newROs++;
        }
    }
    return newROs;
}
