#include "ImageProcessor.h"

using namespace cv;
using namespace std;

ImageProcessor::ImageProcessor(const char * imgMaskPath, int mediaWidth, int mediaHeight, bool displayFrame, CarCounter * c) :
    frameCount(0),
    displayFrame(displayFrame),
    counter(c)
{
    imgSize.width = mediaWidth;
    imgSize.height = mediaHeight;

    imgMask = imread(imgMaskPath, CV_LOAD_IMAGE_ANYCOLOR); // Read the file

    bgImg = imread("/Users/j3bennet/king_st_bg.jpg", CV_LOAD_IMAGE_ANYCOLOR); // Read the file // TODO: WTF is this ?
    if (displayFrame) {
        cvNamedWindow("display", CV_WINDOW_AUTOSIZE);
    }
}

int ImageProcessor::processFrame(Mat frame)
{
    // Mask Image
    CvBlobs cvBlobs;

    try {
        Mat masked;

        cvtColor(frame, frame, CV_BGR2RGB); // TODO: fix innefficiency
        cv::add(frame, imgMask, masked);

        // Background Subtraction
        if (fgimg.empty()) {
            fgimg.create(img.size(), img.type());
        }

        //update the model
        bool update_bg_model = true; //(blobs.size() > 0 && frameCount > 300) ? false : true;
        bg_model(masked, fgmask, update_bg_model ? -1 : 0);

        fgimg = Scalar::all(0);
        masked.copyTo(fgimg, fgmask);
        //filtered.copyTo(fgimg, fgimg);

        Mat bgimg;
        bg_model.getBackgroundImage(bgimg);

        cvtColor(fgimg, fgimg, CV_BGR2GRAY);
        //threshold(fgimg, fgimg, 100, 255, 3);
        //fgimg = GetThresholdedImage(fgimg);
        IplImage filtered_img = fgimg;

        IplImage *labelImg = cvCreateImage(imgSize, IPL_DEPTH_LABEL, 1);
        IplImage *dstImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 3);

        unsigned int result = cvLabel(&filtered_img, labelImg, cvBlobs);

        cvFilterByArea(cvBlobs, 100, 2000);

        if (counter) {
            vector<Blob> blobs;
            for (CvBlobs::iterator it = cvBlobs.begin(); it != cvBlobs.end(); ++it) {
                    CvBlob blob = *(it->second);
                    Blob * b = new Blob(blob, frameCount++);
                    blobs.push_back(*b);
            }
        }

        if (displayFrame) {
            cvRenderBlobs(labelImg, cvBlobs, &filtered_img, dstImg);
            cvShowImage("dstImg", dstImg);
        }


        cvReleaseImage(&labelImg);
        cvReleaseImage(&dstImg);

        if (!bgimg.empty()) {
            //imshow("mean background image", bgimg );
        }

    } catch (cv::Exception& e) {
        printf("Caught Exception in ImageProcessor: %s\n", e.what());
    }
    return cvBlobs.size();
}

ImageProcessor::~ImageProcessor()
{

}

