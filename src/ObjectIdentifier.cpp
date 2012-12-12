#include "ObjectIdentifier.h"

#include <limits>

// Forward declaration
pair<double,double> leastSqrRegression(vector<CvBlob> &blobs, int numPointsToUse);

ObjectIdentifier::ObjectIdentifier(CvBlob b) :
    lastSeen(0),
    frameCount(0),
    id(++globalID),
    closestDistToOrigin(0),
    furthestDistToOrigin(0)
{
    addBlob(b);
}

ObjectIdentifier::~ObjectIdentifier()
{
    //printf("~%d (#pts %d): (%.2f, %.2f, %.2f, %.2f) size %.2f\n", id, points.size(), minx, maxx, miny, maxy, size());
}

void ObjectIdentifier::incrementFrameCount()
{
    frameCount++;
    lastSeen++;
}

int ObjectIdentifier::getLastSeenNFramesAgo()
{
    return lastSeen;
}

int ObjectIdentifier::getNumBlobs()
{
    return blobs.size();
}

int ObjectIdentifier::getFirstFrame()
{
    return (blobs.at(0).label);
}

int ObjectIdentifier::lifetime()
{
    return (lastBlob.label - blobs.at(0).label);
}

double ObjectIdentifier::getSpeed()
{
    double speed = (lifetime() > 0 ? distanceTravelled() / lifetime() : 0);
    return speed;
}

bool ObjectIdentifier::addBlob(CvBlob b)
{
    lastBlob = b;
    blobs.push_back(b);
    lastSeen = 0;
    lastBlobFrameNum = frameCount;

    // Keep track of closest and furthest blobs from origin
    double distanceToOrigin = distance(b.centroid.x, b.centroid.y, 0, 0);

    if (distanceToOrigin > furthestDistToOrigin) {
        furthestBlob = b;
        furthestDistToOrigin = distanceToOrigin;
    }

    if (distanceToOrigin < closestDistToOrigin) {
        closestBlob = b;
        closestDistToOrigin = distanceToOrigin;
    }
}

void ObjectIdentifier::printPoints()
{
    for (int i = 0; i < blobs.size(); i++) {
        unsigned int frameNum = blobs.at(i).label; // HACK, stored the frameNumber in the blob data
        printf("%d,%f,%f,%d,%d\n", frameNum, blobs.at(i).centroid.x, blobs.at(i).centroid.y, blobs.at(i).area, id);
    }
}

double ObjectIdentifier::distanceTravelled()
{
    return distanceBetweenBlobs(furthestBlob, closestBlob);
}

double ObjectIdentifier::errFromLine(CvBlob b)
{
    if (blobs.size()  < 2) {
        return std::numeric_limits<int>::max(); // MAX_INT
    }

    pair<double,double> line = leastSqrRegression(blobs, blobs.size());
    double slope = line.first;
    double y_int = line.second;
    double y_exp = slope * b.centroid.x + y_int;
    return distance(b.centroid.x, b.centroid.y, b.centroid.x, y_exp);
}

CvBlob ObjectIdentifier::getLastBlob()
{
    return lastBlob;
}

int ObjectIdentifier::getId()
{
    return id;
}

double ObjectIdentifier::distanceBetweenBlobs(CvBlob b1, CvBlob b2)
{
    return distance(b1.centroid.x, b1.centroid.y, b2.centroid.x, b2.centroid.y);
}

#if 0
vector<CvBlob> * ObjectIdentifier::getBlobs()
{
    return &blobs;
}
#endif

