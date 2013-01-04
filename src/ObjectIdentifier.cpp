#include "ObjectIdentifier.h"

#include <limits>

using namespace std;

// Forward declaration
pair<double,double> leastSqrRegression(vector<Blob> &blobs, int numPointsToUse);

ObjectIdentifier::ObjectIdentifier(Blob b) :
    lastSeen(0),
    frameCount(0),
    id(1 + (globalID++ % 8)),
    closestDistToOrigin(0),
    furthestDistToOrigin(0),
    closestBlob(b),
    furthestBlob(b),
    numBlobs(0)
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
    return numBlobs;
}

int ObjectIdentifier::getFirstFrame()
{
    return (blobs.at(0).frameNum);
}

int ObjectIdentifier::lifetime()
{
    return (lastBlob.frameNum - blobs.at(0).frameNum);
}

double ObjectIdentifier::getSpeed()
{
    double speed = (lifetime() > 0 ? distanceTravelled() / lifetime() : 0);
    return speed;
}

bool ObjectIdentifier::addBlob(Blob b)
{
    numBlobs++;

    if (distanceFromLastBlob(b) < 15 && false) {
        // If very close, just treat as a single point
        int avgd = ++blobs.at(blobs.size() - 1).blobsAvgd;
        printf("AVGD %d\n", avgd);
        double x = (1/avgd) * b.x + ((avgd - 1) / avgd) * blobs.at(blobs.size() - 1).x;
        double y = (1/avgd) * b.y + ((avgd - 1) / avgd) * blobs.at(blobs.size() - 1).y;
        double area = (1/avgd) * b.area + ((avgd - 1) / avgd) * blobs.at(blobs.size() - 1).area;
        blobs.at(blobs.size() - 1).x = x;
        blobs.at(blobs.size() - 1).y = y;
        blobs.at(blobs.size() - 1).area = area;
    } else {
    }
    lastBlob = b;
    blobs.push_back(b);

    lastSeen = 0;
    lastBlobFrameNum = frameCount;

    // Keep track of closest and furthest blobs from origin
    double distanceToOrigin = distance(b.x, b.y, 0, 0);

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
        unsigned int frameNum = blobs.at(i).frameNum; // HACK, stored the frameNumber in the blob data
        printf("%d,%f,%f,%d,%d\n", frameNum, blobs.at(i).x, blobs.at(i).y, blobs.at(i).area, id);
    }
}

double ObjectIdentifier::distanceTravelled()
{
    return distanceBetweenBlobs(furthestBlob, closestBlob);
}

double ObjectIdentifier::errFromLine(Blob b)
{
    if (blobs.size()  < 2) {
        return std::numeric_limits<int>::max(); // MAX_INT
    }

    pair<double,double> line = xyLeastSqrRegression(blobs, blobs.size());
    double slope = line.first;
    double y_int = line.second;
    double y_exp = slope * b.x + y_int;
    return distance(b.x, b.y, b.x, y_exp);
}

double ObjectIdentifier::errXY(double x, double y)
{
    pair<double,double> line = xyLeastSqrRegression(blobs, blobs.size());
    double slope = line.first;
    double y_int = line.second;

    // Standard form
    double A = slope;
    double B = -1;
    double C = y_int;

    double distance = abs(A*x + B*y + C) / sqrt(A*A + B*B);
    return distance;
}

double ObjectIdentifier::errTX(int frameNum, double x)
{
    pair<double,double> line = txLeastSqrRegression(blobs, blobs.size());
    double slope = line.first;
    double y_int = line.second;

    // Standard form
    double A = slope;
    double B = -1;
    double C = y_int;

    double distance = abs(A*frameNum + B*x + C) / sqrt(A*A + B*B);
    return distance;
}

double ObjectIdentifier::errTY(int frameNum, double y)
{
    pair<double,double> line = tyLeastSqrRegression(blobs, blobs.size());
    double slope = line.first;
    double y_int = line.second;

    // Standard form
    double A = slope;
    double B = -1;
    double C = y_int;

    double distance = abs(A*frameNum + B*y + C) / sqrt(A*A + B*B);
    return distance;
}

bool ObjectIdentifier::continuesTrend(Blob b)
{
    // TODO: find more generic way to do this
    return true;
}

double ObjectIdentifier::distFromExpectedY(double x, double y)
{
        if (blobs.size()  < 2) {
            return std::numeric_limits<double>::max(); // MAX_INT
        }

        pair<double,double> line = xyLeastSqrRegression(blobs, blobs.size());
        double slope = line.first;
        double y_int = line.second;
        double y_exp = slope * x + y_int;
        return abs(y - y_exp);
}

double ObjectIdentifier::distFromExpectedY(double y, int frameNum)
{
    pair<double,double> line = tyLeastSqrRegression(blobs, blobs.size());
    double slope = line.first;
    double y_int = line.second;
    double y_exp = slope * frameNum + y_int;
    return abs(y - y_exp);
}

double ObjectIdentifier::distFromExpectedX(double x, int frameNum)
{
    pair<double,double> line = txLeastSqrRegression(blobs, blobs.size());
    double slope = line.first;
    double x_int = line.second;
    double x_exp = slope * frameNum + x_int;
    return abs(x - x_exp);
}

Blob ObjectIdentifier::getLastBlob()
{
    return lastBlob;
}

int ObjectIdentifier::getId()
{
    return id;
}

double ObjectIdentifier::distanceBetweenBlobs(Blob b1, Blob b2)
{
    return distance(b1.x, b1.y, b2.x, b2.y);
}

#if 0
vector<Blob> * ObjectIdentifier::getBlobs()
{
    return &blobs;
}
#endif

