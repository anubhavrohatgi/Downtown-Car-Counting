#include "RoadObject.h"

// Forward declaration
pair<double,double> leastSqrRegression(vector<CvBlob> &points, int numPointsToUse);

RoadObject::RoadObject::RoadObject(int inID, CvBlob t) :
    lastSeen(0),
    lastPoint(t),
    id(inID),
    minx(t.centroid.x),
    maxx(t.centroid.x),
    miny(t.centroid.y),
    maxy(t.centroid.y),
    frameCount(0),
    lastBlobFrameNum(0)
{
    //printf("Creating %d ... (%02f,%02f,%02f,%02f)\n", id, minx, maxx, miny, maxy);
    //printf("%d Setting A minxx %f\n", id, minx);
    points.push_back(t);
}

RoadObject::~RoadObject()
{
    //printf("~%d (#pts %d): (%.2f, %.2f, %.2f, %.2f) size %.2f\n", id, points.size(), minx, maxx, miny, maxy, size());
}

Rect RoadObject::getBounds()
{
    return Rect(minx, miny, maxx - minx, maxy - miny);
}

double RoadObject::speedPixelsPerFrame()
{
    return (distanceTravelled() / ((lastBlobFrameNum > 0) ? lastBlobFrameNum : 1));
}

double RoadObject::speedPixelsPerFrame(int lastNFrames)
{
    if (lastNFrames < 2) return 0;
    return (distanceTravelledLastNFrames(lastNFrames) / lastNFrames);
}

double RoadObject::size()
{
    double areaSum = 0;
    for (int i = 0; i < points.size(); i++) {
        double area = (points.at(i).maxx - points.at(i).minx) * (points.at(i).maxy - points.at(i).miny);
        areaSum += area;
    }
    return areaSum / points.size();
}

void RoadObject::incrementFrameCount()
{
    frameCount++;
    lastSeen++;
}

int RoadObject::getLastSeenNFramesAgo()
{
    return lastSeen;
}

void RoadObject::addTrack(CvBlob t)
{
    lastPoint = t;
    points.push_back(t);
    lastSeen = 0;
    lastBlobFrameNum = frameCount;

    minx = (t.centroid.x < minx) ? t.centroid.x : minx;
    maxx = (t.centroid.x > maxx) ? t.centroid.x : maxx;
    miny = (t.centroid.y < miny) ? t.centroid.y : miny;
    maxy = (t.centroid.y > maxy) ? t.centroid.y : maxy;
    //printf("%d ADDING %f,%f\n", id, t.centroid.x, t.centroid.y);
    //printf("%d Setting B minxx %f\n", id, minx);
}

void RoadObject::printPoints()
{
    printf("%d: minx %f maxx %f\n", id, minx, maxx);
    for (int i = 0; i < points.size(); i++) {
        double area = (points.at(i).maxx - points.at(i).minx) * (points.at(i).maxy - points.at(i).miny);
        printf("%f,%f area %f\n", points.at(i).centroid.x, points.at(i).centroid.y, area);
    }
}

double RoadObject::slopeOfPath()
{
    return leastSqrRegression(points, points.size()).first;
}

// NOTE: Not exactly accurate ... measures last blobs, regardless when exactly they were recorded.
// FIXME
double RoadObject::distanceTravelledLastNFrames(int numFrames)
{
    if (getLastSeenNFramesAgo() >= numFrames) return 0;

    numFrames = (numFrames <= points.size()) ? numFrames : points.size();
    double dminx = 99999; // MAX_INT
    double dmaxx = 0;
    double dminy = 99999; // MAX_INT
    double dmaxy = 0;
    for (int i = 0; i < numFrames; i++) {
        dminx = (points.at(i).centroid.x < dminx) ? points.at(i).centroid.x : dminx;
        dmaxx = (points.at(i).centroid.x > dmaxx) ? points.at(i).centroid.x : dminx;
        dminy = (points.at(i).centroid.x < dminy) ? points.at(i).centroid.y : dminy;
        dmaxy = (points.at(i).centroid.x > dmaxy) ? points.at(i).centroid.y : dmaxy;
    }
    return distance(dminx, dmaxx, dminy, dmaxy);
}

double RoadObject::distanceTravelled()
{
    return abs(minx - maxx);
}

double RoadObject::errFromLine(int numPoints, CvBlob b)
{
    if (numPoints == 1 || points.size() == 1) {
        return 999999; // MAX_INT
    }

    pair<double,double> line = leastSqrRegression(points, numPoints);
    double slope = line.first;
    double y_int = line.second;
    double y_exp = slope * b.centroid.x + y_int;
    return (abs(b.centroid.y - y_exp));
}

double RoadObject::distanceFromLastPoint(int numPoints, CvBlob p)
{
    int numPointToAvg = (points.size() < numPoints) ? points.size() : numPoints;
    double distanceSum = 0;
    for (int i = 0; i < numPointToAvg; i++) {
        distanceSum += distanceBetweenPoints(p, points.at(points.size() - 1 - i));
    }
    return (distanceSum / numPointToAvg);
}

CvBlob RoadObject::getLastBlob()
{
    return lastPoint;
}

int RoadObject::getNumPoints()
{
    return points.size();
}

int RoadObject::getId()
{
    return id;
}

vector<CvBlob> * RoadObject::getPoints()
{
    return &points;
}

double RoadObject::distanceBetweenPoints(CvBlob b1, CvBlob b2)
{
    double dist = 0;
    if (points.size() != 0) {
        dist = distance(b1.centroid.x, b2.centroid.x, b1.centroid.y, b2.centroid.y);
    }
    return dist;
}

double RoadObject::distance(double x1, double x2, double y1, double y2)
{
    return sqrt(abs((x2 - x1) * (x2 - x1)) + abs((y2 - y1) * (y2 - y1)));
}

// HELPER FUNCTION

pair<double,double> leastSqrRegression(vector<CvBlob> &points, int numPointsToUse)
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

   numPointsToUse = (numPointsToUse <= points.size()) ? numPointsToUse : points.size();

   //calculate various sums
   for (int i = 0; i < numPointsToUse; i++)
   {
      //sum of x
      SUMx = SUMx + points.at(points.size() - 1 - i).centroid.x;
      //sum of y
      SUMy = SUMy + points.at(points.size() - 1 - i).centroid.y;
      //sum of squared x*y
      SUMxy = SUMxy + points.at(points.size() - 1 - i).centroid.x * points.at(points.size() - 1 - i).centroid.y;
      //sum of squared x
      SUMxx = SUMxx + points.at(points.size() - 1 - i).centroid.x * points.at(points.size() - 1 - i).centroid.x;
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
      Yres = pow((points.at(points.size() - 1 - i).centroid.y - y_intercept - (slope * points.at(points.size() - 1 - i).centroid.x)), 2);

      //sum of (y_i - a0 - a1 * x_i)^2
      SUM_Yres += Yres;

      //current residue squared (y_i - AVGy)^2
      res = pow(points.at(points.size() - 1 - i).centroid.y - AVGy, 2);

      //sum of squared residues
      SUMres += res;
#if 0
      printf ("   (%0.2f %0.2f)      %0.5E         %0.5E\n",
       points.at(points.size() - 1 - i).centroid.x, points.at(points.size() - 1 - i).centroid.y, res, Yres);
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
