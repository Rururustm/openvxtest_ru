
#include "../stdafx.h"

#include <opencv2/opencv.hpp>

extern "C"
{
#include <Kernels/ref.h>
#include <types.h>
}

#include "../DemoEngine.h"
class demo_ConvexHull : public IDemoCase
{
public:
    ///@brief default ctor
    demo_ConvexHull()
        : m_threshold(110)
    {
        // nothing to do
    }

    ///@see IDemoCase::ReplyName
    virtual std::string ReplyName() const override
    {
        return "ConvexHull";
    }

private:
    ///@see IDemoCase::execute
    virtual void execute() override;

    ///@brief provide interactive demo
    static void applyParameters(int pos, void* data);

private:
    int m_threshold;
    cv::Mat m_srcImage;
};

///////////////////////////////////////////////////////////////////////////////
namespace
{
    const std::string m_openVXWindow = "openVX";
    const std::string m_openCVWindow = "openCV";
    const std::string m_originalWindow = "original";
    const std::string m_diffWindow = "Diff of " + m_openVXWindow + " and " + m_openCVWindow;
}

///////////////////////////////////////////////////////////////////////////////
void demo_ConvexHull::execute()
{
    cv::namedWindow(m_originalWindow, CV_WINDOW_NORMAL);
    cv::namedWindow(m_openVXWindow, CV_WINDOW_NORMAL);
    cv::namedWindow(m_openCVWindow, CV_WINDOW_NORMAL);
    cv::namedWindow(m_diffWindow, CV_WINDOW_NORMAL);

    const std::string imgPath = "D:/openvx/openvxtest_ru/Image/hand.png";
    m_srcImage = cv::imread(imgPath, CV_LOAD_IMAGE_GRAYSCALE);

    blur(m_srcImage, m_srcImage, cv::Size(3, 3));
    cv::imshow(m_originalWindow, m_srcImage);

    cv::createTrackbar("ConvexHull:", m_originalWindow, &m_threshold, 255, applyParameters, static_cast<void*>(this));
    applyParameters(m_threshold, this);

    cv::waitKey(0);
}


///////////////////////////////////////////////////////////////////////////////
void demo_ConvexHull::applyParameters(int, void* data)
{
    auto demo = static_cast<demo_ConvexHull*>(data);
    const cv::Size imgSize(demo->m_srcImage.cols, demo->m_srcImage.rows);
    cv::Mat canny_output;
    Canny(demo->m_srcImage, canny_output, demo->m_threshold, demo->m_threshold * 2);
    ///@{ OPENCV
    std::vector<cv::Point> pts;
    std::vector<cv::Point> hull;
    findNonZero(canny_output, pts);
    cv::convexHull(pts, hull);
    std::vector<std::vector<cv::Point> >contours;
    cv::findContours(canny_output, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    cv::Mat drawing = cv::Mat::zeros(canny_output.size(), CV_8UC1);
    polylines(drawing, hull, true, cv::Scalar(255, 255, 255), 1);
    drawContours(drawing, contours, 0, 255);
    imshow(m_openCVWindow, drawing);
    ///@}
    
    ///@{ OPENVX
    _vx_image srcVXImage = {
       canny_output.data,
       uint32_t(imgSize.width),
       uint32_t(imgSize.height),
       VX_DF_IMAGE_U8,
       VX_COLOR_SPACE_DEFAULT
    };
    size_t size = 0;
    vx_coordinates2d_t* out_points = (vx_coordinates2d_t*)malloc(imgSize.width * imgSize.height * sizeof(vx_coordinates2d_t));
    ref_ConvexHull(&srcVXImage, out_points,&size);
    std::vector<cv::Point> myhull(size);
    for (int i = 0; i < size; ++i) {
        myhull[i].x = out_points[i].x;
        myhull[i].y = out_points[i].y;

    }
    cv::Mat mydrawing = cv::Mat::zeros(canny_output.size(), CV_8UC1);
    polylines(mydrawing, myhull, true, cv::Scalar(255, 255, 255), 1);
    drawContours(mydrawing, contours, 0, 255);
    imshow(m_openVXWindow, mydrawing);
    ///@}
   
    // Show difference of OpenVX and OpenCV
     const cv::Mat diffImage(imgSize, CV_8UC1);
     cv::absdiff(drawing, mydrawing, diffImage);
     cv::imshow(m_diffWindow, diffImage);
}
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
IDemoCasePtr CreateConvexHullDemo()
{
    return std::make_unique<demo_ConvexHull>();
}
