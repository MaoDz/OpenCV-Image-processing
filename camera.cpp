#include "camera.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>


Camera::Camera()
{
	m_fps = 30;
}

bool Camera::open(std::string filename)
{
	m_fileName = filename;

	// Convert filename to number if you want to
	// open webcam stream
	std::istringstream iss(filename.c_str());
	int devid;
	bool isOpen;
	if(!(iss >> devid))
	{
		isOpen = m_cap.open(filename.c_str());
	}
	else
	{
		isOpen = m_cap.open(devid);
	}

	if(!isOpen)
	{
		std::cerr << "Unable to open video file." << std::endl;
		return false;
	}

	// set framerate, if unable to read framerate, set it to 30
	m_fps = m_cap.get(CV_CAP_PROP_FPS);
	if(m_fps == 0)
		m_fps = 30;
}

void Camera::play()
{
	// Create main window
	namedWindow("Video", CV_WINDOW_AUTOSIZE);
	bool isReading = true;
	bool oneTime = true;
	// Bordures de voies
	Mat fEdges;
	// Image precedante
	Mat fSuper;


	// Mouvement
	Mat fMove, fMoveRef, fMoveGray,fMoveBin;
	Mat frame_gray;
	// Compute time to wait to obtain wanted framerate
	int timeToWait = 1000/m_fps;
	int tour = 0;



// Coordonnees pour les lignes de detection
	int xGaucheMin = 0;
	int xDroiteMax = 800;

	int yHaut = 55;


// Compteur pour les lignes de detection
	int cntLeft = 0;
	int cntRight = 0;

// Police pour texte
	int font = FONT_HERSHEY_SIMPLEX;


	int prevNbBoundLeft=0;
	int prevNbBoundRight=0;

	while(isReading)
	{

		// Get frame from stream
		isReading = m_cap.read(m_frame);

		// attendre 1 tour de boucle, car image noire au debut
		if(oneTime && tour > 0)
		{
			fMoveRef = m_frame.clone();
			cvtColor(fMoveRef, fMoveRef, CV_RGB2GRAY);
			/* OBTENTION DES VOIES SUR LA ROUTE */
			fEdges = edge(m_frame);
			cvtColor(fEdges, fEdges, COLOR_GRAY2RGB);
			oneTime = false;
		}


		if(isReading)
		{
			// Show frame in main window
			imshow("Video",m_frame);


			/// A completer
			// attendre que fEdges prennent les bordures
			if(!oneTime){
				Mat fSomme;
				addWeighted(fEdges,1, m_frame,1 ,1, fSomme);
				// imshow("Edges", fSomme);

				// Début de détection de mouvement
				// fMove = m_frame.clone() - fPrev.clone();
				frame_gray = m_frame.clone();
				cvtColor(m_frame, frame_gray, CV_RGB2GRAY);

				fMoveGray = fMoveRef.clone() - frame_gray.clone();
				//imshow("Gray Movement", fMoveGray);

				threshold(fMoveGray, fMoveBin,25,255, THRESH_BINARY);
				imshow("Mouvement pré-traitement", fMoveBin);


				int sizeMaskOpen = 1;
				int sizeMaskClose = 2;
				int sizeMaskDilate = 3;
				int sizeMaskErode = 2;

				auto maskOpen = getStructuringElement(MORPH_RECT, Size(2*sizeMaskOpen + 1, 2*sizeMaskOpen + 1), Point(sizeMaskOpen,sizeMaskOpen));
				auto maskClose = getStructuringElement(MORPH_RECT, Size(2*sizeMaskClose + 1, 2*sizeMaskClose + 1), Point(sizeMaskClose,sizeMaskClose));
				auto maskDilate = getStructuringElement(MORPH_RECT, Size(2*sizeMaskDilate + 1, 2*sizeMaskDilate + 1), Point(sizeMaskDilate,sizeMaskDilate));
				auto maskErode = getStructuringElement(MORPH_RECT, Size(2*sizeMaskErode + 1, 2*sizeMaskErode + 1), Point(sizeMaskErode,sizeMaskErode));
				//
				//
				morphologyEx(fMoveBin, fMoveBin, MORPH_OPEN, maskOpen, Point(-1,-1), 1);
				// imshow("Ouverture", fMoveBin);

				GaussianBlur(fMoveBin, fMoveBin, Size(15,15), 0);
				threshold(fMoveBin, fMoveBin,1,255, THRESH_BINARY);

				morphologyEx(fMoveBin, fMoveBin, MORPH_DILATE, maskDilate, Point(-1, -1), 2);
				// imshow("Dilatation", fMoveBin);
				morphologyEx(fMoveBin, fMoveBin, MORPH_CLOSE, maskClose, Point(-1,-1),1);
				// imshow("Fermeture", fMoveBin);

				imshow("Traitement mouvement", fMoveBin);


				// Separation de l'image en deux pour compter la voie de gauche et celle de droite
				Mat left = fMoveBin(Rect(0, 0, fMoveBin.cols/2, fMoveBin.rows));
				Mat right = fMoveBin(Rect(fMoveBin.cols/2, 0, fMoveBin.cols/2, fMoveBin.rows));
				imshow("left image", left);
				imshow("right image", right);

				vector<vector<Point> > contoursLeft;
				vector<vector<Point> > contoursRight;
		    findContours( left, contoursLeft, RETR_TREE, CHAIN_APPROX_SIMPLE );
				findContours( right, contoursRight, RETR_TREE, CHAIN_APPROX_SIMPLE );
		    vector<vector<Point> > contours_polyLeft( contoursLeft.size() );
				vector<vector<Point> > contours_polyRight( contoursRight.size() );
		    vector<Rect> boundRectLeft( contoursLeft.size() );
				vector<Rect> boundRectRight( contoursRight.size() );

				Mat drawing = Mat::zeros( fMoveBin.size(), CV_8UC3 );

				// OBTENTION DES CONTOURS SOUS FORME RECTANGULAIRE
				// AJOUTS DES RECTANGLES SUR drawing
		    for( size_t i = 0; i < contoursLeft.size(); i++ )
		    {
		        approxPolyDP( contoursLeft[i], contours_polyLeft[i], 1, true );
		        boundRectLeft[i] = boundingRect( contours_polyLeft[i] );

						Scalar color = Scalar(0,0,255);
						rectangle( drawing, boundRectLeft[i], color, 3 );
		    }

				for( size_t i = 0; i < contoursRight.size(); i++ )
		    {
		        approxPolyDP( contoursRight[i], contours_polyRight[i], 1, true );
		        boundRectRight[i] = boundingRect( contours_polyRight[i] );
						Scalar color = Scalar(0,255,255);
						boundRectRight[i].x+=fMoveBin.cols/2;
						rectangle( drawing, boundRectRight[i], color, 3 );
		    }


				// SUPPRESSION DES RECTANGLES TROP PETITS POUR
				// ETRE DES VEHICULES
				// UNIQUEMENT POUR LE COMPTAGE (VISUALISATION EFFECTIVE)
				for( size_t i = 0; i < boundRectLeft.size(); i++){
					if(boundRectLeft[i].width<20 || boundRectLeft[i].height<20)
						boundRectLeft.erase(boundRectLeft.begin()+ int(i));
				}

				for( size_t i = 0; i < boundRectRight.size(); i++){
					if(boundRectRight[i].width<20 || boundRectRight[i].height<20)
						boundRectRight.erase(boundRectRight.begin()+ int(i));
				}

				// SUPPRESSION DES RECTANGLES AU SEIN D'AUTRES RECTANGLES
				// UNIQUEMENT POUR LE COMPTAGE (VISUALISATION EFFECTIVE)
				for(size_t i = 0; i < boundRectLeft.size(); i++){
					for(size_t j = i+1; j < boundRectLeft.size(); j++){
						int area = (boundRectLeft[i] & boundRectLeft[j]).area();
						if ( area == boundRectLeft[i].area() )
							boundRectLeft.erase(boundRectLeft.begin()+ int(i));
						else if ( area == boundRectLeft[j].area() ){
							boundRectLeft.erase(boundRectLeft.begin()+ int(j));
						}
					}
				}

				for(size_t i = 0; i < boundRectRight.size(); i++){
					for(size_t j = i+1; j < boundRectRight.size(); j++){
						int area = (boundRectRight[i] & boundRectRight[j]).area();
						if ( area == boundRectRight[i].area() )
							boundRectRight.erase(boundRectRight.begin()+ int(i));
						if ( area == boundRectRight[j].area() ){
							boundRectRight.erase(boundRectRight.begin()+ int(j));
						}
					}
				}

				// COMPTEURS GAUCHE ET DROITE
				if(boundRectLeft.size()>prevNbBoundLeft){
					cntLeft +=boundRectLeft.size()-prevNbBoundLeft;
				}
				prevNbBoundLeft=boundRectLeft.size();

				if(boundRectRight.size()>prevNbBoundRight){
					cntRight+=boundRectRight.size()-prevNbBoundRight;
				}
				prevNbBoundRight=boundRectRight.size();

				// AJOUT DU TEXTE
				putText(drawing, to_string(cntLeft), Point(xGaucheMin+40, yHaut-10), font, 1, Scalar(150,150,150), 3 ,LINE_4);
				putText(drawing, to_string(cntRight), Point(xDroiteMax-40, yHaut-10), font, 1, Scalar(150,150,150), 3 ,LINE_4);
				imshow("Contours et comptages", drawing);


				addWeighted(m_frame,1, drawing,1 ,1, fSuper);
				addWeighted(fSuper, 1, fEdges, 1, 1, fSuper);
				imshow("Superposition Finale", fSuper);

			}

			/// --------------------------------
		}
		else
		{
			std::cerr << "Unable to read device" << std::endl;
		}

		// If escape key is pressed, quit the program
		if(waitKey(timeToWait)%256 == 27)
		{

			std::cerr << "Stopped by user" << std::endl;
			isReading = false;
		}
		tour++;
	}

	cout << "Nombre de vehicules à gauche : " << cntLeft << "\n";
	cout << "Nombre de vehicules à droite : " << cntRight << "\n";
	cout << "Nombre de vehicules au total : " << cntLeft+cntRight << "\n";
}