// Distance between 2 points
double ObjectIdentifier::distance(double x1, double y1, double x2, double y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

bool ObjectIdentifier::inRange(CvBlob b)
{
    // TODO: CONTS for these and override for EastboundObjectDetector
    //printf("inRange BLOB %f, %f   LAST_BLOB %f,%f\n", b.centroid.x, b.centroid.y, lastBlob.centroid.x, lastBlob.centroid.y);
    if (b.centroid.x <= lastBlob.centroid.x &&
            (lastBlob.centroid.x - b.centroid.x < 25)) {
        // New point is left of the last blob
        return true;
    } else if (lastBlob.centroid.x - b.centroid.x <= 75) {
        // New point is right of the last blob
        return true;
    }
    return false;
}

bool ObjectIdentifier::inStartingZone(CvBlob b)
{
    double x = b.centroid.x;
    double y = b.centroid.y;
    return (x > 275 && x < 350 && y > 300 && y < 375);
}

bool ObjectIdentifier::inEndZone()
{
    double x = furthestBlob.centroid.x;
    double y = furthestBlob.centroid.y;
    return (x > 500 && y > 360);
}

double ObjectIdentifier::expectedY(double x)
{
    // TODO: CONTST for these and override for EastboundObjectDetector
    double slope = 0.185;
    double y_int = 270;
    double y_exp = slope * x + y_int;
    return y_exp;
}

double ObjectIdentifier::distFromExpectedPath(CvBlob b)
{
    // TODO: CONTST for these and override for EastboundObjectDetector
    double slope = 0.185;
    double y_int = 270;
    double y_exp = slope * b.centroid.x + y_int; // TODO: Remove code duplication
    return distance(b.centroid.x, b.centroid.y, b.centroid.x, y_exp);
}

double ObjectIdentifier::distanceFromLastBlob(CvBlob b)
{
    return distanceBetweenBlobs(lastBlob, b);
}

// HELPER FUNCTION

pair<double,double> ObjectIdentifier::leastSqrRegression(vector<CvBlob> &blobs, int numPointsToUse)
{
   double SUMx = 0;     //sum of x values
   double SUMy = 0;     //sum of y values
   double SUMxy = 0;    //sum of x * y
   double SUMxx = 0;    //sum of x^2
   double SUMres = 0;   //sum of squared residue
   double res = 0;      //residue squared
   double slope = 0;    //slope of regression line
   double y_intercept = 0; //y intercept of regression line
   double SUM_Yres = 0; //sum of squared of the discrepancies
   double AVGy = 0;     //mean of y
   double AVGx = 0;     //mean of x
   double Yres = 0;     //squared of the discrepancies
   double Rsqr = 0;     //coefficient of determination

   numPointsToUse = (numPointsToUse <= blobs.size()) ? numPointsToUse : blobs.size();

   //calculate various sums
   for (int i = 0; i < numPointsToUse; i++)
   {
      //sum of x
      SUMx = SUMx + blobs.at(blobs.size() - 1 - i).centroid.x;
      //sum of y
      SUMy = SUMy + blobs.at(blobs.size() - 1 - i).centroid.y;
      //sum of squared x*y
      SUMxy = SUMxy + blobs.at(blobs.size() - 1 - i).centroid.x * blobs.at(blobs.size() - 1 - i).centroid.y;
      //sum of squared x
      SUMxx = SUMxx + blobs.at(blobs.size() - 1 - i).centroid.x * blobs.at(blobs.size() - 1 - i).centroid.x;
   }

   //calculate the means of x and y
   AVGy = SUMy / numPointsToUse;
   AVGx = SUMx / numPointsToUse;

   //slope or a1
   slope = (numPointsToUse * SUMxy - SUMx * SUMy) / (numPointsToUse * SUMxx - SUMx*SUMx);

   //y itercept or a0
   y_intercept = AVGy - slope * AVGx;
#if 0
   printf("x mean(AVGx) = %0.5E\n", AVGx);
   printf("y mean(AVGy) = %0.5E\n", AVGy);

   printf ("\n");
   printf ("The linear equation that best fits the given data:\n");
   printf ("       y = %2.8lfx + %2.8f\n", slope, y_intercept);
   printf ("------------------------------------------------------------\n");
   printf ("   Original (x,y)   (y_i - y_avg)^2     (y_i - a_o - a_1*x_i)^2\n");
   printf ("------------------------------------------------------------\n");
#endif
   //calculate squared residues, their sum etc.
   for (int i = 0; i < numPointsToUse; i++)
   {
      //current (y_i - a0 - a1 * x_i)^2
      Yres = pow((blobs.at(blobs.size() - 1 - i).centroid.y - y_intercept - (slope * blobs.at(blobs.size() - 1 - i).centroid.x)), 2);

      //sum of (y_i - a0 - a1 * x_i)^2
      SUM_Yres += Yres;

      //current residue squared (y_i - AVGy)^2
      res = pow(blobs.at(blobs.size() - 1 - i).centroid.y - AVGy, 2);

      //sum of squared residues
      SUMres += res;
#if 0
      printf ("   (%0.2f %0.2f)      %0.5E         %0.5E\n",
       blobs.at(blobs.size() - 1 - i).centroid.x, blobs.at(blobs.size() - 1 - i).centroid.y, res, Yres);
#endif
   }

   //calculate r^2 coefficient of determination
   Rsqr = (SUMres - SUM_Yres) / SUMres;
#if 0
   printf("--------------------------------------------------\n");
   printf("Sum of (y_i - y_avg)^2 = %0.5E\t\n", SUMres);
   printf("Sum of (y_i - a_o - a_1*x_i)^2 = %0.5E\t\n", SUM_Yres);
   printf("Standard deviation(St) = %0.5E\n", sqrt(SUMres / (numPointsToUse - 1)));
   printf("Standard error of the estimate(Sr) = %0.5E\t\n", sqrt(SUM_Yres / (numPointsToUse-2)));
   printf("Coefficent of determination(r^2) = %0.5E\t\n", (SUMres - SUM_Yres)/SUMres);
   printf("Correlation coefficient(r) = %0.5E\t\n", sqrt(Rsqr));
#endif
   pair<double,double> result(slope,y_intercept);
   return result;
}
