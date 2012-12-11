#include "CarCounter.h"

CarCounter::CarCounter(unsigned int xPos) :
    xBoundary(xPos),
    carCount(0),
    bikeCount(0),
    streetcarCount(0),
    rosCreated(0)
{

}

CarCounter::~CarCounter()
{

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

int CarCounter::updateStats(CvBlobs& tracks) {
    int newROs = 0;
    CvBlobs::iterator it = tracks.begin();

    // No objects created yet, but we have some blobs ... create first road object
    if (objects.size() == 0 && tracks.size() != 0) {
        //printf ("NO ROs: CREATE NEW RO\n");
        // TODO: better memory mgmt - this is copied and deleted

        // TODO: Remove code duplication
        CvBlob * track = tracks.begin()->second;

        RoadObject obj(rosCreated++, *track);
        objects.push_back(obj);
        ++it;
    }

    for (; it != tracks.end(); ++it) {

        CvBlob track = *(it->second);
#if 1
        RoadObject * closestFit = NULL;

        double minDistance = 999999; //MAX_INT

        for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

            // Road Object we're testing
            RoadObject * ro = &*obj;

            // TODO: prefer to add it to a Road Object that has been active recently

            // Track RO that is the closest match
            double distanceFromLastPt = ro->distanceFromLastPoint(1, track); // NUM_POINTS_TO_AVG

            int trackArea = (track.maxx - track.minx) * (track.maxy - track.miny);

            bool closeInSize = abs(ro->size() - trackArea) <= (0.50 * ro->size());

            if (distanceFromLastPt < minDistance && closeInSize) {
                // Only use this track if we the trend is moving westward
                if (track.centroid.x < ro->getLastBlob().centroid.x) { // TODO: Make more general.
                    minDistance = distanceFromLastPt;
                    closestFit = ro;
                }
            }

            //printf("BLOB %d CHECK RO %d DIST %f MIN_DST %f OVERLAP_PCT %f\n", blob.label, ro->getId(), distanceFromLastPt, minDistance, overlap);
        }

        // Look at metrics calculated and find the best RO to add the blob to, or create a new one
        if (closestFit && minDistance < 20) { // distanceThreshold = 20 // TODO FIX
            //printf("DISTANCE - ADD TO ID %d dist %f\n", closestFit->getId(), minDistance);
            closestFit->addTrack(track);
        } else {
            // Create new road object
            RoadObject obj(rosCreated++, track);
            objects.push_back(obj);
        }
#endif
    }

    // Iterate through RoadObjects and see if we can classify (or discard) them
    for (list<RoadObject>::iterator obj = objects.begin(); obj != objects.end(); obj++) {

        RoadObject * ro = &*obj;
        ro->incrementFrameCount(); // Update 'age' counter
        int lastSeen = ro->getLastSeenNFramesAgo();

        double distanceTravelled = ro->distanceTravelled();

        // TODO: Could check how linear path has been as one indicator

        if (ro->getNumPoints() >= MIN_NUM_POINTS && lastSeen > MIN_FRAME_TIMEOUT) {
            // If last seen where we expect the car to exit, count it
            int distanceToExit = abs(xBoundary - (int)ro->getLastBlob().minx);
            if (distanceToExit < 5 && distanceTravelled > 50) {
                printf("CAR: Short Timeout (To Exit: %d, Travelled: %f)\n", distanceToExit, distanceTravelled);
                ro->printPoints();
                carCount++;
                obj = objects.erase(obj);
                newROs++;
            } else if (lastSeen >= MAX_FRAME_TIMEOUT && distanceTravelled > 50) {
                // Evidence of a car, but haven't seen it in a while.
                // Count it as one
                carCount++;
                obj = objects.erase(obj);
                newROs++;
                printf("CAR: Long Timeout (%d)\n", distanceToExit);
            }

        } else if (ro->getLastSeenNFramesAgo() >= MAX_FRAME_TIMEOUT) {
            // TODO: check if we can call this a questionable car
            obj = objects.erase(obj);
            newROs++;
            printf("NO CAR\n");
        }
    }
    return newROs;
}
