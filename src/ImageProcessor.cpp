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
        printf("TYPE %d %d\n", frame.type(), CV_8UC4);

        Rect roi(265, 230, 375, 250);
        Mat cropped = frame(roi);
        Mat masked(375, 250, CV_8UC3);// = new Mat(375, 250, CV_8UC3);
        masked = frame(roi);

        cvtColor(cropped, masked, CV_RGBA2RGB); // TODO: fix innefficiency

        printf("W %d H %d T %d\n", masked.rows, masked.cols, masked.type());

        //cv::add(frame, imgMask, masked);
        //masked = frame;


        //masked = frame;

        // Background Subtraction
        if (fgimg.empty()) {
            fgimg.create(img.size(), img.type());
        }

        //update the model
        bg_model.operator()(masked,fgmask); //(masked, fgmask);
        cv::imshow("MASKED1", masked);

        //fgimg = Scalar::all(0);
        masked.copyTo(fgimg, fgmask);
        //filtered.copyTo(fgimg, fgimg);
cv::imshow("FGIMG", fgimg);
cv::imshow("FGMASK", fgmask);

// Help with quality of data?
//cv::erode(fgimg,fgimg,cv::Mat());
//cv::dilate(fgimg,fgimg,cv::Mat());

cv::imshow("ERODED", fgimg);

#if 0
                bg.operator ()(frame,fore);
                bg.getBackgroundImage(back);
                cv::findContours(fore,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
                cv::drawContours(frame,contours,-1,cv::Scalar(0,0,255),2);
                cv::imshow("Frame",frame);
                cv::imshow("Background",back);

#endif

        cvtColor(fgimg, fgimg, CV_BGR2GRAY);
        //threshold(fgimg, fgimg, 100, 255, 3);
        //fgimg = GetThresholdedImage(fgimg);
        IplImage filtered_img = fgimg;

        //Mat labelImg(imgSize, IPL_DEPTH_LABEL, 1);
        IplImage *labelImg = cvCreateImage(imgSize, IPL_DEPTH_LABEL, 1);
        IplImage *dstImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 3);
        cvShowImage("dstImg1", dstImg);

        unsigned int result = cvLabel(&filtered_img, labelImg, cvBlobs);

        cvFilterByArea(cvBlobs, 20, 20000);

        printf("Blobs Size: %d\n", cvBlobs.size());

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

    } catch (cv::Exception& e) {
        printf("Caught Exception in ImageProcessor: %s\n", e.what());
    }
    return cvBlobs.size();
}

ImageProcessor::~ImageProcessor()
{

}