bool Camera::close()
{
	// Close the stream
	m_cap.release();

	// Close all the windows
	destroyAllWindows();
	usleep(100000);
}

Mat Camera::edge(Mat& src)
{
	Mat fBin;
	Mat fHSV;
	Mat fEdges;
	Mat fBlur;
	Mat fOpen, fClose;

	int low_H = 60, high_H = 180;
	int low_S = 50, high_S = 255;
	int low_V = 20, high_V = 255;

	cvtColor(src, fHSV, CV_RGB2HSV);
	inRange(fHSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), fHSV);
	bitwise_not(fHSV,fBin);
	// imshow("HSV", fHSV);

	// imshow("Bin", fBin);

	medianBlur(fBin,fBlur,5);
	// imshow("Blur", fBlur);

	int sizeMaskOpen = 13;
	int sizeMaskClose = 3;
	int sizeMaskDilate = 1;

	auto maskOpen = getStructuringElement(MORPH_ELLIPSE, Size(2*sizeMaskOpen + 1, 2*sizeMaskOpen + 1), Point(sizeMaskOpen,sizeMaskOpen));
	auto maskClose = getStructuringElement(MORPH_ELLIPSE, Size(2*sizeMaskClose + 1, 2*sizeMaskClose + 1), Point(sizeMaskClose,sizeMaskClose));
	auto maskDilate = getStructuringElement(MORPH_ELLIPSE, Size(2*sizeMaskDilate + 1, 2*sizeMaskDilate + 1), Point(sizeMaskDilate,sizeMaskDilate));

	morphologyEx(fBlur, fClose, MORPH_CLOSE, maskClose, Point(-1,-1),5);
	// imshow("Fermeture", fClose);

	morphologyEx(fClose, fOpen, MORPH_OPEN, maskOpen, Point(-1,-1), 2);
	// imshow("Ouverture", fOpen);

	Canny(fOpen, fEdges, 100, 200, 3, false);
	morphologyEx(fEdges, fEdges, MORPH_DILATE, maskDilate, Point(-1,-1));
	// imshow("EDGES", fEdges);
	return fEdges;
}


// Vestige de notre premier comptage
// // io :
// // 1 : detect au dessus
// // -1 : detect en dessous
// bool Camera::crossingLine(Rect rect, int yLine, int xLineMin, int xLineMax, int io)
// {
// 	Point centerRect = (rect.br()+rect.tl())/2;
// 	Point bottom = rect.br();
// 	if(bottom.x >= xLineMin && bottom.x <= xLineMax)
// 	{
// 		int absY = abs(bottom.y - yLine);
// 		if(absY <= 3 && io*bottom.y < io*yLine)
// 			return true;
// 	}
// 	return false;
// }
