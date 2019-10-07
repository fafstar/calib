
#include "cpprest/http_client.h"

#include "opencv2/opencv.hpp"

#include <Windows.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <random>
#include <fstream>

#include "CS200.h"

struct DisplayInfo {
	int width, height;
	int left, right, top, bottom;
};

std::vector< DisplayInfo>  disp_info;


#ifdef _MSC_VER
#define DISABLE_C4996   __pragma(warning(push)) __pragma(warning(disable:4996))
#define ENABLE_C4996    __pragma(warning(pop))
#else
#define DISABLE_C4996
#define ENABLE_C4996
#endif

std::string time_str() {
	std::locale::global(std::locale("ja_JP.utf8"));
	std::time_t t = std::time(nullptr);
	char mbstr[100];
	DISABLE_C4996
		if (std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d-%H%M%S", std::localtime(&t))) {
			return mbstr;
		}
	ENABLE_C4996
		return std::string();
}


BOOL CALLBACK MyMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX monitorInfo;

	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfo(hMonitor, &monitorInfo);
	
	int screen_width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	int screen_height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	std::cout << "Display number : " << disp_info.size() << std::endl;
	std::wcout << "szDevice  : " << monitorInfo.szDevice << std::endl;
	if (static_cast<int>(monitorInfo.dwFlags) > 0) {
		std::cout << "dwFlags   : " << monitorInfo.dwFlags << " (Main Display)" << std::endl;
	} else {
		std::cout << "dwFlags   : " << monitorInfo.dwFlags << std::endl;
	}
	std::cout << "ScreenSize (width,height) : " << screen_width << ", " << screen_height << std::endl;
	std::cout << "rcMonitor (left,top,right,bottom) : "
		<< monitorInfo.rcMonitor.left  << ", " << monitorInfo.rcMonitor.top << ", " 
		<< monitorInfo.rcMonitor.right << ", " << monitorInfo.rcMonitor.bottom << std::endl;
	std::cout << "rcWork    (left,top,right,bottom) : " 
		<< monitorInfo.rcWork.left << ", " << monitorInfo.rcWork.top << ", "
		<< monitorInfo.rcWork.right << ", " << monitorInfo.rcWork.bottom << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;

	DisplayInfo di;
	di.width = screen_width;
	di.height = screen_height;
	di.left = monitorInfo.rcMonitor.left;
	di.right = monitorInfo.rcMonitor.right;
	di.top = monitorInfo.rcMonitor.top;
	di.bottom = monitorInfo.rcMonitor.bottom;

	disp_info.push_back(di);

	return TRUE;
}


void remove_chars(std::string& str, const std::string& remove_target) {
	for (size_t i = 0; i < remove_target.size(); ++i) {
		str.erase(std::remove(str.begin(), str.end(), remove_target[i]), str.end());
	}

}


std::vector<std::tuple<int, int, int>> digits;

void create_mesvec() {
	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());

	// 0~255
	for (int j = 0; j < 3; ++j) {
		std::vector<std::tuple<int, int, int>> temp_vec;

		for (int i = 0; i < 256; ++i) {
			temp_vec.push_back(std::make_tuple(i, 127, 127));
		}
		for (int i = 0; i < 256; ++i) {
			temp_vec.push_back(std::make_tuple(127, i, 127));
		}
		for (int i = 0; i < 256; ++i) {
			temp_vec.push_back(std::make_tuple(127, 127, i));
		}

		//　 シャッフル
		if (true)
		{
			std::shuffle(temp_vec.begin(), temp_vec.end(), engine);
		}
		// 追加

		for (int i = 0; i < temp_vec.size(); ++i) {
			digits.push_back(temp_vec[i]);
		}
	}

	// 最後にグレー測定
	for (int i = 0; i < 256; ++i) {
		digits.push_back(std::make_tuple(i, i, i));
	}
}


int main() {
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);

	// init CS200
	CS200 cs200;
	cs200.open(0);
	//if (!cs200.is_open()) return -1;


	// show display info 
	EnumDisplayMonitors(NULL, NULL, MyMonitorEnumProc, 0);
	// select display
	int disp_number;
	std::cout << "select display number : ";
	std::cin >> disp_number;
	if (disp_number < 0 || disp_number >= disp_info.size()) return -1;
	const auto & display = disp_info[disp_number];

	// set borderless window
	std::string win_title = "cv_measure";
	cv::namedWindow(win_title, cv::WINDOW_NORMAL);
	cv::moveWindow(win_title, display.left, display.top);
	cv::setWindowProperty(win_title, cv::WND_PROP_FULLSCREEN,cv::WINDOW_FULLSCREEN);//set fullscreen property
	
	// create setting image
	cv::Mat img_camset(display.width, display.height, CV_8UC3, cv::Scalar(0, 0, 0));
	cv::line(img_camset, cv::Point(display.height / 2,0), cv::Point(display.height / 2, display.width), cv::Scalar(0, 0, 255));
	cv::line(img_camset, cv::Point(0, display.width / 2), cv::Point(display.height, display.width / 2), cv::Scalar(0, 0, 255));
	cv::putText(img_camset, "Setting CS200 and Press [ENTER] to start", cv::Point(0,30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255,255,255), 2, cv::LineTypes::LINE_AA);
	cv::imshow(win_title, img_camset);
	
	// wait for enter key
	for (int keycode = 0; keycode != ' '; keycode = cv::waitKey(0));
	
	std::string time_stamp = time_str();

	create_mesvec();
	for (int i = 0; i < digits.size(); ++i) {
		int r = std::get<0>(digits[i]);
		int g = std::get<1>(digits[i]);
		int b = std::get<2>(digits[i]);
		cv::Mat image(display.width, display.height, CV_8UC3, cv::Scalar(b,g,r));
		cv::imshow(win_title, image);
		Sleep(2000);
		int mes_time = cs200.measure_start();
		Sleep(mes_time * 1000);
		
		std::string result;
		do {
			bool is_mesend = cs200.get_result(result);
			if (!is_mesend) Sleep(1000);
		} while (cs200.is_measuring());
		remove_chars(result, " \t\n\r");
	
		std::ofstream output("measured.txt", std::ios::app);
		output << time_stamp << ",";
		output << "," << r; // R
		output << "," << g; // G
		output << "," << b; // B
		output << "," << result;// << "\n";
		output.close();

	}
	
	cs200.close();

	return 0;
}