// Distance between 2 points
double ObjectIdentifier::distance(double x1, double y1, double x2, double y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

bool ObjectIdentifier::inRange(Blob b)
{
    // TODO: CONTS for these and override for EastboundObjectDetector
    //printf("inRange BLOB %f, %f   LAST_BLOB %f,%f\n", b.x, b.y, lastBlob.x, lastBlob.y);
    if (b.x <= lastBlob.x &&
            (lastBlob.x - b.x < 25)) {
        // New point is left of the last blob
        return true;
    } else if (lastBlob.x - b.x <= 75) {
        // New point is right of the last blob
        return true;
    }
    return false;
}

bool ObjectIdentifier::inStartingZone(Blob b)
{
    double x = b.x;
    double y = b.y;
    return (x > 275 && x < 350 && y > 300 && y < 375);
}

bool ObjectIdentifier::inEndZone()
{
    double x = furthestBlob.x;
    double y = furthestBlob.y;
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

double ObjectIdentifier::distFromExpectedPath(Blob b)
{
    // TODO: CONTST for these and override for EastboundObjectDetector
    double slope = 0.185;
    double y_int = 270;
    double y_exp = slope * b.x + y_int; // TODO: Remove code duplication
    return distance(b.x, b.y, b.x, y_exp);
}

double ObjectIdentifier::distanceFromLastBlob(Blob b)
{
    return distanceBetweenBlobs(lastBlob, b);
}



// HELPER FUNCTION

pair<double,double> ObjectIdentifier::txLeastSqrRegression(vector<Blob> &blobs, int numPointsToUse)
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
      SUMx = SUMx + blobs.at(blobs.size() - 1 - i).frameNum;
      //sum of y
      SUMy = SUMy + blobs.at(blobs.size() - 1 - i).x;
      //sum of squared x*y
      SUMxy = SUMxy + blobs.at(blobs.size() - 1 - i).frameNum * blobs.at(blobs.size() - 1 - i).x;
      //sum of squared x
      SUMxx = SUMxx + blobs.at(blobs.size() - 1 - i).frameNum * blobs.at(blobs.size() - 1 - i).frameNum;
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
      Yres = pow((blobs.at(blobs.size() - 1 - i).x - y_intercept - (slope * blobs.at(blobs.size() - 1 - i).frameNum)), 2);

      //sum of (y_i - a0 - a1 * x_i)^2
      SUM_Yres += Yres;

      //current residue squared (y_i - AVGy)^2
      res = pow(blobs.at(blobs.size() - 1 - i).x - AVGy, 2);

      //sum of squared residues
      SUMres += res;
#if 0
      printf ("   (%0.2f %0.2f)      %0.5E         %0.5E\n",
       blobs.at(blobs.size() - 1 - i).frameNum, blobs.at(blobs.size() - 1 - i).x, res, Yres);
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
   txR = sqrt(Rsqr);
   pair<double,double> result(slope,y_intercept);
   return result;
}

pair<double,double> ObjectIdentifier::tyLeastSqrRegression(vector<Blob> &blobs, int numPointsToUse)
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
      SUMx = SUMx + blobs.at(blobs.size() - 1 - i).frameNum;
      //sum of y
      SUMy = SUMy + blobs.at(blobs.size() - 1 - i).y;
      //sum of squared x*y
      SUMxy = SUMxy + blobs.at(blobs.size() - 1 - i).frameNum * blobs.at(blobs.size() - 1 - i).y;
      //sum of squared x
      SUMxx = SUMxx + blobs.at(blobs.size() - 1 - i).frameNum * blobs.at(blobs.size() - 1 - i).frameNum;
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
      Yres = pow((blobs.at(blobs.size() - 1 - i).y - y_intercept - (slope * blobs.at(blobs.size() - 1 - i).frameNum)), 2);

      //sum of (y_i - a0 - a1 * x_i)^2
      SUM_Yres += Yres;

      //current residue squared (y_i - AVGy)^2
      res = pow(blobs.at(blobs.size() - 1 - i).y - AVGy, 2);

      //sum of squared residues
      SUMres += res;
#if 0
      printf ("   (%0.2f %0.2f)      %0.5E         %0.5E\n",
       blobs.at(blobs.size() - 1 - i).frameNum, blobs.at(blobs.size() - 1 - i).y, res, Yres);
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
   tyR = sqrt(Rsqr);
   pair<double,double> result(slope,y_intercept);
   return result;
}

pair<double,double> ObjectIdentifier::xyLeastSqrRegression(vector<Blob> &blobs, int numPointsToUse)
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
      SUMx = SUMx + blobs.at(blobs.size() - 1 - i).x;
      //sum of y
      SUMy = SUMy + blobs.at(blobs.size() - 1 - i).y;
      //sum of squared x*y
      SUMxy = SUMxy + blobs.at(blobs.size() - 1 - i).x * blobs.at(blobs.size() - 1 - i).y;
      //sum of squared x
      SUMxx = SUMxx + blobs.at(blobs.size() - 1 - i).x * blobs.at(blobs.size() - 1 - i).x;
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
      Yres = pow((blobs.at(blobs.size() - 1 - i).y - y_intercept - (slope * blobs.at(blobs.size() - 1 - i).x)), 2);

      //sum of (y_i - a0 - a1 * x_i)^2
      SUM_Yres += Yres;

      //current residue squared (y_i - AVGy)^2
      res = pow(blobs.at(blobs.size() - 1 - i).y - AVGy, 2);

      //sum of squared residues
      SUMres += res;
#if 0
      printf ("   (%0.2f %0.2f)      %0.5E         %0.5E\n",
       blobs.at(blobs.size() - 1 - i).x, blobs.at(blobs.size() - 1 - i).y, res, Yres);
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
   xyR = sqrt(Rsqr);
   pair<double,double> result(slope,y_intercept);
   return result;
}
