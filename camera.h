#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <string.h>

using namespace cv;
using namespace std;

class Camera
{
public:
	Camera();

	bool open(std::string name);
	void play();
	bool close();

	Mat edge(Mat& src);

	bool crossingLine(Rect rect, int yLine, int xLineMin, int xLineMax, int io);

private:
	std::string m_fileName;
	VideoCapture m_cap;
	int m_fps;

	Mat m_frame;
};
