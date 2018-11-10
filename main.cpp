#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#define WINDOW_NAME "Cork"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	const char* file = "data/1.jpg";

	//VideoCapture cap(0);
	Mat src = imread(file, IMREAD_COLOR);

	//int width = cap.get(CAP_PROP_FRAME_WIDTH);
	//int height = cap.get(CAP_PROP_FRAME_HEIGHT);
	
	//Prepare output window
	namedWindow(WINDOW_NAME, WINDOW_AUTOSIZE);
	//setMouseCallback(WINDOW_NAME, mouseEvents, &original);

	/*if(cap.isOpened())
	{
		cap >> src;
	}*/

	while(1)
	{
		Mat gray;
		cvtColor(src, gray, COLOR_BGR2GRAY);
		medianBlur(gray, gray, 5);
		vector<Vec3f> circles;

		HoughCircles(gray, circles, HOUGH_GRADIENT, 1, gray.rows / 16, 100, 30, 1, 100);

		//Draw circle outline
		for(size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			Point center = Point(c[0], c[1]);

			//Circle center
			circle( src, center, 1, Scalar(0,100,100), 3, LINE_AA);

			//Circle outline
			int radius = c[2];
			circle( src, center, radius, Scalar(255,0,255), 3, LINE_AA);
		}

		imshow(WINDOW_NAME, src);
		
		int key = waitKey(16);
		if(key != -1)
		{
			if(key == 27)
			{
				return 0;
			}
		}

		/*if(cap.isOpened())
		{
			cap >> src;
		}*/
	}

	return 0;
}

//Called on every mouse action
void mouseEvents(int event, int x, int y, int flags, void *param)
{
	//if(event == EVENT_LBUTTONDOWN){}
	//else if(event == EVENT_RBUTTONDOWN)
	//else if(event == EVENT_MBUTTONDOWN)
	//else if(event == EVENT_MOUSEMOVE)
}
